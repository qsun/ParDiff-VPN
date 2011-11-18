/*
 *  plugin.c -- Challenge Handshake Authentication Protocol for the
 *              Point-to-Point Protocol (PPP) shared functions.
 *
 *  Copyright (c) 2008-2009 Maik Broemme <mbroemme@plusserver.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* plugin includes. */
#include "plugin.h"
#include "str.h"

/* this function set whether the peer must authenticate itself to us via CHAP. */
int32_t pppd__chap_check(void) {

	/* return one, because we need to ask peer everytime for authentication. */
	return 1;
}

/* this function set whether the peer must authenticate itself to us via PAP. */
int32_t pppd__pap_check(void) {

	/* return one, because we need to ask peer everytime for authentication. */
	return 1;
}

/* this function will look for a valid client ip address. */
void pppd__ip_choose(uint32_t *addrp) {

	/* set ip address to client one fetched from database. */
	*addrp = client_ip;

	/* set ip address to server one fetched from database. */
	ipcp_gotoptions[0].ouraddr = server_ip;
}

/* this function set whether the plugin is allowed to set client ip addresses. */
int32_t pppd__allowed_address(uint32_t addr) {
	return 1;

	/* check if ip address is equal to client address from database. */
	if (addr != client_ip) {

		/* seems that we are using an invalid ip address. */
		return 0;
	}

	/* return one, because we are allowed to assign ip addresses. */
	return 1;
}

/* this function will execute a script when IPCP comes up. */
int32_t pppd__ip_up(uint8_t *username, uint8_t *program) {

	/* some common variables. */
	uint8_t strspeed[32];
	uint8_t strlocal[32];
	uint8_t strremote[32];
	uint8_t *argv[9];
	int32_t script_status;
	pid_t script_pid;

	/* create the parameters. */
	slprintf((char *)strspeed, sizeof(strspeed), "%d", baud_rate);
	slprintf((char *)strlocal, sizeof(strlocal), "%I", ipcp_gotoptions[0].ouraddr);
	slprintf((char *)strremote, sizeof(strremote), "%I", ipcp_hisoptions[0].hisaddr);

	/* build argument list. */
	argv[0] = program;
	argv[1] = (uint8_t *)ifname;
	argv[2] = (uint8_t *)devnam;
	argv[3] = strspeed;
	argv[4] = strlocal;
	argv[5] = strremote;
	argv[6] = (uint8_t *)ipparam;
	argv[7] = username;
	argv[8] = NULL;

	/* execute script. */
	script_pid = run_program((char *)program, (char **)argv, 0, NULL, NULL, 0);

	/* check if file exists and fork was successful. */
	if (script_pid <= 0) {

		/* something failed on script execution. */
		return PPPD_SQL_ERROR_SCRIPT;
	}

	/* wait until script has finished. */
	while (waitpid(script_pid, &script_status, 0) < 0) {

		/* continue on unblocked signal or a SIGCHLD. */
		if (errno == EINTR) {
			continue;
		}
	}

	/* check if script execution was successful. */
	if (WEXITSTATUS(script_status) != 0) {

		/* something failed on script execution. */
		return PPPD_SQL_ERROR_SCRIPT;
	}

	/* if no error was found, return zero. */
	return 0;
};

/* this function will execute a script when IPCP goes down. */
int32_t pppd__ip_down(uint8_t *username, uint8_t *program) {

	/* some common variables. */
	uint8_t str_speed[32];
	uint8_t str_local[32];
	uint8_t str_remote[32];
	uint8_t str_bytes_received[32];
	uint8_t str_bytes_transmitted[32];
	uint8_t str_duration[32];
	uint8_t *argv[12];
	int32_t script_status;
	pid_t script_pid;

	/* create the parameters. */
	slprintf((char *)str_speed, sizeof(str_speed), "%d", baud_rate);
	slprintf((char *)str_local, sizeof(str_local), "%I", ipcp_gotoptions[0].ouraddr);
	slprintf((char *)str_remote, sizeof(str_remote), "%I", ipcp_hisoptions[0].hisaddr);
	slprintf((char *)str_bytes_received, sizeof(str_bytes_received), "%d", link_stats.bytes_in);
	slprintf((char *)str_bytes_transmitted, sizeof(str_bytes_transmitted), "%d", link_stats.bytes_out);
	slprintf((char *)str_duration, sizeof(str_duration), "%d", link_connect_time);

	/* build argument list. */
	argv[0] = program;
	argv[1] = (uint8_t *)ifname;
	argv[2] = (uint8_t *)devnam;
	argv[3] = str_speed;
	argv[4] = str_local;
	argv[5] = str_remote;
	argv[6] = (uint8_t *)ipparam;
	argv[7] = username;
	argv[8] = str_bytes_received;
	argv[9] = str_bytes_transmitted;
	argv[10] = str_duration;
	argv[11] = NULL;

	/* execute script. */
	script_pid = run_program((char *)program, (char **)argv, 0, NULL, NULL, 0);

	/* check if file exists and fork was successful. */
	if (script_pid <= 0) {

		/* something failed on script execution. */
		return PPPD_SQL_ERROR_SCRIPT;
	}

	/* wait until script has finished. */
	while (waitpid(script_pid, &script_status, 0) < 0) {

		/* continue on unblocked signal or a SIGCHLD. */
		if (errno == EINTR) {
			continue;
		}
	}

	/* check if script execution was successful. */
	if (WEXITSTATUS(script_status) != 0) {

		/* something failed on script execution. */
		return PPPD_SQL_ERROR_SCRIPT;
	}

	/* if no error was found, return zero. */
	return 0;
};

/* this function verify the given password. */
int32_t pppd__verify_password(uint8_t *passwd, uint8_t *secret_name, uint8_t *encryption, uint8_t *key) {

	/* some common variables. */
	uint8_t passwd_aes[MAXSECRETLEN / 2];
	uint8_t passwd_key[SIZE_AES];
	uint8_t passwd_md5[SIZE_MD5];
	uint8_t passwd_crypt[SIZE_CRYPT];
	uint32_t count      = 0;
	int32_t passwd_size = 0;
	int32_t temp_size   = 0;
	EVP_MD_CTX ctx_md5;
	EVP_CIPHER_CTX ctx_aes;

	/* cleanup the static array. */
	memset(passwd_aes, 0, sizeof(passwd_aes));
	memset(passwd_key, 0, sizeof(passwd_key));
	memset(passwd_md5, 0, sizeof(passwd_md5));
	memset(passwd_crypt, 0, sizeof(passwd_crypt));

	/* check if we use no algorithm. */
	if (strcasecmp((char *)encryption, "NONE") == 0) {

		/* check if we found valid password. */
		if (strcmp((char *)passwd, (char *)secret_name) != 0) {

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}
	}

	/* check if we use des crypt algorithm. */
	if (strcasecmp((char *)encryption, "CRYPT") == 0) {

		/* check if secret from database is shorter than an expected crypt() result. */
		if (strlen((char *)secret_name) < (SIZE_CRYPT * 2)) {

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* check if password was successfully encrypted. */
		if ((uint8_t *)DES_fcrypt((char *)passwd, (char *)key, (char *)passwd_crypt) == NULL) {

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* loop through every byte and compare it. */
		for (count = 0; count < (strlen((char *)secret_name) / 2); count++) {

			/* check if our hex value matches the hash byte. (this isn't the fastest way, but hash is everytime 13 byte) */
			if (pppd__htoi(secret_name[2 * count]) * 16 + pppd__htoi(secret_name[2 * count + 1]) != passwd_crypt[count]) {

				/* clear the memory with the hash, so nobody is able to dump it. */
				memset(passwd_crypt, 0, sizeof(passwd_crypt));

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_PASSWORD;
			}
		}
	}

	/* check if we use md5 hashing algorithm. */
	if (strcasecmp((char *)encryption, "MD5") == 0) {

		/* check if secret from database is shorter than an expected md5 hash. */
		if (strlen((char *)secret_name) < (SIZE_MD5 * 2)) {

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* initialize the openssl context. */
		EVP_MD_CTX_init(&ctx_md5);

		/* check if cipher initialization is working. */
		if (EVP_DigestInit_ex(&ctx_md5, EVP_md5(), NULL) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_MD_CTX_cleanup(&ctx_md5);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* encrypt the input buffer. */
		if (EVP_DigestUpdate(&ctx_md5, passwd, strlen((char *)passwd)) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_MD_CTX_cleanup(&ctx_md5);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* encrypt the last block from input buffer. */
		if (EVP_DigestFinal_ex(&ctx_md5, passwd_md5, (uint32_t *)&passwd_size) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_MD_CTX_cleanup(&ctx_md5);

			/* clear the memory with the hash, so nobody is able to dump it. */
			memset(passwd_md5, 0, sizeof(passwd_md5));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* cleanup cipher context to prevent memory dumping. */
		EVP_MD_CTX_cleanup(&ctx_md5);

		/* loop through every byte and compare it. */
		for (count = 0; count < (strlen((char *)secret_name) / 2); count++) {

			/* check if our hex value matches the hash byte. (this isn't the fastest way, but hash is everytime 16 byte) */
			if (pppd__htoi(secret_name[2 * count]) * 16 + pppd__htoi(secret_name[2 * count + 1]) != passwd_md5[count]) {

				/* clear the memory with the hash, so nobody is able to dump it. */
				memset(passwd_md5, 0, sizeof(passwd_md5));

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_PASSWORD;
			}
		}

		/* clear the memory with the hash, so nobody is able to dump it. */
		memset(passwd_md5, 0, sizeof(passwd_md5));
	}

	/* check if we use aes block cipher algorithm. */
	if (strcasecmp((char *)encryption, "AES") == 0) {

		/* check if secret from database is shorter than an expected minimum aes size. */
		if (strlen((char *)secret_name) < (((strlen((char *)passwd) / 16) + 1) * 16)) {

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* check if we have to truncate source pointer. */
		if (strlen((char *)key) < SIZE_AES) {

			/* copy the key to the static buffer. */
			memcpy(passwd_key, key, strlen((char *)key));
		} else {

			/* copy the key to the static buffer. */
			memcpy(passwd_key, key, SIZE_AES);
		}

		/* initialize the openssl context. */
		EVP_CIPHER_CTX_init(&ctx_aes);

		/* check if cipher initialization is working. */
		if (EVP_EncryptInit_ex(&ctx_aes, EVP_aes_128_ecb(), NULL, passwd_key, NULL) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_CIPHER_CTX_cleanup(&ctx_aes);

			/* clear the memory with the aes key, so nobody is able to dump it. */
			memset(passwd_key, 0, sizeof(passwd_key));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* encrypt the input buffer. */
		if (EVP_EncryptUpdate(&ctx_aes, passwd_aes, &passwd_size, passwd, strlen((char *)passwd)) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_CIPHER_CTX_cleanup(&ctx_aes);

			/* clear the memory with the aes key and buffer, so nobody is able to dump it. */
			memset(passwd_aes, 0, sizeof(passwd_aes));
			memset(passwd_key, 0, sizeof(passwd_key));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* encrypt the last block from input buffer. */
		if (EVP_EncryptFinal_ex(&ctx_aes, passwd_aes + passwd_size, &temp_size) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_CIPHER_CTX_cleanup(&ctx_aes);

			/* clear the memory with the aes key and buffer, so nobody is able to dump it. */
			memset(passwd_aes, 0, sizeof(passwd_aes));
			memset(passwd_key, 0, sizeof(passwd_key));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* cleanup cipher context to prevent memory dumping. */
		EVP_CIPHER_CTX_cleanup(&ctx_aes);

		/* compute final size. */
		passwd_size += temp_size;

		/* loop through every byte and compare it. */
		for (count = 0; count < (strlen((char *)secret_name) / 2); count++) {

			/* check if our hex value matches the hash byte. (this isn't the fastest way, but okay) */
			if (pppd__htoi(secret_name[2 * count]) * 16 + pppd__htoi(secret_name[2 * count + 1]) != passwd_aes[count]) {

				/* clear the memory with the aes key and buffer, so nobody is able to dump it. */
				memset(passwd_aes, 0, sizeof(passwd_aes));
				memset(passwd_key, 0, sizeof(passwd_key));

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_PASSWORD;
			}
		}

		/* clear the memory with the aes key and buffer, so nobody is able to dump it. */
		memset(passwd_aes, 0, sizeof(passwd_aes));
		memset(passwd_key, 0, sizeof(passwd_key));
	}

	/* if no error was found, establish link. */
	return 0;
}

/* this function decrypt the given password. */
int32_t pppd__decrypt_password(uint8_t *secret_name, int32_t *secret_length, uint8_t *encryption, uint8_t *key) {

	/* some common variables. */
	uint8_t passwd_aes[MAXSECRETLEN / 2];
	uint8_t passwd_key[SIZE_AES];
	uint32_t count        = 0;
	int32_t passwd_size   = 0;
	int32_t temp_size     = 0;
	EVP_CIPHER_CTX ctx_aes;

	/* check if we use no algorithm. */
	if (strcasecmp((char *)encryption, "NONE") == 0 ||
	    strcasecmp((char *)encryption, "CRYPT") == 0 ||
	    strcasecmp((char *)encryption, "MD5") == 0) {

		/* no encryption or non-symmetric algorithm used. */
		return 0;
	}

	/* check if we use aes block cipher algorithm. */
	if (strcasecmp((char *)encryption, "AES") == 0) {

		/* cleanup the static array. */
		memset(passwd_aes, 0, sizeof(passwd_aes));
		memset(passwd_key, 0, sizeof(passwd_key));

		/* check if we have to truncate source pointer. */
		if (strlen((char *)key) < SIZE_AES) {

			/* copy the key to the static buffer. */
			memcpy(passwd_key, key, strlen((char *)key));
		} else {

			/* copy the key to the static buffer. */
			memcpy(passwd_key, key, SIZE_AES);
		}

		/* loop through every byte and convert it. */
		for (count = 0; count < (*secret_length / 2); count++) {

			/* create binary data for decryption. */
			passwd_aes[count] = pppd__htoi(secret_name[2 * count]) * 16 + pppd__htoi(secret_name[2 * count + 1]);
		}

		/* initialize the openssl context. */
		EVP_CIPHER_CTX_init(&ctx_aes);

		/* check if cipher initialization is working. */
		if (EVP_DecryptInit_ex(&ctx_aes, EVP_aes_128_ecb(), NULL, passwd_key, NULL) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_CIPHER_CTX_cleanup(&ctx_aes);

			/* clear the memory with the aes key and password, so nobody is able to dump it. */
			memset(passwd_aes, 0, sizeof(passwd_aes));
			memset(passwd_key, 0, sizeof(passwd_key));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* decrypt the input buffer. */
		if (EVP_DecryptUpdate(&ctx_aes, secret_name, &passwd_size, passwd_aes, *secret_length / 2) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_CIPHER_CTX_cleanup(&ctx_aes);

			/* clear the memory with the aes key, password and buffer, so nobody is able to dump it. */
			memset(passwd_aes, 0, sizeof(passwd_aes));
			memset(passwd_key, 0, sizeof(passwd_key));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* decrypt the last block from input buffer. */
		if (EVP_DecryptFinal_ex(&ctx_aes, secret_name + passwd_size, &temp_size) == 0) {

			/* cleanup cipher context to prevent memory dumping. */
			EVP_CIPHER_CTX_cleanup(&ctx_aes);

			/* clear the memory with the aes key, password and buffer, so nobody is able to dump it. */
			memset(passwd_aes, 0, sizeof(passwd_aes));
			memset(passwd_key, 0, sizeof(passwd_key));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_PASSWORD;
		}

		/* cleanup cipher context to prevent memory dumping. */
		EVP_CIPHER_CTX_cleanup(&ctx_aes);

		/* compute final size. */
		passwd_size += temp_size;

		/* terminate the cleartext password. */
		secret_name[passwd_size] = '\0';
		*secret_length = passwd_size;

		/* clear the memory with the aes key, password and buffer, so nobody is able to dump it. */
		memset(passwd_aes, 0, sizeof(passwd_aes));
		memset(passwd_key, 0, sizeof(passwd_key));
	}

	/* if no error was found, establish link. */
	return 0;
}

/*
 *  auth-mysql.c -- Challenge Handshake Authentication Protocol and Password
 *                  Authentication Protocol for the Point-to-Point Protocol
 *                  (PPP) via MySQL.
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

/* generic plugin includes. */
#include "plugin.h"
#include "plugin-mysql.h"
#include "str.h"

/* auth plugin includes. */
#include "auth-mysql.h"

/* store username in global variable, because ip down did not know it. */
uint8_t username[MAXNAMELEN];

/* this function handles the mysql_error() result. */
int32_t pppd__mysql_error(uint32_t error_code, const uint8_t *error_state, const uint8_t *error_message) {

	/* show error header. */
	error("Plugin %s: Fatal Error Message (MySQL):\n", PLUGIN_NAME_MYSQL);

	/* show the detailed error. */
	error("Plugin %s: * %d (%s): %s\n", PLUGIN_NAME_MYSQL, error_code, error_state, error_message);

	/* if no error was found, return zero. */
	return 0;
}

/* this function check the parameter. */
int32_t pppd__mysql_parameter(void) {

	/* check if all information are supplied. */
	if (pppd_mysql_host		== NULL ||
	    pppd_mysql_port		== NULL ||
	    pppd_mysql_user		== NULL ||
	    pppd_mysql_pass		== NULL ||
	    pppd_mysql_pass_encryption	== NULL ||
	    pppd_mysql_database		== NULL ||
	    pppd_mysql_table		== NULL ||
	    pppd_mysql_column_user	== NULL ||
	    pppd_mysql_column_pass	== NULL ||
	    pppd_mysql_column_client_ip	== NULL ||
	    pppd_mysql_column_server_ip	== NULL) {

		/* something failed on mysql initialization. */
		error("Plugin %s: MySQL information are not complete\n", PLUGIN_NAME_MYSQL);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_INCOMPLETE;
	}

	/* check if passwords are encrypted. */
	if (strcasecmp(pppd_mysql_pass_encryption, "CRYPT")	== 0 ||
	    strcasecmp(pppd_mysql_pass_encryption, "AES")	== 0) {

		/* check if key or salt is given. */
		if (pppd_mysql_pass_key == NULL) {

			/* some required encryption information are missing. */
			error("Plugin: %s: MySQL encryption information are not complete\n", PLUGIN_NAME_MYSQL);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_INCOMPLETE;
		}
	}

	/* check if concurrent connection from one user should be denied. */
	if (pppd_mysql_exclusive == 1) {

		/* check if update column is given. */
		if (pppd_mysql_column_update == NULL ||
		    pppd_mysql_authoritative == 0) {

			/* some required exclusive information are missing. */
			error("Plugin: %s: MySQL exclusive information are not complete\n", PLUGIN_NAME_MYSQL);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_INCOMPLETE;
		}
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function connect to a mysql database. */
int32_t pppd__mysql_connect(MYSQL **mysql) {

	/* some common variables. */
	uint32_t count = 0;

	/* check if mysql initialization was successful. */
	if ((*mysql = mysql_init(NULL)) == NULL) {

		/* something failed on mysql initialization. */
		error("Plugin %s: MySQL initialization failed\n", PLUGIN_NAME_MYSQL);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_INIT;
	}

	/* set mysql connect timeout. */
	if (mysql_options(*mysql, MYSQL_OPT_CONNECT_TIMEOUT, (uint8_t *)&pppd_mysql_connect_timeout) != 0) {

		info("Plugin %s: MySQL options are unknown\n", PLUGIN_NAME_MYSQL);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_OPTION;
	}

	/* loop through number of connection retries. */
	for (count = pppd_mysql_retry_connect; count > 0 ; count--) {

		/* check if mysql connection was successfully established. */
		if (mysql_real_connect(*mysql, pppd_mysql_host, pppd_mysql_user, pppd_mysql_pass, pppd_mysql_database, (uint32_t)atoi(pppd_mysql_port), (uint8_t *)NULL, 0) == 0) {

			/* check if it was last connection try. */
			if (count == 1) {

				/* something on establishing connection failed. */
				pppd__mysql_error(mysql_errno(*mysql), mysql_sqlstate(*mysql), mysql_error(*mysql));

				/* close the connection. */
				mysql_close(*mysql);

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_CONNECT;
			}
		} else {

			/* check if disable auto commit of database changes was successful. */
			if (mysql_autocommit(*mysql, 0) == 0) {

				/* connection is working. */
				break;
			}
		}
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function disconnect from a mysql database. */
int32_t pppd__mysql_disconnect(MYSQL **mysql) {

	/* check if mysql is allocated. */
	if (*mysql != NULL) {

		/* close the connection. */
		mysql_close(*mysql);
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function return the password from database. */
int32_t pppd__mysql_password(MYSQL **mysql, uint8_t *name, uint8_t *secret_name, int32_t *secret_length) {

	/* some common variables. */
	uint8_t query[1024];
	uint8_t query_extended[1024];
	uint32_t count     = 0;
	uint32_t found     = 0;
	MYSQL_RES *result  = NULL;
	MYSQL_ROW row      = NULL;
	MYSQL_FIELD *field = NULL;

	/* build query for database. */
	snprintf(query, 1024, "SELECT %s, %s, %s FROM %s WHERE %s='%s'", pppd_mysql_column_pass, pppd_mysql_column_client_ip, pppd_mysql_column_server_ip, pppd_mysql_table, pppd_mysql_column_user, name);

	/* check if we have an additional mysql condition. */
	if (pppd_mysql_condition != NULL) {

		/* build extended query for database. */
		snprintf(query_extended, 1024, " AND %s", pppd_mysql_condition);

		/* only write 1023 bytes, because strncat writes 1023 bytes plus the terminating null byte. */
		strncat(query, query_extended, 1023);

		/* clear the memory with the extended query, to build next one if required. */
		memset(query_extended, 0, sizeof(query_extended));
	}

	/* check if we should set an exclusive read lock. */
	if (pppd_mysql_exclusive     == 1 &&
	    pppd_mysql_authoritative == 1 &&
	    pppd_mysql_column_update != NULL) {

		/* only write 1023 bytes, because strncat writes 1023 bytes plus the terminating null byte. */
		strncat(query, " FOR UPDATE", 1023);
	}

	/* loop through number of query retries. */
	for (count = pppd_mysql_retry_query; count > 0 ; count--) {

		/* check if query was successfully executed. */
		if (mysql_query(*mysql, query) == 0) {

			/* indicate that we fetch a result. */
			found = 1;

			/* query result was ok, so break loop. */
			break;
		}
	}

	/* check if no query was executed successfully, very bad :) */
	if (found == 0) {

		/* something on executing query failed. */
		pppd__mysql_error(mysql_errno(*mysql), mysql_sqlstate(*mysql), mysql_error(*mysql));

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* check if mysql result was successfully stored. */
	if ((result = mysql_store_result(*mysql)) == NULL) {

		/* check if mysql should return data. */
		if (mysql_field_count(*mysql) == 0) {

			/* something on executing query failed. */
			pppd__mysql_error(mysql_errno(*mysql), mysql_sqlstate(*mysql), mysql_error(*mysql));

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_QUERY;
		}
	}

	/* check if we have multiple user accounts. */
	if ((mysql_num_rows(result) > 1) && (pppd_mysql_ignore_multiple == 0)) {

		/* multiple user accounts found. */
		error("Plugin %s: Multiple accounts for %s found in database\n", PLUGIN_NAME_MYSQL, name);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* check if we have at least one row. */
	if (mysql_num_rows(result) == 0) {

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* fetch mysql row, we only take care of first row. */
	row = mysql_fetch_row(result);

	/* loop through all columns. */
	for (count = 0; count < mysql_num_fields(result); count++) {

		/* fetch mysql field name. */
		field = mysql_fetch_field(result);

		/* check if column is NULL. */
		if ((row[count] == NULL) && (pppd_mysql_ignore_null == 0)) {

			/* NULL user account found. */
			error("Plugin %s: The column %s for %s is NULL in database\n", PLUGIN_NAME_MYSQL, field->name, name);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_QUERY;
		}

		/* if we reach this point, check only if column is NULL and transform it. */
		row[count] = row[count] ? row[count] : "NULL";

		/* check if we found password. */
		if (count == 0) {

			/* cleanup memory. */
			memset(secret_name, 0, sizeof(secret_name));

			/* copy password to secret. */
			strncpy(secret_name, row[count], MAXSECRETLEN);
			*secret_length = strlen(secret_name);
		}

		/* check if we found client ip. */
		if (count == 1) {

			/* check if ip address was successfully converted into binary data. */
			if (inet_aton(row[count], (struct in_addr *) &client_ip) == 0) {

				/* error on converting ip address. */
				error("Plugin %s: Client IP address %s is not valid\n", PLUGIN_NAME_MYSQL, row[count]);

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_QUERY;
			}
		}

		/* check if we found server ip. */
		if (count == 2) {

			/* check if ip address was successfully converted into binary data. */
			if (inet_aton(row[count], (struct in_addr *) &server_ip) == 0) {

				/* error on converting ip address. */
				error("Plugin %s: Server IP address %s is not valid\n", PLUGIN_NAME_MYSQL, row[count]);

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_QUERY;
			}
		}
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function update the login status in database. */
int32_t pppd__mysql_status(MYSQL **mysql, uint8_t *name, uint32_t status) {

	/* some common variables. */
	uint8_t query[1024];
	uint32_t count = 0;
	uint32_t found = 0;

	/* build query for database. */
	snprintf(query, 1024, "UPDATE %s SET %s='%d' WHERE %s='%s'", pppd_mysql_table, pppd_mysql_column_update, status, pppd_mysql_column_user, name);

	/* loop through number of query retries. */
	for (count = pppd_mysql_retry_query; count > 0 ; count--) {

		/* check if query was successfully executed. */
		if (mysql_query(*mysql, query) == 0) {

			/* check if commit change to database was successfully executed. */
			if (mysql_commit(*mysql) == 0) {

				/* indicate that we fetch a result. */
				found = 1;

				/* query result was ok, so break loop. */
				break;
			}
		}
	}

	/* check if no query was executed successfully, very bad :) */
	if (found == 0) {

		/* something on executing query failed. */
		pppd__mysql_error(mysql_errno(*mysql), mysql_sqlstate(*mysql), mysql_error(*mysql));

		/* rollback execution. */
		mysql_rollback(*mysql);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function is the ip up notifier for the ppp daemon. */
void pppd__mysql_up(void *opaque, int32_t arg) {

	/* some common variables. */
	MYSQL *mysql = NULL;

	/* check if we should execute a script. */
	if (pppd_mysql_ip_up != NULL) {

		/* execute script. */
		if (pppd__ip_up(username, pppd_mysql_ip_up) != 0) {

			/* check if we should fail. */
			if (pppd_mysql_ip_up_fail == 1) {

				/* show the error. */
				error("Plugin %s: Script '%s' returned with non-zero status\n", PLUGIN_NAME_MYSQL, pppd_mysql_ip_up);

				/* check if status should be updated. */
				if (pppd_mysql_exclusive     == 1 &&
				    pppd_mysql_authoritative == 1 &&
				    pppd_mysql_column_update != NULL) {

					/* check if mysql connect is working. */
					if (pppd__mysql_connect(&mysql) == 0) {

						/* update database. (ignore return code, because what should I do, stop the disconnect?) */
						pppd__mysql_status(&mysql, username, 0);

						/* disconnect from mysql. */
						pppd__mysql_disconnect(&mysql);
					}
				}

				/* die bitch die. */
				die(1);
			}
		}
	}
}

/* this function is the ip down notifier for the ppp daemon. */
void pppd__mysql_down(void *opaque, int32_t arg) {

	/* some common variables. */
	MYSQL *mysql = NULL;

	/* check if we should execute a script. */
	if (pppd_mysql_ip_down != NULL) {

		/* execute script. */
		if (pppd__ip_down(username, pppd_mysql_ip_down) != 0) {

			/* check if we should fail. */
			if (pppd_mysql_ip_down_fail == 1) {

				/* show the error. */
				error("Plugin %s: Script '%s' returned with non-zero status\n", PLUGIN_NAME_MYSQL, pppd_mysql_ip_up);

				/* die bitch die. */
				die(1);
			}
		}
	}

	/* check if status should be updated. */
	if (pppd_mysql_exclusive     == 1 &&
	    pppd_mysql_authoritative == 1 &&
	    pppd_mysql_column_update != NULL) {

		/* check if mysql connect is working. */
		if (pppd__mysql_connect(&mysql) == 0) {

			/* update database. (ignore return code, because what should I do, stop the disconnect?) */
			pppd__mysql_status(&mysql, username, 0);

			/* disconnect from mysql. */
			pppd__mysql_disconnect(&mysql);
		}
	}
}

/* this function check the chap authentication information against a mysql database. */
int32_t pppd__chap_verify_mysql(char *name, char *ourname, int id, struct chap_digest_type *digest, unsigned char *challenge, unsigned char *response, char *message, int message_space) {

	/* some common variables. */
	uint8_t secret_name[MAXSECRETLEN];
	int32_t secret_length = 0;
	MYSQL *mysql          = NULL;

	/* check if parameters are complete. */
	if (pppd__mysql_parameter() == 0) {

		/* check if mysql connect is working. */
		if (pppd__mysql_connect(&mysql) == 0) {

			/* check if mysql fetching was successful. */
			if (pppd__mysql_password(&mysql, name, secret_name, &secret_length) == 0) {

				/* check if password decryption was correct. */
				if (pppd__decrypt_password(secret_name, &secret_length, pppd_mysql_pass_encryption, pppd_mysql_pass_key) == 0) {

					/* verify discovered secret against the client's response. */
					if (digest->verify_response(id, name, secret_name, secret_length, challenge, response, message, message_space) == 1) {

						/* check if database update was successful. */
						if (pppd__mysql_status(&mysql, name, 1) == 0) {

							/* store username for ip down configuration. */
							strncpy(username, name, MAXNAMELEN);

							/* disconnect from mysql. */
							pppd__mysql_disconnect(&mysql);

							/* clear the memory with the password, so nobody is able to dump it. */
							memset(secret_name, 0, sizeof(secret_name));

							/* if no error was found, establish link. */
							return 1;
						}
					}
				}
			}

			/* disconnect from mysql. */
			pppd__mysql_disconnect(&mysql);
		}
	}

	/* check if mysql is not authoritative. */
	if (pppd_mysql_authoritative == 0) {

		/* get the secret that the peer is supposed to know. */
		if (get_secret(0, name, ourname, secret_name, &secret_length, 1) == 1) {

			/* verify discovered secret against the client's response. */
			if (digest->verify_response(id, name, secret_name, secret_length, challenge, response, message, message_space) == 1) {

				/* clear the memory with the password, so nobody is able to dump it. */
				memset(secret_name, 0, sizeof(secret_name));

				/* if no error was found, establish link. */
				return 1;
			}
		}

		/* show user that fallback also fails. */
		error("No CHAP secret found for authenticating %q", name);
	}

	/* clear the memory with the password, so nobody is able to dump it. */
	memset(secret_name, 0, sizeof(secret_name));

	/* return with error and terminate link. */
	return 0;
}

/* this function check the pap authentication information against a mysql database. */
int32_t pppd__pap_auth_mysql(char *user, char *passwd, char **msgp, struct wordlist **paddrs, struct wordlist **popts) {

	/* some common variables. */
	uint8_t secret_name[MAXSECRETLEN];
	int32_t secret_length = 0;
	MYSQL *mysql          = NULL;

	/* check if parameters are complete. */
	if (pppd__mysql_parameter() == 0) {

		/* check if mysql connect is working. */
		if (pppd__mysql_connect(&mysql) == 0) {

			/* check if mysql fetching was successful. */
			if (pppd__mysql_password(&mysql, user, secret_name, &secret_length) == 0) {

				/* check if the password is correct. */
				if (pppd__verify_password(passwd, secret_name, pppd_mysql_pass_encryption, pppd_mysql_pass_key) == 0) {

					/* check if database update was successful. */
					if (pppd__mysql_status(&mysql, user, 1) == 0) {

						/* store username for ip down configuration. */
						strncpy(username, user, MAXNAMELEN);

						/* disconnect from mysql. */
						pppd__mysql_disconnect(&mysql);

						/* clear the memory with the password, so nobody is able to dump it. */
						memset(secret_name, 0, sizeof(secret_name));

						/* if no error was found, establish link. */
						return 1;
					}
				}
			}

			/* disconnect from mysql. */
			pppd__mysql_disconnect(&mysql);
		}
	}

	/* check if mysql is not authoritative. */
	if (pppd_mysql_authoritative == 0) {

		/* return with error and look in pap file. */
		return -1;
	}

	/* clear the memory with the password, so nobody is able to dump it. */
	memset(secret_name, 0, sizeof(secret_name));

	/* return with error and terminate link. */
	return 0;
}

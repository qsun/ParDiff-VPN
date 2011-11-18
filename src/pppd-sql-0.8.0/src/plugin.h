/*
 *  plugin.h -- Challenge Handshake Authentication Protocol for the
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

#ifndef _PLUGIN_H
#define _PLUGIN_H

/* configuration includes. */
#include "config.h"

/* autoconf declares VERSION which is declared in pppd.h too. */
#ifdef VERSION
#undef VERSION
#endif

/* generic includes. */
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>

/* ppp generic includes. */
#include <pppd/chap-new.h>
#include <pppd/md5.h>
#include <pppd/pppd.h>

/* ppp ipcp includes. */
#include <pppd/fsm.h>
#include <pppd/ipcp.h>

/* openssl includes. */
#include <openssl/des.h>
#include <openssl/evp.h>

/* define errors. */
#define PPPD_SQL_ERROR_INCOMPLETE	-1	/* the supplied sql information from configuration file are not complete. */
#define PPPD_SQL_ERROR_INIT		-2	/* the initialization of the sql structure failed. */
#define PPPD_SQL_ERROR_OPTION		-3	/* some unknown sql options were given. */
#define PPPD_SQL_ERROR_CONNECT		-4	/* none of the supplied sql servers are working. */
#define PPPD_SQL_ERROR_QUERY		-5	/* the given sql query failed. */
#define PPPD_SQL_ERROR_PASSWORD		-6	/* the given password is wrong. */
#define PPPD_SQL_ERROR_SCRIPT		-7	/* the up or down script failed. (returned with non-zero exit code) */

/* define constants. */
#define SIZE_AES			16	/* the size of an AES128 result. */
#define SIZE_MD5			16	/* the size of a MD5 hash. */
#define SIZE_CRYPT			13	/* the size of the crypt() DES result. */

/* client and server ip address must be stored in global variable, because
 * at IPCP time we no longer know the username.
 */
extern uint32_t client_ip;
extern uint32_t server_ip;

/* this function set whether the peer must authenticate itself to us via CHAP. */
int32_t pppd__chap_check(
	void
);

/* this function set whether the peer must authenticate itself to us via PAP. */
int32_t pppd__pap_check(
	void
);

/* this function will look for a valid client ip address. */
void pppd__ip_choose(
	uint32_t	*addrp
);

/* this function set whether the plugin is allowed to set client ip addresses. */
int32_t pppd__allowed_address(
	uint32_t	addr
);

/* this function will execute a script when IPCP comes up. */
int32_t pppd__ip_up(
	uint8_t		*username,
	uint8_t		*program
);

/* this function will execute a script when IPCP goes down. */
int32_t pppd__ip_down(
	uint8_t		*username,
	uint8_t		*program
);

/* this function verify the given password. */
int32_t pppd__verify_password(
	uint8_t		*passwd,
	uint8_t		*secret_name,
	uint8_t		*encryption,
	uint8_t		*key
);

/* this function decrypt the given password. */
int32_t pppd__decrypt_password(
	uint8_t		*secret_name,
	int32_t		*secret_length,
	uint8_t		*encryption,
	uint8_t		*key
);

#endif					/* _PLUGIN_H */

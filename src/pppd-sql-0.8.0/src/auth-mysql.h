/*
 *  auth-mysql.h -- Challenge Handshake Authentication Protocol and Password
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

#ifndef _AUTH_MYSQL_H
#define _AUTH_MYSQL_H

/* this function handles the mysql_error() result. */
int32_t pppd__mysql_error(
	uint32_t	error_code,
	const uint8_t	*error_message,
	const uint8_t	*error_state
);

/* this function check the parameter. */
int32_t pppd__mysql_parameter(
	void
);

/* this function connect to a mysql database. */
int32_t pppd__mysql_connect(
	MYSQL		**mysql
);

/* this function disconnect from a mysql database. */
int32_t pppd__mysql_disconnect(
	MYSQL		**mysql
);

/* this function return the password from database. */
int32_t pppd__mysql_password(
	MYSQL		**mysql,
	uint8_t		*name,
	uint8_t		*secret_name,
	int32_t		*secret_length
);

/* this function update the login status in database. */
int32_t pppd__mysql_status(
	MYSQL		**mysql,
	uint8_t		*name,
	uint32_t	status
);

/* this function is the ip up notifier for the ppp daemon. */
void pppd__mysql_up(
	void		*opaque,
	int32_t		arg
);

/* this function is the ip down notifier for the ppp daemon. */
void pppd__mysql_down(
	void		*opaque,
	int32_t		arg
);

/* this function check the chap authentication information against a mysql database. */
int32_t pppd__chap_verify_mysql(
	char		*name,
	char		*ourname,
	int		id,
	struct chap_digest_type		*digest,
	unsigned char	*challenge,
	unsigned char	*response,
	char		*message,
	int		message_space
);

/* this function check the pap authentication information against a mysql database. */
int32_t pppd__pap_auth_mysql(
	char		*user,
	char		*passwd,
	char		**msgp,
	struct wordlist	**paddrs,
	struct wordlist	**popts
);

#endif					/* _AUTH_MYSQL_H */

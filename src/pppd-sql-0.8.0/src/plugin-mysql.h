/*
 *  plugin-mysql.h -- MySQL Authentication plugin for Point-to-Point
 *                    Protocol (PPP).
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

#ifndef _PLUGIN_MYSQL_H
#define _PLUGIN_MYSQL_H

/* generic includes. */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/* mysql includes. */
#include <mysql/mysql.h>

/* global define to indicate that plugin only works with compile time pppd. */
extern uint8_t pppd_version[];

/* global configuration variables. */
extern uint8_t *pppd_mysql_host;
extern uint8_t *pppd_mysql_port;
extern uint8_t *pppd_mysql_user;
extern uint8_t *pppd_mysql_pass;
extern uint8_t *pppd_mysql_pass_encryption;
extern uint8_t *pppd_mysql_pass_key;
extern uint8_t *pppd_mysql_database;
extern uint8_t *pppd_mysql_table;
extern uint8_t *pppd_mysql_column_user;
extern uint8_t *pppd_mysql_column_pass;
extern uint8_t *pppd_mysql_column_client_ip;
extern uint8_t *pppd_mysql_column_server_ip;
extern uint8_t *pppd_mysql_column_update;
extern uint8_t *pppd_mysql_condition;
extern uint32_t pppd_mysql_exclusive;
extern uint32_t pppd_mysql_authoritative;
extern uint32_t pppd_mysql_ignore_multiple;
extern uint32_t pppd_mysql_ignore_null;
extern uint32_t pppd_mysql_connect_timeout;
extern uint32_t pppd_mysql_retry_connect;
extern uint32_t pppd_mysql_retry_query;
extern uint8_t *pppd_mysql_ip_up;
extern uint32_t pppd_mysql_ip_up_fail;
extern uint8_t *pppd_mysql_ip_down;
extern uint32_t pppd_mysql_ip_down_fail;

/* extra option structure. */
extern option_t options[];

/* plugin initialization routine. */
void plugin_init(
	void
);

#endif					/* _PLUGIN_MYSQL_H */

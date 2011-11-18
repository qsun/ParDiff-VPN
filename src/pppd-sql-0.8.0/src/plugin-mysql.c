/*
 *  plugin-mysql.c -- MySQL Authentication plugin for Point-to-Point
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

/* generic plugin includes. */
#include "plugin.h"
#include "plugin-mysql.h"

/* plugin auth includes. */
#include "auth-mysql.h"

/* global define to indicate that plugin only works with compile time pppd. */
uint8_t pppd_version[]			= VERSION;

/* global configuration variables. */
uint8_t *pppd_mysql_host		= NULL;
uint8_t *pppd_mysql_port		= NULL;
uint8_t *pppd_mysql_user		= NULL;
uint8_t *pppd_mysql_pass		= NULL;
uint8_t *pppd_mysql_pass_encryption	= NULL;
uint8_t *pppd_mysql_pass_key		= NULL;
uint8_t *pppd_mysql_database		= NULL;
uint8_t *pppd_mysql_table		= NULL;
uint8_t *pppd_mysql_column_user		= NULL;
uint8_t *pppd_mysql_column_pass		= NULL;
uint8_t *pppd_mysql_column_client_ip	= NULL;
uint8_t *pppd_mysql_column_server_ip	= NULL;
uint8_t *pppd_mysql_column_update	= NULL;
uint8_t *pppd_mysql_condition		= NULL;
uint32_t pppd_mysql_exclusive		= 0;
uint32_t pppd_mysql_authoritative	= 0;
uint32_t pppd_mysql_ignore_multiple	= 0;
uint32_t pppd_mysql_ignore_null		= 0;
uint32_t pppd_mysql_connect_timeout	= 5;
uint32_t pppd_mysql_retry_connect	= 5;
uint32_t pppd_mysql_retry_query		= 5;
uint8_t *pppd_mysql_ip_up		= NULL;
uint32_t pppd_mysql_ip_up_fail		= 0;
uint8_t *pppd_mysql_ip_down		= NULL;
uint32_t pppd_mysql_ip_down_fail	= 0;

/* client and server ip address must be stored in global variable, because
 * at IPCP time we no longer know the username.
 */
uint32_t client_ip			= 0;
uint32_t server_ip			= 0;

/* extra option structure. */
option_t options[] = {
	{ "mysql-host", o_string, &pppd_mysql_host, "Set MySQL server host" },
	{ "mysql-port", o_string, &pppd_mysql_port, "Set MySQL server port" },
	{ "mysql-user", o_string, &pppd_mysql_user, "Set MySQL username" },
	{ "mysql-pass", o_string, &pppd_mysql_pass, "Set MySQL password" },
	{ "mysql-pass-encryption", o_string, &pppd_mysql_pass_encryption, "Set MySQL password encryption algorithm" },
	{ "mysql-pass-key", o_string, &pppd_mysql_pass_key, "Set MySQL password encryption key or salt" },
	{ "mysql-database", o_string, &pppd_mysql_database, "Set MySQL database name" },
	{ "mysql-table", o_string, &pppd_mysql_table, "Set MySQL authentication table" },
	{ "mysql-column-user", o_string, &pppd_mysql_column_user, "Set MySQL username field" },
	{ "mysql-column-pass", o_string, &pppd_mysql_column_pass, "Set MySQL password field" },
	{ "mysql-column-client-ip", o_string, &pppd_mysql_column_client_ip, "Set MySQL client ip address field" },
	{ "mysql-column-server-ip", o_string, &pppd_mysql_column_server_ip, "Set MySQL server ip address field" },
	{ "mysql-column-update", o_string, &pppd_mysql_column_update, "Set MySQL update field" },
	{ "mysql-condition", o_string, &pppd_mysql_condition, "Set MySQL condition clause" },
	{ "mysql-exclusive", o_bool, &pppd_mysql_exclusive, "Set MySQL to forbid concurrent connection from one user", 0 | 1 },
	{ "mysql-authoritative", o_bool, &pppd_mysql_authoritative, "Set MySQL to be authoritative authenticator", 0 | 1 },
	{ "mysql-ignore-multiple", o_bool, &pppd_mysql_ignore_multiple, "Set MySQL to cover first row from multiple rows", 0 | 1 },
	{ "mysql-ignore-null", o_bool, &pppd_mysql_ignore_null, "Set MySQL to cover NULL results as string", 0 | 1 },
	{ "mysql-connect-timeout", o_int, &pppd_mysql_connect_timeout, "Set MySQL connection timeout" },
	{ "mysql-retry-connect", o_int, &pppd_mysql_retry_connect, "Set MySQL connection retries" },
	{ "mysql-retry-query", o_int, &pppd_mysql_retry_query, "Set MySQL query retries" },
	{ "mysql-ip-up", o_string, &pppd_mysql_ip_up, "Set MySQL script to execute when IPCP has come up" },
	{ "mysql-ip-up-fail", o_bool, &pppd_mysql_ip_up_fail, "Set MySQL IPCP up script to terminate link on unsuccessful execution", 0 | 1 },
	{ "mysql-ip-down", o_string, &pppd_mysql_ip_down, "Set MySQL script to execute when IPCP goes down" },
	{ "mysql-ip-down-fail", o_bool, &pppd_mysql_ip_down_fail, "Set MySQL IPCP down script to terminate link on unsuccessful execution", 0 | 1 },
	{ NULL }
};

/* plugin initilization routine. */
void plugin_init(void) {

	/* show initialization information. */
	info("Plugin %s: pppd-sql-%s initialized, compiled pppd-%s, linked mysql-%s\n", PLUGIN_NAME_MYSQL, PACKAGE_VERSION, pppd_version, PLUGIN_VERSION_MYSQL);

	/* add hook for chap authentication. */
	chap_check_hook		= pppd__chap_check;
	chap_verify_hook	= pppd__chap_verify_mysql;

	/* add hook for pap authentication. */
	pap_check_hook		= pppd__pap_check;
	pap_auth_hook		= pppd__pap_auth_mysql;

	/* plugin is aware of assigning ip addresses on IPCP negotiation. */
	//ip_choose_hook		= pppd__ip_choose;
	allowed_address_hook	= pppd__allowed_address;

	/* add ip notifiers. */
	add_notifier(&ip_up_notifier, pppd__mysql_up, NULL);
	add_notifier(&ip_down_notifier, pppd__mysql_down, NULL);

	/* point extra options to our array. */
	add_options(options);
}

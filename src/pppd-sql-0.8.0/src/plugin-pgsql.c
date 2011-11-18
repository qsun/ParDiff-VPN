/*
 *  plugin-pgsql.c -- PostgreSQL Authentication plugin for Point-to-Point
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
#include "plugin-pgsql.h"

/* plugin auth includes. */
#include "auth-pgsql.h"

/* global define to indicate that plugin only works with compile time pppd. */
uint8_t pppd_version[]			= VERSION;

/* global configuration variables. */
uint8_t *pppd_pgsql_host		= NULL;
uint8_t *pppd_pgsql_port		= NULL;
uint8_t *pppd_pgsql_user		= NULL;
uint8_t *pppd_pgsql_pass		= NULL;
uint8_t *pppd_pgsql_pass_encryption	= NULL;
uint8_t *pppd_pgsql_pass_key		= NULL;
uint8_t *pppd_pgsql_database		= NULL;
uint8_t *pppd_pgsql_table		= NULL;
uint8_t *pppd_pgsql_column_user		= NULL;
uint8_t *pppd_pgsql_column_pass		= NULL;
uint8_t *pppd_pgsql_column_client_ip	= NULL;
uint8_t *pppd_pgsql_column_server_ip	= NULL;
uint8_t *pppd_pgsql_column_update	= NULL;
uint8_t *pppd_pgsql_condition		= NULL;
uint32_t pppd_pgsql_exclusive		= 0;
uint32_t pppd_pgsql_authoritative	= 0;
uint32_t pppd_pgsql_ignore_multiple	= 0;
uint32_t pppd_pgsql_ignore_null		= 0;
uint32_t pppd_pgsql_connect_timeout	= 5;
uint32_t pppd_pgsql_retry_connect	= 5;
uint32_t pppd_pgsql_retry_query		= 5;
uint8_t *pppd_pgsql_ip_up		= NULL;
uint32_t pppd_pgsql_ip_up_fail		= 0;
uint8_t *pppd_pgsql_ip_down		= NULL;
uint32_t pppd_pgsql_ip_down_fail	= 0;

/* client and server ip address must be stored in global variable, because
 * at IPCP time we no longer know the username.
 */
uint32_t client_ip			= 0;
uint32_t server_ip			= 0;

/* extra option structure. */
option_t options[] = {
	{ "pgsql-host", o_string, &pppd_pgsql_host, "Set PostgreSQL server host" },
	{ "pgsql-port", o_string, &pppd_pgsql_port, "Set PostgreSQL server port" },
	{ "pgsql-user", o_string, &pppd_pgsql_user, "Set PostgreSQL username" },
	{ "pgsql-pass", o_string, &pppd_pgsql_pass, "Set PostgreSQL password" },
	{ "pgsql-pass-encryption", o_string, &pppd_pgsql_pass_encryption, "Set PostgreSQL password encryption algorithm" },
	{ "pgsql-pass-key", o_string, &pppd_pgsql_pass_key, "Set PostgreSQL password encryption key or salt" },
	{ "pgsql-database", o_string, &pppd_pgsql_database, "Set PostgreSQL database name" },
	{ "pgsql-table", o_string, &pppd_pgsql_table, "Set PostgreSQL authentication table" },
	{ "pgsql-column-user", o_string, &pppd_pgsql_column_user, "Set PostgreSQL username field" },
	{ "pgsql-column-pass", o_string, &pppd_pgsql_column_pass, "Set PostgreSQL password field" },
	{ "pgsql-column-client-ip", o_string, &pppd_pgsql_column_client_ip, "Set PostgreSQL client ip address field" },
	{ "pgsql-column-server-ip", o_string, &pppd_pgsql_column_server_ip, "Set PostgreSQL server ip address field" },
	{ "pgsql-column-update", o_string, &pppd_pgsql_column_update, "Set PostgreSQL update field" },
	{ "pgsql-condition", o_string, &pppd_pgsql_condition, "Set PostgreSQL condition clause" },
	{ "pgsql-exclusive", o_bool, &pppd_pgsql_exclusive, "Set PostgreSQL to forbid concurrent connection from one user", 0 | 1 },
	{ "pgsql-authoritative", o_bool, &pppd_pgsql_authoritative, "Set PostgreSQL to be authoritative authenticator", 0 | 1 },
	{ "pgsql-ignore-multiple", o_bool, &pppd_pgsql_ignore_multiple, "Set PostgreSQL to cover first row from multiple rows", 0 | 1 },
	{ "pgsql-ignore-null", o_bool, &pppd_pgsql_ignore_null, "Set PostgreSQL to cover NULL results as string", 0 | 1 },
	{ "pgsql-connect-timeout", o_int, &pppd_pgsql_connect_timeout, "Set PostgreSQL connection timeout" },
	{ "pgsql-retry-connect", o_int, &pppd_pgsql_retry_connect, "Set PostgreSQL connection retries" },
	{ "pgsql-retry-query", o_int, &pppd_pgsql_retry_query, "Set PostgreSQL query retries" },
	{ "pgsql-ip-up", o_string, &pppd_pgsql_ip_up, "Set PostgreSQL script to execute when IPCP has come up" },
	{ "pgsql-ip-up-fail", o_bool, &pppd_pgsql_ip_up_fail, "Set PostgreSQL IPCP up script to terminate link on unsuccessful execution", 0 | 1 },
	{ "pgsql-ip-down", o_string, &pppd_pgsql_ip_down, "Set PostgreSQL script to execute when IPCP goes down" },
	{ "pgsql-ip-down-fail", o_bool, &pppd_pgsql_ip_down_fail, "Set PostgreSQL IPCP down script to terminate link on unsuccessful execution", 0 | 1 },
	{ NULL }
};

/* plugin initilization routine. */
void plugin_init(void) {

	/* show initialization information. */
	info("Plugin %s: pppd-sql-%s initialized, compiled pppd-%s, linked pgsql-%s\n", PLUGIN_NAME_PGSQL, PACKAGE_VERSION, pppd_version, PLUGIN_VERSION_PGSQL);

	/* add hook for chap authentication. */
	chap_check_hook		= pppd__chap_check;
	chap_verify_hook	= pppd__chap_verify_pgsql;

	/* add hook for pap authentication. */
	pap_check_hook		= pppd__pap_check;
	pap_auth_hook		= pppd__pap_auth_pgsql;

	/* plugin is aware of assigning ip addresses on IPCP negotiation. */
	ip_choose_hook		= pppd__ip_choose;
	allowed_address_hook	= pppd__allowed_address;

	/* add ip notifiers. */
	add_notifier(&ip_up_notifier, pppd__pgsql_up, NULL);
	add_notifier(&ip_down_notifier, pppd__pgsql_down, NULL);

	/* point extra options to our array. */
	add_options(options);
}

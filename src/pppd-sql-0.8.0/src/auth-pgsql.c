/*
 *  auth-pgsql.c -- Challenge Handshake Authentication Protocol and Password
 *                  Authentication Protocol for the Point-to-Point Protocol
 *                  (PPP) via PostgreSQL.
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
#include "str.h"

/* auth plugin includes. */
#include "auth-pgsql.h"

/* store username in global variable, because ip down did not know it. */
uint8_t username[MAXNAMELEN];

/* this function handles the PQerrorMessage() result. */
int32_t pppd__pgsql_error(uint8_t *error_message) {

	/* some common variables. */
	uint8_t *error_token;
	uint32_t first = 0;

	/* show error header. */
	error("Plugin %s: Fatal Error Message (PostgreSQL):\n", PLUGIN_NAME_PGSQL);

	/* loop though error message and separate it by tab delimiter. */
	while ((error_token = pppd__strsep(&error_message, (unsigned char *)"\t")) != NULL) {

		/* check if we show first line. */
		if (first == 0) {

			/* show the detailed error. */
			error("Plugin %s: * %s\n", PLUGIN_NAME_PGSQL, error_token);

			/* increase counter. */
			first++;
		} else {

			/* show the detailed error. */
			error("Plugin %s:   %s\n", PLUGIN_NAME_PGSQL, error_token);
		}
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function check the parameter. */
int32_t pppd__pgsql_parameter(void) {

	/* check if all information are supplied. */
	if (pppd_pgsql_host		== NULL ||
	    pppd_pgsql_port		== NULL ||
	    pppd_pgsql_user		== NULL ||
	    pppd_pgsql_pass		== NULL ||
	    pppd_pgsql_pass_encryption	== NULL ||
	    pppd_pgsql_database		== NULL ||
	    pppd_pgsql_table		== NULL ||
	    pppd_pgsql_column_user	== NULL ||
	    pppd_pgsql_column_pass	== NULL ||
	    pppd_pgsql_column_client_ip	== NULL ||
	    pppd_pgsql_column_server_ip	== NULL) {

		/* something failed on postgresql initialization. */
		error("Plugin %s: PostgreSQL information are not complete\n", PLUGIN_NAME_PGSQL);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_INCOMPLETE;
	}

	/* check if passwords are encrypted. */
	if (strcasecmp((char *)pppd_pgsql_pass_encryption, "CRYPT")	== 0 ||
	    strcasecmp((char *)pppd_pgsql_pass_encryption, "AES")	== 0) {

		/* check if key or salt is given. */
		if (pppd_pgsql_pass_key == NULL) {

			/* some required encryption information are missing. */
			error("Plugin: %s: PostgreSQL encryption information are not complete\n", PLUGIN_NAME_PGSQL);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_INCOMPLETE;
		}
	}

	/* check if concurrent connection from one user should be denied. */
	if (pppd_pgsql_exclusive == 1) {

		/* check if update column is given. */
		if (pppd_pgsql_column_update == NULL ||
		    pppd_pgsql_authoritative == 0) {

			/* some required exclusive information are missing. */
			error("Plugin: %s: PostgreSQL exclusive information are not complete\n", PLUGIN_NAME_PGSQL);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_INCOMPLETE;
		}
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function begin or end a transaction. */
int32_t pppd__pgsql_transaction(PGconn *pgsql, uint8_t *transaction) {

	/* some common variables. */
	uint32_t count   = 0;
	uint32_t found   = 0;
	PGresult *result = NULL;

	/* loop through number of query retries. */
	for (count = pppd_pgsql_retry_query; count > 0 ; count--) {

		/* check if query was successfully executed. */
		if ((result = PQexec(pgsql, (char *)transaction)) != NULL) {

			/* indicate that we fetch a result. */
			found = 1;

			/* query result was ok, so break loop. */
			break;
		}

		/* clear memory to avoid leaks. */
		PQclear(result);
	}

	/* check if no query was executed successfully, very bad :) */
	if (found == 0) {

		/* something on executing query failed. */
		pppd__pgsql_error((uint8_t *)PQerrorMessage(pgsql));

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* clear memory to avoid leaks. */
	PQclear(result);

	/* if no error was found, return zero. */
	return 0;
}

/* this function connect to a postgresql database. */
int32_t pppd__pgsql_connect(PGconn **pgsql) {

	/* some common variables. */
	uint8_t connection_info[1024];
	uint32_t count = 0;

	/* clear connection info from previous connection. */
	memset(connection_info, 0, sizeof(connection_info));

	/* create the connection information for postgresql. */
	snprintf((char *)connection_info, 1024, "host='%s' port='%s' user='%s' password='%s' dbname='%s' connect_timeout='%i'", pppd_pgsql_host, pppd_pgsql_port, pppd_pgsql_user, pppd_pgsql_pass, pppd_pgsql_database, pppd_pgsql_connect_timeout);

	/* loop through number of connection retries. */
	for (count = pppd_pgsql_retry_connect; count > 0 ; count--) {

		/* connect to postgresql database. */
		*pgsql = PQconnectdb((char *)connection_info);

		/* check if postgresql connection was successfully established. */
		if (PQstatus(*pgsql) != CONNECTION_OK) {

			/* check if it was last connection try. */
			if (count == 1) {

				/* something on establishing connection failed. */
				pppd__pgsql_error((uint8_t *)PQerrorMessage(*pgsql));

				/* close the connection. */
				PQfinish(*pgsql);

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_CONNECT;
			}
		} else {

			/* connection is working. */
			break;
		}
	}

	/* check if transaction begin was successful. */
	if (pppd__pgsql_transaction(*pgsql, (uint8_t *)"BEGIN") < 0) {

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_CONNECT;
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function disconnect from a postgresql database. */
int32_t pppd__pgsql_disconnect(PGconn **pgsql) {

	/* finish transaction. (ignore return code, because what should I do, stop the disconnect?) */
	pppd__pgsql_transaction(*pgsql, (uint8_t *)"END");

	/* check if pgsql is allocated. */
	if (*pgsql != NULL) {

		/* close the connection. */
		PQfinish(*pgsql);
	}

	/* if no error was found, return zero. */
	return 0;
}

/* this function return the password from database. */
int32_t pppd__pgsql_password(PGconn **pgsql, uint8_t *name, uint8_t *secret_name, int32_t *secret_length) {

	/* some common variables. */
	uint8_t query[1024];
	uint8_t query_extended[1024];
	int32_t is_null  = 0;
	uint32_t count   = 0;
	uint32_t found   = 0;
	uint8_t *row     = 0;
	uint8_t *field   = NULL;
	PGresult *result = NULL;

	/* build query for database. */
	snprintf((char *)query, 1024, "SELECT %s, %s, %s FROM %s WHERE %s='%s'", pppd_pgsql_column_pass, pppd_pgsql_column_client_ip, pppd_pgsql_column_server_ip, pppd_pgsql_table, pppd_pgsql_column_user, name);

	/* check if we have an additional postgresql condition. */
	if (pppd_pgsql_condition != NULL) {

		/* build extended query for database. */
		snprintf((char *)query_extended, 1024, " AND %s", pppd_pgsql_condition);

		/* only write 1023 bytes, because strncat writes 1023 bytes plus the terminating null byte. */
		strncat((char *)query, (char *)query_extended, 1023);

		/* clear the memory with the extended query, to build next one if required. */
		memset(query_extended, 0, sizeof(query_extended));
	}

	/* check if we should set an exclusive read lock. */
	if (pppd_pgsql_exclusive     == 1 &&
	    pppd_pgsql_authoritative == 1 &&
	    pppd_pgsql_column_update != NULL) {

		/* only write 1023 bytes, because strncat writes 1023 bytes plus the terminating null byte. */
		strncat((char *)query, " FOR UPDATE", 1023);
	}

	/* loop through number of query retries. */
	for (count = pppd_pgsql_retry_query; count > 0 ; count--) {

		/* check if query was successfully executed. */
		if ((result = PQexec(*pgsql, (char *)query)) != NULL) {

			/* indicate that we fetch a result. */
			found = 1;

			/* query result was ok, so break loop. */
			break;
		}

		/* clear memory to avoid leaks. */
		PQclear(result);
	}

	/* check if no query was executed successfully, very bad :) */
	if (found == 0) {

		/* something on executing query failed. */
		pppd__pgsql_error((uint8_t *)PQerrorMessage(*pgsql));

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* check if postgresql should return data. */
	if (PQnfields(result) == 0) {

		/* something on executing query failed. */
		pppd__pgsql_error((uint8_t *)PQerrorMessage(*pgsql));

		/* clear memory to avoid leaks. */
		PQclear(result);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* check if we have multiple user accounts. */
	if ((PQntuples(result) > 1) && (pppd_pgsql_ignore_multiple == 0)) {

		/* multiple user accounts found. */
		error("Plugin %s: Multiple accounts for %s found in database\n", PLUGIN_NAME_PGSQL, name);

		/* clear memory to avoid leaks. */
		PQclear(result);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* loop through all columns. */
	for (count = 0; count < PQnfields(result); count++) {

		/* fetch postgresql field name. */
		field = (uint8_t *)PQfname(result, count);

		/* fetch NULL information. */
		is_null = PQgetisnull(result, 0, count);

		/* first check what result we should get. */
		if (is_null == 0) {

			/* fetch column. */
			row = (uint8_t *)PQgetvalue(result, 0, count);
		} else {

			/* set row to string NULL. */
			row = (uint8_t *)"NULL";
		}

		/* check if column is NULL. */
		if ((is_null == 1) && (pppd_pgsql_ignore_null == 0)) {

			/* NULL user account found. */
			error("Plugin %s: The column %s for %s is NULL in database\n", PLUGIN_NAME_PGSQL, field, name);

			/* clear memory to avoid leaks. */
			PQclear(result);

			/* return with error and terminate link. */
			return PPPD_SQL_ERROR_QUERY;
		}

		/* check if we found password. */
		if (count == 0) {

			/* cleanup memory. */
			memset(secret_name, 0, sizeof(secret_name));

			/* copy password to secret. */
			strncpy((char *)secret_name, (char *)row, MAXSECRETLEN);
			*secret_length = strlen((char *)secret_name);
		}

		/* check if we found client ip. */
		if (count == 1) {

			/* check if ip address was successfully converted into binary data. */
			if (inet_aton((char *)row, (struct in_addr *) &client_ip) == 0) {

				/* error on converting ip address. */
				error("Plugin %s: Client IP address %s is not valid\n", PLUGIN_NAME_PGSQL, row);

				/* clear memory to avoid leaks. */
				PQclear(result);

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_QUERY;
			}
		}

		/* check if we found server ip. */
		if (count == 2) {

			/* check if ip address was successfully converted into binary data. */
			if (inet_aton((char *)row, (struct in_addr *) &server_ip) == 0) {

				/* error on converting ip address. */
				error("Plugin %s: Server IP address %s is not valid\n", PLUGIN_NAME_PGSQL, row);

				/* clear memory to avoid leaks. */
				PQclear(result);

				/* return with error and terminate link. */
				return PPPD_SQL_ERROR_QUERY;
			}
		}
	}

	/* clear memory to avoid leaks. */
	PQclear(result);

	/* if no error was found, return zero. */
	return 0;
}

/* this function update the login status in database. */
int32_t pppd__pgsql_status(PGconn **pgsql, uint8_t *name, uint32_t status) {

	/* some common variables. */
	uint8_t query[1024];
	uint32_t count = 0;
	uint32_t found = 0;
	PGresult *result = NULL;

	/* build query for database. */
	snprintf((char *)query, 1024, "UPDATE %s SET %s='%d' WHERE %s='%s'", pppd_pgsql_table, pppd_pgsql_column_update, status, pppd_pgsql_column_user, name);

	/* loop through number of query retries. */
	for (count = pppd_pgsql_retry_query; count > 0 ; count--) {

		/* check if query was successfully executed. */
		if ((result = PQexec(*pgsql, (char *)query)) != NULL) {

			/* check if the result is okay. */
			if (PQresultStatus(result) == PGRES_COMMAND_OK) {

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
		pppd__pgsql_error((uint8_t *)PQerrorMessage(*pgsql));

		/* clear memory to avoid leaks. */
		PQclear(result);

		/* return with error and terminate link. */
		return PPPD_SQL_ERROR_QUERY;
	}

	/* clear memory to avoid leaks. */
	PQclear(result);

	/* if no error was found, return zero. */
	return 0;
}

/* this function is the ip up notifier for the ppp daemon. */
void pppd__pgsql_up(void *opaque, int32_t arg) {

	/* some common variables. */
	PGconn *pgsql = NULL;

	/* check if we should execute a script. */
	if (pppd_pgsql_ip_up != NULL) {

		/* execute script. */
		if (pppd__ip_up(username, pppd_pgsql_ip_up) != 0) {

			/* check if we should fail. */
			if (pppd_pgsql_ip_up_fail == 1) {

				/* show the error. */
				error("Plugin %s: Script '%s' returned with non-zero status\n", PLUGIN_NAME_PGSQL, pppd_pgsql_ip_up);

				/* check if status should be updated. */
				if (pppd_pgsql_exclusive     == 1 &&
				    pppd_pgsql_authoritative == 1 &&
				    pppd_pgsql_column_update != NULL) {

					/* check if postgresql connect is working. */
					if (pppd__pgsql_connect(&pgsql) == 0) {

						/* update database. (ignore return code, because what should I do, stop the disconnect?) */
						pppd__pgsql_status(&pgsql, username, 0);

						/* disconnect from pgsql. */
						pppd__pgsql_disconnect(&pgsql);
					}
				}

				/* die bitch die. */
				die(1);
			}
		}
	}
}

/* this function is the ip down notifier for the ppp daemon. */
void pppd__pgsql_down(void *opaque, int32_t arg) {

	/* some common variables. */
	PGconn *pgsql = NULL;

	/* check if we should execute a script. */
	if (pppd_pgsql_ip_down != NULL) {

		/* execute script. */
		if (pppd__ip_down(username, pppd_pgsql_ip_down) != 0) {

			/* check if we should fail. */
			if (pppd_pgsql_ip_down_fail == 1) {

				/* show the error. */
				error("Plugin %s: Script '%s' returned with non-zero status\n", PLUGIN_NAME_PGSQL, pppd_pgsql_ip_up);

				/* die bitch die. */
				die(1);
			}
		}
	}

	/* check if status should be updated. */
	if (pppd_pgsql_exclusive     == 1 &&
	    pppd_pgsql_authoritative == 1 &&
	    pppd_pgsql_column_update != NULL) {

		/* check if postgresql connect is working. */
		if (pppd__pgsql_connect(&pgsql) == 0) {

			/* update database. (ignore return code, because what should I do, stop the disconnect?) */
			pppd__pgsql_status(&pgsql, username, 0);

			/* disconnect from postgresql. */
			pppd__pgsql_disconnect(&pgsql);
		}
	}
}

/* this function check the chap authentication information against a postgresql database. */
int32_t pppd__chap_verify_pgsql(char *name, char *ourname, int id, struct chap_digest_type *digest, unsigned char *challenge, unsigned char *response, char *message, int message_space) {

	/* some common variables. */
	uint8_t secret_name[MAXSECRETLEN];
	int32_t secret_length = 0;
	PGconn *pgsql         = NULL;

	/* check if parameters are complete. */
	if (pppd__pgsql_parameter() == 0) {

		/* check if postgresql connect is working. */
		if (pppd__pgsql_connect(&pgsql) == 0) {

			/* check if postgresql fetching was successful. */
			if (pppd__pgsql_password(&pgsql, (uint8_t *)name, secret_name, &secret_length) == 0) {

				/* check if password decryption was correct. */
				if (pppd__decrypt_password(secret_name, &secret_length, pppd_pgsql_pass_encryption, pppd_pgsql_pass_key) == 0) {

					/* verify discovered secret against the client's response. */
					if (digest->verify_response(id, name, secret_name, secret_length, challenge, response, message, message_space) == 1) {

						/* check if database update was successful. */
						if (pppd__pgsql_status(&pgsql, (uint8_t *)name, 1) == 0) {

							/* store username for ip down configuration. */
							strncpy((char *)username, name, MAXNAMELEN);

							/* disconnect from postgresql. */
							pppd__pgsql_disconnect(&pgsql);

							/* clear the memory with the password, so nobody is able to dump it. */
							memset(secret_name, 0, sizeof(secret_name));

							/* if no error was found, establish link. */
							return 1;
						}
					}
				}
			}

			/* disconnect from postgresql. */
			pppd__pgsql_disconnect(&pgsql);
		}
	}

	/* check if postgresql is not authoritative. */
	if (pppd_pgsql_authoritative == 0) {

		/* get the secret that the peer is supposed to know. */
		if (get_secret(0, name, ourname, (char *)secret_name, &secret_length, 1) == 1) {

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

/* this function check the pap authentication information against a postgresql database. */
int32_t pppd__pap_auth_pgsql(char *user, char *passwd, char **msgp, struct wordlist **paddrs, struct wordlist **popts) {

	/* some common variables. */
	uint8_t secret_name[MAXSECRETLEN];
	int32_t secret_length = 0;
	PGconn *pgsql         = NULL;

	/* check if parameters are complete. */
	if (pppd__pgsql_parameter() == 0) {

		/* check if postgresql connect is working. */
		if (pppd__pgsql_connect(&pgsql) == 0) {

			/* check if postgresql fetching was successful. */
			if (pppd__pgsql_password(&pgsql, (uint8_t *)user, secret_name, &secret_length) == 0) {

				/* check if the password is correct. */
				if (pppd__verify_password((uint8_t *)passwd, secret_name, pppd_pgsql_pass_encryption, pppd_pgsql_pass_key) == 0) {

					/* check if database update was successful. */
					if (pppd__pgsql_status(&pgsql, (uint8_t *)user, 1) == 0) {

						/* store username for ip down configuration. */
						strncpy((char *)username, user, MAXNAMELEN);

						/* disconnect from postgresql. */
						pppd__pgsql_disconnect(&pgsql);

						/* clear the memory with the password, so nobody is able to dump it. */
						memset(secret_name, 0, sizeof(secret_name));

						/* if no error was found, establish link. */
						return 1;
					}
				}
			}

			/* disconnect from postgresql. */
			pppd__pgsql_disconnect(&pgsql);
		}
	}

	/* check if postgresql is not authoritative. */
	if (pppd_pgsql_authoritative == 0) {

		/* return with error and look in pap file. */
		return -1;
	}

	/* clear the memory with the password, so nobody is able to dump it. */
	memset(secret_name, 0, sizeof(secret_name));

	/* return with error and terminate link. */
	return 0;
}

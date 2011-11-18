#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>
#include "openvpn-plugin.h"

# ifdef WITH_DMALLOC
#  include <dmalloc.h>
# endif

#define def_hostname  "127.0.0.1" 
#define def_username  "vpn" 
#define def_password  "vpn" 
#define def_dbname    "ppp" 

MYSQL *conn;
MYSQL_RES *res_set;

/* our context, where we keep our state. */
// struct plugin_context {
//  const char *username;
//  const char *password;
// };

/* given an environmental variable name, search the envp array for its 
   value, returning it if found or NULL otherwise. */
static const char * get_env (const char *name, const char *envp[]) {
 if (envp) {
  int i;
  const int namelen = strlen (name);
  for (i = 0; envp[i]; ++i) {
   if (!strncmp (envp[i], name, namelen)) {
    const char *cp = envp[i] + namelen;
    if (*cp == '=') return cp + 1;
   }
  }
 }
 return NULL;
}

/* check for a-aA-Z0-9 plus some chars */
static int as_mysql_validate(const char *name) {
 if (name == NULL || *name == 0) return -1;
 do {
  if ((*name >= 'a' && *name <= 'z') ||
      (*name >= 'A' && *name <= 'Z') ||
      (*name >= '0' && *name <= '9') ||
       *name == ' ' || *name == '-' ||
       *name == '_' || *name == '.' ||
       *name == ':' || *name == '@') {
      /* God bless the Perl 'unless' keyword */
  } else return -1;
  name++;
 } while (*name != 0);
 return 0;
}

static char *as_mysql_escape_string(MYSQL * const id_sql_server, const char *from) {
 size_t from_len;
 size_t to_len;
 char *to;
 unsigned long tolen;    
 unsigned int t;
 unsigned char t1, t2;
 
 if (from == NULL) return NULL;

 from_len = strlen(from);
 to_len = from_len * 2U + (size_t) 1U;
 if ((to = malloc(to_len + 2U)) == NULL) return NULL;
 t = rand();
 t1 = t & 0xff;
 t2 = (t >> 8) & 0xff;
 to[to_len] = (char) t1;
 to[to_len + 1] = (char) t2;
 /*
  * I really hate giving a buffer without any size to a 3rd party function.
  * The "to" buffer is allocated on the heap, not on the stack, if
  * mysql_real_escape_string() is buggy, the stack shouldn't be already
  * smashed at this point, but data from other malloc can be corrupted and
  * bad things can happen. It make sense to wipe this area as soon as
  * possible instead of doing anything with the heap. We'll end up with
  * a segmentation violation, but without any possible exploit.
  */
#ifdef HAVE_MYSQL_REAL_ESCAPE_STRING
 tolen = mysql_real_escape_string(id_sql_server, to, from, from_len);
#else
 /* MySQL 3.22.x and earlier are obsolete. Better use 3.23.x or 4.x */
 tolen = mysql_escape_string(to, from, from_len);    
#endif
 if (tolen >= to_len || (unsigned char) to[to_len] != t1 || (unsigned char) to[to_len + 1] != t2)
  for (;;) *to++ = 0;
 to[tolen] = 0;
 
 return to;
}

OPENVPN_EXPORT openvpn_plugin_handle_t openvpn_plugin_open_v1 (unsigned int *type_mask, const char *argv[], const char *envp[]) {
    /* load things in this function */
 
    // struct plugin_context *context;
 
    /* allocate our context */
    // context = (struct plugin_context *) calloc (1, sizeof (struct plugin_context));
 
    *type_mask = OPENVPN_PLUGIN_MASK (OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY);
 
    // return (openvpn_plugin_handle_t) context;
}
 
OPENVPN_EXPORT int openvpn_plugin_func_v1 (openvpn_plugin_handle_t handle, const int type, const char *argv[], const char *envp[]) {
    // struct plugin_context *context = (struct plugin_context *) handle;
 
    /* get username/password from envp string array */
    const char *username = get_env ("username", envp);
    const char *password = get_env ("password", envp);

    MYSQL_ROW row;
    char *query = NULL; 
    unsigned int num_fields;

    char *escaped_username = NULL;
    char *escaped_password = NULL;

    /* validate username */
    if (as_mysql_validate(username) != 0) {
        printf("Error: Invalid user!\n");
        return OPENVPN_PLUGIN_FUNC_ERROR;
    }
 
    /* validate password */
    if (as_mysql_validate(password) != 0) {
        printf("Error: Invalid password!\n");
        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* connect */
    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, def_hostname, def_username, def_password, def_dbname, 3306, NULL, 0) == 0){
        printf("Error: %u (%s)\n", mysql_errno (conn), mysql_error (conn));

        mysql_close(conn);
        return OPENVPN_PLUGIN_FUNC_ERROR;
    }
 
    /* escape username */
    if ((escaped_username = as_mysql_escape_string(conn, username)) == NULL) {
        mysql_close(conn);
        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* escape password */
    if ((escaped_password = as_mysql_escape_string(conn, password)) == NULL) {
        mysql_close(conn);
        return OPENVPN_PLUGIN_FUNC_ERROR;
    }
 
    /* build the query */
    char *fmt = "SELECT username FROM login WHERE password='%s' AND username='%s' AND credit > 0 AND account_state = 'ACTIVE'";
    size_t lenq = strlen(escaped_username) + strlen(escaped_password) + strlen(fmt); 
    query = alloca(lenq);
    snprintf(query, lenq, "SELECT username FROM login WHERE password='%s' AND username='%s' AND credit > 0 AND account_state = 'ACTIVE'", escaped_password, escaped_username);

    /* connect and auth */
    if (mysql_query(conn, query) == 0) {
        if ((res_set = mysql_store_result(conn))) {
            if (res_set != NULL) {
                if ((num_fields = mysql_num_fields(res_set))) {
                    if ((row = mysql_fetch_row(res_set))) {
                        if (username && !strcmp(username, row[0])) {
                            mysql_close(conn);
                            return OPENVPN_PLUGIN_FUNC_SUCCESS;
                        }
         
                        else { 
                            printf("Error: Invalid password!\n");
                            mysql_close(conn);
                            return OPENVPN_PLUGIN_FUNC_ERROR; 
                        }
                    } else { 
                        printf("Error: Unable to fetch row!\n");
                        mysql_close(conn);
                        return OPENVPN_PLUGIN_FUNC_ERROR; 
                    }
                } else {
                    printf("Error: Unable to numfields!\n");
                    mysql_close(conn);
                    return OPENVPN_PLUGIN_FUNC_ERROR;
                }
            } else {
                printf("Error: Null results!\n");
                mysql_close(conn);
                return OPENVPN_PLUGIN_FUNC_ERROR;
            }
        } else {
            printf("Error: Unable to store results!\n");
            mysql_close(conn);
            return OPENVPN_PLUGIN_FUNC_ERROR;
        }
    } else {
        printf("Error: Unable to query!\n");
        mysql_close(conn);
        return OPENVPN_PLUGIN_FUNC_ERROR; 
    }
 
    mysql_close(conn);
    return OPENVPN_PLUGIN_FUNC_ERROR;
}
 
OPENVPN_EXPORT void openvpn_plugin_close_v1 (openvpn_plugin_handle_t handle) {
 /* free things in this function */
 
 // struct plugin_context *context = (struct plugin_context *) handle;
 // free (context);
}

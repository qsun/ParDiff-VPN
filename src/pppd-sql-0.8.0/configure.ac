# the autoconf initilization.
AC_INIT(pppd-sql, 0.8.0, [mbroemme@plusserver.de], [pppd-sql])

# detect the canonical host and target build environment.
AC_CANONICAL_SYSTEM

# initialize autoconf and automake system.
AM_INIT_AUTOMAKE([no-dependencies])
AC_CONFIG_HEADERS([config.h:config.h.in])

# notices.
AC_PREREQ(2.53)
AC_REVISION($Revision: 1.6 $)

# checking for required binaries.
AC_PATH_PROG([rmpath], [rm], [no])
if test "$rmpath" = "no"; then
	AC_MSG_ERROR([*** rm binary is required, install coreutils files])
fi
AC_PATH_PROG([sedpath], [sed], [no])
if test "$sedpath" = "no"; then
	AC_MSG_ERROR([*** sed binary is required, install sed files])
fi

# checking if static is enabled, which is impossible to use.
if test "$enable_static" = "yes"; then
	AC_MSG_ERROR([*** static linking is not possible, dlopen() is required by module loading])
fi

# checking for programs.
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PROG_CC

# checking for pppd binary.
AC_PATH_PROG([pppdpath], [pppd], [no])
if test "$pppdpath" = "no"; then
	AC_MSG_ERROR([*** pppd binary is required, install ppp server files])
else

	# checking for pppd version.
	AC_MSG_CHECKING([for pppd version])
	PPPD_VERSION="$($pppdpath --version 2>&1 | $sedpath 's/.* //')"
	case "$PPPD_VERSION" in
		0.*|1.*|2.0.*|2.1.*|2.2.*|2.3.1|2.3.1.*|2.3.2*|2.3.3*|2.3.4*|2.3.5*|2.3.6*|2.3.7*|2.3.8*|2.3.9*)
			AC_MSG_ERROR([*** pppd version below 2.3.10 has no plugin support])
		;;
		*)
			AC_MSG_RESULT([yes (found $PPPD_VERSION)])
		;;
	esac
fi

# checking for ppp include.
AC_CHECK_HEADER([pppd/pppd.h], [], [AC_MSG_ERROR([*** pppd.h is required, install ppp header files])])

# checking for plugin path.
AC_ARG_WITH([plugin_path], AS_HELP_STRING([--with-plugin-path=<path>], [the PPP plugin directory for installation]))
if test "$withval" = "no" -o "$withval" = "yes"; then
	if test "$with_plugin_path" = "yes"; then
		AC_MSG_WARN([*** configure option '--with-plugin-path' but no path was specified, using default])
		libdir="$libdir/pppd/$PPPD_VERSION"
	else
		libdir="$libdir/pppd/$PPPD_VERSION"
	fi
fi
if test "$withval" != "no" -a "$withval" != "yes" -a -n "$withval"; then
	case "$with_plugin_path" in
		/*)
			libdir="$with_plugin_path"
		;;
		*)
			AC_MSG_ERROR([*** configure option '--with-plugin-path=$with_plugin_path' but path must be absolute])
		;;
	esac
fi

# adding new command line switch for enabling mysql.
AC_ARG_ENABLE([mysql], [AS_HELP_STRING([--enable-mysql], [enable mysql plugin [default=autodetect]])], [enable_mysql=$enableval])

# checking if mysql was enabled and must be available.
if test "$enable_mysql" = "yes"; then

	# checking for mysql library.
	AC_CHECK_HEADER([mysql/mysql.h], [], [AC_MSG_ERROR([*** mysql.h is required, install mysql header files])])
	AC_PATH_PROG([mysql_config], [mysql_config], [AC_MSG_ERROR([*** mysql_config is required, install mysql development files])])
fi

# checking openssl library.
AC_CHECK_HEADER([openssl/des.h], [], [AC_MSG_ERROR([*** des.h is required, install openssl header files])])
AC_CHECK_HEADER([openssl/evp.h], [], [AC_MSG_ERROR([*** evp.h is required, install openssl header files])])
AC_CHECK_LIB([crypto], [EVP_CIPHER_CTX_init], [], [AC_MSG_ERROR([*** EVP_CIPHER_CTX_init is required, install openssl library files])])
AC_CHECK_LIB([crypto], [EVP_MD_CTX_init], [], [AC_MSG_ERROR([*** EVP_MD_CTX_init is required, install openssl library files])])
AC_CHECK_LIB([crypto], [DES_crypt], [], [AC_MSG_ERROR([*** DES_crypt is required, install openssl library files])])

# checking if mysql should be autodetected.
if test -z "$enable_mysql"; then

	# checking for mysql library.
	AC_CHECK_HEADER([mysql/mysql.h])
	AC_PATH_PROG([mysql_config], [mysql_config])
fi

# checking for mysql environment.
if test "$ac_cv_header_mysql_mysql_h" = "yes" -a -n "$mysql_config"; then
	MYSQL_CFLAGS="$(mysql_config --cflags)"
	MYSQL_LDFLAGS="$(mysql_config --libs)"
	AC_SUBST(MYSQL_CFLAGS)
	AC_SUBST(MYSQL_LDFLAGS)

	# define the mysql name and version.
	AC_DEFINE_UNQUOTED(PLUGIN_NAME_MYSQL, "mysql", [Plugin name as Prefix.])
	AC_DEFINE_UNQUOTED(PLUGIN_VERSION_MYSQL, "$(mysql_config --version)", [Plugin version for MySQL.])
fi

# define automake rule for compiling.
AM_CONDITIONAL([HAVE_MYSQL], [test "$ac_cv_header_mysql_mysql_h" = "yes"])

# adding new command line switch for enabling postgresql.
AC_ARG_ENABLE([postgresql], [AS_HELP_STRING([--enable-postgresql], [enable postgresql plugin [default=autodetect]])], [enable_postgresql=$enableval])

# checking if mysql was enabled and must be available.
if test "$enable_postgresql" = "yes"; then

	# checking for postgresql library.
	AC_CHECK_HEADER([libpq-fe.h], [], [AC_MSG_ERROR([*** libpq-fe.h is required, install postgresql header files])])
	AC_PATH_PROG([pg_config], [pg_config], [AC_MSG_ERROR([*** pg_config is required, install postgresql development files])])
fi

# checking if mysql should be autodetected.
if test -z "$enable_postgresql"; then

	# checking for postgresql library.
	AC_CHECK_HEADER([libpq-fe.h])
	AC_PATH_PROG([pg_config], [pg_config])
fi

# checking for postgresql environment.
if test "$ac_cv_header_libpq_fe_h" = "yes" -a -n "$pg_config"; then
	PGSQL_CFLAGS="$(pg_config --cflags)"
	PGSQL_LDFLAGS="-L$(pg_config --libdir) -lpq"
	AC_SUBST(PGSQL_CFLAGS)
	AC_SUBST(PGSQL_LDFLAGS)

	# define the postgresql name and version.
	AC_DEFINE_UNQUOTED(PLUGIN_NAME_PGSQL, "pgsql", [Plugin name as Prefix.])
	AC_DEFINE_UNQUOTED(PLUGIN_VERSION_PGSQL, "$(pg_config --version | $sedpath 's/.* //')", [Plugin version for PostgreSQL.])
fi

# define automake rule for compiling.
AM_CONDITIONAL([HAVE_PGSQL], [test "$ac_cv_header_libpq_fe_h" = "yes"])

# check if no database backends are available, that doesn't make sense for a sql plugin. :)
if test -z "$ac_cv_header_mysql_mysql_h" -a \
        -z "$ac_cv_header_libpq_fe_h"; then
	AC_MSG_ERROR([*** no database backend found, install development and library files of at least one])
fi

# creating files.
AC_OUTPUT([
Makefile
doc/Makefile
src/Makefile
])

# show user the configuration summary.
echo ""
echo "'$PACKAGE' is configured with the following database backends. Please"
echo "verify that this configuration matches your expectations."
echo ""
if test "$ac_cv_header_mysql_mysql_h" = "yes" -a -n "$mysql_config"; then
echo "  * mysql"
fi
if test "$ac_cv_header_libpq_fe_h" = "yes" -a -n "$pg_config"; then
echo "  * postgresql"
fi
echo ""

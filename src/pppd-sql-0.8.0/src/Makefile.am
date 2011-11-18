# minimum required automake 1.6
AUTOMAKE_OPTIONS	= 1.6

# library information and headers which should not be installed.
lib_LTLIBRARIES		=
if HAVE_MYSQL
lib_LTLIBRARIES		+= mysql.la
endif
if HAVE_PGSQL
lib_LTLIBRARIES		+= pgsql.la
endif

# headers which are only for internal use.
noinst_HEADERS		= auth-mysql.h auth-pgsql.h plugin.h plugin-mysql.h plugin-pgsql.h str.h

if HAVE_MYSQL
# sources to compile.
mysql_la_SOURCES	= auth-mysql.c \
			  plugin.c \
			  plugin-mysql.c \
			  str.c
# compile flags.
mysql_la_CFLAGS		= @MYSQL_CFLAGS@

# linker options.
mysql_la_LDFLAGS	= @MYSQL_LDFLAGS@ \
			  -module \
			  -avoid-version
endif

if HAVE_PGSQL
# sources to compile.
pgsql_la_SOURCES	= auth-pgsql.c \
			  plugin.c \
			  plugin-pgsql.c \
			  str.c

# compile flags.
pgsql_la_CFLAGS		= @PGSQL_CFLAGS@

# linker options.
pgsql_la_LDFLAGS	= @PGSQL_LDFLAGS@ \
			  -module \
			  -avoid-version
endif

# avoid installation of .la files.
install-exec-hook:
if HAVE_MYSQL
	$(rmpath) ${DESTDIR}${libdir}/mysql.la
endif
if HAVE_PGSQL
	$(rmpath) ${DESTDIR}${libdir}/pgsql.la
endif

# remove modules on uninstallation.
uninstall-hook:
if HAVE_MYSQL
	$(rmpath) -f ${DESTDIR}${libdir}/mysql.so
endif
if HAVE_PGSQL
	$(rmpath) -f ${DESTDIR}${libdir}/pgsql.so
endif

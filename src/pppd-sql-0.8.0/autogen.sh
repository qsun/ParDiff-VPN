#!/bin/sh
echo "Generating build information using aclocal, autoheader, automake and autoconf"
echo "This may take a while ..."

# touch the timestamps on all the files since cvs or svn messes them up.
directory=`dirname $0`
touch $directory/configure.ac

# regenerate configuration files.
libtoolize --copy
aclocal
autoheader
automake --foreign --add-missing --copy
autoconf

# configure created and ready.
echo "Now you are ready to run ./configure"

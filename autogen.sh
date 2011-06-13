#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="Pioneers"

REQUIRED_AUTOCONF_VERSION="2.61"
REQUIRED_AUTOMAKE_VERSION="1.9"
REQUIRED_INTLTOOL_VERSION="0.35"

(test -f $srcdir/configure.ac \
  && test -f $srcdir/ChangeLog \
  && test -d $srcdir/client \
  && test -d $srcdir/server) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

# Create acinclude.m4 from the extra macro
cp macros/type_socklen_t.m4 acinclude.m4

which gnome-autogen.sh || {
    echo "gnome-common not found, using the included version"
    . $srcdir/macros/gnome-autogen.sh
    exit 0
}
. gnome-autogen.sh

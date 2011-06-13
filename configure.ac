# Pioneers - Implementation of the excellent Settlers of Catan board game.
#   Go buy a copy.
#
# Copyright (C) 1999 Dave Cole
# Copyright (C) 2003-2007 Bas Wijnen <shevek@fmf.nl>
# Copyright (C) 2004-2011 Roland Clobus <rclobus@bigfoot.com>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AC_PREREQ(2.61)
AC_INIT([pioneers],[0.12.5],[pio-develop@lists.sourceforge.net])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_SRCDIR([client])
AC_CONFIG_HEADERS([config.h])
 
PROTOCOL_VERSION=0.12
META_PROTOCOL_VERSION=1.3
PIONEERS_DEFAULT_GAME_PORT=5556
PIONEERS_DEFAULT_GAME_HOST=localhost
PIONEERS_DEFAULT_ADMIN_PORT=5555
PIONEERS_DEFAULT_META_PORT=5557
PIONEERS_DEFAULT_META_SERVER=pioneers.debian.net

GLIB_REQUIRED_VERSION=2.16
GTK_REQUIRED_VERSION=2.20
GTK_OPTIMAL_VERSION=2.24

AM_MAINTAINER_MODE

AC_PROG_LIBTOOL
AC_PATH_PROG(ECHO, echo)

AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_MKDIR_P
AC_HEADER_STDC

if test $USE_MAINTAINER_MODE = yes; then
	GOB2_CHECK([[2.0.0]])
	pioneers_warnings=yes;
	pioneers_debug=yes;
	pioneers_deprecationChecks=yes;
else
	pioneers_warnings=no;
	pioneers_debug=no;
	pioneers_deprecationChecks=no;
fi

# Try to find a suitable renderer for the svg images
AC_PATH_PROG(svg_renderer_path, rsvg)
AC_SUBST(whitespace_trick, [" "])
if test x$svg_renderer_path != x; then
	AC_SUBST(svg_renderer_width, ["--width \$(whitespace_trick)"])
	AC_SUBST(svg_renderer_height, ["\$(whitespace_trick) --height \$(whitespace_trick)"])
else
AC_PATH_PROG(svg_renderer_path, convert)
if test x$svg_renderer_path != x; then
	AC_SUBST(svg_renderer_width, ["-background \"\#000001\" -transparent \"\#000001\" -resize \$(whitespace_trick)"])
	AC_SUBST(svg_renderer_height, ["x"])
else 
	# Add other SVG rendering programs here
	# Don't let configure fail, in the distributed tarballs is already
	# a current .png file
	AC_SUBST(svg_renderer_path, [false])
fi
fi

# Is netpbm installed (should be needed only for maintainer builds)
# netpbm will generate the icon for the Windows executables
AC_PATH_PROG(pngtopnm, pngtopnm)
if test "$pngtopnm" = ""; then
	# Don't let configure fail, in the distributed tarballs is already
	# a current .png file
	AC_SUBST(pngtopnm, [false])
fi

AC_ARG_ENABLE([warnings], AS_HELP_STRING([--enable-warnings],
	[Compile with check for compiler warnings (gcc-only).]),
[case "${enableval}" in
  full) pioneers_warnings=full;;
  yes)  pioneers_warnings=yes ;;
  "")   pioneers_warnings=yes ;;
  *)    pioneers_warnings=no  ;;
esac])

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug],
	[Enable debug information.]),
[case "${enableval}" in
  yes)  pioneers_debug=yes ;;
  "")   pioneers_debug=yes ;;
  *)    pioneers_debug=no  ;;
esac])

AC_ARG_ENABLE([deprecation-checks],
	AS_HELP_STRING([--enable-deprecation-checks],
		[Enable strict deprecation checks.]),
[case "${enableval}" in
  yes)  pioneers_deprecationChecks=yes ;;
  "")   pioneers_deprecationChecks=yes ;;
  *)    pioneers_deprecationChecks=no  ;;
esac])

## The warnings are in the same order as in 'man gcc'
if test "x$GCC" = xyes; then
	if test "$pioneers_warnings" = yes -o "$pioneers_warnings" = full; then
		AC_SUBST(WARNINGS, "-Wall")
		AC_SUBST(WARNINGS, "$WARNINGS -W")
		AC_SUBST(WARNINGS, "$WARNINGS -Wpointer-arith")
		AC_SUBST(WARNINGS, "$WARNINGS -Wcast-qual")
		AC_SUBST(WARNINGS, "$WARNINGS -Wwrite-strings")
		AC_SUBST(WARNINGS, "$WARNINGS -Wno-sign-compare")
		AC_SUBST(WARNINGS, "$WARNINGS -Waggregate-return")
		AC_SUBST(WARNINGS, "$WARNINGS -Wstrict-prototypes")
		AC_SUBST(WARNINGS, "$WARNINGS -Wmissing-prototypes")
		AC_SUBST(WARNINGS, "$WARNINGS -Wmissing-declarations")
		AC_SUBST(WARNINGS, "$WARNINGS -Wredundant-decls")
		AC_SUBST(WARNINGS, "$WARNINGS -Wnested-externs")
		AC_SUBST(WARNINGS, "$WARNINGS -O")
	fi
	if test "$pioneers_warnings" = full; then
		flags="-Wfloat-equal"
		flags="$flags -Wdeclaration-after-statement"
		flags="$flags -Wundef"
		flags="$flags -Wendif-labels"
		flags="$flags -Wshadow"
		flags="$flags -Wbad-function-cast"
		flags="$flags -Wconversion"
		flags="$flags -Wold-style-definition"
		flags="$flags -Wunreachable-code"
		flags="$flags -pedantic"

		# This for loop comes from gnome-compiler-flags.m4
		for option in $flags; do
			SAVE_CFLAGS="$CFLAGS"
			CFLAGS="$CFLAGS $option"
			AC_MSG_CHECKING([whether gcc understands $option])
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[has_option=yes],[has_option=no])
			CFLAGS="$SAVE_CFLAGS"
			AC_MSG_RESULT($has_option)
			if test $has_option = yes; then
				AC_SUBST(WARNINGS, "$WARNINGS $option")
			fi
			unset has_option
			unset SAVE_CFLAGS
		done
		unset option
		unset flags
	fi
fi
AC_SUBST(WARNINGS)

if test "$pioneers_debug" = yes; then
	AC_SUBST(DEBUGGING, "-ggdb3")
fi
AC_SUBST(DEBUGGING)

if test "$pioneers_deprecationChecks" = yes; then
	AC_SUBST(GLIB_DEPRECATION, "-DG_DISABLE_DEPRECATED")
	AC_SUBST(GLIB_DEPRECATION, "$GLIB_DEPRECATION -DG_DISABLE_SINGLE_INCLUDES")
	AC_SUBST(GTK_DEPRECATION, "-DGDK_DISABLE_DEPRECATED")
	AC_SUBST(GTK_DEPRECATION, "$GTK_DEPRECATION -DGTK_DISABLE_DEPRECATED")
	AC_SUBST(GTK_DEPRECATION, "$GTK_DEPRECATION -DGNOME_DISABLE_DEPRECATED")
	AC_SUBST(GTK_DEPRECATION, "$GTK_DEPRECATION -DGDK_DISABLE_SINGLE_INCLUDES")
	AC_SUBST(GTK_DEPRECATION, "$GTK_DEPRECATION -DGTK_DISABLE_SINGLE_INCLUDES")
fi
AC_SUBST(GLIB_DEPRECATION)
AC_SUBST(GTK_DEPRECATION)

if test "$with_help" = no; then
	pioneers_help="no, disabled in configure"
	have_scrollkeeper=no
else
	## Scrollkeeper dependency test taken from gnome-games 2.6.2
	## Begin tests for scrollkeeper
	# SCROLLKEEPER_REQUIRED is never used?
	SCROLLKEEPER_REQUIRED=0.3.8
	AC_SUBST(SCROLLKEEPER_REQUIRED)

	AC_PATH_PROG(SCROLLKEEPER_CONFIG, scrollkeeper-config,no)
	if test x$SCROLLKEEPER_CONFIG = xno; then 
		have_scrollkeeper=no
		pioneers_help="no, scrollkeeper not found"
	else
		have_scrollkeeper=yes
		pioneers_help="yes"
	fi
fi

# glib is always needed
PKG_CHECK_MODULES(GLIB2, glib-2.0 >= $GLIB_REQUIRED_VERSION)
PKG_CHECK_MODULES(GOBJECT2, gobject-2.0 >= $GLIB_REQUIRED_VERSION)

# Gtk+ support
if test x$with_gtk = xno; then
	have_gtk2="no, disabled in configure"
else
	PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= $GTK_REQUIRED_VERSION,
	have_gtk2=yes, 
	[PKG_CHECK_EXISTS(gtk+-2.0,
		[have_gtk2="no, Gtk+ version too old"],
		[have_gtk2="no, Gtk+ not installed"])
	AC_MSG_RESULT($have_gtk2)
	])
fi
AM_CONDITIONAL(HAVE_GTK2, [test "$have_gtk2" = "yes"])
if test "$have_gtk2" = "yes"; then
	PKG_CHECK_MODULES(GTK_OPTIMAL_VERSION, gtk+-2.0 >= $GTK_OPTIMAL_VERSION,
		[],
		AC_DEFINE(HAVE_OLD_GTK, [1],
			[Defined if an older version of GTK is available]))
fi

# Gtk ScrollKeeper -> Test     Build
#                     libgnome Help
#  N       X           N        N
#  Y       N           N        N
#  Y       Y           Y        if have libgnome
# libgnome is only needed for help
if test "$have_gtk2" = "yes"; then
	test_libgnome=$have_scrollkeeper;
else
	test_libgnome=no;
	with_ms_icons=no;
fi

have_graphical=$have_gtk2;
if test "$test_libgnome" = "yes"; then
	# libgnome-2.0 support
	PKG_CHECK_MODULES(GNOME2, libgnome-2.0,
		[have_gnome="yes"],
		[have_gnome="no, libgnome-2.0 not installed"
		AC_MSG_RESULT($have_gnome)])
	if test "$have_gnome" = "yes"; then
		AC_DEFINE(HAVE_LIBGNOME, 1,
			[Defined if libgnome is present and needed])
	fi
	if test "$have_scrollkeeper" = "yes"; then
		# Turn off the help if libgnome not installed
		have_scrollkeeper=$have_gnome;
	fi
fi
AM_CONDITIONAL(HAVE_GNOME, [test "$have_graphical" = "yes"])

AM_CONDITIONAL(HAVE_SCROLLKEEPER, [test "$have_scrollkeeper" = "yes"])

# Avahi support
if test x$with_avahi = xno; then
	have_avahi="no, disabled in configure"
else
	PKG_CHECK_MODULES(AVAHI_CLIENT, avahi-client,
		[PKG_CHECK_MODULES(AVAHI_GLIB, avahi-glib,
			[have_avahi="yes"
			AC_DEFINE(HAVE_AVAHI, [1], [Define if AVAHI available])
			AC_DEFINE(AVAHI_ANNOUNCE_NAME, ["_pioneers._tcp"], [The name of the Avahi service])],
			[have_avahi="no, avahi-glib is missing"
			AC_MSG_RESULT($have_avahi)])],
		[have_avahi="no, avahi-client is missing"
		AC_MSG_RESULT($have_avahi)])

fi

AC_ARG_ENABLE([admin-gtk], AS_HELP_STRING([--enable-admin-gtk],
	[Turn on (unstable) network administration support.]),
[case "${enableval}" in
  yes) admin_gtk_support=yes  ;;
  "")  admin_gtk_support=yes  ;;
  *)   admin_gtk_support=no ;;
esac], [admin_gtk_support=no])
AM_CONDITIONAL(ADMIN_GTK_SUPPORT, [test x$admin_gtk_support = xyes])

AC_ARG_ENABLE([protocol],
	AS_HELP_STRING([--enable-protocol],
	[Specify the network protocol (IPv4/unspecified)]),
[case "${enableval}" in
  IPv4)        pioneers_network_protocol=AF_INET;
               avahi_network_protocol=AVAHI_PROTO_INET;;
  *)           pioneers_network_protocol=AF_UNSPEC;
               avahi_network_protocol=AVAHI_PROTO_UNSPEC;;
esac], [pioneers_network_protocol=AF_UNSPEC; avahi_network_protocol=AVAHI_PROTO_UNSPEC])
AC_DEFINE_UNQUOTED([NETWORK_PROTOCOL],[$pioneers_network_protocol],
	[The network protocol value])
AC_DEFINE_UNQUOTED([AVAHI_NETWORK_PROTOCOL],[$avahi_network_protocol],
	[The Avahi network protocol value])

AC_CHECK_HEADERS([netdb.h fcntl.h netinet/in.h sys/socket.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([syslog.h],
	[pioneers_have_syslog=yes;],
	[pioneers_have_syslog=no;])
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

AC_C_CONST

# Functions
AC_FUNC_FORK
AC_FUNC_SELECT_ARGTYPES

# Mathematics
AC_CHECK_FUNC(rint,
	AC_DEFINE(HAVE_RINT, 1, [Define to 1 if you have the rint function.]),
	AC_CHECK_LIB(m, rint, 
		[ AC_DEFINE(HAVE_RINT) LIBS="$LIBS -lm"]))
AC_CHECK_FUNCS([sqrt])
# String functions
AC_CHECK_FUNCS([strchr strspn strstr strcspn])
AC_CHECK_FUNCS([memmove memset])
# Network and I/O functions
AC_CHECK_FUNCS([gethostname gethostbyname select socket])
getsockopt_arg3="void *";
# getaddrinfo and friends
AC_CHECK_FUNCS([getaddrinfo gai_strerror freeaddrinfo], 
	AC_DEFINE_UNQUOTED(HAVE_GETADDRINFO_ET_AL, [1],
		[Defined when getaddrinfo. gai_strerror and freeaddrinfo are present]))

# The Windows ports (Cygwin and MinGW) are client-only
pioneers_is_mingw_port=no;
case $host in
	*-*-cygwin*)
		pioneers_is_windows_port=yes;;
	*-*-mingw*)
		pioneers_is_windows_port=yes;
		pioneers_is_mingw_port=yes;;
	*)
		pioneers_is_windows_port=no;;
esac

# Can a non-blocking socket be created?
pioneers_have_non_blocking_sockets=no;
AC_CHECK_FUNCS([fcntl],[pioneers_have_non_blocking_sockets=yes])
if test "$pioneers_have_non_blocking_sockets" = "no"; then
	# Only check for ws2tcpip now,
	# because it will cause problems under Cygwin
	AC_CHECK_HEADERS([ws2tcpip.h],
		[pioneers_have_non_blocking_sockets=yes;
		 getsockopt_arg3="char *";
		])
fi
AC_DEFINE_UNQUOTED(GETSOCKOPT_ARG3, $getsockopt_arg3, [The type of the third argument of getsockopt])

# Functions needed for the hack to override the language
AC_CHECK_FUNCS([setlocale])

# Data types
AC_STRUCT_TM
AC_TYPE_PID_T
AC_TYPE_SIZE_T
# Check if socklen_t is present
TYPE_SOCKLEN_T

# Defines, accessible for all source files
AC_DEFINE_UNQUOTED(PROTOCOL_VERSION, "$PROTOCOL_VERSION",
		   [Protocol version used by the program])
AC_DEFINE_UNQUOTED(META_PROTOCOL_VERSION, "$META_PROTOCOL_VERSION",
		   [Protocol version used by the meta server])
AC_DEFINE_UNQUOTED(PIONEERS_DEFAULT_GAME_PORT, "$PIONEERS_DEFAULT_GAME_PORT",
		   [The default port for a new game])
AC_DEFINE_UNQUOTED(PIONEERS_DEFAULT_GAME_HOST, "$PIONEERS_DEFAULT_GAME_HOST",
		   [The default host for a new game])
AC_DEFINE_UNQUOTED(PIONEERS_DEFAULT_ADMIN_PORT, "$PIONEERS_DEFAULT_ADMIN_PORT",
		   [The default port for the admin interface])
AC_DEFINE_UNQUOTED(PIONEERS_DEFAULT_META_PORT, "$PIONEERS_DEFAULT_META_PORT",
		   [The port for the meta server])
AC_DEFINE_UNQUOTED(PIONEERS_DEFAULT_META_SERVER, 
		   "$PIONEERS_DEFAULT_META_SERVER", [The default meta server])

## internationalization support
IT_PROG_INTLTOOL([0.35])

GETTEXT_PACKAGE=pioneers
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [The gettext package name])
AM_GLIB_GNU_GETTEXT

if test $pioneers_is_mingw_port = yes; then
	# The check for WSACleanup in ws2_32 needs an include of ws2tcpip.h
	# This is not possible with the AC_CHECK_LIB macro
	# AC_CHECK_LIB(ws2_32, WSACleanup)
	# Just add ws2_32 to the list of libraries
	AC_SUBST(LIBS, "-lws2_32 $LIBS")
	AC_SUBST(AM_CFLAGS, "-mms-bitfields $AM_CFLAGS")
	AC_SUBST(AM_CFLAGS, "$AM_CFLAGS -DWIN32_LEAN_AND_MEAN")
	# No console window for the graphical applications
	AC_SUBST(GTK2_LIBS, "$GTK2_LIBS -mwindows")
	# Don't use bin, lib and share subdirectories
	datadir='${prefix}'
	bindir='${prefix}'
	libdir='${prefix}'
	pioneers_datadir=.
	pioneers_themedir=$datadir/themes
	pioneers_themedir_embed=themes
	pioneers_localedir=locale
	DATADIRNAME=.
else
	pioneers_datadir=$datadir
	pioneers_themedir=$datadir/games/pioneers/themes
	pioneers_themedir_embed=$pioneers_themedir
	pioneers_localedir=$datadir/locale
fi
AC_SUBST(pioneers_datadir)
AC_SUBST(pioneers_themedir)
AC_SUBST(pioneers_themedir_embed)
AC_SUBST(pioneers_localedir)

# All checks are completed.
# Determine which executables cannot be built
pioneers_build_client_ai=yes;
pioneers_build_client_gtk=yes;
pioneers_build_editor=yes;
pioneers_build_server_console=yes;
pioneers_build_server_gtk=yes;
pioneers_build_metaserver=yes;

if test "$pioneers_have_syslog" = "no"; then
	pioneers_build_metaserver=no;
fi
if test "$have_graphical" != "yes"; then
	pioneers_build_client_gtk=$have_graphical;
	pioneers_build_editor=$have_graphical;
	pioneers_build_server_gtk=$have_graphical;
fi
if test "$pioneers_have_non_blocking_sockets" = "no"; then
	pioneers_build_client_ai=no;
	pioneers_build_client_gtk=no;
	pioneers_build_server_console=no;
	pioneers_build_server_gtk=no;
	pioneers_build_metaserver=no;
fi

# The server functionality is not ported to MinGW yet
if test "$pioneers_is_mingw_port" = "yes"; then
	pioneers_build_server_console="no, not implemented for MinGW";
	pioneers_build_server_gtk="no, not implemented for MinGW";
fi
# The metaserver functionality is not ported to MS Windows
if test "$pioneers_is_windows_port" = "yes"; then
	pioneers_build_metaserver="no, not implemented for MS Windows";
fi

# Construct a nice text about Microsoft Windows icons
if test $USE_MAINTAINER_MODE = yes; then
	if test "$with_ms_icons" = "no"; then
		if test $pioneers_is_windows_port = yes; then
			AC_MSG_ERROR([Icons cannot be disabled in maintainermode in MS Windows])
		fi
		pioneers_ms_icons="Disabled, make dist is disabled too"
	else
		if test "$pngtopnm" = "false"; then
			AC_MSG_ERROR([Icons cannot be generated, rerun configure with --without-ms-icons or install netpbm])
		fi
		pioneers_ms_icons="Icons generated"
	fi
else
	if test $pioneers_is_windows_port = yes; then
		pioneers_ms_icons="Icons are used"
	else
		pioneers_ms_icons="Not needed"
	fi
fi

AM_CONDITIONAL(CREATE_WINDOWS_ICON, [test $USE_MAINTAINER_MODE = yes -a "$with_ms_icons" != "no"])
AM_CONDITIONAL(USE_WINDOWS_ICON, [test $pioneers_is_windows_port = yes])

AM_CONDITIONAL(BUILD_CLIENT, [test "$pioneers_build_client_gtk" = "yes" -o "$pioneers_build_client_ai" = yes])
AM_CONDITIONAL(BUILD_EDITOR, [test "$pioneers_build_editor" = "yes"])
AM_CONDITIONAL(BUILD_SERVER, [test "$pioneers_build_server_gtk" = "yes" -o "$pioneers_build_server_console" = "yes"])
AM_CONDITIONAL(BUILD_META_SERVER, [test "$pioneers_build_metaserver" = "yes"])
AM_CONDITIONAL(IS_MINGW_PORT, [test "$pioneers_is_mingw_port" = "yes"])

if test "$have_scrollkeeper" = "yes"; then
	AC_DEFINE(HAVE_HELP, 1,
		[Defined when online help is present])
	pioneers_help="yes, scrollkeeper";
fi

AC_CONFIG_FILES([ \
	Makefile \
	pioneers.spec \
	pioneers.nsi \
	po/Makefile.in \
	client/help/C/Makefile \
	])

AC_OUTPUT

AC_MSG_NOTICE([
$PACKAGE v$VERSION configuration:

        Source code location:              ${srcdir}
        Install location:                  ${prefix}
        Compiler:                          ${CC}
        Build graphical client             $pioneers_build_client_gtk
        Build computer player              $pioneers_build_client_ai
        Build game editor                  $pioneers_build_editor
        Build graphical server             $pioneers_build_server_gtk
        Build console server               $pioneers_build_server_console
        Build metaserver                   $pioneers_build_metaserver
        Build help                         $pioneers_help
        AVAHI support                      $have_avahi
    Developers only:
        Use compiler warnings              $pioneers_warnings
        Add debug information              $pioneers_debug
        Enable deprecation checks          $pioneers_deprecationChecks
   Maintainers only:
        Microsoft Windows icons            $pioneers_ms_icons
])

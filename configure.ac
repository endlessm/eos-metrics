dnl Process this file with autoconf to produce configure

dnl Copyright 2013, 2014, 2015 Endless Mobile, Inc.

dnl This file is part of eos-metrics.
dnl
dnl eos-metrics is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation, either version 2.1 of the License, or
dnl (at your option) any later version.
dnl
dnl eos-metrics is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with eos-metrics.  If not, see
dnl <http://www.gnu.org/licenses/>.

dnl ------------
dnl configure.ac
dnl ------------
dnl Please keep this file well-commented. Autotools involve a lot of magical
dnl incantations, and it is easy to mess things up if you don't know what you
dnl are doing.

# Version numbers
# ---------------
# API name and version for pkg-config; increment when breaking API/ABI.
# This version number is also the major version number.
m4_define([_EMTR_API_VERSION_MACRO], [0])

# Minor and micro versions: increment micro version when making a release. Minor
# version is even for a stable release and odd for a development release.
# When making any release, if the API changes, set the interface age to 0.
m4_define([_EMTR_MINOR_VERSION_MACRO], [5])
m4_define([_EMTR_MICRO_VERSION_MACRO], [0])
m4_define([_EMTR_INTERFACE_AGE_MACRO], [0])

# Full version, for use in AC_INIT
m4_define([_EMTR_VERSION_MACRO],
    [_EMTR_API_VERSION_MACRO._EMTR_MINOR_VERSION_MACRO._EMTR_MICRO_VERSION_MACRO])

# Initialization
# --------------
# Initialize Autoconf: package name, version, bug report address, tarball name,
# website
AC_INIT([EOS Metrics Kit], [_EMTR_VERSION_MACRO],
    [], [eos-metrics], [http://endlessm.com])
# Verify that the source directory can be found
AC_CONFIG_SRCDIR([eosmetrics/eosmetrics.h])
# Initialize Automake: enable all warnings and do not insist on GNU standards.
# no-portability suppresses warnings about syntax specific to GNU make.
# parallel-tests specifies that we use the new parallel-running test harness.
# Unlike serial-tests, this option is accepted by Automake 1.11.
# subdir-objects ensures forward compatibility with Automake 2.0 and can be
# removed when this behavior becomes the default.
# tar-ustar is required because there may be files whose entire paths exceed
# 99 characters.
AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign 1.11 parallel-tests
    subdir-objects tar-ustar])
# Avoid spewing garbage over the terminal ('make V=1' to see the garbage)
AM_SILENT_RULES([yes])
# Keep Autotools macros local to this source tree
AC_CONFIG_MACRO_DIR([m4])

AC_CACHE_SAVE

# Variables to define
# -------------------
# Version numbers and macros
EMTR_API_VERSION=_EMTR_API_VERSION_MACRO
AC_SUBST(EMTR_API_VERSION)
EMTR_API_NAME="eosmetrics-$EMTR_API_VERSION"
AC_SUBST(EMTR_API_NAME)
EMTR_INTERFACE_AGE=_EMTR_INTERFACE_AGE_MACRO
AC_SUBST([EMTR_MAJOR_VERSION], [_EMTR_API_VERSION_MACRO])
AC_SUBST([EMTR_MINOR_VERSION], [_EMTR_MINOR_VERSION_MACRO])
AC_SUBST([EMTR_MICRO_VERSION], [_EMTR_MICRO_VERSION_MACRO])
AC_DEFINE([EMTR_VERSION], [_EMTR_VERSION_MACRO], [Full version number])

# Shared library versioning; calculated automatically.
# See http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
EMTR_LT_CURRENT=m4_eval(100 * _EMTR_MINOR_VERSION_MACRO + _EMTR_MICRO_VERSION_MACRO - _EMTR_INTERFACE_AGE_MACRO)
EMTR_LT_REVISION=$EMTR_INTERFACE_AGE
EMTR_LT_AGE=$EMTR_LT_CURRENT
EMTR_LT_VERSION_INFO="$EMTR_LT_CURRENT:$EMTR_LT_REVISION:$EMTR_LT_AGE"
AC_SUBST(EMTR_LT_VERSION_INFO)

# Required versions of libraries
# Update these whenever you use a function that requires a certain API version
# Keep these lists sorted alphabetically.
GIO_REQUIREMENT="gio-unix-2.0 >= 2.40"
GOBJECT_REQUIREMENT="gobject-2.0"
UUID_REQUIREMENT="uuid"
# These go into the pkg-config file as Requires: and Requires.private:
# (Requires.private is for dependencies that are not exposed to the
# application code; if we expose a type from those dependencies, we
# need to put the corresponding library in Requires:).
EMTR_REQUIRED_MODULES="$GOBJECT_REQUIREMENT"
EMTR_REQUIRED_MODULES_PRIVATE="$GIO_REQUIREMENT $UUID_REQUIREMENT"
AC_SUBST(EMTR_REQUIRED_MODULES)
AC_SUBST(EMTR_REQUIRED_MODULES_PRIVATE)

# Required build tools
# --------------------
# Make sure we can create directory hierarchies
AC_PROG_MKDIR_P
# C compiler
AC_PROG_CC
# Make sure the C compiler supports per-target CFLAGS
AC_PROG_CC_C_O
# Make sure the C compiler supports C99
AC_PROG_CC_C99
# Initialize Libtool; don't build static libraries
AM_PROG_AR  dnl must call before LT_INIT
LT_INIT([disable-static])
# Library configuration tool
PKG_PROG_PKG_CONFIG
# Gtk-doc; 0.18 required for Markdown parsing
GTK_DOC_CHECK([1.18], [--flavour no-tmpl])
# GObject Introspection
GOBJECT_INTROSPECTION_REQUIRE([1.30])
# Various tools
AC_PATH_PROG([GIRDOCTOOL], [g-ir-doc-tool], [notfound])
AC_ARG_VAR([GIRDOCTOOL], [Path to g-ir-doc-tool])
AC_PATH_PROG([YELPBUILD], [yelp-build], [notfound])
AC_ARG_VAR([YELPBUILD], [Path to yelp-build])
AC_PATH_PROG([GDBUS_CODEGEN], [gdbus-codegen], [notfound])
AS_IF([test "x$GDBUS_CODEGEN" = "xnotfound"],
    [AC_MSG_ERROR([Could not find gdbus-codegen])])
AC_ARG_VAR([GDBUS_CODEGEN], [Path to gdbus-codegen])

AC_CACHE_SAVE

# Configure options
# -----------------
# --enable-strict-flags: Compile with strict compiler flags. Done automatically
# during 'make distcheck'.
AC_ARG_ENABLE([strict-flags],
    [AS_HELP_STRING([--enable-strict-flags=@<:@no/yes/error@:>@],
        [Use strict compiler flags @<:@default=yes@:>@])],
    [],
    [enable_strict_flags=yes])
# Emmanuele's list of flags
STRICT_COMPILER_FLAGS="$STRICT_COMPILER_FLAGS
    -Wall
    -Wcast-align
    -Wuninitialized
    -Wno-strict-aliasing
    -Werror=pointer-arith
    -Werror=missing-declarations
    -Werror=redundant-decls
    -Werror=empty-body
    -Werror=format
    -Werror=format-security
    -Werror=format-nonliteral
    -Werror=init-self"
AS_CASE([$enable_strict_flags],
    [yes],
        [AS_COMPILER_FLAGS([STRICT_CFLAGS], [$STRICT_COMPILER_FLAGS])],
    [no],
        [],
        [error],
        [
            STRICT_COMPILER_FLAGS="$STRICT_COMPILER_FLAGS -Werror"
            AS_COMPILER_FLAGS([STRICT_CFLAGS], [$STRICT_COMPILER_FLAGS])
        ],
    [AC_MSG_ERROR([Invalid option for --enable-strict-flags])])
dnl Strip leading spaces
STRICT_CFLAGS=${STRICT_CFLAGS#*  }
AC_SUBST(STRICT_CFLAGS)

# --enable-gir-doc: Build GIR documentation for JavaScript. Done automatically
# during 'make distcheck'.
AC_ARG_ENABLE([gir-doc],
    [AS_HELP_STRING([--enable-gir-doc],
        [Build GIR documentation for JavaScript @<:@default=yes@:>@])],
    [],
    [enable_gir_doc=yes])
AS_IF([test "x$enable_gir_doc" = "xyes"], [
    AS_IF([test "x$GIRDOCTOOL" = "xnotfound"],
        [AC_MSG_ERROR([g-ir-doc-tool must be installed for --enable-gir-doc])])
    AS_IF([test "x$YELPBUILD" = "xnotfound"],
        [AC_MSG_ERROR([yelp-build must be installed for --enable-gir-doc])])])
AM_CONDITIONAL([ENABLE_GIR_DOC], [test "x$enable_gir_doc" = "xyes"])

# For 'make dist' or 'make distcheck', both --enable-gtk-doc and
# --enable-gir-doc are required
AC_MSG_CHECKING([whether this configuration allows building distributions])
can_make_dist=yes
AM_COND_IF([ENABLE_GTK_DOC], [], [can_make_dist=no])
AM_COND_IF([ENABLE_GIR_DOC], [], [can_make_dist=no])
AM_CONDITIONAL([CAN_MAKE_DIST], [test "x$can_make_dist" = "xyes"])
AC_MSG_RESULT([$can_make_dist])

# Required libraries
# ------------------
PKG_CHECK_MODULES([EOSMETRICS], [
    $EMTR_REQUIRED_MODULES
    $EMTR_REQUIRED_MODULES_PRIVATE])

# This indicates a required dbus mocking interface for unit testing.
AX_PYTHON_MODULE_VERSION([dbusmock], 0.10)

# The Python dbus module is required for unit testing.
AX_PYTHON_MODULE([dbus])

# Code coverage
# -------------
EOS_COVERAGE_REPORT([c])

AC_CACHE_SAVE

# Output
# ------
# List files here that the configure script should output
AC_CONFIG_FILES([
    Makefile
    docs/reference/eosmetrics/Makefile
    docs/reference/eosmetrics/version.xml
    eosmetrics/emtr-version.h
    $EMTR_API_NAME.pc
])
AC_CONFIG_HEADERS([config.h])  dnl Header with system-dependent #defines
AC_OUTPUT

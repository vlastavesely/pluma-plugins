#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([pluma-plugins], [1.0], [vlastavesely@protonmail.ch])
AC_CONFIG_SRCDIR([length/plugin.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES(PLUMA, pluma)
PKG_CHECK_MODULES(IAST, iast)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

AC_PREFIX_DEFAULT([/usr])

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

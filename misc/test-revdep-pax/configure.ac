AC_PREREQ([2.68])
AC_INIT([testrevdeppax],[0.1])
AC_CONFIG_SRCDIR([libmyrevdeppax.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])

LT_PREREQ([2.4])
LT_INIT([dlopen])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC

AC_CHECK_LIB([myrevdeppax], [main])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

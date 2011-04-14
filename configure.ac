#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([elf-misc], [0.1], [blueness@gentoo.org])
AC_CONFIG_SRCDIR([fix-gnustack.c])
#AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign])

# Checks for programs.
AC_PROG_CC
AC_CHECK_PROG([has_yasm],[yasm],["yes"],["no"])
AS_IF([test "x$has_yasm" = "xno"],AC_MSG_ERROR(["Missing yasm assembler"]))

# Checks for libraries.
AC_CHECK_LIB([elf], [elf_begin])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

AC_CONFIG_FILES([Makefile
                 tests/Makefile])
AC_OUTPUT

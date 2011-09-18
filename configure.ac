#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([elfix], [0.3], [http://bugs.gentoo.org/])
AC_CONFIG_SRCDIR([src/fix-gnustack.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign])

AC_ARG_ENABLE(
	[tests],
	AS_HELP_STRING(
		[--enable-tests],
		[perform tests]
	),
	[
		AS_IF(
			[test "x$enable_tests" = "xyes"],
			[
				AC_CHECK_PROG([has_yasm],[yasm],["yes"],["no"])
				AS_IF([test "x$has_yasm" = "xno"],AC_MSG_ERROR(["Missing yasm assembler"]))
			]
		)
	]
)
AM_CONDITIONAL([TEST],[test "x$has_yasm" = "xyes"])

AC_ARG_ENABLE(
	[poc],
	AS_HELP_STRING(
		[--enable-poc],
		[build proof of concept code]
	)
)
AM_CONDITIONAL([POC],[test "x$enable_poc" = "xyes"])

# Checks for programs.
AC_PROG_CC
AC_CHECK_FUNCS([strerror])
AC_FUNC_MMAP

# Checks for libraries.
AC_CHECK_LIB([elf], [elf_begin])

# Checks for header files.
AC_CHECK_HEADERS([errno.h])
AC_CHECK_HEADERS([error.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([gelf.h])
AC_CHECK_HEADERS([libgen.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/mman.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 doc/Makefile
                 poc/Makefile
                 tests/Makefile])

AC_OUTPUT

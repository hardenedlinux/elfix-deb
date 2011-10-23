#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([elfix], [0.4], [http://bugs.gentoo.org/])
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

# Checks for programs.
AC_PROG_CC
AC_CHECK_FUNCS([memset strerror])
AC_FUNC_MMAP

# Checks for libraries.
AC_CHECK_LIB([elf], [elf_begin])
AC_CHECK_LIB([attr], [fgetxattr])

# Checks for header files.

AC_CHECK_HEADERS([attr/xattr.h errno.h error.h fcntl.h gelf.h libgen.h stdio.h \
	stdlib.h string.h sys/mman.h sys/stat.h sys/types.h unistd.h
],
[], [AC_MSG_ERROR(["Missing necessary header"])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 scripts/Makefile
                 doc/Makefile
                 tests/Makefile])

AC_OUTPUT

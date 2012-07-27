#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([elfix], [0.5], [http://bugs.gentoo.org/])
AC_CONFIG_SRCDIR([src/fix-gnustack.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])

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
	[xattr],
	AS_HELP_STRING(
		[--enable-xattr],
		[enable support for pax markings in xattrs]
	),
	[
		AS_IF(
			[test "x$enable_xattr" = "xyes"],
			[
				AC_CHECK_HEADERS(
					[attr/xattr.h],
					[],
					[AC_MSG_ERROR(["Missing necessary attr/xattr.h"])]
				)
				AC_CHECK_LIB(
					[attr],
					[fgetxattr],
					[],
					[AC_MSG_ERROR(["Missing necessary function fgetxattr in libattr"])]
				)
				CFLAGS+=" -DXATTR"
			],
			[
				CFLAGS+=" -UXATTR"
			]
		)
	],
	[
		CFLAGS+=" -DXATTR"
	]
)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AM_PROG_AS

# Checks for libraries.
AC_CHECK_LIB([elf], [elf_begin],[],[AC_MSG_ERROR(["Missing necessary function elf_begin in libelf"])])

# Checks for header files.

AC_CHECK_HEADERS(
	[errno.h error.h fcntl.h gelf.h libgen.h stdio.h stdlib.h string.h \
	sys/mman.h sys/stat.h sys/types.h unistd.h],
	[],
	[AC_MSG_ERROR(["Missing necessary header"])]
)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset strerror])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	scripts/Makefile
	doc/Makefile
	tests/Makefile
	tests/gnustack/Makefile
	tests/pxtpax/Makefile
])

AC_OUTPUT

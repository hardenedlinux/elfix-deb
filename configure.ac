#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([elfix], [0.6], [http://bugs.gentoo.org/])
AC_CONFIG_SRCDIR([src/fix-gnustack.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([no])

AC_ARG_ENABLE(
	[tests],
	AS_HELP_STRING(
		[--enable-tests],
		[perform tests]
	),
	[test "x$enable_tests" = "xyes"]
)
AM_CONDITIONAL([TEST],[test "x$enable_tests" = "xyes"])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_SED

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

# Note: this is always needed for fix-gnustack
# and for paxctl-ng only with --enable-ptpax
AC_CHECK_LIB(
	[elf],
	[elf_begin],
	[],
	[AC_MSG_ERROR(["Missing necessary function elf_begin in libelf"])]
)

AC_ARG_ENABLE(
	[ptpax],
	AS_HELP_STRING(
		[--enable-ptpax],
		[enable support for pax markings in PT_PAX ELF phdr]
	)
)

AS_IF(
	[test "x$enable_ptpax" != "xno"],
	[
		AC_CHECK_DECLS(
			[PT_PAX_FLAGS, PF_PAGEEXEC, PF_MPROTECT, PF_RANDMMAP],
			[],
			[
				CFLAGS+=" -DNEED_PAX_DECLS"
			],
			[[#include <gelf.h>]]
		)
		CFLAGS+=" -DPTPAX"
	],
	[
		CFLAGS+=" -UPTPAX -DNEED_PAX_DECLS"
	]
)

AC_ARG_ENABLE(
	[xtpax],
	AS_HELP_STRING(
		[--enable-xtpax],
		[enable support for pax markings in xattrs]
	)
)

AS_IF(
	[test "x$enable_xtpax" != "xno"],
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
		CFLAGS+=" -DXTPAX"
	],
	[
		CFLAGS+=" -UXTPAX"
	]
)

if [test "x$enable_ptpax" = "xno" -a  "x$enable_xtpax" = "xno" ]; then
	AC_MSG_ERROR(["You must enable either ptpax or xtpax"])
fi

AM_CONDITIONAL([DUALTEST],[test "x$enable_ptpax" = "xyes" -a  "x$enable_xtpax" = "xyes"])

# Ready to configure our files
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	scripts/Makefile
	doc/Makefile
	tests/Makefile
	tests/gnustack/Makefile
	tests/pxtpax/Makefile
	tests/paxmodule/Makefile
])

AC_OUTPUT

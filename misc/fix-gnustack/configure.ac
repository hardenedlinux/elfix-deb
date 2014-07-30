#
#	configure.ac: this file is part of the elfix package
#	Copyright (C) 2011  Anthony G. Basile
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.69])
AC_INIT([fix-gnustack], [0.1], [http://bugs.gentoo.org/])
AC_CONFIG_SRCDIR([fix-gnustack.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.12 foreign])
AM_SILENT_RULES([no])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_SED

# Checks for header files.
AC_CHECK_HEADERS(
    [err.h fcntl.h gelf.h libgen.h stdlib.h string.h sys/stat.h sys/types.h unistd.h],
    [],
    [AC_MSG_ERROR(["Missing necessary header"])]
)

# Checks for DECLs.
AC_CHECK_DECL([ELF_C_RDWR_MMAP],[],[],[[#include <libelf.h>]])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_LIB(
    [elf],
    [elf_begin],
    [],
    [AC_MSG_ERROR(["Missing necessary function elf_begin in libelf"])]
)

AC_CONFIG_FILES([
    Makefile
    doc/Makefile
    tests/Makefile
])

AC_OUTPUT

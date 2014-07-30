#!/bin/sh
#
#	make.sh: this file is part of the elfix package
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

# This is run on the developer side with autogen.sh

PKG=$(cat ../configure.ac | grep ^AC_INIT | sed -e 's/^.*(\[//' -e 's/\].*$//')
VERSION=$(cat ../configure.ac | grep ^AC_INIT | sed -e "s/^.*$PKG\], \[//" -e 's/\].*$//')

pod2man \
 --official \
 --section="1" \
 --release="$PKG $VERSION" \
 --center="Documentation for fix-gnustack" \
 --date=$(date +%Y-%m-%d) \
 fix-gnustack.pod > fix-gnustack.1

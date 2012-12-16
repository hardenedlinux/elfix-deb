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

#Run this on developer side, and distribute troff
#in case the end user doesn't have pod2man

rm -f fix-gnustack.1

pod2man \
 --official \
 --section="1" \
 --release="elfix 0.3" \
 --center="Documentation for elfix" \
 --date="2011-04-14" \
 fix-gnustack.pod > fix-gnustack.1

pod2man \
 --official \
 --section="1" \
 --release="elfix 0.3" \
 --center="Documentation for elfix" \
 --date="2011-08-18" \
 paxctl-ng.pod > paxctl-ng.1

pod2man \
 --official \
 --section="1" \
 --release="elfix 0.3" \
 --center="Documentation for elfix" \
 --date="2011-10-19" \
 revdep-pax.pod > revdep-pax.1

#!/bin/bash
#
#    gnustacktest.sh: this file is part of the elfix package
#    Copyright (C) 2011, 2012  Anthony G. Basile
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

before=$(../fix-gnustack -f bad-gnustack)
before=$(echo ${before} | awk '{ print $2 }')
after=$(../fix-gnustack bad-gnustack)
after=$(echo ${after} | awk '{ print $2 }')
rm bad-gnustack
if [ "${before}" = "RWX" -a "${after}" = "RW" ]; then
  exit 0
else
  exit 1
fi

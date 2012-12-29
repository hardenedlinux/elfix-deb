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

verbose=${1-0}

echo "================================================================================"
echo
echo " RUNNING GNU_STACK TEST"
before=$(../../src/fix-gnustack -f bad-gnustack)
before=$(echo ${before} | awk '{ print $2 }')
after=$(../../src/fix-gnustack bad-gnustack)
after=$(echo ${after} | awk '{ print $2 }')
rm bad-gnustack
if [ "${verbose}" != 0 ]; then
  echo " BEFRE=${before}"
  echo " AFTER=${after}"
fi
if [ "${before}" = "RWX" -a "${after}" = "RW" ]; then
  echo " OK"
  ret=0
else
  echo " NOT OKAY"
  ret=1
fi
echo
echo "================================================================================"

exit $ret

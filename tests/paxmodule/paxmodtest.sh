#!/bin/bash
#
#    paxmodtest.sh: this file is part of the elfix package
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

echo "================================================================================"
echo
echo " RUNNIG PAXMODULE TEST"
echo

verbose=${1-0}
shift

TESTFILE="$(pwd)/dummy"
PAXCTLNG="$(pwd)/../../src/paxctl-ng"
PYPAXCTL="$(pwd)/../../scripts/pypaxctl"

unamem=$(uname -m)
pythonversion=$(python --version 2>&1)
pythonversion=$(echo ${pythonversion} | awk '{ print $2 }')
pythonversion=${pythonversion%\.*}
export PYTHONPATH="$(pwd)/../../scripts/build/lib.linux-${unamem}-${pythonversion}"

#NOTE: the last -D or -U wins as it does for gcc $CFLAGS
for f in $@; do
  [ $f = "-UXTPAX" ] && unset XTPAX
  [ $f = "-DXTPAX" ] && XTPAX=1
  [ $f = "-UPTPAX" ] && unset PTPAX
  [ $f = "-DPTPAX" ] && PTPAX=1
done
export XTPAX
export PTPAX

if [ -d ${PYTHONPATH} ]; then
  rm -rf ${PYTHONPATH}
  echo " (Re)building pax module"
  ( cd ../../scripts; exec ./setup.py build ) >/dev/null
fi

count=0

for pf in "p" "P" "-"; do
  for ef in "e" "E" "-"; do
    for mf in "m" "M" "-"; do
      for rf in "r" "R" "-"; do
        for sf in "s" "S" "-"; do

          pflags="${pf}${ef}${mf}${rf}${sf}"
          if [ "${verbose}" != 0 ] ;then
            echo "SET TO :" ${pflags}
          fi

          flags="${pf/-/Pp}${ef/-/Ee}${mf/-/Mm}${rf/-/Rr}${sf/-/Ss}"
          ${PAXCTLNG} -"${flags}" ${TESTFILE} >/dev/null 2>&1

          sflags=$(${PYPAXCTL} -g ${TESTFILE})

          if [ "${verbose}" != 0 ] ;then
            echo "GOT    :"  ${sflags}
          fi

          if [ "${pflags}" != "${sflags}" ]; then
            (( count = count + 1 ))
            if [ "${verbose}" != 0 ] ;then
              echo "Mismatch: ${pflags} ${sflags}"
            fi
          fi

          if [ "${verbose}" != 0 ] ;then
            echo
          else
            echo -n "."
          fi

        done
      done
    done
  done
done

echo

for pf in "p" "P" "-"; do
  for ef in "e" "E" "-"; do
    for mf in "m" "M" "-"; do
      for rf in "r" "R" "-"; do
        for sf in "s" "S" "-"; do

          pflags="${pf}${ef}${mf}${rf}${sf}"
          if [ "${verbose}" != 0 ] ;then
            echo "SET TO :" ${pflags}
          fi

          flags="${pf/-/Pp}${ef/-/Ee}${mf/-/Mm}${rf/-/Rr}${sf/-/Ss}"
          ${PYPAXCTL} -s "${flags}" ${TESTFILE} >/dev/null 2>&1

          sflags=$(${PAXCTLNG} -v ${TESTFILE})
          sflags=$(echo ${sflags} | awk '{print $4}')

          if [ "${verbose}" != 0 ] ;then
            echo "GOT    :"  ${sflags}
          fi

          if [ "${pflags}" != "${sflags}" ]; then
            (( count = count + 1 ))
            if [ "${verbose}" != 0 ] ;then
              echo "Mismatch: ${pflags} ${sflags}"
            fi
          fi

          if [ "${verbose}" != 0 ] ;then
            echo
          else
            echo -n "."
          fi

        done
      done
    done
  done
done

if [ "${verbose}" = 0 ] ;then
  echo
  echo
fi
echo " Mismatches = ${count}"
echo
echo "================================================================================"

exit $count

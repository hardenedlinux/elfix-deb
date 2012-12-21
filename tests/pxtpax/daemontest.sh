#!/bin/bash
#
#	daemontest.sh: this file is part of the elfix package
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

# dotest = 0 -> do only XATTR_PAX or PT_PAX test
# dotest = 1 -> do both
dotest=${1-0}
verbose=${2-0}
unamem=$(uname -m)

PWD=$(pwd)
INITSH="${PWD}"/init.sh
DAEMON="${PWD}"/daemon
PIDFILE="${PWD}"/daemon.pid
PAXCTLNG="../../src/paxctl-ng"

${PAXCTLNG} -cv ${DAEMON} 2>&1 1>/dev/null

count=0

echo "================================================================================"
echo
echo " RUNNIG DAEMON TEST"
echo
echo " NOTE:"
echo "   1) This test is only for amd64 and i686"
echo "   2) This test will fail on amd64 unless the following are enabled in the kernel:"
echo "        CONFIG_PAX_PAGEEXEC"
echo "        CONFIG_PAX_EMUTRAMP"
echo "        CONFIG_PAX_MPROTECT"
echo "        CONFIG_PAX_RANDMMAP"
echo "   3) This test will fail on i686 unless the following are enbled in the kernel:"
echo "        CONFIG_PAX_EMUTRAMP"
echo "        CONFIG_PAX_MPROTECT"
echo "        CONFIG_PAX_RANDMMAP"
echo "        CONFIG_PAX_SEGMEXEC"
echo

if [ "$unamem" != "i686" -a "$unamem" != "x86_64" ]; then
  echo "This test is only for i686 or x86_64"
  echo
  echo "================================================================================"
  exit 0
fi

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
          ${PAXCTLNG} -"${flags}" ${DAEMON} >/dev/null 2>&1

          if [ "${verbose}" != 0 ] ;then
            sflags=$(${PAXCTLNG} -v ${DAEMON})
            if [ "${dotest}" = "0" ]; then
              sflags=$(echo ${sflags} | awk '{print $3}')
              echo "GOT    :"  ${sflags}
            else
              ptsflags=$(echo ${sflags} | awk '{print $3}')
              xtsflags=$(echo ${sflags} | awk '{print $5}')
              echo "PT_PAX    :"  ${ptsflags}
              echo "XATTR_PAX :"  ${xtsflags}
            fi
          fi

          ${INITSH} start
          if [ -f "${PIDFILE}" ]
          then
            rflags=$(cat /proc/$(cat ${PIDFILE})/status | grep ^PaX | awk '{ print $2 }')
            if [ "${verbose}" != 0 ] ;then
              echo "RUNNING: "${rflags}
            fi
            ${INITSH} stop
          else
            if [ "${verbose}" != 0 ] ;then
              echo "RUNNING: no daemon"
            fi
            rflags="-----"
          fi


          if [ "$unamem" = "i686" ]; then
            # Skip i = 0 which is P which is not set on i686
            list="1 2 3 4"
          else
            # Skip i = 4 which is S which is not set on amd64
            list="0 1 2 3"
          fi

          for i in $list; do
            p=${pflags:$i:1}
            r=${rflags:$i:1}
            if [ $p != "-" ]; then
              if [ $p != $r -a $r != "-" ]; then
                (( count = count + 1 ))
                echo "Mismatch: ${pflags} ${rflags}"
              fi
            fi
          done
          if [ "${verbose}" != 0 ] ;then
            echo
          fi

        done
      done
    done
  done
done

echo " Mismatches = ${count}"
echo
echo "================================================================================"

exit $count

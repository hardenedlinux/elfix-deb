#!/bin/bash

# dotest = 0 -> do only XT_PAX or PT_PAX test
# dotest = 1 -> do both
dotest=${1-0}
verbose=${2-0}

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
              echo "PT_PAX :"  ${ptsflags}
              echo "XT_PAX :"  ${xtsflags}
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

          # Skip i = 4 which is S which is not set
          for i in 0 1 2 3; do
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

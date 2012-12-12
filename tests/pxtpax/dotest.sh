#!/bin/bash

PWD=$(pwd)
INITSH="${PWD}"/init.sh
DAEMON="${PWD}"/daemon
PIDFILE="${PWD}"/daemon.pid
PAXCTLNG="../../src/paxctl-ng"

${PAXCTLNG} -cv ${DAEMON} 2>&1 1>/dev/null

for pf in "p" "P" "-"; do
  for ef in "e" "E" "-"; do
    for mf in "m" "M" "-"; do
      for rf in "r" "R" "-"; do
        for sf in "s" "S" "-"; do

          pflags="${pf}${ef}${mf}${rf}${sf}"
          echo "SET TO :" ${pflags}

          flags="${pf/-/Pp}${ef/-/Ee}${mf/-/Mm}${rf/-/Rr}${sf/-/Ss}"
          ${PAXCTLNG} -"${flags}" ${DAEMON} >/dev/null 2>&1

          sflags=$(${PAXCTLNG} -v ${DAEMON})
          sflags=$(echo ${sflags} | awk '{print $3}')
          echo "GOT    :"  ${sflags}

          ${INITSH} start
          if [ -f "${PIDFILE}" ]
          then
            rflags=$(cat /proc/$(cat ${PIDFILE})/status | grep ^PaX | awk '{ print $2 }')
            echo -n "RUNNING: "${rflags}
            ${INITSH} stop
          else
            echo -n "RUNNING: no daemon"
          fi

          echo
          echo
        done
      done
    done
  done
done

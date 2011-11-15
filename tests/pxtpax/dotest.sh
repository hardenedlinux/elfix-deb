#!/bin/bash

PWD=$(pwd)
INITSH="${PWD}"/init.sh
DAEMON="${PWD}"/daemon
PIDFILE="${PWD}"/daemon.pid
PAXCTLNG="../../src/paxctl-ng"

${PAXCTLNG} -cv ${DAEMON} 2>&1 1>/dev/null

echo "xattr  process"
for pf in "p" "P"; do
  for ef in "e" "E"; do
    for mf in "m" "M"; do
      for rf in "r" "R"; do
        for sf in "s" "S"; do
          flags="${pf}${ef}${mf}${rf}${sf}"
          echo -n ${flags} " "
          ${PAXCTLNG} -"${flags}" ${DAEMON} 2>&1 1>/dev/null
          ${INITSH} start
          if [ -f "${PIDFILE}" ]
          then
            rflags=$(cat /proc/$(cat ${PIDFILE})/status | grep ^PaX | awk '{ print $2 }')
            echo -n ${rflags}
            ${INITSH} stop
          else
            echo -n "no daemon"
          fi
          echo
        done
      done
    done
  done
done

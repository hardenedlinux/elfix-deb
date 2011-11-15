#!/bin/bash

PWD=$(pwd)
INITSH="${PWD}"/init.sh
DAEMON="${PWD}"/daemon
PIDFILE="${PWD}"/daemon.pid

paxctl-ng -cv ${DAEMON}

for pf in "p" "P"; do
  for ef in "e" "E"; do
    for mf in "m" "M"; do
      for rf in "r" "R"; do
        for sf in "s" "S"; do
          flags="${pf}${ef}${mf}${rf}${sf}"
          echo -n ${flags} " "
          paxctl-ng -"${flags}" ${DAEMON} 2>&1 1>/dev/null
          ${INITSH} start
          if [ -f "${PIDFILE}" ]
          then
            rflags=$(cat /proc/$(cat ${PIDFILE})/status | grep ^PaX | awk '{ print $2 }')
            echo -n ${rflags}
            ${INITSH} stop
          fi
          echo
        done
      done
    done
  done
done

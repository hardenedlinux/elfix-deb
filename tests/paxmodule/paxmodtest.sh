#!/bin/bash

verbose=${1-0}

PWD=$(pwd)
TESTFILE="${PWD}"/dummy
PAXCTLNG="../../src/paxctl-ng"
PYPAXCTL="../../scripts/pypaxctl"

count=0

echo "================================================================================"
echo
echo " RUNNIG PAXMODULE TEST"
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

#!/bin/bash

echo "================================================================================"
echo
echo " RUNNIG PAXMODULE TEST"
echo

verbose=${1-0}
shift

TESTFILE="$(pwd)/dummy"
PAXCTLNG="../../src/paxctl-ng"
PYPAXCTL="../../scripts/pypaxctl"

unamem=$(uname -m)
pythonversion=$(python --version 2>&1)
pythonversion=$(echo ${pythonversion} | awk '{ print $2 }')
pythonversion=${pythonversion%\.*}
PYTHONPATH="../../scripts/build/lib.linux-${unamem}-${pythonversion}"

if [ ! -d ${PYTHONPATH} ]; then
	echo "  (Re)building pax module"

	#NOTE: the last -D or -U wins as it does for gcc $CFLAGS
	for f in $@; do
		[ $f = "-UXTPAX" ] && unset XTPAX
		[ $f = "-DXTPAX" ] && XTPAX=1
		[ $f = "-UPTPAX" ] && unset PTPAX
		[ $f = "-DPTPAX" ] && PTPAX=1
	done
	export XTPAX
	export PTPAX

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
          fi

        done
      done
    done
  done
done

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
          sflags=$(echo ${sflags} | awk '{print $3}')

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

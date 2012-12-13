#!/bin/bash

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

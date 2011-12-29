#!/bin/bash

PAXCTLNG="/usr/sbin/paxctl-ng"
PAXCTL="/sbin/paxctl"
BINARY="/usr/bin/testrevdeppax"
LIBRARY="/usr/lib/libmyrevdeppax.so.0.0.0"
REVDEPPAX="/usr/sbin/revdep-pax"

[[ ! -x $PAXCTLNG || ! -x $BINARY || ! -e $LIBRARY || ! -x $REVDEPPAX ]] && {
	echo "Critical file not found"
	exit 1
}

echo
echo "Testing reverse migration $LIBRARY -> $BINARY"
echo "(This will take a while)"
echo

for i in "R" "r" "Rr"
do
	for j in "R" "r" "Rr"
	do
		echo "============================================================================"
		$PAXCTLNG -z $BINARY
		$PAXCTLNG -$i $BINARY
		$PAXCTLNG -z $LIBRARY
		$PAXCTLNG -m$j $LIBRARY
		p=$i; [[ "$p" == "Rr" ]] && p="-"
		echo "Binary  -> $p"
		p=$j; [[ "$p" == "Rr" ]] && p="-"
		echo "Library -> $p"
		$REVDEPPAX -m -y -l $LIBRARY
		echo
		$PAXCTLNG -v $BINARY
		$PAXCTLNG -v $LIBRARY
		$PAXCTL -v $BINARY 2>/dev/null
		$PAXCTL -v $LIBRARY 2>/dev/null
	done
done

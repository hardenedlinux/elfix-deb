#!/bin/bash

echo "================================================================================"
echo
echo " REVDEP-PAX TEST"
echo

ID=$(id -u)

if [ "$ID" != 0 ]; then
	echo " MUST BE ROOT"
	echo
	echo "================================================================================"
	exit 1
fi

PAXCTLNG="../../src/paxctl-ng"
REVDEPPAX="../../scripts/revdep-pax"

BINARY="revdepbin"
LIBRARY="librevdeplib.so.0.0.0"
SONAME="librevdeplib.so.0"

RM="/bin/rm -f"
MKDIR="/bin/mkdir -p"
RMDIR="/bin/rmdir"
LDD="/usr/bin/ldd"
LDCONFIG="/sbin/ldconfig"
LDCONFIGD="/etc/ld.so.conf.d/revdeptest.conf"


# create a ld.so.conf.d/ file
LIBSPATH="$(pwd)/.libs"
cat << EOF > ${LDCONFIGD}
${LIBSPATH}
EOF
${LDCONFIG}

# create our /var/db/pkg/${CAT}/${PKG}/NEEDED
CAT="zzz"
PKG="revdepbin-1"
VARDBPKG="/var/db/pkg"
${MKDIR} ${VARDBPKG}/${CAT}/${PKG}
cat << EOF > ${VARDBPKG}/${CAT}/${PKG}/NEEDED
${LIBSPATH}/${BINARY} ${SONAME}
EOF

#
# do test here
#
${REVDEPPAX} -vs "${SONAME}"
#
# do test here
#

# clean up our /var/db/pkg/${CAT}/${PKG}/NEEDED
${RM} ${VARDBPKG}/${CAT}/${PKG}/NEEDED
${RMDIR} ${VARDBPKG}/${CAT}/${PKG}
${RMDIR} ${VARDBPKG}/${CAT}

# clean up our ld.so.conf.d/ file
${RM} ${LDCONFIGD}
${LDCONFIG}

echo "================================================================================"
exit 0

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
	done
done

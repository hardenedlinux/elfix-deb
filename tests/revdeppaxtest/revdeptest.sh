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

LIBSPATH="$(pwd)/.libs"
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
cat << EOF > "${LDCONFIGD}"
${LIBSPATH}
EOF
${LDCONFIG}

# create our /var/db/pkg/${CAT}/${PKG}/NEEDED
CAT="zzz"
PKG="revdepbin-1"
VARDBPKG="/var/db/pkg"
${MKDIR} "${VARDBPKG}/${CAT}/${PKG}"
cat << EOF > "${VARDBPKG}/${CAT}/${PKG}/NEEDED"
${LIBSPATH}/${BINARY} ${SONAME}
EOF

#
# do test here
#
#${REVDEPPAX} -vs "${SONAME}"
#
for i in "R" "r" "Rr"
do
	for j in "R" "r" "Rr"
	do
		$PAXCTLNG -z   "${LIBSPATH}/${BINARY}"
		$PAXCTLNG -e$i  "${LIBSPATH}/${BINARY}"
		$PAXCTLNG -z   "${LIBSPATH}/${LIBRARY}"
		$PAXCTLNG -m$j "${LIBSPATH}/${LIBRARY}"

		echo " BEFORE: "
		p=$($PAXCTLNG -v ${LIBSPATH}/${BINARY})
		p=$(echo $p | awk '{ print $3 }')
		echo "  Binary:  $p"

		p=$($PAXCTLNG -v ${LIBSPATH}/${LIBRARY})
		p=$(echo $p | awk '{ print $3 }')
		echo "  Library: $p"

		$REVDEPPAX -m -y -s ${SONAME} >/dev/null 2>&1

		echo " AFTER: "
		p=$($PAXCTLNG -v ${LIBSPATH}/${BINARY})
		p=$(echo $p | awk '{ print $3 }')
		echo "  Binary:  $p"

		p=$($PAXCTLNG -v ${LIBSPATH}/${LIBRARY})
		p=$(echo $p | awk '{ print $3 }')
		echo "  Library: $p"
		echo
		echo
	done
done
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


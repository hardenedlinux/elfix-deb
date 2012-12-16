#!/bin/bash
#
#	revdeptest.sh: this file is part of the elfix package
#	Copyright (C) 2011  Anthony G. Basile
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

echo "================================================================================"
echo
echo " REVDEP-PAX TEST"
echo

verbose=${1-0}
shift

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

if [ "${verbose}" = 0 ] ;then
	echo -n "  "
fi

unamem=$(uname -m)
pythonversion=$(python --version 2>&1)
pythonversion=$(echo ${pythonversion} | awk '{ print $2 }')
pythonversion=${pythonversion%\.*}
PYTHONPATH="../../scripts/build/lib.linux-${unamem}-${pythonversion}"

# Assume the pax.so module is built if the path exists
# otherwise build it ourselves!
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

for bf in "R" "r" "Rr"
do
	for lf in "R" "r" "Rr"
	do
		$PAXCTLNG -z       "${LIBSPATH}/${BINARY}"
		$PAXCTLNG -e${bf}  "${LIBSPATH}/${BINARY}"
		$PAXCTLNG -z       "${LIBSPATH}/${LIBRARY}"
		$PAXCTLNG -m${lf}  "${LIBSPATH}/${LIBRARY}"

		p=$($PAXCTLNG -v ${LIBSPATH}/${BINARY})
		p=$(echo $p | awk '{ print $3 }')
		if [ "${verbose}" != 0 ] ;then
			echo " BEFORE: "
			echo "  Binary:  $p"
		fi

		p=$($PAXCTLNG -v ${LIBSPATH}/${LIBRARY})
		p=$(echo $p | awk '{ print $3 }')
		if [ "${verbose}" != 0 ] ;then
			echo "  Library: $p"
		fi

		$REVDEPPAX -m -y -s ${SONAME} >/dev/null 2>&1

		ba=$($PAXCTLNG -v ${LIBSPATH}/${BINARY})
		ba=$(echo $ba | awk '{ print $3 }')
		if [ "${verbose}" != 0 ] ;then
			echo " AFTER: "
			echo "  Binary:  $ba"
		fi

		p=$($PAXCTLNG -v ${LIBSPATH}/${LIBRARY})
		p=$(echo $p | awk '{ print $3 }')
		if [ "${verbose}" != 0 ] ;then
			echo "  Library: $p"
		fi

		be="-em"
		unset x

		if   [ "$bf" != "$lf" -a "$bf" != "Rr" ]; then
			x="$bf"
		elif [ "$bf" = "$lf" ]; then
			x="$bf"
		elif [ "$lf" = "Rr" ]; then
			x="$bf"
		elif [ "$bf" = "Rr" ]; then
			x="$lf"
		fi

		be+="${x/Rr/-}-"

		if [ "$be" != "$ba" ]; then
			(( count = count + 1 ))
			if [ "${verbose}" != 0 ] ;then
				echo "   Mismatch: Expected Binary: ${be}"
			fi
		fi

		if [ "${verbose}" != 0 ] ;then
			echo
			echo
		else
			echo -n "."
		fi
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

if [ "${verbose}" = 0 ] ;then
	echo
	echo
fi
echo " Mismatches = ${count}"
echo
echo "================================================================================"
exit $count


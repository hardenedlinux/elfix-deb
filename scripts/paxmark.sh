#!/bin/bash -l

has() {
	f=$1
	shift
	[[ "${@/$f/}" != "$@" ]] && return 0
	return 1
}

paxmarksh() {
	local f					# loop over paxables
	local flags				# pax flags
	local ret=0				# overal return code of this function

	# Only the actual PaX flags and z are accepted
	# 1. The leading '-' is optional
	# 2. -C -c only make sense for paxctl, but are unnecessary
	#    because we progressively do -q -qc -qC
	# 3. z is allowed for the default

	flags="${1//[!zPpEeMmRrSs]}"
	[[ "${flags}" ]] || return 0
	shift

	# z = default. For XATTR_PAX, the default is no xattr field at all
	local dodefault=""
	[[ "${flags//[!z]}" ]] && dodefault="yes"

	if has PT ${PAX_MARKINGS}; then
		for f in "$@"; do

			#First try paxctl -> this might try to create/convert program headers
			if type -p paxctl > /dev/null; then
				# First, try modifying the existing PAX_FLAGS header
				paxctl -q${flags} "${f}" >/dev/null 2>&1 && continue
				# Second, try creating a PT_PAX header (works on ET_EXEC)
				# Even though this is less safe, most exes need it, eg bug #463170
				paxctl -qC${flags} "${f}" >/dev/null 2>&1 && continue
				# Third, try stealing the (unused under PaX) PT_GNU_STACK header
				paxctl -qc${flags} "${f}" >/dev/null 2>&1 && continue
			fi

			#Next try paxctl-ng -> this will not create/convert any program headers
			if type -p paxctl-ng > /dev/null && paxctl-ng -L ; then
				flags="${flags//z}"
				[[ ${dodefault} == "yes" ]] && paxctl-ng -L -z "${f}" >/dev/null 2>&1
				[[ "${flags}" ]] || continue
				paxctl-ng -L -${flags} "${f}" >/dev/null 2>&1 && continue
			fi

			#Finally fall back on scanelf
			if type -p scanelf > /dev/null && [[ ${PAX_MARKINGS} != "none" ]]; then
				scanelf -Xxz ${flags} "$f" >/dev/null 2>&1
			#We failed to set PT_PAX flags
			elif [[ ${PAX_MARKINGS} != "none" ]]; then
				ret=1
			fi
		done
	fi

	if has XT ${PAX_MARKINGS}; then
		flags="${flags//z}"
		for f in "$@"; do

			#First try paxctl-ng
			if type -p paxctl-ng > /dev/null && paxctl-ng -l ; then
				[[ ${dodefault} == "yes" ]] && paxctl-ng -d "${f}" >/dev/null 2>&1
				[[ "${flags}" ]] || continue
				paxctl-ng -l -${flags} "${f}" >/dev/null 2>&1 && continue
			fi

			#Next try setfattr
			if type -p setfattr > /dev/null; then
				[[ "${flags//[!Ee]}" ]] || flags+="e" # bug 447150
				[[ ${dodefault} == "yes" ]] && setfattr -x "user.pax.flags" "${f}" >/dev/null 2>&1
				setfattr -n "user.pax.flags" -v "${flags}" "${f}" >/dev/null 2>&1 && continue
			fi

			#We failed to set XATTR_PAX flags
			if [[ ${PAX_MARKINGS} != "none" ]]; then
				ret=1
			fi
		done
	fi

	return ${ret}
}

MAKE_CONF="/etc/portage/make.conf"

if [[ -d $MAKE_CONF ]]; then
	for MC in $MAKE_CONF/*; do
		source $MC
	done
elif [[ -e $MAKE_CONF ]]; then
	source $MAKE_CONF
fi

PAX_MARKINGS=${PAX_MARKINGS:="PT XT"}
paxmarksh "$@"

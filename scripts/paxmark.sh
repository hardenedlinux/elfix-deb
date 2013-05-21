#!/bin/bash -l

has() {
	[[ "${2/$1/}" != "$2" ]] && return 0
	return 1
}

paxmarksh() {

	local f					# loop over paxables
	local flags				# pax flags
	local pt_fail=0 pt_failures=""		# record PT_PAX failures
	local xt_fail=0 xt_failures=""		# record xattr PAX marking failures
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

	if has PT "${PAX_MARKINGS}"; then

		#First try paxctl -> this might try to create/convert program headers
		if type -p paxctl > /dev/null; then
			for f in "$@"; do
				# First, try modifying the existing PAX_FLAGS header
				paxctl -q${flags} "${f}" >/dev/null 2>&1 && continue
				# Second, try creating a PT_PAX header (works on ET_EXEC)
				# Even though this is less safe, most exes need it, eg bug #463170
				paxctl -qC${flags} "${f}" >/dev/null 2>&1 && continue
				# Third, try stealing the (unused under PaX) PT_GNU_STACK header
				paxctl -qc${flags} "${f}" >/dev/null 2>&1 && continue
				pt_fail=1
				pt_failures="${pt_failures} ${f}"
			done

		#Next try paxctl-ng -> this will not create/convert any program headers
		elif type -p paxctl-ng > /dev/null && paxctl-ng -L ; then
			flags="${flags//z}"
			for f in "$@"; do
				[[ ${dodefault} == "yes" ]] && paxctl-ng -L -z "${f}" >/dev/null 2>&1
				[[ "${flags}" ]] || continue
				paxctl-ng -L -${flags} "${f}" >/dev/null 2>&1 && continue
				pt_fail=1
				pt_failures="${pt_failures} ${f}"
			done

		#Finally fall back on scanelf
		elif type -p scanelf > /dev/null && [[ ${PAX_MARKINGS} != "none" ]]; then
			scanelf -Xxz ${flags} "$@" >/dev/null 2>&1

		#We failed to set PT_PAX flags
		elif [[ ${PAX_MARKINGS} != "none" ]]; then
			pt_failures="$*"
			pt_fail=1
		fi

		if [[ ${pt_fail} == 1 ]]; then
			ret=1
		fi
	fi

	if has XT "${PAX_MARKINGS}"; then

		flags="${flags//z}"

		#First try paxctl-ng
		if type -p paxctl-ng > /dev/null && paxctl-ng -l ; then
			for f in "$@"; do
				[[ ${dodefault} == "yes" ]] && paxctl-ng -d "${f}" >/dev/null 2>&1
				[[ "${flags}" ]] || continue
				paxctl-ng -l -${flags} "${f}" >/dev/null 2>&1 && continue
				xt_fail=1
				xt_failures="${tx_failures} ${f}"
			done

		#Next try setfattr
		elif type -p setfattr > /dev/null; then
			[[ "${flags//[!Ee]}" ]] || flags+="e" # bug 447150
			for f in "$@"; do
				[[ ${dodefault} == "yes" ]] && setfattr -x "user.pax.flags" "${f}" >/dev/null 2>&1
				setfattr -n "user.pax.flags" -v "${flags}" "${f}" >/dev/null 2>&1 && continue
				xt_fail=1
				xt_failures="${tx_failures} ${f}"
			done

		#We failed to set XATTR_PAX flags
		elif [[ ${PAX_MARKINGS} != "none" ]]; then
			xt_failures="$*"
			xt_fail=1
		fi

		if [[ ${xt_fail} == 1 ]]; then
			ret=1
		fi
	fi

	return ${ret}
}

PAX_MARKINGS=${PAX_MARKINGS:="PT XT"}
paxmarksh "$@"
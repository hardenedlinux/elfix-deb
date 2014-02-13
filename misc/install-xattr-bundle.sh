#!/bin/bash

if [[ -z ${1} ]]; then
	echo "Usage $0 <version>"
	exit
fi

tar jcvf install-xattr-${1}.tar.bz2 install-xattr/
gpg -s -a -b install-xattr-${1}.tar.bz2 


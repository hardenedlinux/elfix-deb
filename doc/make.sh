#!/bin/sh

#Run this on developer side, and distribute troff
#in case the end user doesn't have pod2man

rm -f fix-gnustack.1

pod2man \
 --official \
 --section="1" \
 --release="elfix 0.2" \
 --center="Documentation for elfix" \
 --date="2011-04-14" \
 fix-gnustack.pod > fix-gnustack.1

pod2man \
 --official \
 --section="1" \
 --release="elfix 0.2" \
 --center="Documentation for elfix" \
 --date="2011-08-18" \
 paxctl-ng.pod > paxctl-ng.1

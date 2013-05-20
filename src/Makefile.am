ACLOCAL_AMFLAGS = -I m4

sbin_PROGRAMS = paxctl-ng
paxctl_ng_SOURCES = paxctl-ng.c

if BUILD_ELF
sbin_PROGRAMS += fix-gnustack
fix_gnustack_SOURCES = fix-gnustack.c
endif

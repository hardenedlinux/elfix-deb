ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = paxctl-ng.1 revdep-pax.1
if BUILD_ELF
dist_man_MANS += fix-gnustack.1
endif

# I don't know why I need this, but without it, I don't get fix-gnustack.1 distributed.
# This is since commit 414cfa1770a8cfc46308149deecf9c0eef60a5bb.  It will be fixed once
# fix-gnustack is broken out.
EXTRA_DIST = fix-gnustack.1

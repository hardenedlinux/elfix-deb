CFLAGS=
noinst_PROGRAMS = install-xattr
install_xattr_SOURCES = install.wrapper.c
install_xattr_CFLAGS = -ggdb -Wall -O3

check_SCRIPTS = checkcopyattrs
TEST = $(check_SCRIPTS)

checkcopyattrs:
	$(srcdir)/checkcopyattrs.sh

EXTRA_DIST = checkcopyattrs.sh
CLEANFILES = a b c x y z d/* e/*

clean-local:
	-rm -rf d e

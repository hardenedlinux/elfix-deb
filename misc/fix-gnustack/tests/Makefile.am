ACLOCAL_AMFLAGS = -I m4

noinst_PROGRAMS = bad-gnustack

EXTRA_DIST = gnustacktest.sh

bad-gnustack.s: bad-gnustack.c
	$(CC) -S $<
	$(SED) -i -e 's/GNU-stack,"",/GNU-stack,"x",/' $@

bad-gnustack$(EXEEXT): bad-gnustack.s
	$(CC) -o $@ $<

check_SCRIPTS = gnustacktest
TEST = $(check_SCRIPTS)

gnustacktest: bad-gnustack
	$(top_srcdir)/tests/gnustacktest.sh

CLEANFILES = *.o *.s

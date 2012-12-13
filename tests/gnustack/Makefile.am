noinst_PROGRAMS = bad-gnustack

bad-gnustack.s: bad-gnustack.c
	$(CC) -S $<
	$(SED) -i -e 's/GNU-stack,"",/GNU-stack,"x",/' $@

bad-gnustack: bad-gnustack.s
	$(CC) -o $@ $<

check_SCRIPTS = gnustacktest
TEST = $(check_SCRIPTS)

gnustacktest:
	./gnustacktest.sh 0

CLEANFILES = *.o *.s

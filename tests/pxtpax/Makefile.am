noinst_PROGRAMS = daemon
daemon_SOURCES = daemon.c

EXTRA_DIST = init.sh dotest.sh

check_SCRIPTS = test.sh
TEST = $(check_SCRIPTS)

test.sh:
	./dotest.sh

CLEANFILES = daemon.pid

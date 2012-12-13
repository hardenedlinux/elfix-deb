noinst_PROGRAMS = daemon
daemon_SOURCES = daemon.c

EXTRA_DIST = init.sh dotest.sh

check_SCRIPTS = daemontest
TEST = $(check_SCRIPTS)

if DUALTEST
daemontest:
	./daemontest.sh 1 0
else
daemontest:
	./daemontest.sh 0 0
endif

CLEANFILES = daemon.pid

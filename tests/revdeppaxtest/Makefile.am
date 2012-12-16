ACLOCAL_AMFLAGS = -I m4

# To generate .libs/revdepbin and .libs/librevdeplib.so.0.0.0
# we must trick automake into thinking they are goint to be
# installed.

tmpdir = /tmp/revdeptest

tmp_PROGRAMS = revdepbin
noinst_PROGRAMS = revdepbin
revdepbin_SOURCES = revdepbin.c
revdepbin_LDADD = librevdeplib.la

tmp_LTLIBRARIES = librevdeplib.la
librevdeplib_la_SOURCES = librevdeplib.c

check_SCRIPTS = revdeptest
TEST = $(check_SCRIPTS)

revdeptest:
	./revdeptest.sh 0 $(CFLAGS)

EXTRA_DIST = revdeptest.sh

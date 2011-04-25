noinst_PROGRAMS = good bad
good_SOURCES = good.c
bad_SOURCES = bad.c
bad_DEPENDENCIES = test-bad.o
bad_LDADD = test-bad.o

ARCH = $(shell uname -m | sed -e 's/i./x/')

test-bad.o: test-bad64.asm test-bad32.asm
	[[ "$(ARCH)" == "x86" ]] && yasm -f elf -m x86 test-bad32.asm && mv test-bad32.o test-bad.o || true
	[[ "$(ARCH)" == "x86_64" ]] && yasm -f elf -m amd64 test-bad64.asm && mv test-bad64.o test-bad.o || true

check_SCRIPTS = test.sh
TEST = $(check_SCRIPTS)

test.sh:
	@echo "================================================================================"
	@echo
	@echo "Good Elf"
	@../src/fix-gnustack good
	@echo
	@echo "Bad Elf"
	@../src/fix-gnustack bad
	@echo
	@echo "Fixing Bad Elf"
	@../src/fix-gnustack -f bad
	@echo
	@echo "Fixed Bad Elf"
	@../src/fix-gnustack bad
	@rm -f good bad
	@echo
	@echo "================================================================================"

CLEANFILES = test-bad.o test.sh

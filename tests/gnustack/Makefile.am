noinst_PROGRAMS = bad-gnustack
bad_gnustack_SOURCES = bad64.s bad32.s

ARCH = $(shell uname -m | sed -e 's/i./x/')

bad-gnustack: bad64.s bad32.s
	[[ "$(ARCH)" == "x86" ]]    && $(CCAS) -m32 -o bad-gnustack bad32.s || true
	[[ "$(ARCH)" == "x86_64" ]] && $(CCAS) -m64 -o bad-gnustack bad64.s || true

check_SCRIPTS = test.sh
TEST = $(check_SCRIPTS)

test.sh:
	@echo "================================================================================"
	@echo
	@echo "Before GNU_STACK Elf: "
	@../../src/fix-gnustack -f bad-gnustack
	@echo
	@echo "After GNU_STACK Elf: "
	@../../src/fix-gnustack bad-gnustack
	@echo
	@echo "================================================================================"

CLEANFILES = *.o test.sh

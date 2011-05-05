noinst_PROGRAMS = bad-gnustack
bad_gnustack_SOURCES = bad-gnustack.c
bad_gnustack_DEPENDENCIES = bad-asm.o
bad_gnustack_LDADD = bad-asm.o

ARCH = $(shell uname -m | sed -e 's/i./x/')

bad-asm.o: bad64.asm bad32.asm
	[[ "$(ARCH)" == "x86" ]] && yasm -f elf -m x86 bad32.asm && mv bad32.o bad-asm.o || true
	[[ "$(ARCH)" == "x86_64" ]] && yasm -f elf -m amd64 bad64.asm && mv bad64.o bad-asm.o || true

check_SCRIPTS = test.sh
TEST = $(check_SCRIPTS)

test.sh:
	@echo "================================================================================"
	@echo
	@echo "Fixing Bad GNU_STACK Elf"
	@../src/fix-gnustack -f bad-gnustack
	@echo
	@echo "Fixed Bad GNU_STACK Elf"
	@../src/fix-gnustack bad-gnustack
	@rm -f good
	@echo
	@echo "================================================================================"

CLEANFILES = bad-asm.o test.sh

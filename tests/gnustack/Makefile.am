noinst_PROGRAMS = bad-gnustack

bad-gnustack.s: bad-gnustack.c
	$(CC) -S $<
	$(SED) -i -e 's/GNU-stack,"",/GNU-stack,"x",/' $@

bad-gnustack: bad-gnustack.s
	$(CC) -o $@ $<

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

CLEANFILES = *.o *.s test.sh

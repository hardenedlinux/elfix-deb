check_SCRIPTS = test.sh
TEST = $(check_SCRIPTS)

test.sh:
	@echo "================================================================================"
	@echo
	@gcc -o good good.c
	@echo "Good result"
	@[[ -x ../get-gnustack ]] && ../get-gnustack good || echo "Run 'make' first"
	@echo
	@yasm -f elf -m amd64 test-bad.asm
	@gcc -c bad.c
	@gcc -o bad bad.o test-bad.o
	@echo "Bad result"
	@[[ -x ../get-gnustack ]] && ../get-gnustack bad || echo "Run 'make' first"
	@echo
	@echo "================================================================================"

CLEANFILES = good good.o bad bad.o test-bad.o test.sh

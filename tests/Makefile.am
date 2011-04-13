check_SCRIPTS = test.sh
TEST = $(check_SCRIPTS)

test.sh:
	@echo "================================================================================"
	@echo
#	@[[ -x ../get-gnustack ]] || echo "Run 'make' first" && exit
#	@[[ -x ../fix-gnustack ]] || echo "Run 'make' first" && exit
	@gcc -o good good.c
	@echo "Good result"
	@../get-gnustack good
	@echo
	@yasm -f elf -m amd64 test-bad.asm
	@gcc -c bad.c
	@gcc -o bad bad.o test-bad.o
	@echo "Bad result"
	@../get-gnustack bad
	@echo
	@echo "Fixed results"
	@../fix-gnustack bad
	@../get-gnustack bad
	@echo
	@echo "================================================================================"

CLEANFILES = good good.o bad bad.o test-bad.o test.sh

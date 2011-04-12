bin_PROGRAMS = good
good_SOURCES = good.c

test-bad.o: test-bad.asm
	yasm -f elf -m amd64 $<

bad.o: bad.c

bad: bad.o test-bad.o

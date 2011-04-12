
CC = gcc
LIBS += -lelf

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
BINS = $(patsubst %.c,%,$(SRCS))


.PHONY: all
#all: $(BINS)
all: $(OBJS) $(BINS)

.PHONY: clean
clean:
	rm -f $(BINS) *.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.asm
	yasm -f elf -m amd64 $<

test_asm: test_asm.o test_bad.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%: %.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)


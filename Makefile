
SRCS = parse_elf test

.PHONY: all
all: $(SRCS)

.PHONY: clean
clean:
	rm -f $(SRCS)

%: %.c
	gcc -o $@ $^ -lelf

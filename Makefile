
SRCS = parse_elf get_gnu_stack test

.PHONY: all
all: $(SRCS)

.PHONY: clean
clean:
	rm -f $(SRCS)

%: %.c
	gcc -o $@ $^ -lelf

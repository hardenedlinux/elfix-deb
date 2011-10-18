all: bad-mmap mangle-paxflags

%: %.c
	gcc -o $@ $^ -lelf

clean:
	rm -rf bad-mmap mangle-paxflags

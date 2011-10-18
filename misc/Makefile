all: clear-dt-path parse-elf print-sections paxctl-xattr

%: %.c
	gcc -o $@ $^ -lelf

clean:
	rm -rf clear-dt-path parse-elf print-sections paxctl-xattr

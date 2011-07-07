all: clear-dt-path parse-elf print-sections

%: %.c
	gcc -o $@ $^ -lelf

clean:
	rm -rf parse-elf print-sections

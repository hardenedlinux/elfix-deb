all: clear-dt-path parse-elf print-sections remove-ptpax

%: %.c
	gcc -o $@ $^ -lelf

clean:
	rm -rf clear-dt-path parse-elf print-sections remove-ptpax

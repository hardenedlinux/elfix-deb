
.PHONY: all
all: test

.PHONY: test
test: daemon init.sh test.sh
	@echo "xattr  process"
	@$(PWD)/test.sh

daemon:	daemon.c
	gcc -o $@ $^
	paxctl-ng -cv $@

clean:
	rm -f daemon daemon.pid

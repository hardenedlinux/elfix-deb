noinst_PROGRAMS = mangle-paxflags bad-mmap
mangle_paxflags_SOURCES = mangle-paxflags.c
mangle_paxflags_LDADD = -lelf
bad_mmap_SOURCES = bad-mmap.c

check_SCRIPTS = poc.sh

poc.sh:
	@echo "================================================================================"
	@echo
	./mangle-paxflags bad-mmap
	./bad-mmap
	@echo
	@echo "========================================"
	@echo
	./mangle-paxflags -p bad-mmap
	./mangle-paxflags bad-mmap
	./bad-mmap
	@echo
	@echo "========================================"
	@echo
	./mangle-paxflags -e bad-mmap
	./mangle-paxflags bad-mmap
	./bad-mmap
	@echo
	@echo "================================================================================"

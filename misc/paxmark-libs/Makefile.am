ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -DPLUGIN='"$(libdir)/libmypax.so"'

bin_PROGRAMS = testpax testdlpax
testpax_SOURCES = testpax.c
testpax_LDFLAGS = -lmypax
testdlpax_SOURCES = testdlpax.c
testdlpax_LDFLAGS = -ldl

lib_LTLIBRARIES = libmypax.la
libmypax_la_SOURCES = libmypax.c

check_SCRIPTS = poc.sh

poc.sh:
	@/sbin/ldconfig
	@echo "================================================================================"
	@echo "= TESTING DYNAMIC LINKING ======================================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testpax
	@/sbin/paxctl -M $(libdir)/libmypax.so
	@/sbin/paxctl -M $(bindir)/testpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testpax 2>/dev/null
	@echo
	$(bindir)/testpax
	@echo
	@echo "========================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testpax
	@/sbin/paxctl -m $(libdir)/libmypax.so
	@/sbin/paxctl -M $(bindir)/testpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testpax 2>/dev/null
	@echo
	$(bindir)/testpax
	@echo
	@echo "========================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testpax
	@/sbin/paxctl -M $(libdir)/libmypax.so
	@/sbin/paxctl -m $(bindir)/testpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testpax 2>/dev/null
	@echo
	$(bindir)/testpax
	@echo
	@echo "========================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testpax
	@/sbin/paxctl -m $(libdir)/libmypax.so
	@/sbin/paxctl -m $(bindir)/testpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testpax 2>/dev/null
	@echo
	$(bindir)/testpax
	@echo
	@echo
	@echo "================================================================================"
	@echo "= TESTING DLOPENING ============================================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testdlpax
	@/sbin/paxctl -M $(libdir)/libmypax.so
	@/sbin/paxctl -M $(bindir)/testdlpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testdlpax 2>/dev/null
	@echo
	$(bindir)/testdlpax
	@echo
	@echo "========================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testdlpax
	@/sbin/paxctl -m $(libdir)/libmypax.so
	@/sbin/paxctl -M $(bindir)/testdlpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testdlpax 2>/dev/null
	@echo
	$(bindir)/testdlpax
	@echo
	@echo "========================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testdlpax
	@/sbin/paxctl -M $(libdir)/libmypax.so
	@/sbin/paxctl -m $(bindir)/testdlpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testdlpax 2>/dev/null
	@echo
	$(bindir)/testdlpax
	@echo
	@echo "========================================"
	@echo
	@/sbin/paxctl -z $(libdir)/libmypax.so $(bindir)/testdlpax
	@/sbin/paxctl -m $(libdir)/libmypax.so
	@/sbin/paxctl -m $(bindir)/testdlpax
	@/sbin/paxctl -Qv $(libdir)/libmypax.so $(bindir)/testdlpax 2>/dev/null
	@echo
	$(bindir)/testdlpax
	@echo
	@echo "================================================================================"


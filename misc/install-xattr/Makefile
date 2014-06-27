CFLAGS ?= -O2 -pipe -g
CFLAGS += -Wall
PWD = $$(pwd)

all: install-xattr

install-xattr: install-xattr.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

PREFIX = /usr
BINDIR = $(PREFIX)/bin
install: install-xattr
	install -m 755 -D install-xattr $(DESTDIR)$(BINDIR)/$<

check: checkcopyattrs.sh install-xattr
	$(PWD)/checkcopyattrs.sh
	rm -rf a b c d e f g h x y z backup-a* mode-a target-a target-install-xattr

clean:
	rm -f *.o *~ install-xattr

.PHONY: all check clean install

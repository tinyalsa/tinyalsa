DESTDIR ?=
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/share/man

DOXYGEN ?= doxygen
DOXYGENFLAGS =

.PHONY: all
all:
	$(DOXYGEN) $(DOXYGENFLAGS)

.PHONY: clean
clean:
	rm -Rf html
	rm -Rf man

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(MANDIR)/man3
	cp -u man/man3/tinyalsa-pcm.3 $(DESTDIR)$(MANDIR)/man3
	cp -u man/man3/tinyalsa-mixer.3 $(DESTDIR)$(MANDIR)/man3


DESTDIR ?=
PREFIX ?= /usr/local

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
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man3
	cp -u man/man3/tinyalsa-pcm.3 $(DESTDIR)$(PREFIX)/share/man/man3
	cp -u man/man3/tinyalsa-mixer.3 $(DESTDIR)$(PREFIX)/share/man/man3


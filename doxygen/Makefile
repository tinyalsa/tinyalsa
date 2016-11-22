DESTDIR ?=
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/share/man

DOXYGEN := $(shell command -v doxygen 2> /dev/null)
DOXYGENFLAGS =

.PHONY: all
all:
ifndef DOXYGEN
	$(warning "doxygen is not available please install it")
else
	$(DOXYGEN) $(DOXYGENFLAGS)
endif

.PHONY: clean
clean:
ifdef DOXYGEN
	rm -Rf html
	rm -Rf man
endif

.PHONY: install
install:
ifdef DOXYGEN
	install -d $(DESTDIR)$(MANDIR)/man3
	install man/man3/libtinyalsa-pcm.3 $(DESTDIR)$(MANDIR)/man3
	install man/man3/libtinyalsa-mixer.3 $(DESTDIR)$(MANDIR)/man3
endif

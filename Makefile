export DESTDIR ?=
export PREFIX ?= /usr/local
export CROSS_COMPILE =

export INCDIR ?= $(PREFIX)/include
export LIBDIR ?= $(PREFIX)/lib
export BINDIR ?= $(PREFIX)/bin
export MANDIR ?= $(PREFIX)/share/man

INCDIR ?= $(PREFIX)/include

.PHONY: all
all:
	$(MAKE) -C src
	$(MAKE) -C utils
	$(MAKE) -C doxygen

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C utils clean
	$(MAKE) -C doxygen clean

.PHONY: install
install:
	install -d $(DESTDIR)$(INCDIR)/tinyalsa
	install include/tinyalsa/pcm.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/mixer.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/asoundlib.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/version.h $(DESTDIR)$(INCDIR)/
	$(MAKE) -C src install
	$(MAKE) -C utils install
	$(MAKE) -C doxygen install


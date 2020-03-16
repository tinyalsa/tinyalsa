export DESTDIR ?=
export PREFIX ?= /usr/local

export INCDIR ?= $(PREFIX)/include/tinyalsa
export LIBDIR ?= $(PREFIX)/lib
export BINDIR ?= $(PREFIX)/bin
export MANDIR ?= $(PREFIX)/share/man

export VERSIONSCRIPT = $(shell pwd)/scripts/version.sh

export TINYALSA_VERSION_MAJOR = $(shell $(VERSIONSCRIPT) -s print major)
export TINYALSA_VERSION       = $(shell $(VERSIONSCRIPT) -s print      )

.PHONY: all
all:
	$(MAKE) -C src
	$(MAKE) -C utils
	$(MAKE) -C doxygen
	$(MAKE) -C examples

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C utils clean
	$(MAKE) -C doxygen clean
	$(MAKE) -C examples clean

.PHONY: install
install:
	install -d $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/attributes.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/pcm.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/mixer.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/asoundlib.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/version.h $(DESTDIR)$(INCDIR)/
	install include/tinyalsa/plugin.h $(DESTDIR)$(INCDIR)/
	$(MAKE) -C src install
	$(MAKE) -C utils install
	$(MAKE) -C doxygen install


export TOPDIR = $(shell pwd)
export SRCDIR = $(TOPDIR)/src
export INCDIR = $(TOPDIR)/include

export DESTDIR ?=
export PREFIX ?= /usr/local
export CROSS_COMPILE =

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
install: $(LIB) $(SHLIB)
	mkdir -p $(DESTDIR)$(PREFIX)/include
	cp -Ru $(INCDIR)/tinyalsa $(DESTDIR)$(PREFIX)/include/
	$(MAKE) -C src install
	$(MAKE) -C utils install
	$(MAKE) -C doxygen install


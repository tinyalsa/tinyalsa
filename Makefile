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

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C utils clean

.PHONY: install
install: $(LIB) $(SHLIB)
	mkdir -p $(PREFIX)/include/tinyalsa
	cp -Ru $(INCDIR)/tinyalsa $(PREFIX)/include/
	$(MAKE) -C src install
	$(MAKE) -C utils install


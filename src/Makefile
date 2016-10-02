TOPDIR ?= ..
INCDIR ?= $(TOPDIR)/include

DEB_HOST_MULTIARCH := $(shell dpkg-architecture -qDEB_HOST_MULTIARCH)

DESTDIR ?=
PREFIX ?= /usr/local
CROSS_COMPILE =

CC = $(CROSS_COMPILE)gcc
override CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -I $(INCDIR) -fPIC $(CFLAGS)

AR = $(CROSS_COMPILE)ar
ARFLAGS = rcs

LD = $(CROSS_COMPILE)gcc
LDFLAGS =

OBJECTS = mixer.o pcm.o

.PHONY: all
all: libtinyalsa.a libtinyalsa.so

pcm.o: pcm.c $(INCDIR)/tinyalsa/pcm.h

mixer.o: mixer.c $(INCDIR)/tinyalsa/mixer.h

libtinyalsa.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libtinyalsa.so: $(OBJECTS)
	$(LD) $(LDFLAGS) -shared $^ -o $@

.PHONY: clean
clean:
	rm -f libtinyalsa.a
	rm -f libtinyalsa.so
	rm -f $(OBJECTS)

.PHONY: install
install: libtinyalsa.a libtinyalsa.so
	mkdir -p $(DESTDIR)$(PREFIX)/lib/$(DEB_HOST_MULTIARCH)
	cp -u libtinyalsa.a $(DESTDIR)$(PREFIX)/lib/$(DEB_HOST_MULTIARCH)
	cp -u libtinyalsa.so $(DESTDIR)$(PREFIX)/lib/$(DEB_HOST_MULTIARCH)


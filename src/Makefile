DESTDIR ?=
PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
BINDIR ?= $(PREFIX)/bin
ifdef DEB_HOST_MULTIARCH
LIBDIR := $(LIBDIR)/$(DEB_HOST_MULTIARCH)
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)gcc

WARNINGS = -Wall -Wextra -Werror -Wfatal-errors
INCLUDE_DIRS = -I ../include
override CFLAGS := $(WARNINGS) $(INCLUDE_DIRS) -fPIC $(CFLAGS)

VPATH = ../include/tinyalsa
OBJECTS = limits.o mixer.o pcm.o

LIBVERSION_MAJOR = $(TINYALSA_VERSION_MAJOR)
LIBVERSION = $(TINYALSA_VERSION)

.PHONY: all
all: libtinyalsa.a libtinyalsa.so

pcm.o: pcm.c pcm.h

limits.o: limits.c limits.h

mixer.o: mixer.c mixer.h

libtinyalsa.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libtinyalsa.so: libtinyalsa.so.$(LIBVERSION_MAJOR)
	ln -sf $< $@

libtinyalsa.so.$(LIBVERSION_MAJOR): libtinyalsa.so.$(LIBVERSION)
	ln -sf $< $@

libtinyalsa.so.$(LIBVERSION): $(OBJECTS)
	$(LD) $(LDFLAGS) -shared -Wl,-soname,libtinyalsa.so.$(LIBVERSION_MAJOR) $^ -o $@

.PHONY: clean
clean:
	rm -f libtinyalsa.a
	rm -f libtinyalsa.so
	rm -f libtinyalsa.so.$(LIBVERSION_MAJOR)
	rm -f libtinyalsa.so.$(LIBVERSION)
	rm -f $(OBJECTS)

.PHONY: install
install: libtinyalsa.a libtinyalsa.so.$(LIBVERSION_MAJOR)
	install -d $(DESTDIR)$(LIBDIR)/
	install libtinyalsa.a $(DESTDIR)$(LIBDIR)/
	install libtinyalsa.so.$(LIBVERSION) $(DESTDIR)$(LIBDIR)/
	ln -sf libtinyalsa.so.$(LIBVERSION) $(DESTDIR)$(LIBDIR)/libtinyalsa.so.$(LIBVERSION_MAJOR)
	ln -sf libtinyalsa.so.$(LIBVERSION_MAJOR) $(DESTDIR)$(LIBDIR)/libtinyalsa.so


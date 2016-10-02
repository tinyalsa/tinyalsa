TOPDIR ?= ..
SRCDIR ?= $(TOPDIR)/src
INCDIR ?= $(TOPDIR)/include

DESTDIR ?=
PREFIX ?= /usr/local
CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
override CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -I $(INCDIR) -L $(SRCDIR) -g -O2 $(CFLAGS)

VPATH = $(SRCDIR)

.PHONY: all
all: -ltinyalsa tinyplay tinycap tinymix tinypcminfo

tinyplay: tinyplay.c -ltinyalsa
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

tinycap: tinycap.c
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

tinymix: tinymix.c
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

tinypcminfo: tinypcminfo.c
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

.PHONY: clean
clean:
	rm -f tinyplay tinycap
	rm -f tinymix
	rm -f tinypcminfo

.PHONY: install
install: tinyplay tinycap tinymix tinypcminfo
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -u tinyplay $(DESTDIR)$(PREFIX)/bin/
	cp -u tinycap $(DESTDIR)$(PREFIX)/bin/
	cp -u tinymix $(DESTDIR)$(PREFIX)/bin/
	cp -u tinypcminfo $(DESTDIR)$(PREFIX)/bin/


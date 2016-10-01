TOPDIR ?= ..
SRCDIR ?= $(TOPDIR)
INCDIR ?= $(TOPDIR)/include

DESTDIR ?=
PREFIX ?= /usr/local
CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Wextra -Werror -Wfatal-errors -I $(INCDIR)

LD = $(CROSS_COMPILE)ld
LDFLAGS ?=

VPATH = $(SRCDIR)

.PHONY: all
all: tinyplay tinycap tinymix tinypcminfo

tinyplay: tinyplay.c -ltinyalsa

tinycap: tinycap.c -ltinyalsa

tinymix: tinymix.c -ltinyalsa

tinypcminfo: tinypcminfo.c -ltinyalsa

.PHONY: clean
clean:
	rm -f tinyplay tinycap
	rm -f tinymix
	rm -f tinypcminfo

.PHONY: install
install: tinyplay tinycap tinymix tinypcminfo
	cp -u tinyplay $(PREFIX)/bin/
	cp -u tinycap $(PREFIX)/bin/
	cp -u tinymix $(PREFIX)/bin/
	cp -u tinypcminfo $(PREFIX)/bin/


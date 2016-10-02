DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/man

CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc

WARNINGS = -Wall -Wextra -Werror -Wfatal-errors
INCLUDE_DIRS = -I ../include
LIBRARY_DIRS = -L ../src
override CFLAGS := $(WARNINGS) $(INCLUDE_DIRS) $(LIBRARY_DIRS) -g -O2 $(CFLAGS)

VPATH = ../src:../include/tinyalsa

.PHONY: all
all: -ltinyalsa tinyplay tinycap tinymix tinypcminfo

tinyplay: tinyplay.c pcm.h mixer.h asoundlib.h -ltinyalsa
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

tinycap: tinycap.c pcm.h mixer.h asoundlib.h
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

tinymix: tinymix.c pcm.h mixer.h asoundlib.h
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

tinypcminfo: tinypcminfo.c pcm.h mixer.h asoundlib.h
	$(CC) $(CFLAGS) $< -o $@ -ltinyalsa

.PHONY: clean
clean:
	rm -f tinyplay tinycap
	rm -f tinymix
	rm -f tinypcminfo

.PHONY: install
install: tinyplay tinycap tinymix tinypcminfo
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -u tinyplay $(DESTDIR)$(BINDIR)/
	cp -u tinycap $(DESTDIR)$(BINDIR)/
	cp -u tinymix $(DESTDIR)$(BINDIR)/
	cp -u tinypcminfo $(DESTDIR)$(BINDIR)/
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	cp -u tinyplay.1 $(DESTDIR)$(MANDIR)/man1/
	cp -u tinycap.1 $(DESTDIR)$(MANDIR)/man1/
	cp -u tinymix.1 $(DESTDIR)$(MANDIR)/man1/
	cp -u tinypcminfo.1 $(DESTDIR)$(MANDIR)/man1/


DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/man

CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -I ../include
CFLAGS += -L ../src
CFLAGS += -O2

VPATH = ../src:../include/tinyalsa

.PHONY: all
all: -ltinyalsa tinyplay tinycap tinymix tinypcminfo

tinyplay: tinyplay.c pcm.h mixer.h asoundlib.h libtinyalsa.a

tinycap: tinycap.c pcm.h mixer.h asoundlib.h libtinyalsa.a

tinymix: tinymix.c pcm.h mixer.h asoundlib.h libtinyalsa.a

tinypcminfo: tinypcminfo.c pcm.h mixer.h asoundlib.h libtinyalsa.a

.PHONY: clean
clean:
	rm -f tinyplay tinycap
	rm -f tinymix
	rm -f tinypcminfo

.PHONY: install
install: tinyplay tinycap tinymix tinypcminfo
	install -d $(DESTDIR)$(BINDIR)
	install tinyplay $(DESTDIR)$(BINDIR)/
	install tinycap $(DESTDIR)$(BINDIR)/
	install tinymix $(DESTDIR)$(BINDIR)/
	install tinypcminfo $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(MANDIR)/man1
	install tinyplay.1 $(DESTDIR)$(MANDIR)/man1/
	install tinycap.1 $(DESTDIR)$(MANDIR)/man1/
	install tinymix.1 $(DESTDIR)$(MANDIR)/man1/
	install tinypcminfo.1 $(DESTDIR)$(MANDIR)/man1/


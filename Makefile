export TOPDIR = $(shell pwd)
export SRCDIR = $(TOPDIR)
export INCDIR = $(TOPDIR)/include

export DESTDIR ?=
export PREFIX ?= /usr/local
export CROSS_COMPILE =

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -Werror -Wfatal-errors -I $(INCDIR)

LD = $(CROSS_COMPILE)ld
LDFLAGS =

OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.a
SHLIB = libtinyalsa.so

.PHONY: all
all: $(LIB) $(SHLIB)
	$(MAKE) -C utils

$(SHLIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -shared $(OBJECTS) -o $(SHLIB)

$(LIB): $(OBJECTS)
	$(CROSS_COMPILE)$(AR) rcs $@ $^

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -fPIC -c $^ -o $@

.PHONY: clean
clean:
	-rm $(LIB) $(SHLIB) $(OBJECTS)
	$(MAKE) -C utils clean

.PHONY: install
install: $(LIB) $(SHLIB)
	cp -u $(SHLIB) $(PREFIX)/lib/
	cp -u $(LIB) $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/tinyalsa
	cp -Ru $(INCDIR)/tinyalsa $(PREFIX)/include/
	$(MAKE) -C utils install


CFLAGS ?= -Wall
LDFLAGS ?=
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.a
SHLIB = libtinyalsa.so
CROSS_COMPILE =
PREFIX = /usr/local

.PHONY: all
all: $(LIB) $(SHLIB)
	$(MAKE) -C utils

$(SHLIB): $(OBJECTS)
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) -shared $(OBJECTS) -o $(SHLIB)

$(LIB): $(OBJECTS)
	$(CROSS_COMPILE)$(AR) rcs $@ $^

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -fPIC -c $^ -I$(INC) -o $@

.PHONY: clean
clean:
	-rm $(LIB) $(SHLIB) $(OBJECTS)
	$(MAKE) -C utils clean

.PHONY: install
install: $(LIB) $(SHLIB)
	cp -u $(SHLIB) $(PREFIX)/lib/$(SHLIB)
	cp -u $(LIB) $(PREFIX)/lib/$(LIB)
	mkdir -p $(PREFIX)/include/tinyalsa
	cp -u $(INC)/tinyalsa/asoundlib.h $(PREFIX)/include/tinyalsa/asoundlib.h


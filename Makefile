CFLAGS ?= -Wall
LDFLAGS ?=
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.a
SHLIB = libtinyalsa.so
CROSS_COMPILE =
PREFIX = /usr/local

.PHONY: all
all: $(LIB) $(SHLIB) tinyplay tinycap tinymix tinypcminfo

tinyplay: $(SHLIB) tinyplay.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinyplay.o -L. -ltinyalsa -o tinyplay

tinycap: $(SHLIB) tinycap.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinycap.o -L. -ltinyalsa -o tinycap

tinymix: $(SHLIB) tinymix.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinymix.o -L. -ltinyalsa -o tinymix

tinypcminfo: $(SHLIB) tinypcminfo.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinypcminfo.o -L. -ltinyalsa -o tinypcminfo

$(SHLIB): $(OBJECTS)
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) -shared $(OBJECTS) -o $(SHLIB)

$(LIB): $(OBJECTS)
	$(CROSS_COMPILE)$(AR) rcs $@ $^

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -fPIC -c $^ -I$(INC) -o $@

.PHONY: clean
clean:
	-rm $(LIB) $(SHLIB) $(OBJECTS) tinyplay.o tinyplay tinycap.o tinycap \
	tinymix.o tinymix tinypcminfo.o tinypcminfo

.PHONY: install
install: $(LIB) $(SHLIB)
	cp -u $(SHLIB) $(PREFIX)/lib/$(SHLIB)
	cp -u $(LIB) $(PREFIX)/lib/$(LIB)
	mkdir -p $(PREFIX)/include/tinyalsa
	cp -u $(INC)/tinyalsa/asoundlib.h $(PREFIX)/include/tinyalsa/asoundlib.h


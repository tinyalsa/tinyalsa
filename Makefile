CFLAGS += -c -fPIC -Wall
INC = include
OBJECTS = mixer.o pcm.o
LIBS = libtinyalsa.a libtinyalsa.so
STATIC_LIB = libtinyalsa.a
SHARED_LIB = libtinyalsa.so
CROSS_COMPILE =

all: $(LIB) tinyplay tinycap tinymix tinypcminfo

tinyplay: $(LIBS) tinyplay.o
	$(CROSS_COMPILE)gcc tinyplay.o -L. $(LDFLAGS) -ltinyalsa -o tinyplay

tinycap: $(LIBS) tinycap.o
	$(CROSS_COMPILE)gcc tinycap.o -L. $(LDFLAGS) -ltinyalsa -o tinycap

tinymix: $(LIBS) tinymix.o
	$(CROSS_COMPILE)gcc tinymix.o -L. $(LDFLAGS) -ltinyalsa -o tinymix

tinypcminfo: $(LIBS) tinypcminfo.o
	$(CROSS_COMPILE)gcc tinypcminfo.o -L. $(LDFLAGS) -ltinyalsa -o tinypcminfo

$(STATIC_LIB): $(OBJECTS)
	$(CROSS_COMPILE)ar rcs $(STATIC_LIB) $(OBJECTS)

$(SHARED_LIB): $(OBJECTS)
	$(CROSS_COMPILE)gcc -shared $(OBJECTS) -o $(SHARED_LIB)

.c.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) $< -I$(INC)

clean:
	-rm $(LIBS) $(OBJECTS) tinyplay.o tinyplay tinycap.o tinycap \
	tinymix.o tinymix tinypcminfo.o tinypcminfo

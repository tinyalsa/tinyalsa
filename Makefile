CFLAGS = -c -fPIC
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.so

all: $(LIB) tinyplay tinymix

tinyplay: $(LIB) tinyplay.o
	gcc tinyplay.o -L. -ltinyalsa -o tinyplay

tinymix: $(LIB) tinymix.o
	gcc tinymix.o -L. -ltinyalsa -o tinymix

$(LIB): $(OBJECTS)
	gcc -shared $(OBJECTS) -o $(LIB)

.c.o:
	gcc $(CFLAGS) $< -I$(INC)
	
clean:
	rm $(LIB) $(OBJECTS) tinyplay.o tinyplay

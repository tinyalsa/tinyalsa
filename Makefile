CFLAGS = -c -fPIC
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.so

all: $(LIB) tinyplay

tinyplay: $(LIB) tinyplay.o
	gcc tinyplay.o -L. -ltinyalsa -o tinyplay

$(LIB): $(OBJECTS)
	gcc -shared $(OBJECTS) -o $(LIB)

.c.o:
	gcc $(CFLAGS) $< -I$(INC)
	
clean:
	rm $(LIB) $(OBJECTS) tinyplay.o tinyplay

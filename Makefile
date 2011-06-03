CFLAGS = -c -fPIC
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.so

tinyplay: $(LIB) tinyplay.o
	gcc tinyplay.o -L. -ltinyalsa -o tinyplay

libtinyalsa.so: $(OBJECTS)
	gcc -shared $(OBJECTS) -o $(LIB)

.c.o:
	gcc $(CFLAGS) $< -I$(INC)
	
clean:
	rm $(LIB) $(OBJECTS) tinyplay.o tinyplay

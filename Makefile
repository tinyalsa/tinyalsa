CFLAGS = -c -fPIC
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.so

libtinyalsa.so: $(OBJECTS)
	gcc -shared $(OBJECTS) -o $(LIB)

.c.o:
	gcc $(CFLAGS) $< -I$(INC)
	
clean:
	rm $(LIB) $(OBJECTS)

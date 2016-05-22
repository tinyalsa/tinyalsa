CC ?= gcc
export CC

CFLAGS = -Iinclude -Wall -Wextra -Werror -Wfatal-errors -std=gnu99
export CFLAGS

AR ?= ar
export AR

ARFLAGS = rcs
export ARFLAGS

.PHONY: all
all: libsalsa.a
	${MAKE} -C examples

libsalsa_a_OBJECTS = pcm.o pcm-2.o mixer.o
libsalsa.a: ${libsalsa_a_OBJECTS}
	$(AR) $(ARFLAGS) $@ $(libsalsa_a_OBJECTS)

pcm.o: pcm.c

pcm-2.o: pcm-2.c

mixer.o: mixer.c

.PHONY: clean
clean:
	rm -f ${libsalsa_a_OBJECTS}
	rm -f libsalsa.a
	$(MAKE) -C examples clean


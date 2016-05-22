CC ?= gcc

CFLAGS = -I../include -Wall -Wextra -Werror -Wfatal-errors -std=gnu99

.PHONY: all
all: tinycap tinyplay tinypcminfo tinywavinfo

tinycap: tinycap.c
	$(CC) $(CFLAGS) $< ../libsalsa.a -o $@

tinyplay: tinyplay.c
	$(CC) $(CFLAGS) $< ../libsalsa.a -o $@

tinypcminfo: tinypcminfo.c
	$(CC) $(CFLAGS) $< ../libsalsa.a -o $@

tinywavinfo: tinywavinfo.c
	$(CC) $(CFLAGS) $< ../libsalsa.a -lm -o $@

.PHONY: clean
clean:
	rm -f tinycap tinycap.o
	rm -f tinyplay tinyplay.o
	rm -f tinypcminfo tinypcminfo.o
	rm -f tinywavinfo tinywavinfo.o


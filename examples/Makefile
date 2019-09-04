CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -Werror -Wfatal-errors -I ../include

VPATH = ../src

EXAMPLES += pcm-readi
EXAMPLES += pcm-writei

.PHONY: all
all: $(EXAMPLES)

pcm-readi: pcm-readi.c -ltinyalsa -ldl

pcm-writei: pcm-writei.c -ltinyalsa -ldl

.PHONY: clean
clean:
	rm -f $(EXAMPLES)


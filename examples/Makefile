CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wfatal-errors -I ../include

VPATH = ../src

.PHONY: all
all: pcm-readi pcm-writei

pcm-readi: pcm-readi.c -ltinyalsa

pcm-writei: pcm-writei.c -ltinyalsa


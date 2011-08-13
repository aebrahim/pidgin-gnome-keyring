CC = gcc
TARGET = gnome-keyring

GLIBFLAGS = `pkg-config --libs --cflags glib2.0`
KEYRINGFLAGS = `pkg-config --libs --cflags gnome-keyring-1`
PURPLEFLAGS = `pkg-config --cflags purple`


all: ${TARGET}.so

install: ${TARGET}.so
	cp ${TARGET}.so ~/.purple/plugins/

clean: 
	rm -f ${TARGET}.so

${TARGET}.so: ${TARGET}.c
	${CC} -Wall -I. -g -O2 ${TARGET}.c -o ${TARGET}.so -shared -fPIC -DPIC -ggdb ${PURPLEFLAGS} ${KEYRINGFLAGS}


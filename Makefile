CC = gcc
TARGET = gnome-keyring

CFLAGS = -I/usr/include/libpurple -I/usr/local/include/libpurple -I/usr/include/gnome-keyring-1 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include -I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include -I/usr/local/include 


all:   ${TARGET}.so install

install: ${TARGET}.so
	cp ${TARGET}.so ~/.purple/plugins/

clean: 
	rm -f ${TARGET}.so

${TARGET}.so: ${TARGET}.c
	${CC} ${CFLAGS} -Wall -I. -g -O2 ${TARGET}.c -o ${TARGET}.so -shared -fPIC -DPIC -ggdb -lgnome-keyring


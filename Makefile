TARGET = gnome-keyring

KEYRINGFLAGS = `pkg-config --libs --cflags gnome-keyring-1`
PURPLEFLAGS = `pkg-config --cflags purple`


all: ${TARGET}.so

clean: 
	rm -f ${TARGET}.so

${TARGET}.so: ${TARGET}.c
	${CC} -Wall -I. -g -O2 ${TARGET}.c -o ${TARGET}.so -shared -fPIC -DPIC -ggdb ${PURPLEFLAGS} ${KEYRINGFLAGS}

install: ${TARGET}.so
	mkdir -p ~/.purple/
	cp ${TARGET}.so ~/.purple

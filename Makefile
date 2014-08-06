TARGET = gnome-keyring

KEYRINGFLAGS = `pkg-config --libs --cflags gnome-keyring-1`
PURPLEFLAGS = `pkg-config --cflags purple`
VERSION = $(shell cat VERSION)
ifeq ($(strip $(VERSION)),)
	VERSION = `git describe --tags`
endif

all: ${TARGET}.so

clean: 
	rm -f ${TARGET}.so ${TARGET}_*.tar.gz pidgin-${TARGET}_*.* *.pyc
	rm -rf pidgin-${TARGET}-*

${TARGET}.so: ${TARGET}.c

	${CC} -Wall -I. -g -O2 ${TARGET}.c -o ${TARGET}.so -shared -fPIC -DPIC -ggdb ${PURPLEFLAGS} ${KEYRINGFLAGS} -DVERSION=\"${VERSION}\"

install: ${TARGET}.so
	mkdir -p /usr/lib/purple-2/
	cp ${TARGET}.so /usr/lib/purple-2/

install_local: ${TARGET}.so
	mkdir -p ~/.purple/plugins
	cp ${TARGET}.so ~/.purple/plugins/


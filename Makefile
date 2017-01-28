TARGET = gnome-keyring

SECRETFLAGS = `pkg-config --libs --cflags libsecret-1`
PURPLEFLAGS = `pkg-config --cflags purple`
VERSION = $(shell cat VERSION)
ifeq ($(strip $(VERSION)),)
	VERSION = `git describe --tags`
endif

PLUGINDIRPURPLE:=$(shell pkg-config --variable=plugindir purple)

all: ${TARGET}.so

clean: 
	rm -f ${TARGET}.so ${TARGET}_*.tar.gz pidgin-${TARGET}_*.* *.pyc
	rm -rf pidgin-${TARGET}-*

${TARGET}.so: ${TARGET}.c

	${CC} ${CFLAGS} ${LDFLAGS} -Wall -I. -g -O2 ${TARGET}.c -o ${TARGET}.so -shared -fPIC -DPIC -ggdb ${PURPLEFLAGS} ${SECRETFLAGS} -DVERSION=\"${VERSION}\"

install: ${TARGET}.so
	mkdir -p ${DESTDIR}${PLUGINDIRPURPLE}
	cp ${TARGET}.so ${DESTDIR}${PLUGINDIRPURPLE}

install_local: ${TARGET}.so
	mkdir -p ~/.purple/plugins
	cp ${TARGET}.so ~/.purple/plugins/


TARGET = gnome-keyring

SECRETFLAGS = `pkg-config --libs --cflags libsecret-1`
PURPLEFLAGS = `pkg-config --cflags purple`
VERSION = $(shell cat VERSION)
ifeq ($(strip $(VERSION)),)
	VERSION = `git describe --tags`
endif

all: ${TARGET}.so

clean: 
	rm -f ${TARGET}.so ${TARGET}_*.tar.gz pidgin-${TARGET}_*.* *.pyc
	rm -rf pidgin-${TARGET}-*

${TARGET}.so: ${TARGET}.c keyring_plugin.c keyring_plugin_defs.h keyring_plugin.h

	${CC} ${CFLAGS} ${LDFLAGS} -Wall -I. -g -O2 ${TARGET}.c keyring_plugin.c -o ${TARGET}.so -shared -fPIC -DPIC -ggdb ${PURPLEFLAGS} ${SECRETFLAGS} -DVERSION=\"${VERSION}\"

install: ${TARGET}.so
	mkdir -p ${DESTDIR}/usr/lib/purple-2/
	cp ${TARGET}.so ${DESTDIR}/usr/lib/purple-2/

install_local: ${TARGET}.so
	mkdir -p ~/.purple/plugins
	cp ${TARGET}.so ~/.purple/plugins/


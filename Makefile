# Makefile for Tux Meteor

# Try to autodetect whether X is installed on this machine.
# To build Tux Meteor without X on a machine that has it (or on OS X),
# comment out the next line:
HAS_X11 := $(shell ls /usr/include/X11/Xlib.h >/dev/null 2>&1 && echo 1)

# OS X users: OS X doesn't handle the construct in the last line.
# if you want to build with X11 on OS X, uncomment the next line:
#HAS_X11 := 1

VERSION = 0.4

INSTALLPREFIX = ${DESTDIR}/usr/local

TARFILE = tuxmeteor-$(VERSION).tar.gz

CFLAGS = -O2 -Wall -DVERSION='"$(VERSION)"'

CWD = $(shell pwd)
CWDBASE = $(shell basename `pwd`)

INSTALL = /usr/bin/install -D

ifeq ($(HAS_X11),1)
  LDFLAGS = -L/usr/X11R6/lib -lX11
else
  CFLAGS += -DNO_X11
endif

tuxmeteor: tuxmeteor.o
	cc -o tuxmeteor tuxmeteor.o $(LDFLAGS)

tar: clean $(TARFILE)

$(TARFILE): clean
	( cd .. && \
	  tar czvf $(TARFILE) $(CWDBASE) && \
	  mv $(TARFILE) $(CWD) && \
	  echo Created $(TARFILE) \
	)

install: tuxmeteor
	$(INSTALL) tuxmeteor $(INSTALLPREFIX)/bin/tuxmeteor

clean:
	rm -f *.o tuxmeteor *.tar.gz

include Makefile.inc

#
# Settings
#

LIBNAME = libnmea.so

DESTDIR ?=

MACHINE=$(shell uname -m)
ifeq ($(strip $(MACHINE)),x86_64)
LIBDIR = usr/lib64
else
LIBDIR = usr/lib
endif
INCLUDEDIR = usr/include

MODULES = context generate generator gmath info parse parser sentence time tok util
OBJ = $(MODULES:%=build/%.o)

LIBS = -lm
INCS = -I ./include


#
# Targets
#

all: all-before lib/$(LIBNAME)

remake: clean all

lib/$(LIBNAME): $(OBJ)
	$(CC) -shared -Wl,-soname=$(LIBNAME) -o lib/$(LIBNAME) $(OBJ) -lc

build/%.o: src/%.c Makefile Makefile.inc
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

samples: all
	make -C samples all


#
# Phony Targets
#

.PHONY: all-before clean doc install uninstall

all-before:
	mkdir -p build lib

clean:
	$(MAKE) -C doc clean
	$(MAKE) -C samples clean
	rm -fr build lib

doc:
	$(MAKE) -C doc all

install: all
	mkdir -p $(DESTDIR)/$(LIBDIR) $(DESTDIR)/$(INCLUDEDIR)
	cp lib/$(LIBNAME) $(DESTDIR)/$(LIBDIR)/$(LIBNAME).$(VERSION)
	$(STRIP) $(DESTDIR)/$(LIBDIR)/$(LIBNAME).$(VERSION)
	ln -sf $(LIBNAME).$(VERSION) $(DESTDIR)/$(LIBDIR)/$(LIBNAME)
	ldconfig -n $(DESTDIR)/$(LIBDIR)
	rm -fr $(DESTDIR)/$(INCLUDEDIR)/nmea
	cp -a include/nmea $(DESTDIR)/$(INCLUDEDIR)

uninstall:
	rm -fr $(DESTDIR)/$(INCLUDEDIR)/nmea
	rm -f $(DESTDIR)/$(LIBDIR)/$(LIBNAME) $(DESTDIR)/$(LIBDIR)/$(LIBNAME).$(VERSION)
	ldconfig -n $(DESTDIR)/$(LIBDIR)
	rmdir -p --ignore-fail-on-non-empty $(DESTDIR)/$(LIBDIR) $(DESTDIR)/$(INCLUDEDIR)

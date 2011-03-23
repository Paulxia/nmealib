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

MODULES = context generate generator gmath info parse parser sentence time tok
OBJ = $(MODULES:%=build/%.o)

LIBS = -lm
INCS = -I ./include


#
# Targets
#

all: all-before lib/$(LIBNAME)

remake: clean all

lib/$(LIBNAME): $(OBJ) Makefile
	$(CC) -shared -Wl,-soname=$(LIBNAME) -o lib/$(LIBNAME) $(OBJ) -lc

build/%.o: src/%.c
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

samples: all
	make -C samples all


#
# Phony Targets
#

.PHONY: all-before clean doc install uninstall

all-before:
	mkdir -p build

clean:
	$(MAKE) -C doc clean
	$(MAKE) -C samples clean
	rm -fr build lib/$(LIBNAME)

doc:
	$(MAKE) -C doc all

install: all
ifneq ($(strip $(DESTDIR)),)
	mkdir -p $(DESTDIR)/{$(LIBDIR),$(INCLUDEDIR)}
endif
	cp lib/$(LIBNAME) $(DESTDIR)/$(LIBDIR)/$(LIBNAME).$(VERSION)
ifeq ($(strip $(DESTDIR)),)
	ldconfig /$(LIBDIR)
endif
	rm -fr $(DESTDIR)/$(INCLUDEDIR)/nmea
	cp -a include/nmea $(DESTDIR)/$(INCLUDEDIR)

uninstall:
	rm -fr $(DESTDIR)/$(INCLUDEDIR)/nmea
	rm -f $(DESTDIR)/$(LIBDIR)/$(LIBNAME).$(VERSION)
ifeq ($(strip $(DESTDIR)),)
	rm -f /$(LIBDIR)/$(LIBNAME)
	ldconfig /$(LIBDIR)
endif

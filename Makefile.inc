GITVERSION = $(shell git describe --dirt='-dirty')

VERSION=$(shell git describe | \
          sed -r 's/.*([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+).*/\1/')

CC = gcc
LIBNAME = libnmea.a
LIBDIR = /usr/lib
INCLUDEDIR = /usr/include
MODULES = context generate generator gmath info parse parser sentence time tok
SAMPLES = generate generator math parse parse_file

BIN = lib/$(LIBNAME)
OBJ = $(MODULES:%=build/nmea_gcc/%.o)

SMPLS = $(SAMPLES:%=samples_%)
SMPLOBJ = $(SAMPLES:%=samples/%/main.o)

CCFLAGS += -fPIC -O2 -Wall
INCS = -I ./include
LIBS = -lm -Llib -lnmea

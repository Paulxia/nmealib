CC = gcc
LIBNAME = libnmea.a
LIBDIR = /usr/lib
INCLUDEDIR = /usr/include
MODULES = generate generator parse parser tok context time info gmath sentence
SAMPLES = generate generator parse parse_file math

BIN = lib/$(LIBNAME)
OBJ = $(MODULES:%=build/nmea_gcc/%.o)
LINKOBJ = $(OBJ) $(RES)

SMPLS = $(SAMPLES:%=samples_%)
SMPLOBJ = $(SAMPLES:%=samples/%/main.o)

CCFLAGS += -fPIC -O2 -Wall
INCS = -I include
LIBS = -lm -Llib -lnmea

VERSION = $(shell git describe)

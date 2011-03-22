GITVERSION = $(shell git describe --dirt='-dirty')

# we expect the version to be like 'v0.5.3-27-g0c2727a' and then strip the 'v',
# and the '-27-g0c2727a' parts
VERSION=$(shell git describe | \
          sed -r -e 's/^([^[:digit:]]+)(.*)/\2/' \
                 -e 's/^([^-]+)(.*)/\1/' | \
          grep -E '[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+')

# protect against no version number
ifeq ($(strip $(VERSION)),)
VERSION=0.0.0
endif

CC = gcc
LIBNAME = libnmea.a
LIBDIR = /usr/lib
INCLUDEDIR = /usr/include
MODULES = context generate generator gmath info parse parser sentence time tok
SAMPLES = generate generator math parse parse_file

OBJ = $(MODULES:%=build/nmea_gcc/%.o)

SMPLS = $(SAMPLES:%=samples_%)
SMPLOBJ = $(SAMPLES:%=samples/%/main.o)

CCFLAGS += -fPIC -O2 -Wall
INCS = -I ./include
LIBS = -lm -Llib -lnmea

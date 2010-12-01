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
 
.PHONY: all all-before all-after clean clean-custom doc install uninstall
 
all: all-before $(BIN) samples all-after 

all-before:
	mkdir -p build/nmea_gcc

clean: clean-custom 
	rm -f $(LINKOBJ) $(BIN) $(SMPLOBJ) $(SMPLS)

doc:
	$(MAKE) -C doc
	
remake: clean all

$(BIN): $(LINKOBJ)
	ar rsc $@ $^
	ranlib $@

build/nmea_gcc/%.o: src/%.c 
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

samples: $(SMPLS)

samples_%: samples/%/main.o
	$(CC) $(CCFLAGS) $< $(LIBS) -o build/$@

samples/%/main.o: samples/%/main.c
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

install: all
	cp $(BIN) $(LIBDIR)/$(LIBNAME)
	rm -fr $(INCLUDEDIR)/nmea
	cp -a include/nmea $(INCLUDEDIR)

uninstall:
	rm -fr $(INCLUDEDIR)/nmea
	rm -f $(LIBDIR)/$(LIBNAME)
	

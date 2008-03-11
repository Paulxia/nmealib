CC = gcc 
 
BIN = lib/libnmea.a 
MODULES = generate generator parse parser tok context time info gmath sentence  
SAMPLES = generate generator parse parse_file math
 
OBJ = $(MODULES:%=build/nmea_gcc/%.o) 
LINKOBJ = $(OBJ) $(RES)

SMPLS = $(SAMPLES:%=samples_%)
SMPLOBJ = $(SAMPLES:%=samples/%/main.o)

INCS = -I include 
LIBS = -lm -Llib -lnmea
 
.PHONY: all all-before all-after clean clean-custom doc
 
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
	$(CC) $(INCS) -c $< -o $@

samples: $(SMPLS)

samples_%: samples/%/main.o
	$(CC) $< $(LIBS) -o build/$@

samples/%/main.o: samples/%/main.c
	$(CC) $(INCS) -c $< -o $@

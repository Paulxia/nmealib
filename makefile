CC = gcc 
 
BIN = lib/libnmea.a 
MODULES = generate generator parse parser tok context time info math sentence  
 
OBJ = $(MODULES:%=build/nmea_gcc/%.o) 
LINKOBJ = $(OBJ) $(RES)
INCS = -I include 
 
.PHONY: all all-before all-after clean clean-custom doc
 
all: all-before $(BIN) all-after 

all-before:
	mkdir -p build/nmea_gcc

clean: clean-custom 
	rm -f $(LINKOBJ) $(BIN) 

doc:
	$(MAKE) -C doc

$(BIN): $(LINKOBJ)
	ar r $(BIN) $(LINKOBJ)
	ranlib $(BIN)

build/nmea_gcc/%.o: src/%.c 
	$(CC) $(INCS) -c $< -o $@


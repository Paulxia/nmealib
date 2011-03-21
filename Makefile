include Makefile.inc

.PHONY: all all-before all-after clean clean-custom doc install uninstall
 
all: all-before $(BIN) samples all-after 

all-before:
	mkdir -p build/nmea_gcc

clean: clean-custom 
	$(MAKE) -C doc clean
	rm -fr build $(LINKOBJ) $(BIN) $(SMPLOBJ) $(SMPLS)

doc:
	$(MAKE) -C doc all
	
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
	

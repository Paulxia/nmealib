include Makefile.inc

all: all-before $(BIN) samples all-after 

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


.PHONY: all-before all-after clean doc install uninstall

all-before:
	mkdir -p build/nmea_gcc

clean:
	$(MAKE) -C doc clean
	rm -fr build $(LINKOBJ) $(BIN) $(SMPLOBJ) $(SMPLS)

doc:
	$(MAKE) -C doc all

install: all
	cp $(BIN) $(LIBDIR)/$(LIBNAME)
	rm -fr $(INCLUDEDIR)/nmea
	cp -a include/nmea $(INCLUDEDIR)

uninstall:
	rm -fr $(INCLUDEDIR)/nmea
	rm -f $(LIBDIR)/$(LIBNAME)
	

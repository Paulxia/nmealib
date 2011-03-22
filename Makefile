include Makefile.inc

all: all-before lib/$(LIBNAME) all-after 

remake: clean all

lib/$(LIBNAME): $(OBJ) Makefile
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
	rm -fr build $(OBJ) lib/$(LIBNAME) $(SMPLOBJ) $(SMPLS)

doc:
	$(MAKE) -C doc all

install: all
ifneq ($(strip $(DESTDIR)),)
	mkdir -p $(DESTDIR)/{$(LIBDIR),$(INCLUDEDIR)}
endif
	cp lib/$(LIBNAME) $(DESTDIR)/$(LIBDIR)/$(LIBNAME)
	rm -fr $(DESTDIR)/$(INCLUDEDIR)/nmea
	cp -a include/nmea $(DESTDIR)/$(INCLUDEDIR)

uninstall:
	rm -fr $(DESTDIR)/$(INCLUDEDIR)/nmea
	rm -f $(DESTDIR)/$(LIBDIR)/$(LIBNAME)

CPP  = g++
CC   = gcc
OBJ  = build/generate.o build/generator.o build/parse.o build/parser.o build/tok.o build/context.o build/time.o build/info.o build/math.o build/sentence.o $(RES)
LINKOBJ  = build/generate.o build/generator.o build/parse.o build/parser.o build/tok.o build/context.o build/time.o build/info.o build/math.o build/sentence.o $(RES)
LIBS =  
INCS =  -I"include" 
CXXINCS =  -I"include" 
BIN  = lib/nmea.a
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  

.PHONY: all all-before all-after clean clean-custom

all: all-before lib/nmea.a all-after


clean: clean-custom
	rm -f $(OBJ) $(BIN)

$(BIN): $(LINKOBJ)
	ar r $(BIN) $(LINKOBJ)
	ranlib $(BIN)

build/generate.o: src/generate.c
	$(CC) -c src/generate.c -o build/generate.o $(CFLAGS)

build/generator.o: src/generator.c
	$(CC) -c src/generator.c -o build/generator.o $(CFLAGS)

build/parse.o: src/parse.c
	$(CC) -c src/parse.c -o build/parse.o $(CFLAGS)

build/parser.o: src/parser.c
	$(CC) -c src/parser.c -o build/parser.o $(CFLAGS)

build/tok.o: src/tok.c
	$(CC) -c src/tok.c -o build/tok.o $(CFLAGS)

build/context.o: src/context.c
	$(CC) -c src/context.c -o build/context.o $(CFLAGS)

build/time.o: src/time.c
	$(CC) -c src/time.c -o build/time.o $(CFLAGS)

build/info.o: src/info.c
	$(CC) -c src/info.c -o build/info.o $(CFLAGS)

build/math.o: src/math.c
	$(CC) -c src/math.c -o build/math.o $(CFLAGS)

build/sentence.o: src/sentence.c
	$(CC) -c src/sentence.c -o build/sentence.o $(CFLAGS)

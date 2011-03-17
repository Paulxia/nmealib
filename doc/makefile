.PHONY: all doc clean doc-clean

VERSION = $(shell git describe)

all: doc

doc: nmea.doxygen
	@echo "Updating version..."
	@sed -r "s/^([[:space:]]*PROJECT_NUMBER[[:space:]]*=).*/\1 $(VERSION)/" $^ > $^.temp
	@echo "Generating HTML and man pages..."
	@doxygen $^.temp
	@rm $^.temp
	@echo "Generating PDF..."
	@make -C latex -s > /dev/null 2>&1
	@mv latex/refman.pdf nmealib.pdf
	@rm -fr latex
	@echo "Done"

clean: doc-clean

doc-clean:
	rm -fr html latex man nmealib.pdf

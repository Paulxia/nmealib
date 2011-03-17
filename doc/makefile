.PHONY: all doc clean doc-clean

all: doc

doc: nmea.doxygen
	@echo "Generating HTML..."
	@doxygen $^
	@echo "Generating PDF..."
	@make -C latex -s > /dev/null 2>&1
	@mv latex/refman.pdf nmealib.pdf
	@rm -fr latex
	@echo "Done"

clean: doc-clean

doc-clean:
	rm -fr html latex man nmealib.pdf

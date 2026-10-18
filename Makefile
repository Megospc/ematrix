.POSIX:

.PHONY: all clean test install uninstall
all: build/ematrix
clean:
	rm -rf build
test: build/ematrix
	if [ -f test.conf ]; then \
		./build/ematrix test.conf; \
	else \
		./build/ematrix; \
	fi
install: build/ematrix
	cp build/ematrix /usr/local/bin
	chmod 755 /usr/local/bin/ematrix
uninstall:
	rm -f /usr/local/bin/ematrix

build/:
	mkdir build

build/font.o: src/font.png build/
	$(LD) -r -b binary src/font.png -o build/font.o -flto=auto -s

build/ematrix: src/ematrix.c src/config.c build/font.o build/
	if [ "$$DEBUG" = "1" ]; then \
		F="-fsanitize=undefined,address -Og -g3"; \
	else \
		F="-DNDEBUG -ffast-math -flto=auto -s"; \
		if [ "$$SMALL" = "1" ]; then \
			F+=" -Os"; \
		else \
			F+=" -O3"; \
		fi; \
	fi; \
	$(CC) $(CFLAGS) $$F -Wall -Wextra src/ematrix.c build/font.o \
		-lglfw -lm -o build/ematrix; \
	if [ "$$DOUPX" = "1" ]; then \
		upx --best build/ematrix; \
	fi

EMBED ?= $(wildcard embed/*)

ifeq ($(DEBUG),1)
	CFLAGS += -fsanitize=undefined,address -Og -g3
else
	CFLAGS += -DNDEBUG -ffast-math -flto=auto -s
	ifeq ($(SMALL),1)
		CFLAGS += -Os
	else
		CFLAGS += -O3
	endif
endif

ifneq ($(DO_UPX),)
	UPX_LEVEL ?= 9
	UPX_COMMAND := $(DO_UPX) -$(UPX_LEVEL) build/ematrix
else
	UPX_COMMAND :=
endif

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

build:
	mkdir build

build/embed.o: $(EMBED) embed | build
	cd embed; \
	$(LD) -r $(patsubst embed/%,-b binary ./%,$(EMBED)) -o ../$@ -s; \
	EXT=""; \
	ARR="static const embed_def_t EMBED[] =\n{\n"; \
	for f in $(patsubst embed/%,%,$(EMBED)); do \
		F="$${f##*/}"; \
		while [ "$$F" != "$${F%.*}" ]; do \
			F="$${F%.*}_$${F##*.}"; \
		done; \
		while [ "$$F" != "$${F%-*}" ]; do \
			F="$${F%-*}_$${F##*-}"; \
		done; \
		while [ "$$F" != "$${F%=*}" ]; do \
			F="$${F%=*}_$${F##*=}"; \
		done; \
		F="_binary___$$F" \
		EXT+="extern const uint8_t $${F}_start[];\n"; \
		EXT+="extern const uint8_t $${F}_end[];\n\n"; \
		ARR+="    { \"$$f\", $${F}_start, $${F}_end },\n"; \
	done; \
	ARR+="};"; \
	echo -e "$$EXT$$ARR" > ../build/embed.c

build/ematrix: src/ematrix.c src/config.c build/embed.o | build
	$(CC) $(CFLAGS) -Wall -Wextra src/ematrix.c build/embed.o \
		-lglfw -lm -o build/ematrix; \
	$(UPX_COMMAND) 

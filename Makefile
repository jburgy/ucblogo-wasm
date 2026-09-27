CC = emcc
UCBLOGO = ucblogo
PTY = node_modules/xterm-pty/emscripten-pty.js

SOURCES = coms.c error.c eval.c files.c graphics.c init.c intern.c lists.c \
	logodata.c main.c math.c mem.c paren.c parse.c print.c term.c \
	turtleshape.c wrksp.c
OBJECTS = $(addprefix build/,$(SOURCES:.c=.o)) build/wasmgraphics.o build/termcap.o

CFLAGS = -O2 -DHAVE_CONFIG_H -Iwasm -I$(UCBLOGO) -Wno-write-strings \
	-Wno-unused-variable -Wno-parentheses -Wno-dangling-else
LOCATIONS = -DLIBLOC='"$(1)/logolib"' -DHELPLOC='"$(1)/helpfiles"' \
	-DCSLSLOC='"$(1)/csls"' -DTEMPLOC='"/tmp"'

# The GC scans the C stack for NODE pointers, but optimized wasm keeps most
# of them in locals where it can't see them: spill-pointers puts them back
# (without it the unit tests die with "Fatal Internal Error"), and needs
# --profiling-funcs to find the stack pointer.
# GROWABLE_ARRAYBUFFERS=0: Chrome's TextDecoder rejects views of resizable
# buffers, which breaks UTF8ToString.
LDFLAGS = -O2 --profiling-funcs -sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 \
	-sBINARYEN_EXTRA_PASSES=--spill-pointers \
	-sALLOW_MEMORY_GROWTH -sSTACK_SIZE=1048576 -sEXIT_RUNTIME \
	-sGROWABLE_ARRAYBUFFERS=0 \
	-sMODULARIZE -sEXPORT_ES6 -sEXPORT_NAME=createLogo

PRELOAD = --preload-file $(UCBLOGO)/logolib@/logolib \
	--preload-file $(UCBLOGO)/helpfiles@/helpfiles \
	--preload-file lib@/lib \
	--exclude-file '*Makefile.am'

WEB = dist/index.html dist/logo.mjs dist/xterm.js dist/xterm.css dist/xterm-pty.mjs

all : $(WEB)

dist/logo.mjs : $(OBJECTS) build/libloc.o $(PTY) $(wildcard lib/*)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -sENVIRONMENT=web -sFORCE_FILESYSTEM --js-library=$(PTY) \
		-sEXPORTED_RUNTIME_METHODS=FS $(PRELOAD) -o $@ $(OBJECTS) build/libloc.o

# Reads stdin and the real file system; used to run the upstream unit tests.
build/logo-node.mjs : $(OBJECTS) build/libloc-node.o
	$(CC) $(LDFLAGS) -sENVIRONMENT=node -sNODERAWFS -o $@ $^

build/%.o : $(UCBLOGO)/%.c wasm/config.h wasm/wasmgraphics.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o : wasm/%.c wasm/config.h wasm/wasmgraphics.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

build/libloc.o : $(UCBLOGO)/libloc.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(call LOCATIONS,) -c -o $@ $<

build/libloc-node.o : $(UCBLOGO)/libloc.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(call LOCATIONS,$(CURDIR)/$(UCBLOGO)) -c -o $@ $<

dist/index.html : web/index.html
dist/xterm.js : node_modules/@xterm/xterm/lib/xterm.js
dist/xterm.css : node_modules/@xterm/xterm/css/xterm.css
dist/xterm-pty.mjs : node_modules/xterm-pty/index.mjs

dist/index.html dist/xterm.js dist/xterm.css dist/xterm-pty.mjs :
	@mkdir -p $(@D)
	cp $< $@

node_modules/% : package-lock.json
	npm ci

# NODERAWFS crashes on TIOCGWINSZ when stdout is a tty, hence the cat.
test : build/logo-node.mjs
	cd $(UCBLOGO)/tests && printf 'RunAll\nbye\n' | node ../../test/run.mjs UnitTests.lg | cat

clean :
	rm -rf build dist

.PHONY : all test clean

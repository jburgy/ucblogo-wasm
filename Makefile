CC = emcc
UCBLOGO = ucblogo
PTY = node_modules/xterm-pty/emscripten-pty.js
VPATH = $(UCBLOGO) wasm

SOURCES = coms.c error.c eval.c files.c graphics.c init.c intern.c libloc.c \
	lists.c logodata.c main.c math.c mem.c paren.c parse.c print.c term.c \
	turtleshape.c wrksp.c wasmgraphics.c termcap.c
OBJECTS = $(addprefix build/,$(SOURCES:.c=.o))

CFLAGS = -O2 -include config.h -Iwasm -I$(UCBLOGO) \
	-Wno-write-strings -Wno-unused-variable

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

dist/logo.mjs : $(OBJECTS) $(PTY) $(wildcard lib/*)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -sENVIRONMENT=web -sFORCE_FILESYSTEM --js-library=$(PTY) \
		-sEXPORTED_RUNTIME_METHODS=FS $(PRELOAD) -o $@ $(OBJECTS)

# Reads stdin and the real file system; used to run the upstream unit tests.
build/logo-node.mjs : $(OBJECTS)
	$(CC) $(LDFLAGS) -sENVIRONMENT=node -sNODERAWFS -sEXPORTED_RUNTIME_METHODS=ENV \
		-o $@ $^

build/%.o : %.c wasm/config.h wasm/wasmgraphics.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

dist/index.html : web/index.html
dist/xterm.js : node_modules/@xterm/xterm/lib/xterm.js
dist/xterm.css : node_modules/@xterm/xterm/css/xterm.css
dist/xterm-pty.mjs : node_modules/xterm-pty/index.mjs

dist/index.html dist/xterm.js dist/xterm.css dist/xterm-pty.mjs :
	@mkdir -p $(@D)
	cp $< $@

# tee keeps NODERAWFS off a real tty, where it crashes on TIOCGWINSZ.
# The OOP tests need --enable-objects, which the native build lacks too.
test : build/logo-node.mjs
	cd $(UCBLOGO)/tests && printf 'RunAll\nbye\n' | node ../../test/run.mjs UnitTests.lg \
		| tee ../../build/test.log
	grep -q 'All tests completed' build/test.log
	! grep -v '^Tests.OOP' build/test.log | grep -E '(Failed|Error)$$'

clean :
	rm -rf build dist

.PHONY : all test clean

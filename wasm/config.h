/* Hand-written config.h for the Emscripten build (no autoconf), included
   into every file with -include. */

#define HAVE_GETTIMEOFDAY 1
#define HAVE_MEMCPY 1
#define HAVE_SRANDOM 1
#define HAVE_STRING_H 1
#define HAVE_TERMCAP_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_UNISTD_H 1
#define STDC_HEADERS 1
#define X_DISPLAY_MISSING 1

#define VERSION "6.2.5"

/* Where the page preloads them; override with $LOGOLIB, $LOGOHELP, $CSLS */
#define LIBLOC "/logolib"
#define HELPLOC "/helpfiles"
#define CSLSLOC "/csls"
#define TEMPLOC "/tmp"

/* Defines TIOCSTI, which keeps setjmp/longjmp (and stale jmp_bufs on ^C)
   out of the input path; glibc gets it through <termio.h>. */
#include <sys/ioctl.h>
/* mem.c relies on getting jmp_buf from globals.h, which only includes
   <setjmp.h> without TIOCSTI */
#include <setjmp.h>

/* Hooks implemented in wasmgraphics.c */
void wasm_poll(void);
void wasm_sleep(unsigned int ms);
void wasm_turtle_moved(void);

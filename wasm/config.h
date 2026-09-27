/* Hand-written config.h for the Emscripten build (no autoconf). */

#define HAVE_GETTIMEOFDAY 1
#define HAVE_MEMCPY 1
#define HAVE_SRANDOM 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TERMCAP_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_UNISTD_H 1
#define HAVE_USLEEP 1
#define STDC_HEADERS 1
#define X_DISPLAY_MISSING 1

#define PACKAGE "ucblogo"
#define PACKAGE_NAME "ucblogo"
#define PACKAGE_VERSION "6.2.5"
#define VERSION "6.2.5"

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

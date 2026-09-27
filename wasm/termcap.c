/* Just enough termcap for term.c, hardwired to xterm (which xterm.js emulates). */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "termcap.h"

/* UCBLogo counts on glibc flushing stdout before it reads stdin (to show
   prompts), which musl doesn't do. */
__attribute__((constructor))
static void unbuffer_stdout(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
}

int tgetent(char *bp, const char *name) {
    return 1;
}

int tgetnum(const char *id) {
    struct winsize ws;

    if (!isatty(1) || ioctl(1, TIOCGWINSZ, &ws) != 0) return -1;
    if (!strcmp(id, "co")) return ws.ws_col;
    if (!strcmp(id, "li")) return ws.ws_row;
    return -1;
}

char *tgetstr(const char *id, char **area) {
    if (!strcmp(id, "cl")) return "\033[H\033[2J";
    if (!strcmp(id, "cm")) return "cm";
    if (!strcmp(id, "so")) return "\033[7m";
    if (!strcmp(id, "se")) return "\033[27m";
    return NULL;
}

char *tgoto(const char *cap, int col, int row) {
    static char buf[32];

    snprintf(buf, sizeof buf, "\033[%d;%dH", row + 1, col + 1);
    return buf;
}

int tputs(const char *str, int affcnt, int (*putc)(int)) {
    if (str == NULL) return -1;
    while (*str) putc(*str++);
    return 0;
}

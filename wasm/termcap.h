/* Just enough termcap for term.c, hardwired to xterm. */

int tgetent(char *bp, const char *name);
int tgetnum(const char *id);
char *tgetstr(const char *id, char **area);
char *tgoto(const char *cap, int col, int row);
int tputs(const char *str, int affcnt, int (*putc)(int));

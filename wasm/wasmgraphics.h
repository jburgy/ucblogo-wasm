/* Canvas graphics for UCBLogo compiled with Emscripten.
 *
 * Lines are rasterized into a framebuffer in linear memory, which the page
 * copies to a <canvas> once per animation frame.  The turtle is not XORed
 * into the framebuffer (see draw_turtle in graphics.c); the page draws it on
 * top of each frame instead.
 */

#include <stdint.h>

#define GR_SIZE 60000

#define NUMCOLORS 512
#define NUMINITCOLORS 16

#define prepare_to_draw nop()
#define done_drawing nop()

#define prepare_to_draw_turtle nop()
#define done_drawing_turtle nop()

/* Smaky 100 resolution */
#define screen_width 640
#define screen_height 400

#define screen_left 0
#define screen_right (screen_width-1)
#define screen_top 0
#define screen_bottom (screen_height-1)

#define screen_x_center (screen_left + (screen_width)/2)
#define screen_y_center (screen_top + (screen_height)/2)

#define turtle_left_max ((screen_left) - (screen_x_center))
#define turtle_right_max ((screen_right) - (screen_x_center))
#define turtle_top_max ((screen_y_center) - (screen_top))
#define turtle_bottom_max ((screen_y_center) - (screen_bottom))

#define screen_x_coord ((screen_x_center) + turtle_x)
#define screen_y_coord ((screen_y_center) - turtle_y)

#define PM_PAINT 0
#define PM_ERASE 1
#define PM_REVERSE 2

/* pen_info is a stucture type with fields for the various
   pen characteristics including the location, size, color,
   mode (e.g. XOR or COPY), pattern, visibility (0 = visible) */

typedef struct {
    int color;
    int xpos;
    int ypos;
    int vis;
    int pw;
    int ph;
    int pm;
} pen_info;

extern pen_info wasm_pen;
extern int back_ground;

void wasm_clear(void);
void wasm_line_to(int x, int y);
void wasm_label(char *s);
void wasm_set_pen_color(int c);
void wasm_set_back_ground(int c);
void save_pen(pen_info *p);
void restore_pen(pen_info *p);
void logofill(void);
void set_palette(int, unsigned int, unsigned int, unsigned int);
void get_palette(int, unsigned int *, unsigned int *, unsigned int *);
void nop();

#define clear_screen wasm_clear()
#define erase_screen() wasm_clear()

#define line_to(a,b) wasm_line_to((a),(b))
#define move_to(a,b) (wasm_pen.xpos=(a), wasm_pen.ypos=(b))
#define draw_string(s) wasm_label(s)
#define label(s) wasm_label(s)

#define set_pen_vis(v) (wasm_pen.vis=(v))
#define set_pen_mode(m) (wasm_pen.pm=(m))
#define set_pen_color(c) wasm_set_pen_color(c)
#define set_pen_width(w) (wasm_pen.pw=(w))
#define set_pen_height(h) (wasm_pen.ph=(h))
#define set_pen_x(x) nop()
#define set_pen_y(y) nop()
#define set_back_ground(c) wasm_set_back_ground(c)

#define p_info_x(p) (p.xpos)
#define p_info_y(p) (p.ypos)

#define pen_width wasm_pen.pw
#define pen_height wasm_pen.ph
#define pen_color wasm_pen.color
#define pen_mode wasm_pen.pm
#define pen_vis wasm_pen.vis
#define pen_x (wasm_pen.xpos)
#define pen_y (wasm_pen.ypos)
#define get_node_pen_pattern (cons(make_intnode(-1), NIL))

#define pen_reverse (pen_mode=PM_REVERSE)
#define pen_erase (pen_mode=PM_ERASE)
#define pen_down (pen_mode=PM_PAINT)
#define plain_xor_pen() pen_reverse

#define button FALSE
#define mouse_x 0
#define mouse_y 0

#define full_screen nop()
#define split_screen nop()
#define text_screen nop()

#define tone(p,d) nop()
#define get_pen_pattern(p) nop()
#define set_pen_pattern(p) nop()
#define set_list_pen_pattern(p) nop()

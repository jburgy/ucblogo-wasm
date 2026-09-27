/* Canvas graphics for UCBLogo compiled with Emscripten. */

#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

#include "logo.h"
#include "wasmgraphics.h"
#include "globals.h"
#include "turtleshape.h"

char *LogoPlatformName = "Wasm";

pen_info wasm_pen = {7, 0, 0, 0, 1, 1, PM_PAINT};
int back_ground = 0;

static uint32_t fb[screen_width * screen_height];
static unsigned int palette[SPECIAL_COLORS + NUMCOLORS][3];
static uint32_t pen_rgba;

/* Read by the page on every animation frame. */
static int dirty = 1;
static struct turtle_shape_point turtle_points[MAX_TURTLE_SHAPE_POINTS];
static int turtle_count;

/* Same 16 colors as the wxWidgets version, per the UCBLogo manual */
static const unsigned char initial_colors[NUMINITCOLORS][3] = {
    {0, 0, 0}, {0, 0, 255}, {0, 255, 0}, {0, 255, 255},
    {255, 0, 0}, {255, 0, 255}, {255, 255, 0}, {255, 255, 255},
    {155, 96, 59}, {197, 136, 18}, {100, 162, 64}, {120, 187, 187},
    {255, 149, 119}, {144, 113, 208}, {255, 163, 0}, {183, 183, 183},
};

void nop() {
}

static unsigned int *slot(int n) {
    if (n >= NUMCOLORS) n %= NUMCOLORS;
    if (n < -SPECIAL_COLORS) n = 7;
    return palette[SPECIAL_COLORS + n];
}

static uint32_t rgba(int n) {
    unsigned int *p = slot(n);
    return 0xff000000u | (p[2] >> 8) << 16 | (p[1] >> 8) << 8 | (p[0] >> 8);
}

void set_palette(int n, unsigned int r, unsigned int g, unsigned int b) {
    unsigned int *p = slot(n);
    p[0] = r;
    p[1] = g;
    p[2] = b;
}

void get_palette(int n, unsigned int *r, unsigned int *g, unsigned int *b) {
    unsigned int *p = slot(n);
    *r = p[0];
    *g = p[1];
    *b = p[2];
}

void save_pen(pen_info *p) {
    memcpy(p, &wasm_pen, sizeof(pen_info));
}

void restore_pen(pen_info *p) {
    memcpy(&wasm_pen, p, sizeof(pen_info));
    pen_rgba = rgba(wasm_pen.color);
}

void wasm_set_pen_color(int c) {
    wasm_pen.color = c % NUMCOLORS;
    pen_rgba = rgba(wasm_pen.color);
    dirty = 1;
}

void wasm_set_back_ground(int c) {
    back_ground = c % NUMCOLORS;
    redraw_graphics();
}

void wasm_clear(void) {
    uint32_t bg = rgba(back_ground);
    for (int i = 0; i < screen_width * screen_height; i++) fb[i] = bg;
    dirty = 1;
}

static void put(int x, int y, uint32_t color) {
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;
    uint32_t *p = &fb[y * screen_width + x];
    switch (wasm_pen.pm) {
        case PM_ERASE: *p = rgba(back_ground); break;
        case PM_REVERSE: *p ^= color & 0xffffffu; break;
        default: *p = color;
    }
}

static void plot(int x, int y) {
    /* XOR must touch each pixel once, so reverse mode ignores pen size */
    int w = wasm_pen.pm == PM_REVERSE ? 1 : wasm_pen.pw;
    int h = wasm_pen.pm == PM_REVERSE ? 1 : wasm_pen.ph;

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            put(x + i - w / 2, y + j - h / 2, pen_rgba);
}

void wasm_line_to(int x, int y) {
    if (wasm_pen.vis == 0) {
        int x0 = wasm_pen.xpos, y0 = wasm_pen.ypos;
        int dx = abs(x - x0), sx = x0 < x ? 1 : -1;
        int dy = -abs(y - y0), sy = y0 < y ? 1 : -1;
        int err = dx + dy;

        for (;;) {
            plot(x0, y0);
            if (x0 == x && y0 == y) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        dirty = 1;
    }
    wasm_pen.xpos = x;
    wasm_pen.ypos = y;
}

#define LABEL_HEIGHT 20

/* Renders s into mask, one byte per pixel, and returns its width */
EM_JS(int, js_label, (const char *s, unsigned char *mask, int max_width,
                      int height, int *ascent), {
    const text = UTF8ToString(s);
    const canvas = Module.labelCanvas ||= new OffscreenCanvas(1, 1);
    const ctx = canvas.getContext("2d", { willReadFrequently: true });
    ctx.font = "16px monospace";
    const metrics = ctx.measureText(text);
    const w = Math.min(max_width, Math.ceil(metrics.width));
    if (!w) return 0;
    const top = Math.ceil(metrics.fontBoundingBoxAscent);
    canvas.width = w;
    canvas.height = height;
    ctx.font = "16px monospace";
    ctx.fillStyle = "#fff";
    ctx.fillText(text, 0, top);
    const rgba = ctx.getImageData(0, 0, w, height).data;
    for (let i = 0; i < w * height; i++) HEAPU8[mask + i] = rgba[4 * i + 3] >= 128;
    HEAP32[ascent >> 2] = top;
    return w;
});

void wasm_label(char *s) {
    static unsigned char mask[screen_width * LABEL_HEIGHT];
    int ascent, w = js_label(s, mask, screen_width, LABEL_HEIGHT, &ascent);

    for (int j = 0; j < LABEL_HEIGHT; j++)
        for (int i = 0; i < w; i++)
            if (mask[j * w + i])
                put(wasm_pen.xpos + i, wasm_pen.ypos - ascent + j, pen_rgba);
    dirty = 1;
}

void logofill(void) {
    int x = wasm_pen.xpos, y = wasm_pen.ypos;
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;

    uint32_t old = fb[y * screen_width + x], color = pen_rgba;
    if (old == color) return;

    int size = 1024, top = 0;
    int *stack = malloc(size * sizeof(int));
    if (stack == NULL) return;
    stack[top++] = y * screen_width + x;

    while (top > 0) {
        int k = stack[--top];
        int row = k / screen_width, left = k % screen_width, right = left;
        uint32_t *line = &fb[row * screen_width];

        if (line[left] != old) continue;
        while (left > 0 && line[left - 1] == old) left--;
        while (right < screen_width - 1 && line[right + 1] == old) right++;
        for (int i = left; i <= right; i++) line[i] = color;

        for (int r = row - 1; r <= row + 1; r += 2) {
            if (r < 0 || r >= screen_height) continue;
            uint32_t *next = &fb[r * screen_width];
            for (int i = left; i <= right; i++) {
                if (next[i] != old || (i > left && next[i - 1] == old)) continue;
                if (top == size) {
                    int *grown = realloc(stack, 2 * size * sizeof(int));
                    if (grown == NULL) { free(stack); dirty = 1; return; }
                    stack = grown;
                    size *= 2;
                }
                stack[top++] = r * screen_width + i;
            }
        }
    }
    free(stack);
    dirty = 1;
}

/* Called from draw_turtle, which runs after every change to the turtle */
void wasm_turtle_moved(void) {
    turtle_count = turtle_shape(turtle_x, turtle_y, turtle_heading,
                                x_scale, y_scale,
                                screen_x_center, screen_y_center,
                                turtle_points);
    dirty = 1;
}

/* A message, unlike setTimeout, is neither clamped to 4ms nor throttled in
   background tabs.  (Node has setImmediate, and an open port would keep it
   from exiting.) */
EM_ASYNC_JS(void, js_yield, (void), {
    if (typeof setImmediate == "function") {
        await new Promise((resolve) => setImmediate(resolve));
        return;
    }
    if (!Module.yieldChannel) {
        Module.yieldChannel = new MessageChannel();
        Module.yieldChannel.port1.onmessage = () => Module.yieldResolve();
    }
    await new Promise((resolve) => {
        Module.yieldResolve = resolve;
        Module.yieldChannel.port2.postMessage(0);
    });
});

/* Called for every instruction: yield to the browser every ~20ms so the
   canvas repaints and ^C gets through. */
void wasm_poll(void) {
    static unsigned int count;
    static double last;

    if (++count % 256) return;
    if (emscripten_get_now() - last < 20) return;
    js_yield();
    last = emscripten_get_now();
}

void wasm_sleep(unsigned int ms) {
    emscripten_sleep(ms);
}

EM_JS(void, js_start, (uint32_t *fb, int w, int h, int *dirty, int *shown,
                       struct turtle_shape_point *points, int *count,
                       uint32_t *color), {
    const canvas = Module.canvas;
    if (!canvas) return;
    canvas.width = w;
    canvas.height = h;
    const ctx = canvas.getContext("2d");
    const frame = () => {
        requestAnimationFrame(frame);
        if (!HEAP32[dirty >> 2]) return;
        HEAP32[dirty >> 2] = 0;
        const pixels = new Uint8ClampedArray(HEAPU8.buffer, fb, 4 * w * h);
        ctx.putImageData(new ImageData(pixels, w, h), 0, 0);
        const n = HEAP32[count >> 2];
        if (!HEAP32[shown >> 2] || n == 0) return;
        const rgba = HEAPU32[color >> 2];
        ctx.strokeStyle = `rgb(${rgba & 255} ${(rgba >> 8) & 255} ${(rgba >> 16) & 255})`;
        ctx.lineWidth = 1;
        ctx.beginPath();
        for (let i = 0; i < n; i++) {
            const x = HEAP32[(points >> 2) + 2 * i] + 0.5;
            const y = HEAP32[(points >> 2) + 2 * i + 1] + 0.5;
            if (i == 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.closePath();
        ctx.stroke();
    };
    requestAnimationFrame(frame);
});

__attribute__((constructor))
static void wasm_graphics_init(void) {
    for (int n = 0; n < NUMCOLORS; n++) {
        const unsigned char *c = initial_colors[n % NUMINITCOLORS];
        set_palette(n, c[0] * 257, c[1] * 257, c[2] * 257);
    }
    for (int n = 1; n <= SPECIAL_COLORS; n++) set_palette(-n, 65535, 65535, 65535);
    set_palette(BACKGROUND_COLOR_OFFSET, 0, 0, 0);
    pen_rgba = rgba(wasm_pen.color);
    wasm_clear();
    /* The canvas is always on the page, so show the turtle right away */
    screen_mode = SCREEN_SPLIT;
    turtle_shown = TRUE;
    wasm_turtle_moved();
    js_start(fb, screen_width, screen_height, &dirty, &turtle_shown,
             turtle_points, &turtle_count, &pen_rgba);
}

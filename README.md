# UCBLogo in the browser

[Berkeley Logo](https://github.com/jrincayc/ucblogo-code) compiled to
WebAssembly with Emscripten, with turtle graphics on a `<canvas>` and the
REPL in an [xterm.js](https://xtermjs.org/) terminal.

**Demo:** <https://jburgy.github.io/ucblogo-wasm/> (French, like the Smaky)
or <https://jburgy.github.io/ucblogo-wasm/?lang=en>.

## Why UCBLogo

The LOGO that ran on EPFL's [Smaky](https://en.wikipedia.org/wiki/Smaky)
computers in Swiss schools doesn't seem to have survived as source code. It was
a French-language member of the MIT Logo family, and UCBLogo is the maintained,
open-source (GPL-3) descendant of that dialect. It also already ships French
messages and accepts `POUR`/`FIN` for `TO`/`END`; [lib/francais.lg](lib/francais.lg)
adds the classic French vocabulary (`AV`, `TD`, `LC`, `REPETE`, `VE`, …).

```logo
pour carre :cote
repete 4 [av :cote td 90]
fin
repete 36 [carre 100 td 10]
```

## Build

Needs [Emscripten](https://emscripten.org/) (tested with 6.0.2) and Node.

```sh
npm ci
make          # dist/ is the static site
make test     # upstream unit tests on a Node build
python3 -m http.server -d dist
```

## How the port works

- [ucblogo/](ucblogo/) is upstream at `3170933`, vendored with
  `git subtree --squash`. Upstream is patched in three places, all under
  `#ifdef __EMSCRIPTEN__`: pick the canvas backend and let the page draw the
  turtle (`graphics.c`), yield to the browser between instructions (`eval.c`),
  and sleep asynchronously for `WAIT` (`coms.c`).
- [wasm/wasmgraphics.c](wasm/wasmgraphics.c) rasterizes lines into a 640×400
  framebuffer (the Smaky 100's resolution) that the page copies to the canvas
  each animation frame, so XOR pens and `FILL` are pixel-exact.
- Blocking reads from the terminal use Asyncify through
  [xterm-pty](https://github.com/mame/xterm-pty), which also turns ^C into
  `SIGINT` so UCBLogo stops a runaway program the way it does natively. No
  `SharedArrayBuffer` or cross-origin isolation is needed.
- UCBLogo's garbage collector scans the C stack for pointers, which optimized
  wasm keeps in locals; Binaryen's `--spill-pointers` pass puts them back on the
  stack. Without it, the unit tests crash.

[n1ywb-1/ucblogo-code](https://github.com/n1ywb-1/ucblogo-code/tree/wasm) is a
more ambitious port (SVG graphics, JSPI, JavaScript interop) discussed in
[jrincayc/ucblogo-code#203](https://github.com/jrincayc/ucblogo-code/issues/203).

## Embedding

```html
<iframe src="https://jburgy.github.io/ucblogo-wasm/" width="680" height="640"></iframe>
```

Not supported: the external editor (`EDIT`), mouse input and `TONE`.

## License

GPL-3.0-or-later, like UCBLogo.

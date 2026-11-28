# Test Matrix Card 2026-11-28 04:55:45 -08:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 2026-11-28 06:47 -08:00 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-11-28 06:47 -08:00 |
| [Windows][`/FCT`][`/Http`] | 2026-11-28 06:48 -08:00 (partial) |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-11-28 06:48 -08:00 (partial) |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 2026-11-28 05:32:37 -08:00 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`] | 2026-11-28 05:36:13 -08:00 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 2026-11-28 05:43:53 -08:00 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 2026-11-28 05:40:32 -08:00 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over stdio `/Cli:<command>`] | 2026-11-28 05:41:31 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 2026-11-28 05:41:46 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`] | 2026-11-28 05:42:04 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 2026-11-28 05:44:14 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 2026-11-28 05:42:36 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over stdio `/Cli:<command>`] | 2026-11-28 05:42:53 -08:00 |

## Build and tests

| Test Item | 1st |
| --- | --- |
| Import, code generation and build | 2026-11-28 04:55 -08:00 |
| Workspace tests | 2026-11-28 06:13:44 -08:00 |

## Observations

- Import, code generation and build passed. Repeating code generation produced the same output. The complete workspace test passed all 11 packages with 330 passing tests and 23 conditional skips. The live entry suite passed 71/71.
- Each of the ten view-model configurations completed normal, idle-loss and blocked-translation runs. Exact greetings survived browser replacement. External network hosts rejected a second peer and stdio children exited on normal shutdown. Each fatal run delivered one Core-authored `!Error` response containing `RemotingTest_RvmHost disconnected.`, the matching page error and a retained fatal overlay. The Core exited 3.
- HTTP and MiniHTTP protocol tests exercised five buttons with all 16 keyboard-modifier combinations on original and replacement renderers, motion, double-click and both wheel axes. Both transports also displayed all three shortcut dialogs before and after takeover, added three grid rows with three nonempty cells each, cleared them and opened the inline Document dialog. Queued File close displayed `Do you want to exit?` and ended each Core with code 0. Fresh intentional fatal runs showed `This is a fatel error!` and the fatal mask.
- On each showcase transport, Chromium rendered the Raw Clock canvas, opened Touching, replaced the renderer, and exited the Core with code 0 using Force Exit. The checked-in live touch suite exercised both HTTP transports. The checked-in canvas pixel and clock-data test ran on HTTP. Fresh 1190×484 browser canvases also contained 164 yellow hand pixels on HTTP and 176 on MiniHTTP.
- Both transports painted the six palette colors in Chromium, displayed the four Layout tabs, and opened and canceled the 240×320 responsive modal. On HTTP and MiniHTTP an original and a replacement browser each rendered independent Ribbon search and document markers. Queued Close ended both Cores with code 0.
- The HTTP Ribbon editor verified exact text after clipboard commands, Undo, Redo, Select All and Delete. Range formatting and the three paragraph alignments retained the selection. The MiniHTTP clipboard tests exercised both editors against an independent browser page, including Unicode text, unsupported formats, a single cut and paste, a standalone bitmap, and private rich text with two embedded images. Both browser editors kept the fake clipboard separate from the real desktop and browser clipboards.

## Issues Found and Fix

- Two Unicode composition cases failed while the initial cold C++ build was running. All four focused editing variants and the complete workspace rerun passed. The headed desktop IME attempt did not establish a Windows candidate-window commit. Controlled Chromium composition evidence is recorded separately.
- One replacement MiniHTTP Alt+Middle transition timed out. The isolated repeat completed the entire original and replacement input matrix.

Local evidence is under <GacUI repo>/Test/Output/RpWindows20261128. See `yarn-test-repeat.log`, `browser-rvm.log`, `browser-rvm-retry.log`, `rpt-followup.stdout.log`, `fct-followup.stdout.log`, `fct-layout-settled.log`, `fct-rawclock-retry.stdout.log`, `fct-browser-http-final.log`, `fct-browser-markers.log` and `clipboard-final.log`.

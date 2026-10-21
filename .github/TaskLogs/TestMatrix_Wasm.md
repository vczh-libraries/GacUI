# Test Matrix Card 2026-10-21T11:19:05.560Z

## Test Matrix

| Test Item | 1st |
| --- | --- |
| FCT | PASS: Chromium and Firefox, including raw/touch/lifetime |
| RPT | PASS: Chromium and Firefox, canceled Exit, normal/fatal, Reload |
| RVMT | PASS: Chromium and Firefox RPC/Unicode/lifetime/Reload |

## Build and packaging

| Check | Result |
| --- | --- |
| Three native compile/link checks, then Wasm -bw -o | PASS: all three native builds/executions, then final Wasm -bw -o |
| GacJS install/import/codegen/build/test | PASS, including final build/test |
| Explicit copy validation and nine runtime hashes | PASS: missing/empty worker and missing destination refused without partial updates; all nine files equal build outputs |
| Shared pthread unit-test launcher | Passed: Firefox, Vlpp 32/32 files and 473/473 cases; one successful completion |
| Public demo local and deployed verification | PASS: local and live gaclib.net, Chromium and Firefox |

## Observations and limitations

- FCT native `-f -o` compile/link and silent executable exit passed. The Wasm rebuild uses the documented Emscripten 3.1.51 SDK; the system SDK is newer and does not emit the required separate worker.
- GacJS install/import/codegen/build/test passed, including the final channel regression and RVM bootstrap cleanup fix.
- WebsiteSource install/build and all package tests passed. No full website export/publication was run.

- Portable Wasm channel regression passed (6 tests total), including 18446744073709551615 sequence IDs, timestamps above 2^53, sparse identities and CaptureLost.

## Issues Found and Fix

- WasmFCT final wrapper `-bw -o` completed with Emscripten 3.1.51. Chromium touch regression passed against the resulting module. The first run reached unexpected-capture-loss checking; the test released B but only moved A, so pending B capture loss was never processed. The driver now moves the released contact before asserting cancellation. No implementation change was required.

- Chromium live Wasm touch regression passes: painted transform/contact pixels, sparse/reused IDs and indices, PlatformCanceled and real capture loss, outside-surface capture after resize, TouchingList identities/clamping/reversal/final-Up deletion and cancellation, recycled rows, rejected buttons, ordinary list selection/scrolling, text selection and keyboard. Repeated after palette refresh and shutdown/Reload. Test setup fixes: establish capture before release; reveal occluded delete label before measuring; sort recycled rows by rendered position; distinguish the two parallel lists; inspect the paragraph after selection splits its spans.

- Final development suite: 10/10 tests passed (public tests opt-in and run separately). A Firefox run exposed a test timing issue: newly measured labels briefly have provisional 1x1 bounds; navigation now waits for stable elements and measured bounds. The complete rerun passed. Additional Firefox Wasm responsive-modal exercise covered all four appearances, 240/350/440/680 widths, height 220/320/520, reverse cycles, equal footers, account selection and reopening at 240x320.

- Final rebuild/repackaging retained all nine exact development artifacts and the module default factories. The complete development suite passed again (10/10); final local public launcher tests passed 2/2.
- Public commit `bec531fff` deployed. Fresh Chromium and Firefox contexts followed Web > Start WASM Now! on gaclib.net and passed Raw Clock, Touching, TouchingList, ordinary input, palette refresh, Exit/Force Exit, refresh, isolation, scoped service worker, completed download progress and worker/observer cleanup (2/2). No missing requests or console errors. The live 53,007,789-byte Wasm SHA-256 is `aeee524454f56eac87e7996949c5eac90d127422b0c69d0a3306f5f9794238dc`, equal to the staged binary.
- Chromium uses native browser touch injection; Firefox touch uses explicitly synthetic PointerEvents. Physical/mobile touch hardware and macOS are unavailable/unexecuted, not inferred from browser injection.
- Later user instruction removed `<Tools repo>/Copilot/Jobs` (commit `27b9058`); reusable job instructions remain in `<GacUI repo>/.github/Jobs`.

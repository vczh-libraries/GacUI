# Test Matrix Card 2026-11-25T12:33:13.658Z

Completed: 2026-11-25T14:49:20Z.

## Test Matrix

| Test Item | 1st |
| --- | --- |
| FCT | Passed Chromium and Firefox regressions and the supplemental document/layout operations below; native IME and other platform limitations are recorded separately |
| RPT | Passed: Chromium/Firefox regressions, exact Core fatal error, shutdown/reload, Chromium supplement with all 16 modifier combinations, five buttons, movement/double-click/wheels, 3-by-3 grid, document dialog, label styling and canceled/confirmed File-menu close |
| RVMT | Passed in Chromium and Firefox: initial greeting, Unicode RPC, Exit/Force Exit, fresh Core/TypeScript host after Reload, working subsequent input |

### FCT supplemental verification

| Test Item | Result |
| --- | --- |
| Ribbon document editor | Passed fixture loading, context/toolbar/shortcut clipboard operations, all six paragraphs selected by Ctrl+A, text effects, exact rendered/exported alignment, Font/Color Cancel and selected-range edits, hyperlink command paths/Preview/persistence, caret-5 image insertion and file cancellation. All eight styles/reapplication, wide/dropdown/collapsed galleries, sample controls, Alignment expansion, mode commands/shortcuts and palette retention passed. Edge popup stayed within the renderer with five separators and expected shortcuts; scrolled point/range/context-menu repetition passed |
| Toolstrip document editor | Passed visible edit commands, toolbar effects, rendered/HTML alignment, context/toolbar Font/Color, hyperlink paths, exact Preview URL, removals, caret-5 image insertion, Undo/Redo and file cancellation/persistence. Preview/Selectable rejected mutation; Editable accepted it. Chromium touch/mouse carets agreed inside forward/reversed ranges, at endpoints, in blank space and at nonzero scroll offsets. Hold/release, Down/cancel/drag popup retention, qualifying-click dismissal and Default/Aurora template refresh passed |
| Layout and responsive modal | Passed Chromium rail gap, palette/control-state retention, keyboard/access keys, four appearances, all 24 films/directors, eight accounts, menu/list selection agreement, independent scrolling, image sizes, height changes and three close actions. Chromium/Firefox strict movie probes passed retained selection, compact row-height restoration and resizing at list ends; Firefox focused account scrolling reached all eight visible accounts. Chromium outside-renderer resize/title capture release and fresh Ember modal passed |
| Actual OS IME | Attempted; native delivery unavailable in this session, as detailed below |

## Build and packaging

| Test Item | Result |
| --- | --- |
| Native compile/link checks: FCT, RPT, RVMT | Passed: all three full native builds and empty native-main executions returned 0 |
| Full Wasm builds: FCT, RPT, RVMT | Passed: all three `-fbw -o` builds returned 0 after their native checks |
| GacJS install / import / codegen / build / tests | Passed: install, import, codegen, build and complete `yarn test --concurrency 1` (279 tests across 11 packages, exit 0) |
| Live Wasm regression suite | Passed: `npm run test-wasm`, 11 passed and 2 public-demo tests skipped because this job targets the local development pages |
| Deployment validation and byte comparison | Passed: 23 packaging cases cover all missing/empty required files and destination pages, separate workers, module workers and stale-worker removal. Website build produced pages without Wasm artifacts. Explicit copy preserved all six files byte-for-byte; original default factories and pthread startup tails remain unchanged. All deployed files have correct MIME and COOP/COEP headers. |
| Shared pthread unit-test launcher | Passed in Chromium and Firefox: fresh VlppOS `-fbw -o` build, 10/10 test files and 107/107 cases, `data-result="0"`, exactly one `wasm_main returns 0.`, 32 module pthreads plus the host worker, no browser errors, COOP `same-origin` and COEP `require-corp` |

## Observations and Limitations

Input revisions: GacUI `9032411cf`, GacJS `afe0b9f4`. Linux only. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125`. Emscripten 3.1.69, Node.js 26.5.0 and Yarn 1.22.22 are installed. Chromium/Firefox use `PLAYWRIGHT_HOST_PLATFORM_OVERRIDE=ubuntu24.04-x64` as prescribed for this Ubuntu 26.04 host.

Chromium 145.0.7632.6 and Firefox 146.0.1 launched successfully. WebKit 26.0 also launched after the runtime adjustment below. These checks establish this run's compatibility, not official Ubuntu 26.04 support.

Emscripten 3.1.69 emitted module pthreads, with no separate `app.worker.js`. Packaging hashes and byte comparisons are recorded in `<GacUI repo>/Test/Output/rpwasm-20261125/deployment-verified.json`; website build and HTTP header evidence are in the same directory. The interactive GacJS entry server was stopped with ENTER after verification and returned 0. Test browsers and the shared-launcher server were closed. No native Core, HTTP protocol listener or external view-model host was involved. Generated builds, deployments and raw evidence remain ignored; only this card changed in the repositories.

The shared launcher used `<VlppOS repo>/Test/Linux/UnitTest/Bin/app.sh` over HTTP with its 32-pthread build. Both browsers loaded pthreads directly from `app.mjs`; retained Wasm cases include thread synchronization and Unicode console callbacks from a pthread. Raw output and headers are in `<GacUI repo>/Test/Output/rpwasm-20261125/shared-launcher-{chromium,firefox}.raw.json`; totals are in `<GacUI repo>/Test/Output/rpwasm-20261125/shared-launcher-summary.json`.

The local regression suite checked Chromium browser-level touch, ordered worker/channel messages, clock and gesture pixels, TouchingList row identities, read-only document guards, and worker shutdown on page close. Firefox touch coverage is DOM-synthetic. RPT preserved exactly one Core-authored `This is a fatel error!` terminal error and kept Reload disabled on failure. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/test-wasm.log` and `<GacUI repo>/Test/Output/rpwasm-20261125/rpt-supplement-payloads.json`.

The default browser renderer advertises only its default font family, `Segoe UI`; the live Font dialog exposed that one family. Size changes and cancellation are testable, but selecting a second advertised family is unavailable with this page configuration. Toolstrip evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Toolstrip-font-20261125.json`. Windows UI Automation and native Core automation do not apply to these Linux Wasm pages; the supplemental checks use the visible browser surface and read rendered metadata.

Both document editors used a fresh OPFS copy of `<GacUI repo>/Test/TestDocument.bin`. Selected title characters 0–5 (`GacUI`) changed from size 28 to 16 or to foreground/background `#112233`; the unselected remainder retained size 28, foreground `#80B8FF` and transparent background. Ribbon title alignment matched exported HTML for Left, Center and Right. Its hyperlink exports contained exactly the `GacUI` anchor with the chosen URL; Preview displayed that URL, both context removal paths removed the anchor, and private reload retained it. Toolstrip image insertion at caret 5 produced `GacUI[Image] Remote Document — 中文 π` and three embedded images; private save/reload/save preserved 185404 bytes exactly after removing the source PNG. Evidence includes `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Ribbon-hyperlink-20261125.json`, `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Toolstrip-image-persistence.json`, and the per-editor Font/Color JSON files.

Additional focused checks passed for both editors' toolbar Font/Color commands and image insertion. Ribbon exercised both Object and Insert-tab image commands at caret 5; Toolstrip exercised its image toolbar button. Canceling actual Load, Save and image dialogs preserved document data. Undo/Redo and private reload retained the expected three images after source-PNG deletion. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/editor-image-file-strict-{Ribbon,Toolstrip}.json`. Ribbon shortcut checks distinguish Preview/Selectable mutation rejection from Editable insertion/deletion, and prove Selectable Copy through a subsequent exact Paste. Ribbon Insert-tab hyperlink creation/edit/Preview/removal passed with exported anchor evidence.

Ribbon context and toolbar clipboard routes both produced `GacUI RemoGacUIte Document — 中文 π` when pasting `GacUI` at caret 10, with exact Undo/Redo, Cut/Paste and Delete/Undo results. Its edge popup settled at `(1073,622)`, size `215×346`, within the renderer, with five separators, shortcut labels and retained selection. Ctrl+A selected all six fixture paragraphs. Toolstrip toolbar hyperlink edits produced exact exported `GacUI` anchors and removal left no anchor; its four font-effect toggles retained range 0–5, and Left/Center/Right matched exported HTML. Evidence includes `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Ribbon-clipboard-visible-20261125.json`, `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Ribbon-context-edge-20261125.json`, `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Toolstrip-toolbar-hyperlink-20261125.json` and `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-Toolstrip-effects-alignment-strict.json`.

The responsive movie checks passed in Chromium and Firefox with actual settled 240/350/440/680-pixel clients. They retained selected `Vertigo`, observed all 24 films inside the clipped viewport, restored the original compact row height, and traversed both ends after resizing near the last film. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/responsive-movies-strict-{chromium,firefox}.json`.

Chromium's focused layout check observed nested access keys `R0`, `R1`, `E`, `T`, and the separate responsive-window action `W1`. It checked configured Default/Ember/Aurora selected and hover RGB values before/after template refresh, the five-pixel rail gap and unchanged editor/checkbox/choice. Releasing normal and immediate resize drags outside the renderer, then moving without a pressed button, left modal bounds unchanged; the title-drag path and a subsequent ordinary resize/close also passed. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/layout-palette-capture-strict.json`.

Synthetic Escape did not dismiss Ribbon's nonactivating context popup. Clicking the main-window corner dismissed it while preserving the range. In Toolstrip, Chromium touch Down/cancel/drag preserved an existing popup, and a qualifying short press dismissed it and placed a caret under both Default and Aurora. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/ribbon-sample-palette-strict.json` and `<GacUI repo>/Test/Output/rpwasm-20261125/toolstrip-popup-strict.json`.

Both editors also passed the scrolled repeat with browser-level touch: real document drags changed paragraph offsets, short presses matched ordinary mouse carets at endpoints/interior positions, and a selected 3–8 range survived context-menu opening and outside dismissal. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/chromium-{Ribbon,Toolstrip}-scrolled-point-strict-20261125.json`.

A fresh headed Firefox 146.0.1 attempt used the available Wayland/X11 session, IBus `table:cangjie3` and XTest directed at a verified owned Wasm FCT window. Candidate commit/cancel sequences were attempted in Search and the central document. No DOM key/composition/input events, candidate UI or text changes were observed; the central document did have its plaintext-editable capture focused. An ordinary `x` with the US engine also produced no DOM input. Actual OS IME composition therefore remains unverified in this session. IBus was restored to `xkb:us::eng` and the owned browser closed. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/native-ime-fct-20261125.json` and corresponding screenshots. No physical touchscreen/software-keyboard or native OS global-hot-key coverage is claimed.

## Issues Found and Fix

The supplemental Firefox layout driver initially clicked Layout before the palette refresh had settled. A focused run that waited for Ember's actual rendered accent and reacquired stable target bounds passed on the first click, with the expected input messages, page contents and no browser errors. This was a test-driver timing issue. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/firefox-layout-minimal.json`.

Supplemental drivers were corrected to wait for actual rendered changes, inspect clipped rows, distinguish the launching button from the modal caption, and use fresh control geometry. The Firefox account sweep initially started after scrolling past the first accounts; the focused run started at the top of a settled 680-by-220 modal and reached all eight accounts, with none missing. Wheel magnitude is normalized to one tick, so long sweeps used repeated events. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/account-scroll-firefox-focused.json`. These were verification-driver issues; no product source fix was required.

The first GacJS test run failed to launch WebKit because Ubuntu 26.04 lacks libraries expected by the Ubuntu 24.04 browser build. The browser launcher overrides `LD_LIBRARY_PATH`. Supplying the required unchanged libraries from a private Ubuntu 24.04 runtime using `LD_PRELOAD` resolved the fresh WebKit launch check; installed packages were not patched. The complete suite then passed with this environment. Evidence: `<GacUI repo>/Test/Output/rpwasm-20261125/gacjs-test.log`, `<GacUI repo>/Test/Output/rpwasm-20261125/webkit-preflight.log`, `<GacUI repo>/Test/Output/rpwasm-20261125/browser-runtime-env.json` and `<GacUI repo>/Test/Output/rpwasm-20261125/gacjs-test-resumed.log` in the evidence directory.

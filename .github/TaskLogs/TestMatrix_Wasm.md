# Wasm and Raw Clock Verification

Started: 2026-10-15T06:54:17-07:00. Runtime verification completed: 2026-10-15T07:42:54-07:00.

## Actual Wasm Applications

| Application | Build | Chromium runtime | Firefox runtime |
| --- | --- | --- | --- |
| FCT | Full Emscripten 3.1.51 rebuild passed | Passed, including actual Raw Clock | Passed, including actual Raw Clock |
| RPT | Full Emscripten 3.1.51 rebuild passed | Passed, including canceled Exit and fatal error | Passed |
| RVMT | Full Emscripten 3.1.51 rebuild passed | Passed, including repeated Unicode RPC | Passed, including repeated Unicode RPC |

`npm run test-wasm` passed all 9 tests: 6 Chromium and 3 Firefox. These ran the compiled C++ applications, not controlled protocol clients. All pages started cross-origin isolated, loaded their module/Wasm/pthread artifacts, and accepted input without startup error masks. Firefox observed worker URLs and resource timing for module loading; Chromium used CDP to inspect workers.

Both browsers verified Exit and Force Exit, disabled Reload while running, fresh UI/input/RPC after Reload, and termination of old workers. RPT canceled Exit without enabling Reload. Chromium additionally checked the exact Core-authored fatal error and termination of workers when closing a live page. RVMT used the in-page TypeScript host with repeated ASCII/Chinese translation. Firefox Unicode input used DOM keyboard events because it has no CDP input endpoint.

FCT verified received `FullControlTest.Clock` IDs and valid time data, nonempty canvas pixels, and changed hands without input. Chromium checked one canvas, record/canvas identity through tab hide/show, pixel-buffer resizing, palette refresh, and fresh painting after both shutdown/reload paths. Existing FCT controls, editors, lists, shortcuts and palette tests and RPT grid/document/input tests remained enabled.

## Builds, Generation and Deployment

- CodePack regenerated the GacUI release from current source. wGac imported the result; the generated raw base, factory helpers, descriptor and collection APIs match the upstream release byte for byte. Existing full/core 32/64 reflection metadata already includes the current raw base and factory return type, so metadata regeneration was unnecessary.
- Both wGac import/synchronization/official-build cycles passed. The second synchronization preserved hashes of the reusable element, local clock helpers, CMake files and FCT hook. The copied remote entry point matches its upstream owner. Link inventories contain the local clock painter once in FCT and both local helpers once in the renderer, with no Windows clock helpers.
- GacJS import, codegen, build and root tests passed. Root tests include controlled raw-element/clock and Wasm transport tests and Workflow RPC integration; Windows live-Core suites are explicitly skipped on Linux. A second codegen pass preserved its generated diff, which only reorders snapshot-index entries. No snapshot pixels or live times were added.
- All three Wasm projects were fully rebuilt through their repository wrapper with `-fbw -o`. All nine deployed artifacts are nonempty and match their corresponding build outputs exactly. Packaging preserves the original module default factory export.
- Deployment validation rejected a missing final worker, an empty final worker, and a missing final destination page before changing any of the nine deployed files. Normal deployment then succeeded. Generated Wasm artifacts remain ignored.

## Live Native MiniHTTP with GacJS

| Application | Firefox observations |
| --- | --- |
| FCT | Actual time/pixels, ticking without input, one canvas and retained identity across hide/show, resize, palette refresh, replacement after browser closure, live takeover, Force Exit |
| RPT | Button, grid and document-dialog interactions, canceled and confirmed Exit |
| RVMT | Only Core plus `index.html?rvmhost`; repeated ASCII/Chinese translation and normal Exit |

All three passed with no page errors. Firefox reported canceled MiniHTTP requests as CORS diagnostics during final Core shutdown; success overlays and Core termination were observed. These shutdown diagnostics are separate from startup, drawing and RPC behavior. Browser console diagnostics, Core output and temporary captures were retained with the local verification artifacts.

## Wayland Runtime and Focused Tests

| Check | Result |
| --- | --- |
| Native FCT | Passed visible clock/ticks, tab hide/show, palette refresh, native buffer resize from 642×517 to 642×839, normal close and fresh reopen |
| Hosted FCT | Passed the same clock/tick, tab, palette, size-change and close/reopen checks |
| Native remote FCT | Passed clock/ticks, tab/palette changes, replacement after renderer closure and live takeover on port 8890; state and clock painting survived each new connection |
| Normal remote shutdown | FCT closed through its Exit tab; Core and renderer both exited 0 |
| Unexpected Core loss | After a live FCT clock check, terminating Core caused the native renderer to exit 0 without a fatal prompt or retry loop, matching the transport-disconnect lifecycle |
| Native remote RPT | Passed button, grid and document dialog, canceled Exit, exact retained `This is a fatel error!` DOM/overlay, rejection of ordinary input, and cleanup using exact `!Exit`; intentional Core fatal exit was 134 |
| Native RVMT | Passed repeated Unicode RPC using the native MiniHTTP host and normal application close; host stopped after requester shutdown |
| `Test_RawElement /C` | 4/4 passed |
| `Test_RawElement /C --hosted` | 3 applicable cases passed; the native-remote case explicitly returns in hosted mode |
| `Test_RenderingLifecycle /C` | Passed |

Focused tests separate concrete factory/common-property checks from drawing. They cover complete state at collection callbacks, empty/equal/ID-only/data-only changes, duplicate IDs, late attachment, uninstall/recreate, current-pass context lifetime, clipping, transforms, source/line state, independent Cairo path restoration, neighboring drawing, scales 1 and 2, offsets, detach/recreation and fixed clock hand positions. The real native remote renderer redraws changed data with no DOM update. After disconnect, an explicit redraw preserves clock pixels; fatal retention still paints the attached clock until actual teardown. Retained element pointers cannot paint after target/window destruction.

Native render evidence came from the application's own Wayland shared-memory buffers. Inspected captures show the white dial/ticks and cyan/green/yellow hands, including the remote clock and retained RPT fatal overlay. These are render-buffer captures, not compositor screenshots.

## Issues Resolved and Coverage Limits

The first incremental Wasm link failed on a libc++ `std::__2::__basic_string_common` symbol; full rebuilds resolved it. System Emscripten 3.1.69 produces no separate `app.worker.js`. Its local changelog documents worker consolidation/removal. An isolated emsdk 3.1.51 supplied the required three-file contract without modifying generated modules; GacJS documentation now states this SDK requirement.

The first Firefox test used page request events, which omit worker fetches. Worker URLs/resource timing fixed the observation method; all three Firefox cases then passed. Temporary HTTP/native helpers also needed completion-overlay waits after shutdown and stable measured coordinates after navigation; these were verification-helper issues, not product changes.

The host lacks Cairo/Pango/GLib development packages, and sudo requires authentication. The full official wGac builds ran in a disposable Bubblewrap environment populated with Ubuntu packages from the documented prerequisites, using ordinary system include/library paths. Native execution used the host's actual Wayland compositor. No temporary dependency paths or download logic entered repository scripts.

Desktop screenshot access was denied. Compositor-driven drag/movement and physical display-DPI switching were not verified; client resize/recreated surfaces and scaled/translated Cairo rendering were verified as described above. Automation injects application input and does not supply the compositor pointer serial required by a native border-drag gesture. Windows/macOS native execution was unavailable. Unchanged prerequisite-task native empty-main/launcher checks were not repeated; this run exercised all three actual Wasm applications and both required native rendering paths.

The shared upstream change is confined to test-application entry code; its owning verification is the synchronized native renderer build/runtime. No upstream framework source implementation changed in this task, so the unrelated GacUI unit snapshot suite was not rerun.

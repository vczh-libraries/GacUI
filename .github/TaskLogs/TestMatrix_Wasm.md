# Test Matrix Card 2026-10-29T14:53:25Z

| Test Item | Result |
| --- | --- |
| FCT | Build passed; Chromium/Firefox local services and both document editors passed; full suite and 25 consecutive Firefox FCT repeats passed |
| RPT | Build passed; Chromium/Firefox input, shutdown/reload and worker checks passed; final suite passed |
| RVMT | Build passed; Chromium/Firefox Unicode RPC, fresh host after Reload and shutdown checks passed; final suite passed |

## Observations and limitations

Environment: Ubuntu 26.04, Emscripten 3.1.51, Playwright Chromium 145.0.7632.6 and Firefox 146.0.1. Playwright browser installation used `PLAYWRIGHT_HOST_PLATFORM_OVERRIDE=ubuntu24.04-x64` because this host version is not recognized by its installer. A stale Firefox cache lock with no owning process was removed before launch.

Commands and packaging:

- Built each of `<GacUI repo>/Test/Linux/WasmFCT`, `WasmRPT` and `WasmRVMT` with `../../../.github/Ubuntu/build.sh -bw -o`, with Emscripten 3.1.51 activated. All three completed successfully. The official wrapper regenerated the source inventories/makefiles; RPT/RVMT include the current injection sources.
- From `<GacJS repo>/Gaclib`, `yarn install`, `yarn run import`, `yarn codegen`, `yarn build` and `yarn test` passed. A second codegen introduced no additional diff. The generated snapshot-index ordering change is retained. Root tests passed 211 cases, including 38 portable entry tests, 100 renderer tests and the Workflow RPC driver integration.
- Ran `<GacJS repo>/copy-wasm.sh` after all three builds. All nine deployed `app.mjs`, `app.wasm` and `app.worker.js` files matched their owning build outputs byte-for-byte; SHA-256 comparisons were recorded. The modules retained their original default factory exports. Generated binaries remain ignored.
- Exercised the real packaging script in an isolated fixture: a missing final worker, an empty final worker and a missing final destination page each failed before any of the nine existing destination files changed. The complete fixture copied all nine files exactly.
- Started the entry package through `npm run start` on port 8896 with its normal isolation headers. Ran `npm run test-wasm`, the focused Firefox FCT case, and the consecutive repeat loop. Final full-suite results are recorded below.

Live service and persistence checks:

- Shared Core startup explicitly sets `injectDateTime`, `injectLocale` and `injectFileSystem` to false. The WASM renderer explicitly passes the same three false capabilities through the shared client. The HTTP opt-in path remains independent.
- Actual worker/channel `ControllerConnect` payloads on all three pages contained all three fields set to false. The live suites inspect startup and Reload connections and reject injection requests during the tested operations.
- Real FCT in Chromium and Firefox exercised fresh localization/date controls and both Ribbon/Toolstrip document editors. Each loaded the same-origin OPFS fixture, accepted Unicode text, saved a Unicode-named private document, reloaded to discard unsaved edits, and exported HTML/RTF with two embedded images per document. Private files grew beyond the original 117713 bytes.
- Force Exit completed with old workers gone; Reload created fresh UI. Both saved files, the empty fixture and the binary fixture survived. Loading the persisted Ribbon document recovered its marker. There were no injection requests during these service/document operations; `ImageSave` was observed 12 times per browser.
- The demos retain their existing local WASM DateTime, `EnUsLocaleImpl` and direct `OpfsFileSystemImpl`; this does not claim that the locale equals browser Intl.
- A freshly rebuilt pthread-enabled `<VlppOS repo>/Test/Linux/UnitTest` used its shared `./Bin/app.sh 8897` / `app.html` launcher. All 107 cases in 10 files passed in each of Chromium and Firefox, including filesystem/stream binary snapshots, close persistence, concurrent pthread access, Unicode paths, folder rename, locale and threading. Its fixture clearing used a separate origin from the demos.

Application and lifecycle regression:

- The initial complete run passed all seven Chromium cases and Firefox RPT/RVMT. It covered real input, lists/editors/palette, Raw Clock, browser touch, RPT confirmation and exact fatal error, Unicode RVMT RPC, Exit/Force Exit, fresh state after Reload, disabled Reload while running and worker/pthread cleanup.
- Final full suite: all 10 local cases passed (7 Chromium, 3 Firefox); the 2 opt-in public-deployment cases were skipped.
- Consecutive focused Firefox FCT verification: all 25 complete lifecycle runs passed, in addition to the first focused pass and the final full suite. Each run exercised input/touch before and after both Exit/Force Exit Reload cycles.

Coverage is local Linux Chromium/Firefox. Chromium touch uses browser-level input; Firefox touch and non-ASCII keyboard coverage use synthetic events on the actual input surface. Neither establishes physical-device or compositor-global-shortcut coverage. Windows-only E2E and the two opt-in public-deployment tests were skipped by their existing gates; no public deployment, macOS, WebKit or Safari run is claimed. Native test-app compile/link variants were optional and were not run; native Core/Wayland integration is recorded separately in `<wGac repo>/TestMatrix_Injection_XPlat2.md`. GacUI framework unit tests were not required for this shared test-application startup change under `<GacUI repo>/Project.md`.

## Issues and fixes

The first full Firefox FCT run failed in a Reload TouchingList check. Row identities already reflected deletion, but the subsequent measured layout still showed the old one-row gap. The test now polls for the expected row position through its existing observable-state helper, retaining the identity checks and half-pixel tolerance. No renderer/layout behavior or fixed delay was added. A focused run, all 25 consecutive complete Firefox FCT lifecycle repeats, and all 10 local cases in the final full suite passed.

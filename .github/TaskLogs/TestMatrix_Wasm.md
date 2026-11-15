# Test Matrix Card 2026-11-15T07:10:03.680Z

## Test Matrix

| Test Item | Result |
| --- | --- |
| FCT | PASS: native build/startup, optimized Wasm, Chromium and Firefox lifecycle/input, editors, Raw Clock and touch regressions (2026-11-15T12:15:58.473Z) |
| RPT | PASS: native build/startup, optimized Wasm, Chromium and Firefox lifecycle/input, Exit cancellation and exact fatal message (2026-11-15T12:15:58.502Z) |
| RVMT | PASS: native build/startup, optimized Wasm, Chromium and Firefox Unicode RPC and fresh hosts after reload (2026-11-15T12:15:58.525Z) |

## Observations and Limitations

- Release CI completed at 2026-11-15T12:52:11.372Z. Optimized native CI passed all nine repositories. All eleven WebAssembly unit suites passed in Firefox with result 0 and no browser errors (4358 cases total); GacUI passed 1,967 cases across 93 files. This also validates the shared pthread-enabled HTTP unit launcher. Parser2/Workflow TypeScript schema checks and all 12 GitView runtime cases passed.
- Wasm compiler output contained only the SDK notices for pthread memory growth and limited post-link optimization with DWARF. The missing-override and scalar-initializer source warnings are fixed.
- Local demo browser regression suite: 11 passed (eight Chromium and three Firefox). Two public-hosting cases were skipped because no public URL was supplied. All demos loaded their pthread module, accepted input, completed both shutdown paths, enabled Reload only after completion and started fresh state after Reload; old workers terminated.
- Chromium touch checks used browser-level input against the actual Wasm renderer and verified ordered channel traffic, painted transforms and list identities. Firefox touch checks used DOM-synthetic input; they are separate evidence.
- Explicit `<GacJS repo>/copy-wasm.sh` deployment preserved all six source artifacts byte-for-byte (SHA-256 checked) and retained each module's original default factory export. This SDK emitted no separate worker artifact; pthreads loaded the generated ES module. The normal website build did not deploy Wasm files.
- A concurrent Windows CI update refreshed 39 file-dialog snapshot paths. All 27 changed frames had identical visible output after resolving element references; the validated Linux snapshots were retained during rebase. No implementation changes arrived with that update.
- Host: Ubuntu 26.04 with Emscripten 3.1.69 and Playwright 1.58.2. Browser runs use `PLAYWRIGHT_HOST_PLATFORM_OVERRIDE=ubuntu24.04-x64`; no installed packages or generated modules were patched.
- Fresh GacJS import, code generation, all ten build packages and all eleven test packages passed. Website tests passed all 63 cases, including deployment validation and browser Raw rendering; C++ RPC integration passed all nine tests. Windows-only native protocol E2E wrappers are skipped on Linux.
- This card records automated release checks. Native desktop/UIA, OS global hot keys and physical touch are separate coverage; no full manual document-editor or responsive-layout SOP run is claimed.

## Issues Found and Fix

- GacGen failed the native release warning check because three `GuiGraphicsResourceManager` overrides lacked annotations in the amalgamated header. Added `override` in the owning source, regenerated `<GacUI repo>/Release/GacUI.h` with CodePack and copied the release files into `<Release repo>/Import`. The focused GacGen rebuild, full native GacUI CI, downstream Release GacGen build and all three Wasm demo/browser checks passed.

- GacJS build failed after protocol import because Raw descriptors lacked the new required `interpretor` string. Updated the default descriptor and authored test fixtures to supply the empty native factory name; the full GacJS build and test phases passed, including renderer/browser tests and Workflow RPC integration.

- The native tooltip test used redundant scalar initializer braces. Changed its client size to `Size(240, 160)`; the optimized UI build passed the warning check and its native test suite passed. Also fixed the CI warning detector in `<Tools repo>/Ubuntu/vl/cmd/vgo`: early-exit grep could cause SIGPIPE under pipefail and hide warnings in long build logs. Synthetic warning, error, clean-output and word-boundary checks passed; the corrected detector passed the complete GacUI and Release native reruns.

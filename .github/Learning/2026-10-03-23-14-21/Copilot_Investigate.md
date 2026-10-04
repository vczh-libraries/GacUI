# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

# Deploy the generated WebAssembly pthread worker separately

Status: Planned. Implementation and verification are pending.

## Requested outcome

Stop patching Emscripten's generated `app.mjs` in `Tools/Ubuntu/vl/wasm.sh`. Update `GacJS/copy-wasm.sh` to deploy `app.mjs`, `app.wasm`, and `app.worker.js` together for GacUI's `WasmFCT`, `WasmRPT`, and `WasmRVMT` demos.

Paths below are relative to the workspace containing the sibling repositories.

## Background

Emscripten already generates `app.worker.js` when linking these pthread-enabled projects. The unmodified `app.mjs` creates pthread workers using that adjacent script, and the worker imports the adjacent `app.mjs` to initialize its environment.

Currently, `embedPthreadWorker: true` makes `wasm.sh` read `app.worker.js`, replace the default export in `app.mjs`, and append a wrapper that creates a Blob URL for the embedded worker. This allows GacJS to copy only two generated files. Deploying the separate worker removes the need for that rewriting step.

## Implementation

1. Remove the JavaScript embedding and `app.mjs` rewriting block from the canonical `Tools/Ubuntu/vl/wasm.sh`. Packaging must leave Emscripten's generated module byte-for-byte unchanged. Keep the ordinary launcher and Wasm target packaging working.

2. Remove the obsolete `embedPthreadWorker` option from `GacUI/Test/Linux/WasmFCT/vbuild`, `WasmRPT/vbuild`, and `WasmRVMT/vbuild`. Remove these configuration files if they have no remaining purpose. Search active build code, configurations, and documentation for other references to the removed option.

3. Update both the validation and copying in `GacJS/copy-wasm.sh` to include `app.worker.js` alongside `app.mjs` and `app.wasm` for every demo. Validate all source files and destination pages before modifying any deployed files, preserving the existing behavior when inputs are missing. Place each matching set in `GacJS/Gaclib/website/entry/lib/dist/wasm-fct`, `wasm-rpt`, or `wasm-rvmt`.

4. Use Emscripten's normal loading of the adjacent worker file. GacJS's existing `Gaclib/website/remote-protocol-wasm/src/worker.ts` imports `app.mjs` and calls its default factory; verify that this continues to work with the original generated export. GacJS's own `wasm-worker.js` hosts the application and is distinct from Emscripten's `app.worker.js` used for pthreads.

5. Update the deployment documentation and verification instructions to describe all three generated files. Relevant starting points are `Tools/Ubuntu/README.md`, `GacUI/Project.md`, `GacUI/.github/Jobs/job.rpWasm.prompt.md`, `GacJS/README.md`, and `GacJS/doc/Projects.md`. Update other active references as needed, including shared instruction copies through their owning source. Generated Wasm outputs remain build artifacts and must not be committed; correct the existing job text that says they are tracked.

6. Release the canonical Ubuntu tool changes using `Tools/Ubuntu/vl/cmd/vgo uci`, following `Tools/MonoRepo.md`, so the C++ repositories and the `Release` repository receive the updated script.

## Verification for the implementation

- Rebuild `GacUI/Test/Linux/WasmFCT`, `WasmRPT`, and `WasmRVMT` from their respective directories with `../../../.github/Ubuntu/build.sh -fbw -o`. Fresh builds must replace previously patched outputs.
- Confirm packaging leaves `app.mjs` unchanged and emits no custom embedded-worker wrapper. Confirm each demo produces the three required files.
- Build GacJS, then run its explicit copy script. Confirm the deployed files match their respective build outputs. A missing or empty `app.worker.js` must cause the copy script to fail before changing any deployed app.
- Serve the GacJS website with its existing isolation headers and verify `/wasm-fct/`, `/wasm-rpt/`, and `/wasm-rvmt/`. Confirm the browser loads the separate `app.worker.js`, worker imports resolve, startup succeeds, and there are no worker loading errors. Include Firefox coverage.
- Run the existing Wasm browser regression suite, following the updated `GacUI/.github/Jobs/job.rpWasm.prompt.md`. Verify rendering and input, RVMT Unicode translation, normal shutdown, Force Exit, and Reload with fresh application state and worker lifetimes.
- Verify the shared `app.sh`/`app.html` unit-test launcher still works with separate worker files.

## Completion

Record actual verification results and any limitations. Commit and push the implementation, documentation, and released tool copies in every affected repository.

# UPDATES

# TEST [CONFIRMED]

Use isolated packaging fixtures to compare app.mjs bytes before/after wasm.sh and verify the launcher, symlink and make target. Use copy-script fixtures to require all three matching files per demo and reject missing/empty inputs or missing destination pages before changing any deployment. Rebuild all three demos with build.sh -fbw -o, verify original exports and unchanged packaging, and compare all nine deployed files. Run GacJS import/codegen/build/test and the existing browser regression suite, including Firefox startup, input, Unicode RVMT, Exit/Force Exit/Reload and separate worker requests. Exercise a pthread-enabled library unit suite through the shared app.sh/app.html launcher. Confirm vgo uci distributes the canonical script and generated outputs remain ignored.

Baseline fixture execution confirmed both problems: packaging changes the generated module bytes, and copying succeeds with every pthread worker missing, deploying zero app.worker.js files.

# PROPOSALS

- No.1 Deploy the unmodified Emscripten module and adjacent pthread worker [CONFIRMED]

## No.1 Deploy the unmodified Emscripten module and adjacent pthread worker

Remove the optional module-rewriting block from the canonical packager. Keep the three vbuild files as minimal {"WASM=YES": {}} opt-ins: the build wrapper rejects Wasm builds without them. Share the three-file inventory between copy validation and copying. Preserve the existing module factory caller and allow Emscripten to resolve the adjacent pthread script/module normally. Update authored deployment guidance and shared copies from their owners, distribute Ubuntu tooling with vgo uci, then rebuild and verify real browser startup and regression behavior.

### CODE CHANGE

Removed the complete rewriting block from Tools/Ubuntu/vl/wasm.sh and propagated it using vgo uci to all nine configured C++/Release repositories. Kept minimal Wasm opt-ins in all three demo vbuild files. The copy script now shares one three-file inventory for validation and copying, and requires nonempty regular source files before writing any deployment. Updated Tools, GacUI and GacJS build/deployment guidance, the Wasm job, browser verification instructions and the three GacUI-owned knowledge-base pages; synchronized their existing shared copies through Tools/Copilot. The existing worker.ts factory invocation and tracked C++ sources remain unchanged. The requested vgo uci release also refreshed four older build-tool copies in VlppParser; its packager already matched the new canonical script.


### CONFIRMED

Verified on Linux with Emscripten 3.1.6 and Playwright Chromium/Firefox (Firefox 146.0.1). The unmodified factory works with the existing GacJS worker.ts caller: both browsers load one GacJS wasm-worker.js and 16 workers at each demo's adjacent app.worker.js URL. Evaluation inside a pthread worker confirms that its resource timing contains the matching adjacent app.mjs import. All six browser/app startup probes rendered the expected application without page or worker loading errors.

| Demo | Full rebuild | Chromium regression coverage | Firefox coverage |
| --- | --- | --- | --- |
| WasmFCT | 183 translation units, passed | Lists, search/rich editors, tab retention, palette, shortcuts/mouse, Exit/Force Exit and fresh Reload | List input, both shutdown paths, worker disappearance before Reload, restored empty lists |
| WasmRPT | 184 translation units, passed | Home, grid, document dialog, shortcuts/mouse, canceled/confirmed Exit, Force Exit, Reload, File-menu close and exact fatal error | Home input, Exit cancellation/confirmation, Force Exit, workers stopped, clicked state reset on Reload |
| WasmRVMT | 183 translation units, passed | Unicode translation, both shutdown paths, recreated host and fresh input after Reload, live-page closure | ASCII and Unicode renderer events round-trip through RPC, both shutdown paths return 0, workers stopped before Reload, fresh host and greeting |

All three full builds used ../../../.github/Ubuntu/build.sh -fbw -o from their own directories, including VCZH_DEBUG_NO_REFLECTION and -O3. Their generated modules retain the original export default Module; and contain no custom embedded-worker wrapper. Repackaging each real module preserves its SHA-256 hash exactly. Each generated app.mjs/app.wasm/app.worker.js exists and is nonempty; all nine deployed files match the corresponding source bytes. A fresh GacJS build creates the three pages without deploying any Wasm artifacts; the explicit copy script performs deployment afterward.

Module SHA-256 values after the full rebuilds and unchanged repackaging:

- FCT: c78925ba137ba4f6b5659b9bd4cdf597d9abda18b945333c4eaa68d99622dffb
- RPT: d4da7b6afd4de541d57d06efdeaeaffbbf10e95dcf5e0751e95f89459ce0aabe
- RVMT: b60c0575e0e55fe81e3d2e5147b80dae3845a14c77c04678b999e4f26b217932

Additional verification:

- Packaging fixtures preserve module, worker and Wasm bytes, executable launcher, index symlink and make target with absent, minimal or obsolete configuration. No JavaScript export syntax is inspected or rewritten.
- Thirty copy rejection fixtures cover every source file as missing, empty or a directory, plus each missing destination page. Every rejection leaves all previously deployed files unchanged, including when the failure is in the last demo. The successful control copies all nine matching files.
- GacJS yarn run import, yarn codegen, yarn build and yarn test passed; all 11 packages' test commands completed successfully. Import/codegen produced no tracked changes.
- The existing npm run test-wasm suite passed all five cases in 48.27 seconds. Its Chromium CDP checks verify actual worker targets disappear after normal shutdown, fatal failure and live-page closure.
- Rebuilt the pthread-enabled VlppOS unit suite with the released wrapper and served its generated Bin/app.sh on port 8897. The shared app.html launcher passed 10/10 files and 107/107 cases in each of Chromium and Firefox, ending with exactly one wasm_main returns 0. and no page errors.
- Canonical wasm.sh matches all nine vgo uci destinations. Shell syntax and git diff --check passed. No demo Bin artifacts are tracked. The old investigation was archived byte-for-byte before replacement; active build/configuration/deployment guidance has no remaining embedding option.

Verification limits and harness corrections: coverage is Linux. Windows/macOS, physical OS hot keys and native IME behavior were not exercised. Chromium's existing suite supplies Unicode through CDP; Firefox uses explicit DOM keydown/keyup events because Playwright press rejects those Unicode key names and type uses insertText without the renderer's key events. Firefox's cached Page.workers list after Page.close is not an authoritative lifetime check (the installed Playwright client does not clear that list on page closure), so its lifetime assertions are made on the live page after normal shutdown, before Reload. An additional Firefox about:blank navigation probe was interrupted by Firefox; it is not counted as verified. Chromium covers live-page closure. No product workaround was introduced for these harness limitations. Native GacUI tests were not rerun for this script/documentation-only change; the rebuilt demos, package tests and shared Wasm unit launcher cover the changed paths.

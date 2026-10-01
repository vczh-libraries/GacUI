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

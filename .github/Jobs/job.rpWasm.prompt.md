# Ensure GacUI Working Properly with WebAssembly on Linux

## Goal

- Work in the sibling `GacUI` and `GacJS` checkouts on Linux.
- Build and verify all three no-reflection Wasm apps with the browser renderer.
- Follow `DebugRemoteProtocolSop.md` for feature operations and expected results. Fix observed failures before finishing.

## Build and Run

1. Install Emscripten, Node.js, Yarn and the GacJS Playwright Chromium and Firefox browsers. Build each GacUI project from its own directory:
   - `Test/Linux/WasmFCT`: `../../../.github/Ubuntu/build.sh -bw -o`
   - `Test/Linux/WasmRPT`: `../../../.github/Ubuntu/build.sh -bw -o`
   - `Test/Linux/WasmRVMT`: `../../../.github/Ubuntu/build.sh -bw -o`
   - Use `-fbw -o` for a full rebuild, including when replacing previously patched modules. All three use `VCZH_DEBUG_NO_REFLECTION`, merged DarkSkin sources from `Source/Skins/DarkSkin/Source`, x86 demo sources and Wasm `-O3`.
   - Native compile/link check: run `../../../.github/Ubuntu/build.sh -f -o` and then `./Bin/WasmFCT`, `./Bin/WasmRPT` or `./Bin/WasmRVMT` in the corresponding directory. Each links the full source inventory with x64 demo sources and runs an empty `VCZH_GCC` main returning 0. Rebuild with `-bw -o` afterward because a full build cleans `Bin`.
2. In `GacJS/Gaclib`, run `yarn install`, `yarn run import`, `yarn codegen`, `yarn build` and `yarn test`.
3. Run `GacJS/copy-wasm.sh` explicitly. It validates every destination page and each nonempty `app.mjs`, `app.wasm` and `app.worker.js` before modifying any deployed file, then copies each matching set beside its page. Missing or empty inputs stop deployment with a warning. Website builds never copy Wasm files.
4. In `GacJS/Gaclib/website/entry`, run `npm run start` in an interactive terminal. Keep it running while testing; press ENTER to stop.
5. Open `http://localhost:8896/wasm-fct/`, `/wasm-rpt/` and `/wasm-rvmt/`. The server supplies module/Wasm MIME types and COOP/COEP headers. Do not use `file://` or a server without isolation headers.

Deploy `Bin/app.mjs`, `Bin/app.wasm` and `Bin/app.worker.js` for each app; all generated outputs remain ignored build artifacts and must not be committed. Packaging leaves `app.mjs` unchanged. Emscripten loads the adjacent `app.worker.js`, which imports the adjacent `app.mjs`. GacJS's `wasm-worker.js` is the separate application host. No native Core, HTTP protocol listener or external view-model host is needed. RVMT uses the browser TypeScript host and generated x86 RPC binding.

## Verification

Maintain current results in `.github/TaskLogs/Copilot_Investigate.md` with one row for FCT, RPT and RVMT. Record failures and fixes as they occur. Run `npm run test-wasm` from the running entry package for the browser regression suite.

- FCT: both lists, search/rich editors, tab-state retention, shortcuts, mouse input, palette refresh and Force Exit.
- RPT: Home button, DataGrid, document dialog, shortcuts, mouse input, confirmed File-menu close and the exact Core-authored fatal error.
- RVMT: initial greeting, repeated Unicode `Translate`, normal close and a recreated TypeScript host after Reload.
- On all pages, verify Reload is disabled initially and while running. Exit or Force Exit must finish Core shutdown before enabling Reload; canceling RPT's Exit confirmation must leave it disabled. Reload the page and verify fresh UI state, working input/RPC and a disabled Reload button. Repeat with both shutdown buttons and verify old workers disappear. Startup installs all transport connections through `StartApplication`; there are no exported connect/disconnect operations.
- Verify startup has no error mask, browser-input round trips work, terminal errors preserve the exact Core message, and closing a page stops its worker and pthreads. Browser-reserved shortcuts and native OS global hot keys are separate limitations; do not report them as native input coverage.
- In Chromium and Firefox, verify separate `app.worker.js` requests, successful worker module imports, startup and input without worker loading errors. Keep the Chromium regression suite's worker lifetime checks.
- Check that missing or empty source files (especially the final demo's `app.worker.js`) and missing destination pages fail before any deployed file changes. Compare all nine deployed files with their respective build outputs; a normal website build must leave Wasm deployment to the explicit copy step.
- Compare generated `app.mjs` bytes before and after packaging, ensure its original default factory export remains and no embedded-worker wrapper is appended. Verify the shared `app.sh`/`app.html` launcher using a pthread-enabled library unit suite over HTTP.

## Finishing

Record actual verification and platform limitations. Commit and push all local changes in every affected repository.

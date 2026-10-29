This is a continuation of previous works:
- TODO_Injection3.md

When you are on Linux, you are working on `wGac` repo.
When you are on macOS, you are working on `iGac` repo, and you need to skip all WASM related content.
According to previous works, make sure new features work on `wGac` or `iGac`:
- Make sure `./import.sh` and `./syncProj.sh` properly updates source files.
- Start `RemotingTest_Core` with `/MiniHttp` and some or all of `/remoteDT`, `/remoteLocale` and `/remoteFS`. Select the GacJS renderer's destination using the `corehost=<host>` URL option, and make sure its offered implementations are working.

### DETAILS

- This is integration and verification of `<GacUI repo>/TODO_Injection3.md`. GacJS already contains the three browser implementations and HTTP query options; synchronize the current sources before deciding what still needs fixing. Linux uses wGac and macOS uses iGac, with their sibling GacUI, GacJS and upstream library checkouts as needed. macOS still performs this native-Core/browser work; only the WASM section is excluded.
- Both platform import scripts consume `<GacUI repo>/Import` and `<GacUI repo>/Release`, including the separate test-helper imports. Publish any upstream changes through their owning release workflow before running the active platform's import script. The platform synchronization scripts regenerate application resources and copy the canonical renderer entry point, shared arguments, FullControlTest event handlers and RemoteViewModelTest initialization. Fix their canonical inputs and script inventories rather than editing imported or generated copies.
- Core is the server and rejects `/corehost`. In `<wGac repo>/test_core.sh` or `<iGac repo>/test_core.sh`, spell the wrapper options `--remoteDT`, `--remoteLocale` and `--remoteFS`; executable arguments remain slash-prefixed. The GacJS HTTP entry uses `corehost=<host>`; native renderers and network view-model hosts use `/corehost:<host>` instead. Protocol port 8888 is independent of automation ports.
- For example, launch Core through the active platform's test-core wrapper with `--app:fct --protocol:minihttp --remoteDT --remoteLocale --remoteFS`, then open the GacJS HTTP entry with `?remoteDT&remoteLocale&remoteFS&corehost=localhost`. Query names are case insensitive; the native switches are case sensitive. The HTTP page's optional `rvmhost` uses the same destination as its renderer. Preserve the existing checkbox/REFRESH behavior from `<GacUI repo>/TODO_Injection3.md`.
- Preserve independent two-sided opt-in. Core disabled uses its predecessor regardless of renderer capabilities. Core enabled with no corresponding renderer capability uses the DateTime/Locale predecessor or an empty read-only `/` filesystem. Both enabled selects the browser provider. Renderer replacement reevaluates capabilities; previously cached application values need not change automatically.
- The browser filesystem is the page origin's OPFS, following `<VlppOS repo>/Source/FileSystem.Wasm.cpp`. It is neither Core's native storage nor a file-picker grant. Preserve the path, stream, metadata and asynchronous completion contracts detailed in `<GacUI repo>/TODO_Injection3.md`. Image encoding remains available independently of filesystem injection.
- Use FCT for renderer-only loss/replacement checks. REFRESH on a page owning `rvmhost` also destroys its accepted view-model host, which is fatal to that RVMT Core; restart Core for that case. Preserve existing transport loss detection without adding heartbeats, recovery or a shutdown handshake.

### VERIFICATION

These are checks for task execution; this review does not run the implementation or platform tests.

1. In the active platform checkout, run its prescribed import, project synchronization and build workflows. Confirm the updated library/test-helper inventories and copied entry points are consumed by the build. Repeat synchronization with unchanged inputs and require no additional source differences. Verify the injection switches are forwarded in both foreground and `--unblock` launcher paths. Run native applications with an installed UTF-8 locale for Unicode filenames.
2. From `<GacJS repo>/Gaclib`, run the prescribed dependency setup, `yarn run import`, `yarn codegen`, `yarn build` and `yarn test`; a second code generation must introduce no additional diff. Use the setup and browser-operation procedures in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithGacJS.md` for the focused injection checks below: Firefox on Linux and Playwright WebKit on macOS, recording any separate Safari coverage. A skipped Windows-only E2E suite is not live browser verification.
3. For each feature independently, run the four Core/browser combinations below with the other features disabled, then check all three enabled together and a mixed selection. Drive actual FullControlTest browser UI and inspect Core automation. Use fresh queries and distinguishable timezone, locale and filesystem fixtures or request traces; identical results or the localization page's cached values cannot prove provider selection.

   | Core flag | Browser flag | Required provider/result |
   | --- | --- | --- |
   | Off | Off | Core predecessor. |
   | Off | On | Core predecessor; no injection requests for that feature. |
   | On | Off | DateTime/Locale predecessor; empty `/` for filesystem. |
   | On | On | Browser provider. |

4. Apply the feature-specific verification in `<GacUI repo>/TODO_Injection3.md`: timezone conversion in both directions, milliseconds and seasonal/date boundaries; locale defaults, enumeration, format patterns, names and case mapping; OPFS Unicode paths, binary/empty files, ReadWrite preservation, WriteOnly replacement, close persistence and ordinary failure behavior. In both FCT document editors, load/edit/save/reload a document with embedded images and export it. Seed fixtures into the same origin's OPFS, verify persistence across REFRESH, and confirm remote filesystem operations leave Core-native storage unchanged. Remove temporary fake providers and probes, rebuild and repeat a real-provider smoke check before the final commit.
5. With FCT, inspect fresh results after Core has actually observed renderer loss, then replace the renderer with capabilities off and on. Require the fallback/provider rules above without a hang or stale response. Do not equate closing a tab with an already delivered disconnect. Verify default localhost and a non-default `corehost`, including supported hostname/IPv4/IPv6 forms, reach the intended server; same-machine tests alone do not prove access from another computer.
6. Smoke-test RPT and RVMT with the updated shared protocol. In RVMT browser-host mode, prove a real Unicode `Translate` call reaches the selected Core and apply the documented Core restart after REFRESH. Record actual commands, flag combinations, browser/platform and observable results; identify unavailable platform coverage separately from successful runs. Update existing affected usage documentation without copying run-specific results into shared instructions.

When you are on Linux, make WASM works:
- WASM version of GacUI core will by default reject remote implementation (including date time, locale and file system), this could be done by correctly configuring remote protocol on each app's main function, instead of hardcoding in GacUI itself.
- Compile WASM apps and verify them with all of GacJS's WASM pages. The WASM core should use its own date/time, locale and filesystem implementations, so these pages do not need a renderer enable/disable test matrix. Report all three renderer injection capabilities as false through the existing `ControllerConnect` event's `ControllerGlobalConfig` argument.

### DETAILS

- The three applications are `<GacUI repo>/Test/Linux/WasmFCT`, `<GacUI repo>/Test/Linux/WasmRPT` and `<GacUI repo>/Test/Linux/WasmRVMT`. They share startup in `<GacUI repo>/Test/Linux/Wasm/WasmApplication.cpp`. Its current `SetupRemoteNativeController` call already inherits an all-false `GuiRemoteControllerConfig`; express the demo policy there explicitly instead of changing GacUI's configurable library behavior or creating three duplicate startup implementations.
- The renderer-to-Core `ControllerConnect` event and its `ControllerGlobalConfig` argument are defined in `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Controller.txt`. The generated API is `OnControllerConnect`. The WASM renderer must advertise `injectDateTime`, `injectLocale` and `injectFileSystem` as false, independently of the Core configuration.
- `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/index.ts` currently creates the shared protocol client without injection options; `<GacJS repo>/Gaclib/website/remote-protocol-http/src/rendererClient.ts` already serializes omitted options as false. Make the WASM demo's disabled choice explicit at its startup composition and preserve the HTTP entry's query-controlled opt-in. Do not add capability toggles or a renderer enable/disable test matrix to the WASM pages.
- Disabling injection retains the platform predecessors. WASM filesystem operations use `OpfsFileSystemImpl` directly; its locale currently uses `EnUsLocaleImpl` from `<VlppOS repo>/Source/Locale.Linux.cpp`. Sharing a browser environment does not imply that this locale equals the renderer's Intl-based implementation. Verify existing local-provider behavior without expanding this task into a new WASM locale implementation.
- WASM pages own their Core, renderer and optional TypeScript view-model host. They use fixed WASM connections, with no native Core process, HTTP protocol listener or external view-model host. Shutdown followed by Reload creates a fresh application; HTTP renderer replacement does not apply. Normal demo reload must preserve OPFS data.

### VERIFICATION

1. On Linux, follow `<GacUI repo>/.github/Jobs/job.rpWasm.prompt.md` for the three application builds and browser regression workflow. Run `<GacUI repo>/.github/Ubuntu/build.sh -bw -o` from each application's directory, using the prescribed full rebuild when required. If performing the native compile/link checks, do them before the final WASM rebuild because they clean the same output directory. Skip this entire section on macOS.
2. After GacJS import/code generation/build/tests, explicitly run `<GacJS repo>/copy-wasm.sh`. Verify all nine matching `app.mjs`, `app.wasm` and `app.worker.js` files are deployed and the generated modules remain unchanged. Start the entry server with the required isolation headers, then run `npm run test-wasm` from `<GacJS repo>/Gaclib/website/entry` and exercise `/wasm-fct/`, `/wasm-rpt/` and `/wasm-rvmt/` in Chromium and Firefox. Retain the job's test card and record actual results.
3. Inspect the actual `ControllerConnect` payload on every page: all three injection fields must be present and false. During fresh DateTime, Locale and filesystem operations, verify no `InjectionLocalToUtc`, `InjectionUtcToLocal`, `InjectionLocale` or `InjectionFileSystem` requests are emitted and the existing local services work. A disabled renderer alone does not prove that Core rejected injection; confirm its startup configuration as well.
4. On WASM FCT, exercise the localization/date controls and both document editors. Verify OPFS load/edit/save/reload/export, Unicode names, empty/binary data and embedded images; saved bytes must survive application shutdown and page Reload. Continue supporting `ImageSave`, which is independent of remote filesystem injection. Do not use the unit-test launcher's storage-clearing fixture setup for demo persistence checks.
5. Preserve the existing WASM job's application and lifecycle assertions: browser input round trips, RPT close confirmation and exact fatal error, RVMT Unicode RPC, Exit/Force Exit, Reload enabled only after successful shutdown, fresh UI/RPC state after Reload, and old worker/pthread termination. Browser touch coverage remains applicable despite native wGac/iGac touch exclusions. Report browser execution separately from static checks, and keep generated WASM binaries ignored.

commit and push all local changes once finishing.

## REVIEW COMMENTS

No unresolved review comments. The details and execution verification above resolve the argument ownership, protocol naming, synchronization and provider-selection gaps using the current contracts.

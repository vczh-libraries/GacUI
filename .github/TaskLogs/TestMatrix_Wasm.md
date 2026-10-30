# Test Matrix Card 2026-10-30T05:48:21.982Z

Completed: 2026-10-30T11:41:01.475Z

| Test Item | Result |
| --- | --- |
| FCT | Passed: optimized native/Wasm builds; Chromium editors/state/palette; both browsers startup, lists, Raw Clock, touch, Exit/Force Exit and Reload |
| RPT | Passed: optimized native/Wasm builds; Chromium and Firefox startup/input/reload; Chromium grid/dialog, shortcuts, confirmed close and exact fatal-error message |
| RVMT | Passed: optimized native/Wasm builds; Chromium and Firefox greeting, repeated Unicode RPC, shutdown/reload and recreated TypeScript host |

## Observations and limitations

The shared pthread-enabled `app.html` launcher passed all eleven unit suites in Firefox over HTTP (4,287 cases), including GacUI’s 93 files and 1,900 cases. Each successful run used a fresh persistent profile, confirmed cross-origin isolation and completed with exactly one `wasm_main returns 0.` signal.

GacJS import, code generation, build and package tests passed (211 tests across 29 test files). Its Windows-only protocol E2E tests were skipped on Linux; the separate Wasm regression suite passed all seven Chromium and three Firefox tests. The two optional public-site tests were skipped because `GACUI_PUBLIC_DEMO_URL` was unset.

Packaging validation passed using an isolated copy of `<GacJS repo>/copy-wasm.sh`: a missing final worker, an empty final worker and a missing final destination page each failed before changing any of the nine destination files. A complete fixture copied all nine files exactly. Actual deployment passed: all nine deployed files match the builds byte for byte. Repackaging preserved all three app.mjs files and their original default factory exports. After the website export rebuilt FCT, all nine deployed artifacts still matched the browser-tested versions byte for byte.

Both browsers loaded separate pthread workers and their modules without startup errors. Exit and Force Exit completed Core shutdown and removed old workers before enabling Reload. Reload recreated fresh UI state and working input/RPC; canceling RPT Exit kept Reload disabled. Chromium also verified that closing a live page terminates its workers.

Chromium touch coverage used browser-level input against the Wasm renderer and checked worker/channel traffic, painted transforms and list identities. Firefox touch and its additional Unicode keyboard events used DOM-synthetic input. Browser-reserved shortcuts and OS global hot keys were not counted as native input coverage. Native demo checks compiled/linked the full source inventories and ran their empty Linux mains; graphical coverage came from the browser apps.

## Issues and fixes

Native resource generation exposed missing method declarations in saved `UNUSED_USER_CONTENT` sections. The corrected merger in `<Workflow repo>/Source/Cpp/WfMergeCpp.cpp` now retains those declarations. FullControlTest's x86 and x64 DocumentEditorBase recovery sections were regenerated with the original bodies from GacUI commit `034ccd709^`. Workflow code-generation regressions passed for both metadata architectures (121 cases each), including repeated removal/restoration, overloads, LF/CRLF input, named regions and legacy markers. Downstream native resource generation, metadata checks, remoting tests and GacUI unit tests passed. The packaged source in `<Release repo>/Tools/Executables/CppMerge` was refreshed from Workflow; its optimized executable passed removal, repeated generation and restoration checks for three overloaded methods. All demo browser checks passed after the fix. The GacUI Wasm unit suite also passed all 1,900 cases.

# Test Matrix Card 2026-11-04T06:38:46.156Z

## Test Matrix

| Test Item | 1st |
| --- | --- |
| FCT | 2026-11-04T09:48:03Z (passed) |
| RPT | 2026-11-04T09:48:03Z (passed) |
| RVMT | 2026-11-04T09:48:03Z (passed) |

## Observations and Limitations

- Linux verification used Chromium and Firefox. All three native `-f -o` compile/link checks and empty-main executions passed. All three full optimized Wasm builds passed with task-local Emscripten 3.1.51, which emits the separately requested `app.worker.js`. Final incremental builds incorporated upstream TUI message-label commits `062ed2a6c` / `fc3385c2d` and the accepted-file-result resource fix.
- GacJS install, import, codegen, build and tests passed; repeated codegen was stable. Explicit deployment matched all nine final artifacts byte-for-byte, preserved each original module/default factory and separate pthread worker, and kept `wasm-worker.js` distinct. Normal website builds left deployment to the explicit copy step.
- The final browser suite passed all 10 applicable tests. Two public-host tests were skipped because no deployment URL was supplied. Evidence: `<GacUI repo>/.github/TaskLogs/rp-20261103/wasm-direct-final-tests.log` and `wasm-direct-final-hashes.json`.
- FCT covered lists, independent editors and retained state, shortcuts/mouse, palette refresh, Raw Clock data and changing pixels, browser-level Chromium touch across the worker channel, and Firefox DOM-synthetic touch. Additional actual-Wasm rail/responsive checks passed in both browsers: four vertical tabs, keyboard selection, palette retention, repeated 240/350/440/680×320 grow/shrink and scrolling, account menus, 680×220/520 account layouts, equal footer widths, and fresh-size OK/Cancel/title-close paths. Evidence: `<GacUI repo>/.github/TaskLogs/rp-20261103/wasm-*-layouts.json` and corresponding screenshots.
- RPT covered Home, DataGrid, document modal, shortcuts/mouse, canceled and confirmed File close, and exact Core-authored `This is a fatel error!`. RVMT covered repeated Unicode RPC, normal close and a recreated TypeScript host.
- All pages verified initial/running Reload disablement, Exit and Force Exit completion before Reload, fresh state and input after Reload, separate worker requests/imports and worker cleanup on page close. Browser-reserved shortcuts and OS-global hot keys are not native desktop coverage; Firefox touch is explicitly DOM-synthetic.
- The shared HTTP launcher ran the pthread-enabled VlppOS suite built with system Emscripten 3.1.69 in both browsers: isolated pages, result 0, application worker plus 32 pthread workers, no page errors. Evidence: `<GacUI repo>/.github/TaskLogs/rp-20261103/library-launcher.*`.
- One Firefox FCT repeat reached the original 120-second limit during concurrent builds; its isolated repeat passed in 105 seconds. The final suite used a 180-second per-test limit and passed, with Firefox FCT finishing in 116 seconds. No second failure occurred.

## Issues Found and Fix

- Emscripten 3.1.51 exposed a missing direct `<algorithm>` include for `std::min`/`std::max` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.cpp`. Added the include; native and full Wasm builds passed.
- Firefox RVMT exposed dropped supplementary input. GacJS now emits ordered UTF-16 units for one Unicode code point, and UTF-32 Core combines valid surrogate pairs. Regression and live Unicode checks passed.
- Removing RVMT's referenced pthread worker originally returned success and overwrote earlier deployed apps. `<GacJS repo>/copy-wasm.sh` now validates every referenced worker before copying. All 23 integration tests passed, including missing/empty modules, Wasm, workers and destination pages, atomic failure, matching nine-file deployment, stale-worker removal and newer module-worker compatibility. Evidence: `<GacUI repo>/.github/TaskLogs/rp-20261103/wasm-deployment.json`.
- Native FCT/TUI verification exposed accepted filenames disappearing after reopening/canceling a picker and repainting. Both resource pages now retain their own copy of accepted paths. The final Wasm FCT resource was rebuilt, redeployed and passed the browser suite.

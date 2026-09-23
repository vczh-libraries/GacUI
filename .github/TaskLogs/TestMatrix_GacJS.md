# Test Matrix Card 2026-09-22 22:38:15 -07:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 22:50:42 |
| [Windows][`/RPT`][`/MiniHttp`] | 22:57:41 |
| [Windows][`/FCT`][`/Http`] | 22:58:16 |
| [Windows][`/FCT`][`/MiniHttp`] | 23:00:38 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 22:54:44 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 22:54:59 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 22:55:12 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 22:55:30 |
| [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`] | 22:55:44 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 22:55:56 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 22:56:15 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 22:56:35 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 22:56:53 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`] | 22:57:11 |

## Issues Found and Fix

## Verification scope

Requested: GacJS on Windows and local CppTest only. All other native targets are excluded; their result cells remain blank.

## Build and test progress

- GacUI Debug x64 build: zero warnings/errors. GacJS import, codegen and build passed.
- Nine non-entry GacJS package test runs and the two entry query-session tests passed.
- IIS-served index.html, index.js and global.css match the fresh build.
- Regenerated snapshot diff is stable across another codegen pass (SHA-256 1728e53358dff665be669c4c7474123b8d675af66fba0a17f4d1841dd436172c); all nine changed imported snapshot files match GacUI originals after newline normalization.
- Checked-in browser suite passed: 10 files / 55 tests. CppTest_Rvm compatibility cases were explicitly excluded.

## Supplemental UI checkpoints

- RPT /Http: exact initial/click marker, three populated DataGrid rows and clear, Document dialog, all three browser shortcut dialogs before/after two renderer handoffs, 200 mouse/modifier events, matching label styles, normal File/Close exit 0. Fresh fatal phase delivered one exact Core error package and one matching page error, displayed the fatal mask and exited 3.

- RPT /MiniHttp: complete normal SOP, all three browser shortcut dialogs before/after two handoffs, 200 mouse/modifier events, DataGrid add/clear, Document modal and normal exit 0. Fresh fatal run produced one exact Core error, one matching page error, fatal mask and exit 3.

- FCT /Http and /MiniHttp: both lists added exactly 0–9 and cleared; exact Search/rich-editor markers persisted through tabs and renderer takeover; Hello[Ab]{Cd} and outgoing bracket key codes 219/221 passed; all three browser shortcut dialogs and 200 mouse/modifier events passed per transport; Force Exit ended Core with exit 0.
- All ten RVM rows passed normal Translate, renderer takeover and subsequent Translate, second-host rejection for the six network/browser rows, and normal exit 0. The four stdio rows additionally posted exact !Exit and verified child reaping. All 20 host-loss runs delivered exactly one !Error and one matching page error with RemotingTest_RvmHost disconnected., then exited 3. Observed completion ranged from 89 to 5102 ms.

## Evidence and coverage notes

- Detailed local logs: %TEMP%/GacUI-rpWindows-20260922-browser.log, -unit-js.log, -ui.jsonl and -rvm-supplement.jsonl.
- Chromium keyboard/CDP input verified browser shortcut and mouse delivery. GacJS maps the global shortcut to IOGlobalShortcutKey while its page is active; these checks do not establish OS-wide browser hotkey registration. The actual Windows global-hot-key path was verified in local CppTest.
- The temporary walkthrough initially read confirmation bounds before modal layout settled and ignored paragraph span text when locating editor markers. Fresh settled geometry and exact editor-container text resolved these procedure errors. No application source fix was needed.

- Stale-input audit: all four RPT/FCT transport rows held an old renderer button-down POST across takeover, then released it after the new renderer showed Right button up!. The detached renderer settled on its success mask and the new state remained unchanged. Fresh FCT replacement renderers also displayed exact Hello[Ab]{Cd} with key codes 219/221 on both transports.
- Final cleanup found no owned Core, RVM host, CppTest or SEA process and no listener on protocol port 8888.

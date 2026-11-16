# Test Matrix Card 2026-11-16 05:01:59 -08:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 2026-11-16 05:30:01 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-11-16 05:31:39 |
| [Windows][`/FCT`][`/Http`] | 2026-11-16 05:39:49 (fixed) |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-11-16 05:39:49 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 2026-11-16 05:33:11 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`] | 2026-11-16 05:35:12 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 2026-11-16 05:38:27 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 2026-11-16 05:37:58 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over stdio `/Cli:<command>`] | 2026-11-16 05:37:11 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 2026-11-16 05:34:36 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`] | 2026-11-16 05:36:20 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 2026-11-16 05:39:05 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 2026-11-16 05:38:12 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over stdio `/Cli:<command>`] | 2026-11-16 05:37:25 |

## Issues Found and Fix

- FCT /Http clipboard shortcut: Ctrl+V was consumed by browser text input, while Copy/Cut and toolbar Paste used Core clipboard. Restored Core routing and browser-default suppression for clipboard shortcuts, with controlled-browser and live-Core regressions. Ribbon and Toolstrip clipboard checks now pass on both transports.
- FCT /Http inline-image caret: repeated clicks could place a caret inside an image placeholder, making inserted text fail to undo. Core hit testing now normalizes the chosen offset through the renderer's existing nearest-valid-caret query. Rebuilt-Core live checks restore the fixture exactly after insertion and Undo; the automated regression checks the leading image boundary.
- FCT document momentum test: both transports initially timed out waiting for an idle after a stationary Down. CDB and live position sampling confirmed correct stopping without repaint. The GacJS shared test now waits for a subsequent visible hover-color change before its unchanged-position assertion. Focused rerun: 2/2 passed; the subsequent full root suite also passed. Final post-fix validation is recorded below.
- FCT click-pairing harness: fresh editors contain an empty paragraph, so selecting the whole document cannot produce a nonempty range. The shared test now seeds text only for an empty document. It also creates its diagnostic directory before writing a timeout snapshot. The browser rerun passed both transports; the final rebuilt-Core rerun is recorded below.

## Verification Evidence

- Scope: Windows GacJS only; all 14 browser matrix rows. Native-renderer and TUI matrices are outside this request.
- Final result: all 14 rows passed their available checks. GacUI Debug x64 built with zero warnings/errors and its supported UnitTest run passed 538 cases in 20 files. GacJS import/codegen/build and all 11 root test packages passed; final entry totals are 81 unit tests and 71 live tests, with 23 expected platform skips. Both rebuilt-Core browser touch/click-pairing runs passed.
- Coverage limits: actual OS IME/candidate input and native global-hotkey activation were unavailable after the interactive-window acquisition failed. Browser CDP touch/composition is separate from physical-device input. UIA was excluded by the Windows job. The standard renderer exposes only Segoe UI in the font picker, so another-family picker selection was unavailable.
- Preparation, failures, fixes and subsequent successful reruns are recorded chronologically below.

- 2026-11-16 05:05:44: GacUI Debug x64 build passed (0 warnings, 0 errors); GacJS import/codegen/build passed. Root yarn test --concurrency 1 is running. IIS already serves the website on 8896; the optional Node server cannot bind that occupied HTTP.sys port.

- 2026-11-16 05:06:37: IIS served entry.js byte-for-byte from the fresh build; Chromium version 145.0.7632.6. Native IME attempt: launched headed Chromium and selected its returned window twice; computer-use get_window/activation failed both times with "window id 10027130 no longer belongs to Chrome; current owner is Chrome". No actual OS IME/candidate coverage is claimed. Automated CDP/composition coverage remains separate.

- 2026-11-16 05:08:10: Repeated yarn codegen passed; SHA-256 comparison found zero additional changes across all 5,470 generated/snapshot files.

- 2026-11-16 05:08:22: Snapshot audit is in progress; repeat generation was unchanged.

- 2026-11-16 05:09:11: Audited all 3597 changed snapshot files: each exactly matches its current GacUI source after CRLF normalization. Existing unrelated retained snapshots were not counted as newly imported evidence.

- 2026-11-16 05:28:12: Root package tests completed: entry unit tests 81 passed / 23 platform skips; live protocol tests 66 passed / 2 failed. Both touch failures required a fresh renderer idle after a stationary stopping Down. Focused reruns reproduced the failure; CDB confirmed OnStarted sets stopped=true, and a live stopping Down held the paragraph at y=286 for over 500 ms without a new idle. A visible hover-state barrier is under verification; no GacUI source change.

- 2026-11-16 05:29:13: Focused touch rerun passed on both /Http and /MiniHttp (2/2). The harness now observes a subsequent button hover-color change before sampling the stopped paragraph; the post-release-motion and 250 ms stationary assertions remain. This is a harness synchronization correction, not a GacUI behavior fix.

- 2026-11-16 05:30:04: [Windows][`/RPT`][`/Http`]: initial UI, button, both local shortcuts and complete five-button/modifier/wheel/double-click payload matrix passed. Browser URL stayed unchanged.

- 2026-11-16 05:30:13: [Windows][`/RPT`][`/Http`]: exactly three populated rows/nine cells and Clear; embedded RIGHT NOW modal opened and dismissed.

- 2026-11-16 05:31:06: [Windows][`/RPT`][`/Http`]: renderer-close reconnect and live takeover preserved state; repeated local shortcuts and five-button modifier checks passed; detached input emitted no further traffic.

- 2026-11-16 05:31:26: [Windows][`/RPT`][`/Http`]: renderer Exit cancellation retained the app; File > self.Close() confirmation ended Core with exit 0 and a normal terminal mask.

- 2026-11-16 05:31:27: [Windows][`/RPT`][`/Http`]: fresh Fatel Error session produced the exact Core fatal mask and one matching pageerror; Core exited nonzero.

- 2026-11-16 05:31:31: [Windows][`/RPT`][`/MiniHttp`]: initial UI, button, both local shortcuts and complete five-button/modifier/wheel/double-click payload matrix passed. Browser URL stayed unchanged.

- 2026-11-16 05:31:31: [Windows][`/RPT`][`/MiniHttp`]: exactly three populated rows/nine cells and Clear; embedded RIGHT NOW modal opened and dismissed.

- 2026-11-16 05:31:35: [Windows][`/RPT`][`/MiniHttp`]: closed-renderer reconnect, active-renderer takeover, retained button marker, detached-input rejection, repeated shortcuts and mouse matrix passed.

- 2026-11-16 05:31:38: [Windows][`/RPT`][`/MiniHttp`]: renderer Exit cancellation retained the app; File > self.Close() confirmation ended Core with exit 0 and a normal terminal mask.

- 2026-11-16 05:31:39: [Windows][`/RPT`][`/MiniHttp`]: fresh Fatel Error session produced the exact Core fatal mask and one matching pageerror; Core exited nonzero.

- 2026-11-16 05:31:57: RPT fatal response audit: exactly one ;!Error;This is a fatel error! package per transport; both Core exit codes were 3. Native OS IME and global-hotkey activation are unavailable because the interactive Chromium window could not be acquired; no synthetic input is credited as native global-hotkey evidence.

- 2026-11-16 05:33:14: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:33:41: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network]: second native host remained blocked without admission; terminating it left the accepted host alive for another successful Translate.

- 2026-11-16 05:34:19: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 258 ms.

- 2026-11-16 05:34:35: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 4945 ms.

- 2026-11-16 05:34:38: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:34:45: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network]: rejected second native network host; accepted service remained usable.

- 2026-11-16 05:34:49: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 307 ms.

- 2026-11-16 05:35:12: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 4987 ms.

- 2026-11-16 05:35:15: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:35:50: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: UI close ended Core with exit 0 and reaped its native stdio child.

- 2026-11-16 05:35:54: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: separate exact !Exit shutdown returned exit 0 and reaped its direct child.

- 2026-11-16 05:35:56: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 272 ms.

- 2026-11-16 05:35:59: [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 218 ms.

- 2026-11-16 05:36:23: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:37:06: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: direct child RemotingTest_RvmHost.exe used protocol /Cli; both UI close and a separate exact !Exit shutdown ended Core with exit 0 and reaped the child. Bracket key codes 0xDB/0xDD passed with/without Shift.

- 2026-11-16 05:37:08: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 236 ms.

- 2026-11-16 05:37:11: [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<command>`]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 275 ms.

- 2026-11-16 05:37:13: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:37:20: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: direct child node.exe used protocol /Cli; both UI close and a separate exact !Exit shutdown ended Core with exit 0 and reaped the child. Bracket key codes 0xDB/0xDD passed with/without Shift.

- 2026-11-16 05:37:22: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 276 ms.

- 2026-11-16 05:37:24: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 221 ms.

- 2026-11-16 05:37:27: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:37:34: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: direct child node.exe used protocol /Cli; both UI close and a separate exact !Exit shutdown ended Core with exit 0 and reaped the child. Bracket key codes 0xDB/0xDD passed with/without Shift.

- 2026-11-16 05:37:36: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 243 ms.

- 2026-11-16 05:37:39: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over stdio `/Cli:<command>`]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 217 ms.

- 2026-11-16 05:38:01: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:38:04: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network]: second Node host rejected, original service retained, UI close exit 0; bracket physical-key payloads passed.

- 2026-11-16 05:38:05: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 272 ms.

- 2026-11-16 05:38:12: [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 4956 ms.

- 2026-11-16 05:38:15: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:38:19: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network]: second Node host rejected, original service retained, UI close exit 0; bracket physical-key payloads passed.

- 2026-11-16 05:38:20: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 312 ms.

- 2026-11-16 05:38:27: [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network]: blocked delivery loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 4984 ms.

- 2026-11-16 05:38:30: [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:38:31: [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`]: second browser host rejected; original host served another Translate.

- 2026-11-16 05:38:52: [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`]: second browser host rejected and normal UI close exited 0 without host-loss error.

- 2026-11-16 05:38:58: [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 5270 ms.

- 2026-11-16 05:39:05: [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`]: blocked delivery/reply loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 5045 ms.

- 2026-11-16 05:39:08: [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`]: initial empty greeting, exact bracket/brace RPC, two renderer takeovers, retained service/state and detached-renderer input rejection passed.

- 2026-11-16 05:39:09: [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`]: second browser host rejected; original host served another Translate.

- 2026-11-16 05:39:11: [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`]: second browser host rejected and normal UI close exited 0 without host-loss error.

- 2026-11-16 05:39:13: [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`]: idle-next-call loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 269 ms.

- 2026-11-16 05:39:19: [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`]: blocked delivery/reply loss produced one exact Core ErrorChannel package, one matching pageerror, nonzero exit 3 in 5065 ms.

- 2026-11-16 05:39:31: All ten RVMT host/transport rows completed. Each kept its accepted host through two renderer takeovers, typed exact RPC markers, shut down normally, and passed fresh idle-next-call and blocked-call host-loss sessions. All fatal runs observed one exact ErrorChannel package and one matching pageerror. Non-CLI rows rejected a second host; all CLI rows reaped direct children on exact !Exit.

- 2026-11-16 05:41:46: The browser click-pairing script failed before touch selection because it required a nonempty Ctrl+Shift+End range in a fresh empty document. A fresh Core Controls read confirmed <Content><p/></Content>. Its failure dump also lacked its output directory. Correcting these harness preconditions before rerunning.

- 2026-11-16 05:44:52: Touching_Remote -Browser -Transport Http,MiniHttp -ClickPairing passed both transports after the harness corrections: manipulation pixels, contact cleanup/capture, swipe identity/recycling, momentum interruption, both editors long/short selection, real folder navigation, double-tap image insertion and Undo/Redo; no page errors, outer page stationary, clean Core shutdown.

- 2026-11-16 05:44:55: [Windows][`/FCT`][`/Http`]: fresh title and both TextLists passed exact 0–9 population and complete clearing.

- 2026-11-16 05:45:06: [Windows][`/FCT`][`/Http`]: Search and central document typed distinct markers and preserved both across tab navigation.

- 2026-11-16 05:46:40: [Windows][`/FCT`][`/Http`]: Ribbon context menu verified 13 ordered commands, five separators, Object submenu, edge placement, range 0:0–0:6 preservation, Copy/Cut/Paste/Undo/Redo/Select All/Delete with exact fixture restoration.

- 2026-11-16 05:49:15: [Windows][`/FCT`][`/Http`]: the standard GacJS connection advertises only Segoe UI; the font picker exposes that one family. Different-family selection is unavailable in this configuration. Size and cancel/apply checks continue; existing fixture font families render correctly.

- 2026-11-16 05:51:32: [Windows][`/FCT`][`/Http`]: Ribbon context and toolbar font size 20/22, foreground #22965A/#23965A and background #283CB4/#293CB4 passed cancel/apply and exact selection preservation.

- 2026-11-16 05:53:21: [Windows][`/FCT`][`/Http`]: Ribbon private save/reload preserved exact text, two images and selected formatting (22 px, #23965A/#293CB4, Right); nonempty RTF/HTML contained Unicode, formatting and embedded-image data.

- 2026-11-16 05:53:38: [Windows][`/FCT`][`/Http`]: Ribbon context hyperlink cancel/set/Preview activation/remove passed for https://example.invalid/Http-Ribbon-0 with range Selection(0,0)-(0,6).

- 2026-11-16 05:53:53: [Windows][`/FCT`][`/Http`]: Ribbon object hyperlink cancel/set/Preview activation/remove passed for https://example.invalid/Http-Ribbon-1 with range Selection(0,0)-(0,6).

- 2026-11-16 05:54:04: [Windows][`/FCT`][`/Http`]: Ribbon toolbar hyperlink cancel/set/Preview activation/remove passed for https://example.invalid/Http-Ribbon-2 with range Selection(0,0)-(0,6).

- 2026-11-16 05:54:55: [Windows][`/FCT`][`/Http`]: Ribbon object image cancellation/insertion/Undo/Redo retained exact image counts 2→3→2→3.

- 2026-11-16 05:57:28: [Windows][`/FCT`][`/Http`]: Ribbon style Header 1 rendered {"size":"24px","color":"rgb(255, 128, 0)","bold":"400","italic":"normal","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:57:32: [Windows][`/FCT`][`/Http`]: Ribbon style Header 2 rendered {"size":"19px","color":"rgb(255, 128, 0)","bold":"400","italic":"normal","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:17: [Windows][`/FCT`][`/Http`]: Ribbon style Header 1 rendered {"size":"24px","color":"rgb(255, 128, 0)","bold":"400","italic":"normal","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:22: [Windows][`/FCT`][`/Http`]: Ribbon style Header 2 rendered {"size":"19px","color":"rgb(255, 128, 0)","bold":"400","italic":"normal","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:27: [Windows][`/FCT`][`/Http`]: Ribbon style Header 3 rendered {"size":"15px","color":"rgb(255, 255, 255)","bold":"400","italic":"normal","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:32: [Windows][`/FCT`][`/Http`]: Ribbon style Strong rendered {"size":"12px","color":"rgb(255, 255, 255)","bold":"700","italic":"normal","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:37: [Windows][`/FCT`][`/Http`]: Ribbon style Quote rendered {"size":"12px","color":"rgb(255, 255, 255)","bold":"400","italic":"italic","decoration":"none"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:42: [Windows][`/FCT`][`/Http`]: Ribbon style Emphasis rendered {"size":"12px","color":"rgb(255, 255, 255)","bold":"400","italic":"italic","decoration":"underline"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:48: [Windows][`/FCT`][`/Http`]: Ribbon style Intense Emphasis rendered {"size":"12px","color":"rgb(128, 128, 255)","bold":"400","italic":"italic","decoration":"underline"}; reapplication kept range 0:0–0:6.

- 2026-11-16 05:58:53: [Windows][`/FCT`][`/Http`]: Ribbon style Deleted rendered {"size":"12px","color":"rgb(255, 255, 255)","bold":"400","italic":"normal","decoration":"line-through"}; reapplication kept range 0:0–0:6.

- 2026-11-16 06:00:20: [Windows][`/FCT`][`/Http`]: all eight ribbon styles passed rendered-attribute and reapplication checks; sizes are 24/19/15 px, matching Core integer truncation. Both wide dropdown and actual 640-pixel collapsed Style popup scrolled through Headers/Content, applied all eight entries, dismissed and retained range 0:0–0:6.

- 2026-11-16 06:02:10: [Windows][`/FCT`][`/Http`]: Ribbon toolbar Copy/Cut/Paste/Undo/Redo/Select All/Delete restored exact document text.

- 2026-11-16 06:09:13: [Windows][`/FCT`][`/Http`]: Ctrl+V failed twice after Core Copy/Cut although toolbar Paste worked. Wire events contained Ctrl but no V: textInput.keyDown swallowed Ctrl/Meta+V for browser paste, splitting keyboard paste from Core fake clipboard. Restoring normal Core shortcut routing and adding controlled/live regressions; browser-originated plaintext paste remains supported.

- 2026-11-16 06:10:40: [Windows][`/FCT`][`/Http`]: Ribbon shortcut Copy/Cut/Paste/Undo/Redo/Select All/Delete restored exact document text.

- 2026-11-16 06:17:46: [Windows][`/FCT`][`/Http`]: Reproduced a second issue three times: clicking blank space in the fixture final paragraph produced caret 5:23 inside the image run (Ctrl+End is 5:29). Typing Z there retained an unundoable Z; the same insertion at valid end 5:29 undid normally. Investigating browser paragraph hit testing.

- 2026-11-16 06:20:15: [Windows][`/FCT`][`/Http`]: Root cause is Core scanning one bounds entry per text offset without normalizing to valid renderer edit-unit boundaries. GetCaretFromPoint now snaps its result through the existing GetNearestCaretFromTextPos request, covering inline objects and glyph clusters without changing the protocol. Added a live image-adjacent click/insertion/Undo regression. Restarting Core for the rebuild; completed checks remain recorded.

- 2026-11-16 06:23:12: [Windows][`/FCT`][`/Http`]: fresh title and both TextLists passed exact 0–9 population and complete clearing.

- 2026-11-16 06:23:25: [Windows][`/FCT`][`/Http`]: rebuilt Core now snaps blank-space hit 5:23 to valid boundary 5:22; insertion and Undo restore exact fixture and both images.

- 2026-11-16 06:27:10: Rebuilt GacUI Debug x64: 0 warnings/errors. Supported UnitTest wrapper passed 20 files and 538 cases; full GacJS root test rerun is in progress.

- 2026-11-16 06:27:49: UnitTest regenerated 13 compiler text snapshots with declaration-order-only differences; line multisets matched HEAD exactly and this unrelated nondeterministic output was restored. No rendering snapshot changes were produced.

- 2026-11-16 06:33:12: Full GacJS root yarn test --concurrency 1 passed all 11 packages: entry unit 81 passed/23 platform skips; live protocol 71/71 passed. Refining the image regression click to remain inside the large image; focused and final checks will cover that adjustment.

- 2026-11-16 06:33:17: [Windows][`/FCT`][`/Http`]: fresh renderer ready

- 2026-11-16 06:33:42: [Windows][`/FCT`][`/Http`]: Ribbon focused Ctrl+Shift+P/S/E switched all three modes and Editable insertion undid exactly

- 2026-11-16 06:34:14: [Windows][`/FCT`][`/Http`]: Ribbon Preview/Selectable rejected mutations, Editable restored insertion and Undo

- 2026-11-16 06:34:30: [Windows][`/FCT`][`/Http`]: Ribbon Search and six Icon Labels fields accepted text, all six Item A/B samples dismissed without document mutation, Alignment expansion showed exact sample dialog

- 2026-11-16 06:37:02: [Windows][`/FCT`][`/Http`]: Ribbon palette/template replacement preserved document/range, one context popup and editing

- 2026-11-16 06:37:07: [Windows][`/FCT`][`/Http`]: Toolstrip exact context command groups, five separators, Object submenu, release-only opening, selection retention and edge placement passed

- 2026-11-16 06:37:19: [Windows][`/FCT`][`/Http`]: Toolstrip context clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 06:37:24: [Windows][`/FCT`][`/Http`]: Toolstrip toolbar clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 06:38:14: [Windows][`/FCT`][`/Http`]: Toolstrip formatting toggles and three paragraph alignments passed with range retained

- 2026-11-16 06:38:38: [Windows][`/FCT`][`/Http`]: Toolstrip context/toolbar font and foreground/background color cancel/apply passed; standard renderer offers Segoe UI only

- 2026-11-16 06:39:07: [Windows][`/FCT`][`/Http`]: Toolstrip private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 06:39:16: [Windows][`/FCT`][`/Http`]: Toolstrip object image cancel/insert/Undo/Redo passed 2→3→2→3

- 2026-11-16 06:39:24: [Windows][`/FCT`][`/Http`]: Toolstrip toolbar image cancel/insert/Undo/Redo passed 2→3→2→3

- 2026-11-16 06:40:47: [Windows][`/FCT`][`/Http`]: Toolstrip private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 06:41:12: [Windows][`/FCT`][`/Http`]: Toolstrip hyperlink cancel/apply/Preview activation/removal passed through context, Object and toolbar

- 2026-11-16 06:42:03: [Windows][`/FCT`][`/Http`]: Toolstrip Preview/Selectable rejected mutations, Editable restored insertion and Undo

- 2026-11-16 06:43:54: [Windows][`/FCT`][`/Http`]: Toolstrip palette/template replacement preserved document/range, one context popup and editing

- 2026-11-16 06:44:25: [Windows][`/FCT`][`/Http`]: Window Manager localized labels and both local shortcuts passed; native global-hotkey path remains unavailable in locked desktop

- 2026-11-16 06:44:29: [Windows][`/FCT`][`/Http`]: Window Manager five-button × five-modifier matrix, both wheel axes and no browser history navigation passed; label font/color matched shortcuts

- 2026-11-16 06:44:31: [Windows][`/FCT`][`/Http`]: Window Manager localized labels and both local shortcuts passed; native global-hotkey path remains unavailable in locked desktop

- 2026-11-16 06:44:35: [Windows][`/FCT`][`/Http`]: Window Manager five-button × five-modifier matrix, both wheel axes and no browser history navigation passed; label font/color matched shortcuts

- 2026-11-16 06:45:53: [Windows][`/FCT`][`/Http`]: Layout vertical header order, Up/Down, ALT labels [L]0,[L]1,[M],[X],[L]3,[W]0,[L]2,[R]0,[R]1,[E],[T],[W]1, five-pixel action gap and Default/Ember/Aurora/Default palette refresh passed with editor, checkbox and choice retained

- 2026-11-16 06:49:06: [Windows][`/FCT`][`/Http`]: Layout rail, keyboard/ALT navigation and four palette passes completed; responsive modal starts at 240×320, exact eight-account menu and initial selections verified.

- 2026-11-16 06:50:15: [Windows][`/FCT`][`/Http`]: Responsive 680×320 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:50:16: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:50:18: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:50:19: [Windows][`/FCT`][`/Http`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:50:20: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:50:22: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:51:31: [Windows][`/FCT`][`/Http`]: Responsive 680×320 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:51:33: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:51:35: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:51:36: [Windows][`/FCT`][`/Http`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:51:38: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:51:40: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:48: [Windows][`/FCT`][`/Http`]: Responsive 680×320 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:50: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:51: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:53: [Windows][`/FCT`][`/Http`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:54: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:56: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:52:59: [Windows][`/FCT`][`/Http`]: Responsive 680×220 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:53:02: [Windows][`/FCT`][`/Http`]: Responsive 680×520 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:53:04: [Windows][`/FCT`][`/Http`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:53:05: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:53:07: [Windows][`/FCT`][`/Http`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:53:08: [Windows][`/FCT`][`/Http`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:53:10: [Windows][`/FCT`][`/Http`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 06:54:09: [Windows][`/FCT`][`/Http`]: Responsive movie virtualization exposed all 24 exact titles/directors; publisher data and menu/list selection agree

- 2026-11-16 06:54:16: [Windows][`/FCT`][`/Http`]: Responsive resize/title drags retained capture outside renderer and released it; ordinary resize, OK/Cancel/title close, fresh dimensions and changed-palette reopen passed

- 2026-11-16 06:54:59: [Windows][`/FCT`][`/Http`]: Raw Clock: changing Core HH:mm:ss data matched changing pixels; hide/show, resize/DPR, palette and renderer replacement each retained one canvas

- 2026-11-16 06:56:02: [Windows][`/FCT`][`/Http`]: Ordinary TextList browser touch selected row 5, scrolled and reversed to its original offset with selection retained and no compatibility mouse down

- 2026-11-16 06:56:35: [Windows][`/FCT`][`/Http`]: HTTP FCT manual SOP completed after fixes; shared final touch regression remains to be rerun on the rebuilt Core.

- 2026-11-16 06:56:35: [Windows][`/FCT`][`/Http`]: Force Exit ended Core normally with no browser errors

- 2026-11-16 06:56:37: [Windows][`/FCT`][`/MiniHttp`]: fresh renderer ready

- 2026-11-16 06:56:39: [Windows][`/FCT`][`/MiniHttp`]: Fresh title, both exact 0–9 lists/clearing and Search/document typing persistence passed

- 2026-11-16 06:56:47: [Windows][`/FCT`][`/MiniHttp`]: Ribbon exact context command groups, five separators, Object submenu, release-only opening, selection retention and edge placement passed

- 2026-11-16 06:57:01: [Windows][`/FCT`][`/MiniHttp`]: Ribbon context clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 06:57:07: [Windows][`/FCT`][`/MiniHttp`]: Ribbon toolbar clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 06:57:13: [Windows][`/FCT`][`/MiniHttp`]: Ribbon shortcut clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 06:57:54: [Windows][`/FCT`][`/MiniHttp`]: Ribbon formatting toggles and three paragraph alignments passed with range retained

- 2026-11-16 06:58:40: [Windows][`/FCT`][`/MiniHttp`]: Ribbon context/toolbar font and foreground/background color cancel/apply passed; standard renderer offers Segoe UI only

- 2026-11-16 06:59:01: [Windows][`/FCT`][`/MiniHttp`]: Ribbon private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 06:59:15: [Windows][`/FCT`][`/MiniHttp`]: Ribbon object image cancel/insert/Undo/Redo passed 2→3→2→3

- 2026-11-16 06:59:24: [Windows][`/FCT`][`/MiniHttp`]: Ribbon toolbar image cancel/insert/Undo/Redo passed 2→3→2→3

- 2026-11-16 06:59:48: [Windows][`/FCT`][`/MiniHttp`]: Ribbon private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:00:59: [Windows][`/FCT`][`/MiniHttp`]: Ribbon hyperlink cancel/apply/Preview activation/removal passed through context, Object and toolbar

- 2026-11-16 07:01:15: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Preview/Selectable rejected mutations, Editable restored insertion and Undo

- 2026-11-16 07:01:25: [Windows][`/FCT`][`/MiniHttp`]: Ribbon focused Ctrl+Shift+P/S/E switched all three modes and Editable insertion undid exactly

- 2026-11-16 07:02:21: [Windows][`/FCT`][`/MiniHttp`]: Ribbon inline gallery Header 1 applied before dropdown coverage

- 2026-11-16 07:02:29: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Header 1 effective style {"size":"24px","color":"rgb(255, 255, 255)","bold":"400","italic":"normal","line":"none"}

- 2026-11-16 07:02:33: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Header 2 effective style {"size":"19px","color":"rgb(255, 255, 255)","bold":"400","italic":"normal","line":"none"}

- 2026-11-16 07:02:37: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Header 3 effective style {"size":"15px","color":"rgb(255, 255, 255)","bold":"400","italic":"normal","line":"none"}

- 2026-11-16 07:02:41: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Strong effective style {"size":"12px","color":"rgb(255, 255, 255)","bold":"700","italic":"normal","line":"none"}

- 2026-11-16 07:02:45: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Quote effective style {"size":"12px","color":"rgb(255, 255, 255)","bold":"400","italic":"italic","line":"none"}

- 2026-11-16 07:02:50: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Emphasis effective style {"size":"12px","color":"rgb(255, 255, 255)","bold":"400","italic":"italic","line":"underline"}

- 2026-11-16 07:02:56: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Intense Emphasis effective style {"size":"12px","color":"rgb(128, 128, 255)","bold":"400","italic":"italic","line":"underline"}

- 2026-11-16 07:03:00: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Deleted effective style {"size":"12px","color":"rgb(255, 255, 255)","bold":"400","italic":"normal","line":"line-through"}

- 2026-11-16 07:03:00: [Windows][`/FCT`][`/MiniHttp`]: Ribbon all eight wide gallery styles applied, scrolled and retained range

- 2026-11-16 07:03:16: [Windows][`/FCT`][`/MiniHttp`]: Ribbon all eight collapsed gallery styles applied, scrolled and retained range

- 2026-11-16 07:03:41: [Windows][`/FCT`][`/MiniHttp`]: Ribbon private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:03:55: [Windows][`/FCT`][`/MiniHttp`]: Ribbon Search and six Icon Labels fields accepted text, all six Item A/B samples dismissed without document mutation, Alignment expansion showed exact sample dialog

- 2026-11-16 07:04:13: [Windows][`/FCT`][`/MiniHttp`]: Ribbon palette/template replacement preserved document/range, one context popup and editing

- 2026-11-16 07:07:18: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip exact context command groups, five separators, Object submenu, release-only opening, selection retention and edge placement passed

- 2026-11-16 07:07:25: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip context clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 07:07:30: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip toolbar clipboard/Undo/Redo/Select All/Delete passed exact text; document endpoint is 5:29 including image placeholder.

- 2026-11-16 07:07:53: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip formatting toggles and three paragraph alignments passed with range retained

- 2026-11-16 07:08:15: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip context/toolbar font and foreground/background color cancel/apply passed; standard renderer offers Segoe UI only

- 2026-11-16 07:08:31: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:09:08: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip object image cancel/insert/Undo/Redo passed 2→3→2→3

- 2026-11-16 07:09:16: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip toolbar image cancel/insert/Undo/Redo passed 2→3→2→3

- 2026-11-16 07:09:32: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:09:57: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip hyperlink cancel/apply/Preview activation/removal passed through context, Object and toolbar

- 2026-11-16 07:10:07: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip Preview/Selectable rejected mutations, Editable restored insertion and Undo

- 2026-11-16 07:11:57: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip palette/template replacement preserved document/range, one context popup and editing

- 2026-11-16 07:11:57: [Windows][`/FCT`][`/MiniHttp`]: Palette retry waited for settled tab geometry and matching visible editor contents; no application change was needed.

- 2026-11-16 07:11:58: [Windows][`/FCT`][`/MiniHttp`]: Window Manager localized labels and both local shortcuts passed; native global-hotkey path remains unavailable in locked desktop

- 2026-11-16 07:12:01: [Windows][`/FCT`][`/MiniHttp`]: Window Manager five-button × five-modifier matrix, both wheel axes and no browser history navigation passed; label font/color matched shortcuts

- 2026-11-16 07:12:10: [Windows][`/FCT`][`/MiniHttp`]: Window Manager localized labels and both local shortcuts passed; native global-hotkey path remains unavailable in locked desktop

- 2026-11-16 07:12:14: [Windows][`/FCT`][`/MiniHttp`]: Window Manager five-button × five-modifier matrix, both wheel axes and no browser history navigation passed; label font/color matched shortcuts

- 2026-11-16 07:12:14: [Windows][`/FCT`][`/MiniHttp`]: Replacement renderer retained state and repeated local shortcut/mouse payload checks.

- 2026-11-16 07:12:21: [Windows][`/FCT`][`/MiniHttp`]: Layout vertical header order, Up/Down, ALT labels [L]0,[L]1,[M],[X],[L]3,[W]0,[L]2,[R]0,[R]1,[E],[T],[W]1, five-pixel action gap and Default/Ember/Aurora/Default palette refresh passed with editor, checkbox and choice retained

- 2026-11-16 07:12:28: [Windows][`/FCT`][`/MiniHttp`]: Responsive 680×320 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:30: [Windows][`/FCT`][`/MiniHttp`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:31: [Windows][`/FCT`][`/MiniHttp`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:33: [Windows][`/FCT`][`/MiniHttp`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:34: [Windows][`/FCT`][`/MiniHttp`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:36: [Windows][`/FCT`][`/MiniHttp`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:39: [Windows][`/FCT`][`/MiniHttp`]: Responsive 680×220 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:41: [Windows][`/FCT`][`/MiniHttp`]: Responsive 680×520 appearance #3 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:43: [Windows][`/FCT`][`/MiniHttp`]: Responsive 440×320 appearance #2 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:45: [Windows][`/FCT`][`/MiniHttp`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:46: [Windows][`/FCT`][`/MiniHttp`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:48: [Windows][`/FCT`][`/MiniHttp`]: Responsive 350×320 appearance #1 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:12:49: [Windows][`/FCT`][`/MiniHttp`]: Responsive 240×320 appearance #0 stable; row height 106, both movie-list ends and footer verified

- 2026-11-16 07:13:01: [Windows][`/FCT`][`/MiniHttp`]: Responsive movie virtualization exposed all 24 exact titles/directors; publisher data and menu/list selection agree

- 2026-11-16 07:13:08: [Windows][`/FCT`][`/MiniHttp`]: Responsive resize/title drags retained capture outside renderer and released it; ordinary resize, OK/Cancel/title close, fresh dimensions and changed-palette reopen passed

- 2026-11-16 07:15:02: [Windows][`/FCT`][`/MiniHttp`]: Raw Clock: changing Core HH:mm:ss data matched changing pixels; hide/show, resize/DPR, palette and renderer replacement each retained one canvas

- 2026-11-16 07:15:48: [Windows][`/FCT`][`/MiniHttp`]: Ordinary TextList Chromium touch selected item 15, scrolled and returned to the initial viewport without losing selection or emitting mouse button input.

- 2026-11-16 07:15:51: [Windows][`/FCT`][`/MiniHttp`]: Force Exit ended Core normally with no browser errors

- 2026-11-16 07:17:02: All 14 Windows GacJS matrix rows completed their available SOP operations. Final rebuilt-Core touch/click-pairing and root GacJS regression reruns follow. Actual OS IME/candidate input and native global-hotkey activation remain unavailable after the documented interactive-session attempt; browser CDP touch/composition is not physical-device evidence. UIA is excluded by the Windows job. The standard renderer advertises Segoe UI only, so another-family picker selection is unavailable.

- 2026-11-16 07:19:21: Final audit: all 3597 changed imported snapshot JSON files still match GacUI upstream after newline normalization; no snapshot was hand-edited. The repeat codegen check previously found zero changes across 5,470 generated files.

- 2026-11-16 07:19:51: Final rebuilt-Core Touching_Remote -Browser -Transport Http,MiniHttp -Backend Direct2D -ClickPairing passed both transports. Each completed manipulation/capture/cancel, swipe identity/recycling, momentum, both-editor short/long point selection and double-tap file insertion with Undo/Redo. Both reported errors: [] and clean Core shutdown.

- 2026-11-16 07:19:53: [Windows][`/FCT`][`/Http`]: fresh renderer ready

- 2026-11-16 07:20:29: [Windows][`/FCT`][`/Http`]: Ribbon private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:20:29: [Windows][`/FCT`][`/Http`]: Private save/reload also preserved exact XML with a retained hyperlink, Deleted style, paragraph alignment and both embedded images.

- 2026-11-16 07:21:35: [Windows][`/FCT`][`/Http`]: Toolstrip private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:21:35: [Windows][`/FCT`][`/Http`]: Toolstrip save/reload preserved exact XML with retained hyperlink, strike/style, alignment and two images; RTF Unicode escapes and HTML hyperlink also verified.

- 2026-11-16 07:21:35: [Windows][`/FCT`][`/Http`]: Force Exit ended Core normally with no browser errors

- 2026-11-16 07:21:37: [Windows][`/FCT`][`/MiniHttp`]: fresh renderer ready

- 2026-11-16 07:22:13: [Windows][`/FCT`][`/MiniHttp`]: Ribbon private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:22:13: [Windows][`/FCT`][`/MiniHttp`]: Ribbon save/reload preserved exact XML with retained hyperlink, strike/style, alignment and two images; RTF Unicode escapes and HTML hyperlink also verified.

- 2026-11-16 07:23:19: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip private persistence, RTF/HTML image/Unicode export and dialog cancellation passed

- 2026-11-16 07:23:19: [Windows][`/FCT`][`/MiniHttp`]: Toolstrip save/reload preserved exact XML with retained hyperlink, strike, alignment and two images; RTF Unicode escapes and HTML hyperlink also verified.

- 2026-11-16 07:23:22: [Windows][`/FCT`][`/MiniHttp`]: Force Exit ended Core normally with no browser errors

- 2026-11-16 07:23:40: Combined persistence checks additionally passed for Ribbon and Toolstrip over both transports: private save/reload retained exact XML with hyperlink, style/strike, alignment and two images; RTF Unicode and HTML link checks passed. All owned manual Core/browser sessions are closed. Final root yarn test --concurrency 1 is now running against the final build and tightened inline-image regression.

- 2026-11-16 07:32:55: Final semantic-regression run passed all 11 GacJS packages: 81 entry unit tests passed with 23 expected platform skips, and 71 live tests passed in 14 files, including the tightened image-boundary insertion/Undo check. Review then made the hover-color condition an explicit nonempty-string comparison to satisfy repository JavaScript conventions; required build/test checks are running again on that final tree.

- 2026-11-16 07:37:09: GacJS handwritten changes are committed as f8bc5cc; imported snapshots are in the separate 3f89bfc [snapshot] commit. Final-tree root validation is still running before publication.

- 2026-11-16 07:40:23: Final-tree validation completed successfully: root yarn build passed, then root yarn test --concurrency 1 passed all 11 packages. Entry unit tests: 81 passed, 23 expected platform skips. Live Core suites: 71/71 tests in 14/14 files, including clipboard shortcuts in both input adapters, inline-image boundary/Undo, and both-transport document momentum. No further source changes followed this run.

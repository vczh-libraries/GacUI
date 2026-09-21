# Test Matrix Card 2026-09-21 12:50:00 -07:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 13:19:14 |
| [Windows][`/RPT`][`/MiniHttp`] | 14:06:44 |
| [Windows][`/FCT`][`/Http`] | 13:29:00 |
| [Windows][`/FCT`][`/MiniHttp`] | 13:57:59 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 14:15:27 (fixed) |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 14:17:05 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 14:17:35 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 14:18:15 |
| [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`] | 14:18:46 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 14:19:34 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 14:20:11 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 14:20:45 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 14:21:23 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`] | 14:22:16 |

## Issues Found and Fix

### `/FCT` `/Http` shortcut assertion

The initial checked-in browser suite failed to find `Ctrl+Q` immediately after selecting Window Manager. Five focused repetitions and the clean browser retry passed without a source change, so the assertion was not reproduced.

### Windows automation `/Http` blocked-loss shutdown

The blocked-loss failure reproduced twice: after a pending Controls read, the remoting renderer entered its fatal error path, but Core did not exit. CDB showed `HttpServerApi::Stop` waiting for HTTP callbacks while the Controls worker waited synchronously for UI work after the fatal event loop ended. The fix makes the Windows automation GET respond asynchronously with shared shutdown cancellation. A fresh Debug x64 rebuild passed with zero warnings and errors, and two exact post-fix blocked-loss reruns delivered one error and one page error before Core exited 3 near the five-second deadline.

## Verification progress

- Fresh GacUI Debug x64 solution build passed with zero warnings and errors. GacJS import, codegen, and build passed; IIS assets at port 8896 match the fresh output.
- Codegen idempotence passed: 5,424 generated files kept the same SHA-256 aggregate, and all 204 changed or new imported snapshots match the GacUI source snapshots after newline normalization.
- Workflow RPC conformance retry passed all 9 tests. A clean website-entry retry passed 11 live browser files / 56 tests plus 2 portable query-session tests; the initial browser assertions did not reproduce.
- UnitTest passed 93/93 files and 1,798/1,798 cases; the final Execute.log has no post-summary leak dump.
- All four supplemental FCT/RPT UI rows completed their available browser SOP operations. Final combined `yarn test` passed: all 10 packages, 11 live browser files, and 58 tests.

## Unavailable physical-input coverage

The Windows desktop-control API returned access denied for cursor/focus access. Headless browser keyboard and protocol-payload checks do not establish real Windows global-hot-key dispatch, so physical global-hot-key coverage remains unverified for applicable rows.

## Supplemental checkpoints

- RPT `/Http` passed the normal feature path, three-row DataGrid/clear, Document modal, both local shortcut dialogs, two state-preserving renderer takeovers, and File/Close with Core exit 0. A fresh fatal run showed one exact Core `!Error`, one matching page error, the expected error mask, and exit 3.
- FCT `/MiniHttp` passed Lists add/clear, exact editor markers, bracket codes 219/221 and text restoration, local shortcut dialogs, and five-pattern mouse payloads before and after renderer replacement. Active Control markers survived replacement; visible Force Exit ended Core with exit 0.
- RPT `/MiniHttp` passed the DataGrid/Document path, five-pattern mouse payloads, two state-preserving renderer takeovers, local shortcut dialogs after visible activation, and File/Close with Core exit 0. After the second handoff, keyboard-only Ctrl+Q showed no dialog until a harmless visible click; after activation both local shortcut dialogs passed, Controls had no subwindows, and no modifiers remained held. A separate fatal run showed one exact Core `!Error`, one matching page error and the expected error mask, then exited 3.
- All 10 supplemental RVM rows passed for native, native CLI, browser host, Node, and SEA hosts on `/Http` and `/MiniHttp`. Normal/replacement cases exited 0; CLI/SEA exact-`!Exit` cases reaped their child; idle-loss and blocked-loss each produced one Core error, one matching page error, and exit 3. Browser-host blocked loss held both the `Hello, ReadyX!` Translate response and replacement poll before host stop. Phase evidence is in the helper JSONL log.

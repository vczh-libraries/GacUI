# Test Matrix Card 2026-08-25 23:16:38

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 2026-08-26 00:44:38 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-08-26 00:46:48 (fixed) |
| [Windows][`/FCT`][`/Http`] | 2026-08-26 00:51:01 (fixed) |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-08-26 00:54:02 (fixed) |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 2026-08-26 01:00:12 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 2026-08-26 01:05:00 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 2026-08-26 01:07:17 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 2026-08-26 01:05:27 |
| [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`] | 2026-08-26 01:16:55 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 2026-08-26 01:12:12 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 2026-08-26 01:15:28 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 2026-08-26 01:07:17 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 2026-08-26 01:05:27 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`] | 2026-08-26 01:16:37 |

## Issues Found and Fix

### [Windows][`/RPT`][`/MiniHttp`] — 1st

The complete normal scenario reached graceful application shutdown, but the verification harness over-constrained the final success mask to `IGacUIRenderer exited due to receiving RequestControllerConnectionStopped.`. The active MiniHTTP renderer instead settled at `HTTP remote protocol disconnected.` with no fatal message or retry loop, which satisfies the SOP's allowed terminal-disconnected outcome. This was a verification-harness expectation error, not a product defect.

After accepting either allowed nonfatal terminal outcome, the full normal run passed with exact UI, Home mutation, three-row DataGrid add/clear, Document dialog, renderer replacement, concurrent takeover, detached-renderer settlement, visible File-menu closure, and Core exit code `0`. A fresh fatal run produced Core exit code `3`, the exact visible error mask `This is a fatel error!`, and exactly one matching page error without reconnect or retry. No source change was made.

### [Windows][`/FCT`][`/Http`] — 1st

The two-list add/clear scenario passed, but the first browser-control attempt used a bulk browser text-entry operation that does not emit the per-key events consumed by this rendered GacUI text box, so the expected Search marker never appeared. This was a browser-control harness mismatch, not a product defect.

The harness now emits individual key events after confirming focus settlement. The Search and rich-editor markers then appeared exactly and remained unchanged across List/Control tab switches. Together with the earlier list check—each item `0` through `9` appeared exactly twice and disappeared after Clear—the complete scenario passed and closed through the application's visible Exit surface with Core exit code `0` and a nonfatal renderer terminal state. No source change was made.

### [Windows][`/FCT`][`/MiniHttp`] — 1st

The first list-add wait searched for substring `0`, which was already present in the visible `Add 10 items` label, so the harness counted rows before the MiniHTTP state update arrived and reported zero item rows. This was a verification-harness predicate error, not a product defect.

The corrected wait requires exact standalone list-item lines. The rerun showed every item `0` through `9` exactly twice, then none after Clear. Distinct Search and rich-editor markers were entered through individual key events, persisted across List/Control switches, and the application closed through its visible Exit surface with Core exit code `0` and an allowed nonfatal disconnected renderer state. No source change was made.

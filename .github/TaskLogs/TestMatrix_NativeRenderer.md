# Test Matrix Card 2026-08-30 09:13:23

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] |  |
| [Windows][CppTest_Rvm][`/Http`] |  |
| [Windows][CppTest_Rvm][`/MiniHttp`] |  |
| [Windows][CppTest_Rvm][`/Cli:<path>`] |  |
| [Windows][`/RPT`][`/Pipe`] |  |
| [Windows][`/RPT`][`/Http`] |  |
| [Windows][`/RPT`][`/MiniHttp`] |  |
| [Windows][`/FCT`][`/Pipe`] |  |
| [Windows][`/FCT`][`/Http`] |  |
| [Windows][`/FCT`][`/MiniHttp`] |  |
| [Windows][`/RVMT`][`/Pipe`] | 2026-08-30 09:16:31 (fixed) |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] | 2026-08-30 09:19:53 |
| [Windows][`/RVMT`][`/Http`] | 2026-08-30 09:20:07 (fixed) |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] | 2026-08-30 09:21:36 |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-08-30 09:21:53 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] | 2026-08-30 09:23:22 |

## Issues Found and Fix

### [Windows][`/RVMT`][`/Pipe`] — 1st

The normal scenario passed, but the first idle-loss automation run timed out while looking for a native fatal dialog through shell-level window-title discovery. Core had exited nonzero and the renderer UI thread was blocked, but that discovery path exposed neither the dialog nor the automation endpoint while the modal prompt was active. This was a verification-harness discovery error, not a product defect.

The verifier now enumerates top-level Win32 windows by the retained renderer PID, requires the `#32770` dialog and its child text to contain both `ERROR from GacUI Core` and exact `RemotingTest_RvmHost disconnected.`, and activates only its `No` button. Fresh idle-next-call and suspended in-flight delivery-loss runs then retained exact `Dom.fatalError`, terminated Core nonzero, and closed through exact `!Exit`. The normal run had already passed initial greeting, both translations, second-host rejection, accepted-host continuity, and graceful shutdown. No source change was made.

### [Windows][`/RVMT`][`/Http`] — 1st

The normal and idle-next-call scenarios passed. The first delivery-loss session then timed out waiting for its initial renderer UI immediately after the preceding fatal session, so that row was not credited. A process preflight found no retained Core, host, or renderer, and the same delivery-loss scenario passed immediately with a fresh process set. It produced the exact native prompt and retained `Dom.fatalError`, Core exited nonzero, and exact `!Exit` reaped the renderer. This was transient listener/process settlement in the verification sequence, not a product defect; no source change was made.

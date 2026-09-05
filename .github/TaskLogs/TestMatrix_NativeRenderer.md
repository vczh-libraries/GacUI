# Test Matrix Card 2026-09-04T18:55:27

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] | 2026-09-04T19:41:11 |
| [Windows][CppTest_Rvm][`/Http`] | 2026-09-04T22:27:18 |
| [Windows][CppTest_Rvm][`/MiniHttp`] | 2026-09-04T20:16:10 |
| [Windows][CppTest_Rvm][`/Cli:<path>`] | 2026-09-04T20:16:43 |
| [Windows][`/RPT`][`/Pipe`] | X |
| [Windows][`/RPT`][`/Http`] | X |
| [Windows][`/RPT`][`/MiniHttp`] | X |
| [Windows][`/FCT`][`/Pipe`] | X |
| [Windows][`/FCT`][`/Http`] | X |
| [Windows][`/FCT`][`/MiniHttp`] | X |
| [Windows][`/RVMT`][`/Pipe`] | 2026-09-04T22:07:21 |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] | 2026-09-04T22:07:35 |
| [Windows][`/RVMT`][`/Http`] | 2026-09-04T22:26:53 (fixed) |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] | 2026-09-04T22:27:12 |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-09-04T22:19:02 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] | 2026-09-04T22:19:21 |

## Issues Found and Fix

### 1st: MiniHTTP takeover can lose responses during an in-flight batch

The fresh `/RPT /MiniHttp` run starting at `2026-09-04T21:14:02` failed during takeover with `GuiRemoteEventFilter::ProcessResponses`: messages sent to the protocol had not all been responded to. Core's renderer-identity guard runs before `BatchWrite`, but admission can detach that renderer while responses are being delivered. The detached responses are ignored while the batch still reports success. Two deterministic regressions now cover renderer detachment and changed transport admission, including discarded partial responses and successful subsequent batches. CDB reproduced the original assertion. The fix checks identity and admission after the batch and discards cancelled responses. Both source configurations build without warnings/errors, and all 1,726 GacUI tests pass. All native Core RVM rows and RPT/FCT automation portions have passed fresh checks after this fix, and all fourteen browser rows passed after the applicable transport fixes. The six RPT/FCT rows retain `X` solely for the missing physical Windows input checks described below. Evidence is saved under `Native-RPT-MiniHttp-normal-2026-09-04T211402-*` in the temporary verification directory.

### 1st: Windows input desktop prerequisite

The `/RPT` and `/FCT` rows require actual Windows input before and after renderer replacement. `GetForegroundWindow()` returns null, target activation fails, and `OpenInputDesktop` returns `ERROR_ACCESS_DENIED` (5). Their `X` results retain the outstanding acceptance checks: the native Super shortcut, the registered Windows global hotkey, and mouse/key input through the Windows provider. These rows must be completed on an interactive desktop before they can pass.

The renderer automation portions continue independently. Completed portions and exact process results are recorded in `C:\Users\vczh\AppData\Local\Temp\VlppOS-TuiVerification\matrix-native-<app>-<transport>.json`. They cover controls, reflected Alt/Super readouts, all five buttons, movement, double clicks, both wheel axes, typing, replacement, normal close, and the RPT fatal case. Automation does not establish the missing Windows input coverage.

### 1st: Windows HTTP response remains queued beside an available poll

The `/RVMT /Http` normal run `2026-09-04T22:09:38` stalled after replacing the renderer. CDB preserved both processes: Core awaited paragraph request 118, whose serialized packet remained in the Windows HTTP connection queue even though its pending poll ID was nonzero. `HttpServerConnection::SubmitResponse` in VlppOS did not deliver excess callback responses to that existing poll. A deterministic Windows HTTP regression reproduced the stall under CDB. The fix delivers one queued message through the available poll and preserves the existing error-reporting path. The complete upstream suite passed all 292 cases in 16 files without a Debug leak report. The generated release was imported and both GacUI source configurations rebuilt without warnings/errors. Five consecutive manual `/RVMT /Http` normal runs passed; manual and stdio RVM host-loss cases, standalone HTTP cases, and full RPT/FCT HTTP automation passed. The earlier intermittent RPT HTTP dialog timeouts may share this cause, but were not captured in CDB; they are not independently attributed to this fix. Debugger evidence is in `http-stall-core-cdb.log`, `http-stall-renderer-cdb.log`, and `http-stall-regression-cdb-verified.log` in the temporary verification directory.

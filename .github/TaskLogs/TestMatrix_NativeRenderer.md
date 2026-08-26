# Test Matrix Card 2026-08-25 23:16:38

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] | 2026-08-25 23:22:18 |
| [Windows][CppTest_Rvm][`/Http`] | 2026-08-25 23:30:57 |
| [Windows][CppTest_Rvm][`/MiniHttp`] | 2026-08-25 23:32:00 (fixed) |
| [Windows][CppTest_Rvm][`/Cli:<path>`] | 2026-08-25 23:49:05 |
| [Windows][`/RPT`][`/Pipe`] | 2026-08-25 23:50:57 |
| [Windows][`/RPT`][`/Http`] | 2026-08-26 00:08:53 (fixed) |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-08-26 00:12:01 |
| [Windows][`/FCT`][`/Pipe`] | 2026-08-26 00:14:28 |
| [Windows][`/FCT`][`/Http`] | 2026-08-26 00:18:00 (fixed) |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-08-26 00:19:55 |
| [Windows][`/RVMT`][`/Pipe`] | 2026-08-26 00:26:43 (fixed) |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] | 2026-08-26 00:30:23 (fixed) |
| [Windows][`/RVMT`][`/Http`] | 2026-08-26 00:33:23 (fixed) |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] | 2026-08-26 00:35:34 |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-08-26 00:37:37 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] | 2026-08-26 00:39:40 |

## Issues Found and Fix

### [Windows][CppTest_Rvm][`/MiniHttp`] — 1st

The requester and host were launched for the normal Workflow RPC scenario, but the first automation probe used a two-second HTTP-client timeout. MiniHTTP's automation response took slightly longer than that under this run, so every probe was canceled even though both processes were alive and the raw `Controls` response already contained the correct `Hello, !` state. This was a verification-harness timeout, not a product defect.

The probe timeout was raised while retaining the overall bounded wait, and host startup was synchronized with the actual port-8888 listener. The normal UI/RPC flow, second-host rejection, accepted-host continuity, normal shutdown, idle-next-call host loss, and suspended delivery-acknowledgement-loss variant all then passed. Both fatal variants terminated the requester with nonzero exit code `-2147483645`. No source change was made.

### [Windows][`/RPT`][`/Http`] — 1st

The first scripted normal-flow run reached the Document page, but it activated the `RIGHT NOW` hyperlink before the HTTP renderer had fully settled after the preceding page transitions. The expected `Pretend to be starting!` modal therefore did not appear before the bounded wait expired. This was a verification-harness timing issue, not a product defect.

The harness now waits for each renderer-visible activation to settle. The full normal flow then passed, including exact initial UI, mutable Home state, three-row DataGrid add/clear, Document hyperlink dialog, renderer replacement, concurrent takeover, detached-renderer behavior, and visible-surface shutdown. A fresh fatal run also produced nonzero Core exit code `3`, the exact native prompt and renderer fatal text `This is a fatel error!`, and clean renderer acknowledgement. No source change was made.

### [Windows][`/FCT`][`/Http`] — 1st

The first scripted run completed the two-list add/clear scenario and reached the Control page, but the harness required exact automation text `Search:` while this HTTP run exposed the visually equivalent label as `Search: ` with one trailing space. It therefore failed to associate the label with its text box. This was a verification-harness selector issue, not a product defect.

The selector now normalizes surrounding whitespace before associating the Search label, while the visible SOP check still requires the `Search:` label. The complete two-list scenario then passed with each item `0` through `9` appearing exactly twice before Clear and zero times afterward. Distinct HTTP markers were entered into the Search box and rich editor, remained exact across List/Control tab switches, and the application closed through the native Exit surface without a fatal terminal state. No source change was made.

### [Windows][`/RVMT`][`/Pipe`] — 1st

The first normal-flow run completed the initial Workflow RPC but the verification harness used unsupported shorthand `!CtrlA` while preparing the second marker. The renderer correctly rejected it with `Syntax Error!` and listed exact `!KeyPress:Key1+Key2+...+KeyN` syntax. A subsequent harness draft also used PowerShell's reserved `$Host` variable for the accepted process; that local scripting error was corrected before rerunning. These were verification-harness errors, not product defects.

With exact `!KeyPress:Ctrl+A` and an unambiguous process variable, the initial greeting, first translation, rejected second host, original-host continuity, and graceful application shutdown all passed. Fresh idle-loss and suspended in-flight delivery-loss runs each terminated Core nonzero with exit code `3`, produced exactly `RemotingTest_RvmHost disconnected.` in the native prompt and retained renderer state, and closed cleanly after exact `!Exit`. No source change was made.

### [Windows][`/RVMT`][`/Pipe /Cli:<path>`] — 1st

The normal UI/RPC path and process shutdown completed, but the harness treated the automation connection reset caused by exact `!Exit` as an ordinary response and then attempted to inspect its missing response body. Core had already exited `0`, and both its owned stdio child and renderer had exited. This was a terminal-response race in the verification harness, not a product defect.

The closure check now tolerates only that response race and independently requires Core exit code `0` plus owned-child and renderer termination. A fresh normal run passed those requirements. Fresh idle-EOF and suspended in-flight EOF runs each terminated Core nonzero with exit code `3`, exposed exactly `RemotingTest_RvmHost disconnected.` in the native prompt and renderer terminal state, and left no owned child or renderer running. No source change was made.

### [Windows][`/RVMT`][`/Http`] — 1st

The normal UI/RPC and second-host-rejection steps completed, but the harness probed the renderer automation endpoint during the short interval after Core's graceful exit had begun and before its process object had refreshed the renderer's terminal state. The renderer closed the endpoint before the probe connected. This was a verification-harness terminal race, not a product defect.

The harness now waits for renderer process settlement before probing a retained terminal. The normal run then passed its exact greeting/translation checks, rejected-host check, accepted-host continuity, and graceful shutdown. Fresh idle-next-call and suspended delivery-acknowledgement-loss runs both completed within their bounds, terminated Core nonzero with exit code `3`, and retained exactly `RemotingTest_RvmHost disconnected.` in the native prompt and renderer fatal state before exact `!Exit`. No source change was made.

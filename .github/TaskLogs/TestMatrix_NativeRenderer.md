# Test Matrix Card 2026-09-02 16:43:50

This round covers the mouse/shortcut regressions in FullControlTest and RemoteProtocolTest. RVM rows are outside this task and remain blank.

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] |  |
| [Windows][CppTest_Rvm][`/Http`] |  |
| [Windows][CppTest_Rvm][`/MiniHttp`] |  |
| [Windows][CppTest_Rvm][`/Cli:<path>`] |  |
| [Windows][`/RPT`][`/Pipe`] | X |
| [Windows][`/RPT`][`/Http`] | X |
| [Windows][`/RPT`][`/MiniHttp`] | X |
| [Windows][`/FCT`][`/Pipe`] | X |
| [Windows][`/FCT`][`/Http`] | X |
| [Windows][`/FCT`][`/MiniHttp`] | X |
| [Windows][`/RVMT`][`/Pipe`] |  |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] |  |
| [Windows][`/RVMT`][`/Http`] |  |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] |  |
| [Windows][`/RVMT`][`/MiniHttp`] |  |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] |  |

## Issues Found and Fix

### Native Windows-key input is blocked

The desktop is noninteractive: `GetForegroundWindow()` returns zero, focus cannot be acquired, and real `SendInput` fails with Access denied (5). Consequently the native Ctrl+Alt+Win+Q and RegisterHotKey/MOD_WIN activation checks remain unverified. A row is marked X while these required checks remain blocked, even when its available checks pass.

### RPT / Pipe, 1st

Exact visible native DOM labels and matching Segoe UI 15 / #F1F1F1 styling passed initially and after renderer replacement/takeover. All five Windows mouse-message down/up pairs passed, with TRUE returned for the X-button messages. Ctrl+Q through renderer automation, the Click Me state marker, repeated Ctrl+Q and Mouse4/Mouse5 after takeover, old-renderer exit, and menu-controlled application close passed. Windows-key activation remains blocked as described above.

An initial harness replacement reused the automation port before the terminated process released it. CDB identified `HttpServerApi` construction failure with result 0xB7 (ERROR_ALREADY_EXISTS). The corrected harness waits for process exit and uses separate automation ports; the replacement checks then passed.

### RPT / Http, 1st

The same available checks passed over HTTP, including both replacement stages and menu-controlled close. The replaced and final renderers exited with code zero. The reattached Core process object did not retain an exit code; its termination was observed and the final renderer exited normally. Fresh launch process objects are retained for the remaining rows. Windows-key activation remains blocked as described above.

### RPT / MiniHttp, 1st

The same available checks passed over MiniHTTP. The first exit-confirmation click left the modal open. The harness now locates buttons in the current visible renderer DOM and checks that each dialog disappears before continuing; the normal-close retry exited both Core and renderer with code zero. Windows-key activation remains blocked as described above.

### FCT / Pipe, 1st

Available checks passed at 2026-09-02 18:15:45: exact labels/style, all five Windows mouse-message pairs, Ctrl+Q, reconnection/takeover, and repeated labels/style/Ctrl+Q/Mouse4/Mouse5. The replaced renderer, normally closed Core, and final renderer all exited zero. The first harness attempt left the shortcut modal open before testing mouse input; requiring actual dialog disappearance resolved that sequencing failure. Windows-key activation remains blocked as described above.

### FCT / Http, 1st

The same available checks passed at 2026-09-02 18:16:27, with code-zero exits for the replaced renderer, normally closed Core, and final renderer. Windows-key activation remains blocked as described above.

### FCT / MiniHttp, 1st

The same available checks passed at 2026-09-02 18:34:39, with code-zero exits for the replaced renderer, normally closed Core, and final renderer. Large DOM reads stalled in the PowerShell HTTP client during the release rebuild. CDB found the renderer in its normal UI message pump with idle network workers and no modal or crash. A direct curl request downloaded the 1.5 MB DOM in 1.85 seconds; using curl for loopback requests and parsing JSON as a hashtable allowed the row to finish. MiniHTTP automation uses explicit IPv4 loopback. Windows-key activation remains blocked as described above.

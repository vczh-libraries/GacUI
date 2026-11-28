# Test Matrix Card 2026-11-28 04:55:45 -08:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] | 2026-11-28 05:08:53 -08:00 |
| [Windows][CppTest_Rvm][`/Http`] | 2026-11-28 05:19:02 -08:00 |
| [Windows][CppTest_Rvm][`/MiniHttp`] | 2026-11-28 06:04:19 -08:00 |
| [Windows][CppTest_Rvm][`/Cli:<command>`] | 2026-11-28 05:08:39 -08:00 |
| [Windows][`/RPT`][`/Pipe`] | 2026-11-28 05:59:46 -08:00 |
| [Windows][`/RPT`][`/Http`] | 2026-11-28 06:48:40 -08:00 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-11-28 06:48:46 -08:00 (partial) |
| [Windows][`/FCT`][`/Pipe`] | 2026-11-28 06:40:25 -08:00 (partial) |
| [Windows][`/FCT`][`/Http`] | 2026-11-28 06:49:04 -08:00 (partial) |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-11-28 06:52:16 -08:00 (partial) |
| [Windows][`/RVMT`][`/Pipe`] | 2026-11-28 05:09:11 -08:00 |
| [Windows][`/RVMT`][`/Pipe /Cli:<command>`] | 2026-11-28 05:09:36 -08:00 |
| [Windows][`/RVMT`][`/Http`] | 2026-11-28 05:20:03 -08:00 |
| [Windows][`/RVMT`][`/Http /Cli:<command>`] | 2026-11-28 05:20:39 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-11-28 05:21:13 -08:00 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<command>`] | 2026-11-28 05:21:44 -08:00 |

## Local baselines

| Test Item | 1st |
| --- | --- |
| Debug x64 solution build | 2026-11-28 06:51 -08:00 |
| CppTest | 2026-11-28 06:51 -08:00 (partial) |
| CppTest_Metaonly | 2026-11-28 06:46 -08:00 (partial) |

## Observations

- All four standalone and six remote view-model configurations displayed the exact initial and edited greetings. Network hosts rejected a second host while the original connection remained usable. Stdio hosts were direct children and exited after normal shutdown. Each remote replacement retained the greeting and completed another translation.
- Fresh idle-loss and blocked-translation runs ended the requester nonzero. The blocked runs suspended the owned host and established an outstanding request. All remote loss runs retained the exact `RemotingTest_RvmHost disconnected.` error, rejected ordinary input, and closed on `!Exit`. A debugger observed the standalone Workflow RPC exception. After the MiniHTTP host was killed, its quiet standalone blocked run ended in 4.83 seconds.
- All three native protocol-test transports retained the changed Home button across replacement. Each displayed three rows with three populated cells, cleared the grid, and opened the document dialog. Both the first and replacement renderers reported exact down/up text for five buttons and the Alt/Super combinations, motion, double-click, and both wheel directions on each axis. The queued File close and confirmation ended the Core and renderer with code 0. Independent fatal runs displayed and retained the exact `This is a fatel error!` message and rejected ordinary input.
- Windows-level shortcut delivery was observed for Ctrl+Alt+Win+Q on named pipe and HTTP, and for Ctrl+Shift+Alt+Win+Q on named pipe and on both HTTP renderers. Desktop focus and competing global registrations limited the MiniHTTP shortcut evidence to the rendered labels and local Ctrl+Q.
- The three showcase transports added and cleared 0–9 in both lists, retained independent Ribbon search and document input, and accepted bracket and brace text. The named-pipe checks also covered explicit renderer replacement. On both TCP transports, the replacement rendered both retained editor markers and the queued Exit action ended both processes with code 0.
- HTTP and MiniHTTP rendered the actual border color of each of the six palettes and returned to Default. Both showed the four vertical Layout headers and the 240×320 responsive modal. All three transports updated Core clock data and handled one- and two-contact translation and scaling, with cancellation returning the contact count to zero. The GDI remote renderer painted the cyan, green and yellow hands against a white rim on all three transports. On each transport, the yellow hand moved with Core time after changing to Aurora, revisiting the page and replacing the renderer. On all three transports the debugger observed the Direct2D clock render, ellipse and line calls. In the final remote Direct2D session the window capture returned a blank surface even on the previously verified Touching page. Direct2D clock pixels in that session remain unverified. Local Direct2D and GDI captures showed the white face, colored hands, dark outlines and moving yellow second hand. Revisiting the page rebuilt the painter.
- The named-pipe GDI touch run verified contact pixels, drag, scrolling, row removal, cancellation, momentum and editor selection before and after template replacement. Direct2D also captured the contact pixels and confirmed Core and renderer routing. Both local backends completed the scoped Ribbon and Toolstrip point-input suite and normal shutdown. On HTTP and MiniHTTP the scoped Direct2D suites passed through the Core and renderer input routes, including the editor fixture and list scrolling. Both processes exited 0.
- Both local GUIs opened the original private document and the TUI-produced document with their original text, styles and two images. The hosted GUI exported RTF with two picture runs and HTML with two embedded images. TUI reopened the private file produced by the GUI. The separate-window Metaonly responsive modal opened at 240×320 and completed two full grow/shrink cycles. At widths 240, 350, 440 and 680 it successively showed titles only, movie directors, the icon-only account list and account names. Heights 220 and 520 retained both footer buttons at width 44. Fresh OK, Cancel and title-bar closes each returned control to the owner.
- Native MiniHTTP used separate Core and renderer automation ports. Both actual editors followed the external clipboard through supported Unicode text, unsupported data, cut and single paste at a new caret. Private-format cut/paste preserved styles and both embedded images. With clipboard injection disabled on both ends, both editors used the Core fake clipboard and preserved the independent external text. The independently supplied 8×8 bitmap pasted in both editors.

## Issues Found and Fix

- While editing a mouse test helper, a diagnostic command accidentally overwrote the Windows clipboard. Clipboard history was disabled, and the earlier in-memory guards had exited. The helper text was identified and removed. The prior clipboard contents could not be recovered.
- Initial links encountered live test executables. The final solutions passed for Debug Win32 and x64, both metadata generators exited 0, and the x64 metadata validation exited 0. The full x64 unit suite passed all 2,032 cases in 100 files.
- A debugger stopped at the final Core process event after the HTTP input sweeps. Fresh debugger-free confirmations established normal exits on both TCP transports. A file-picker retry targeted the native filename edit and opened both fixtures in Metaonly.
- The standalone Direct2D and GDI scripted momentum walkthroughs timed out on a late viewport sample. Faster viewport reads then showed continued motion after release in all eight combinations of Direct2D or GDI, editable or selectable text, and the original or Aurora template. Native Windows pointer injection acquired, scrolled and released TouchingList, and a native mouse click added items. A later Windows pointer retry returned error 203. The editor range collapsed on a service click. For the independent Windows mouse comparison, Windows denied reading the actual cursor position with error 5. Physical touch, second-monitor DPI, the full set of remote responsive widths and some editor command permutations remain unverified.

Local evidence is under <GacUI repo>/Test/Output/RpWindows20261128. See the native view-model logs, `rpt-close-live.log`, `fct-tcp-live.log`, `clock-pixels-final.log`, `remote-clock-pipe.log`, `remote-clock-verify.log`, `momentum-d2d-retest.log`, `momentum-observe.log`, `clipboard-final.log` and the scoped touch logs. `full-responsive-final.log` and the debugger clock traces record the native window and drawing checks.

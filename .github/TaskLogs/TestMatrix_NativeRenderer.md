# Test Matrix Card 2026-09-21 12:50:00 -07:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] | 2026-09-21 15:01:26 -07:00 |
| [Windows][CppTest_Rvm][`/Http`] | 2026-09-21 14:27:10 -07:00 |
| [Windows][CppTest_Rvm][`/MiniHttp`] | 2026-09-21 14:28:21 -07:00 |
| [Windows][CppTest_Rvm][`/Cli:<path>`] | 2026-09-21 14:24:48 -07:00 |
| [Windows][`/RPT`][`/Pipe`] | 2026-09-21 14:18:45 -07:00 |
| [Windows][`/RPT`][`/Http`] | 2026-09-21 15:31:08 -07:00 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-09-21 15:38:32 -07:00 |
| [Windows][`/FCT`][`/Pipe`] | 2026-09-21 15:13:20 -07:00 |
| [Windows][`/FCT`][`/Http`] | 2026-09-21 15:48:39 -07:00 |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-09-21 15:56:24 -07:00 |
| [Windows][`/RVMT`][`/Pipe`] | 2026-09-21 15:01:53 -07:00 |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] | 2026-09-21 15:04:05 -07:00 |
| [Windows][`/RVMT`][`/Http`] | 2026-09-21 14:29:35 -07:00 |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] | 2026-09-21 14:36:10 -07:00 |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-09-21 14:38:47 -07:00 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] | 2026-09-21 14:41:20 -07:00 |

## Native Matrix Checkpoints

- **Post-replacement shortcuts, FCT `/Pipe`** — Follow-up at 16:12:46 used Core 34660 and renderers 32004 → 32564. Replacement Controls and active Dom had all three exact canonical labels; the exact Ctrl+Q dialog appeared and one click at settled localized confirmation bounds dismissed it. Exit → `self.Close() (InvokeInMainThread)` ended Core and renderer 32564 with retained exit code 0; renderer 32004 also exited 0 on takeover.

- **Post-replacement shortcuts, RPT `/Pipe`** — Follow-up at 16:12:30 used Core 11300 and renderers 27840 → 4724 → 12116. Third-renderer Controls and active Dom had all three exact canonical labels. The exact Ctrl+Q dialog appeared and one click at settled confirmation bounds dismissed it. File → `self.Close() (InvokeInMainThread)` ended Core and renderer 12116 with retained exit code 0; renderer 4724 exited 0 on takeover. Renderer 27840 was deliberately terminated (Node reported SIGTERM). An earlier temporary helper incorrectly required a numeric exitCode for intentional signal termination; that procedure assertion was corrected before this passing run.

- **Post-replacement shortcuts, FCT `/Http`** — Follow-up at 16:05:03 used Core 31936 and renderers 28024 → 18356. Replacement Window Manager Controls and active Dom had all three exact canonical labels. The exact Ctrl+Q dialog appeared, its localized confirmation dismissed it after fresh layout reads, and no dialog remained. Exit → `self.Close() (InvokeInMainThread)` ended Core and renderer 18356 with retained exit code 0; renderer 28024 was deliberately stopped.

- **Post-replacement shortcuts, RPT `/MiniHttp`** — Follow-up at 16:03:11 used Core 3572 and renderers 36596 → 30800 → 22172. Third-renderer Controls and active Dom had all three exact canonical labels; the exact Ctrl+Q dialog appeared and was dismissed. File/Close ended Core and renderer 22172 with retained exit code 0; renderer 30800 exited 0 on takeover, and renderer 36596 was deliberately stopped.

- **Post-replacement shortcuts, RPT `/Http`** — Focused follow-up at 16:01:16 used Core 36644 and renderers 37004 → 21264 → 15276. Third-renderer Controls and active Dom had all three exact canonical labels, then the exact Ctrl+Q dialog appeared and was dismissed. File/Close ended Core and renderer 15276 with retained exit code 0; renderer 21264 exited 0 on takeover, and renderer 37004 was deliberately stopped.

- **[Windows][`/FCT`][`/MiniHttp`]** — Core 26820 started at 15:56:24; renderer 38396 was deliberately stopped and renderer 38324 reconnected. Both lists passed exact 0–9 add/clear; exact `Hello[Ab]{Cd}` and distinct `FctMiniS` / `FctMiniDocument` markers passed and survived tab changes and replacement. Initial and replacement renderers passed the 80-event payload. Exact canonical shortcut labels and the hosted `You pressed Ctrl+Q!` dialog passed before and after replacement; consecutive fresh localized confirmation bounds settled before one queued dismissal. Exit → `self.Close()` ended Core and renderer 38324 with retained exit code 0.

Final SOP audit complete: all six native RPT/FCT rows have explicit post-replacement canonical shortcut labels and exact Ctrl+Q dialog/dismissal evidence. All 16 native matrix rows completed their available checks; physical-input limits are recorded below.

- **[Windows][`/FCT`][`/Http`]** — Core 17584 started at 15:48:39, initial renderer 30312 was deliberately stopped, renderer 5776 exited 0 on takeover by renderer 33056. Lists contained exactly 0–9 in both trees and cleared; exact `Hello[Ab]{Cd}` and distinct `FctHttpS` / `FctHttpDocument` editor markers passed and persisted through tab changes and both handoffs. Each of the three renderers passed the 80-event input payload. Initial Ctrl+Q opened the exact hosted dialog and its localized confirmation dismissed it. Exit → `self.Close()` ended Core and renderer 33056 with retained exit code 0.

- **[Windows][`/RPT`][`/MiniHttp`]** — Core 10440 completed initial marker/input, hosted Ctrl+Q, DataGrid, marked reconnect to renderer 21688 and active takeover to 37260, post-takeover input, and File/Close. Core and renderers 21688/37260 exited 0; first renderer 33708 was deliberately stopped. Follow-up Core 13996/renderer 6700 verified all nine grid values added and cleared in both trees and the exact Document dialog. The first OK bounds moved from center (320,307) to (320,293); the fresh click dismissed it. Fatal verification then showed the exact native error prompt, No dismissal, retained fatal Dom, rejected ordinary IO, and accepted !Exit; Core exited 3 and renderer 0. CDB counted exactly one error callback.

- **[Windows][`/RPT`][`/Http`]** — Initial UI, Ctrl+Q hosted dialog, DataGrid/Document, initial and post-reconnect 80-event payloads, and normal close passed (Core 25024 and renderer 26936 exit 0; first renderer 35008 deliberately stopped). Fresh fatal Core 16224 exited 3; renderer 28564 showed the exact native error prompt, retained exact fatal Dom after No, rejected ordinary IO, accepted !Exit, and exited 0. CDB counted exactly one error callback. Focused continuity follow-up at 15:45:32 (Core 21832) set the marker before renderer 18056 stopped, preserved it on renderer 26804, and preserved it again on active takeover to renderer 29920; renderer 26804 exited 0. Renderer 29920 repeated all 80 events; File/Close ended Core and renderer 29920 with exit 0. All nine DataGrid values cleared in both trees. Five Document repetitions each showed initial OK bounds (284,297)-(356,317) moving to (284,283)-(356,303); after consecutive stable fresh reads, a single click dismissed each dialog.

- **[Windows][CppTest_Rvm][`/Pipe`]** — 2026-09-21 15:01:26 -07:00
- Standalone `/Pipe` manual evidence: normal exit 0, second-host rejection, idle host loss exit `0x80000003` in 184 ms, and blocked suspended-host Controls proof followed by exit `0x80000003` in 253 ms. Owned processes and ports were cleaned.
- **[Windows][`/RVMT`][`/Pipe`]** — 2026-09-21 15:01:53 -07:00 (pass; Core/RVMT manual `/Pipe` normal replacement and second-host rejection passed; idle-loss prompt in 342 ms/Core exit 3; blocked Controls read pending 150 ms then prompt in 699 ms/Core exit 3. CDB confirmed exactly one error callback in both renderer-loss phases.)
- **[Windows][`/RVMT`][`/Pipe /Cli:<path>`]** — 2026-09-21 15:04:05 -07:00 (pass; Core/RVMT `/Pipe /Cli` normal/replacement and exact child `!Exit` exited 0 with child reaped; idle-loss prompt in 331 ms/Core exit 3; blocked Controls read pending 150 ms then prompt in 712 ms/Core exit 3. CDB confirmed exactly one error callback on each renderer-loss phase.)
- **[Windows][`/FCT`][`/Pipe`]** — Final full UI session started 15:23:00 (Core 10952, renderer 14768, replacement 25536). Both TextLists had exactly 0–9 in Controls and active Dom, then cleared. Ribbon Search and DocumentViewer retained distinct `WinPipeFinalS` / `WinPipeFinalDocument` markers through tab changes and renderer takeover; exact `Hello[Ab]{Cd}` also passed. Each renderer completed 80 mouse events across all five modifier patterns with exact label/readout/style assertions. Ctrl+Q opened a hosted logical dialog containing exactly `You pressed Ctrl+Q!`; its localized `确定` button dismissed it and no subwindow remained. Exit → `self.Close()` ended the full UI session. Numeric exit codes were not captured in that session; a focused fresh lifecycle at 15:29:57 captured Core 25224, old renderer 7524, and replacement 14692 all exiting 0 after takeover and the same UI close path. Earlier helper failures were procedure errors (tab selection, monotonic Add counter, Search Ctrl+A scope, and replacement endpoint readiness), not source defects.
- **[Windows][CppTest_Rvm][`/Http`]** — 2026-09-21 14:27:10 -07:00 (pass; standalone `CppTest_Rvm /Http /AsPort:8910` with separate `RemotingTest_RvmHost /Http`; normal exit 0, second host rejected; idle host loss exited 0x80000003 in 185 ms; blocked-loss Controls read was pending, then exit 0x80000003 in 5,234 ms; app PIDs 29012/25924/13432, blocked host 35060; all owned processes/ports cleaned)
- **[Windows][CppTest_Rvm][`/MiniHttp`]** — 2026-09-21 14:28:21 -07:00 (pass; standalone `CppTest_Rvm /MiniHttp /AsPort:8910` with separate `RemotingTest_RvmHost /MiniHttp`; normal exit 0, second host rejected; idle loss 0x80000003 in 137 ms; blocked-loss Controls read was pending, then exit 0x80000003 in 5,133 ms; app PIDs 15212/37824/28212, blocked host 18276; all owned processes/ports cleaned)
- **[Windows][CppTest_Rvm][`/Cli:<path>`]** — 2026-09-21 14:24:48 -07:00 (pass; standalone `CppTest_Rvm /Cli:<RemotingTest_RvmHost.exe> /AsPort:8910`; normal and exact `!Exit` both exit 0 and CLI child reaped; idle host loss exited 0x80000003 in 185 ms; blocked loss exited 0x80000003 in 309 ms with pending Controls read; app PIDs 30524/10532/32652/5720, blocked host 8880; all owned processes/ports cleaned)
- **[Windows][`/RPT`][`/Pipe`]** — 2026-09-21 14:18:45 -07:00 (pass; Core PID32928 `/AsPort:8895`; renderer1 PID2992 `/AsPort:8889`; replacement renderer2 PID31848 and renderer3 PID23592. Initial controls, five-pattern payload, DataGrid, Document, shortcut Ctrl+Q modal, replacement continuity, full post-replacement 80-event payload, exact two-tree assertions and styles all passed. One Alt Mouse5Up transient returned Left-up; five focused fresh-geometry pairs and the subsequent full payload repeat then passed, leaving one unexplained non-reproducing mismatch. File→`self.Close()` prompt was confirmed and OK closed Core/renderer3 exit0; renderer2 exit0, renderer1 intentionally -1. Fresh fatal phase Core PID30004/renderer PID13716 under CDB82151: exact Win32 `ERROR from GacUI Core` dialog text `This is a fatel error!` with `&Yes`/`&No`, No dismissed, modal gone; Dom fatalError exact `This is a fatel error!`, title `[STOPPED] Remote Protocol Test`; ordinary IO rejected and exact `!Exit` queued; Core exit3/renderer exit0. No editable text control. Win-key shortcut activations unverified.)
- **[Windows][`/RVMT`][`/Http`]** — 2026-09-21 14:29:35 -07:00 (pass; Core/RVMT manual `/Http` PID 20252 `/AsPort:8910`, host PID 9164, renderer PID 4888 `/AsPort:8911`; normal workflow typing, second-host rejection, renderer replacement/state continuity and WM_CLOSE normal exit0; idle and corrected blocked host-loss phases showed exact native fatal prompt, `No` retained exact `Dom.fatalError`, ordinary IO rejected, `!Exit` accepted, Core exit3 and renderer exit0. CDB traced exactly one `OnReadError` per valid phase. Corrected blocked proof awaited the pending Controls request before host termination; prompt after 5,581 ms. Initial blocked attempt raced queued renderer input and was rerun as procedure error, not application failure)
- **[Windows][`/RVMT`][`/Http /Cli:<path>`]** — 2026-09-21 14:36:10 -07:00 (pass; Core/RVMT `/Http /Cli` normal replacement exit0 and exact child `!Exit` exit0 with host child reaped. Idle-loss prompt in 389 ms/Core exit3; blocked-loss pending Controls read proved blocked for 150 ms, then prompt in 752 ms/Core exit3. CDB confirmed exactly one error callback in both fatal phases)
- **[Windows][`/RVMT`][`/MiniHttp`]** — 2026-09-21 14:38:47 -07:00 (pass; Core/RVMT `/MiniHttp` manual PID 5596, host PID 26072, renderer PID 38472. Normal replacement and second-host rejection passed; idle-loss prompt in 399 ms/Core exit 3; blocked-loss Controls read was pending 150 ms, then prompt in 5,497 ms/Core exit 3. CDB confirmed exactly one error callback on each renderer-loss phase; cleanup clear on ports 8910–8912)
- **[Windows][`/RVMT`][`/MiniHttp /Cli:<path>`]** — 2026-09-21 14:41:20 -07:00 (pass; Core/RVMT `/MiniHttp /Cli` normal replacement and exact CLI `!Exit` passed, Core exit0 and child reaped; idle-loss prompt in 393 ms/Core exit3; blocked Controls read pending for 150 ms then prompt in 752 ms/Core exit3. CDB confirmed exactly one error callback on each renderer-loss phase)

### Common Native Coverage Limits

Physical Win-key chords and global shortcut activation remain unverified for applicable rows because the Windows desktop was unavailable to the native input provider (`GetForegroundWindow()` was null and input-desktop access was denied). Renderer IO verified the five-button mouse matrix and Super modifier payloads; those checks do not establish physical Win-key delivery.

All 15 required native Core-authored fatal phases (12 RVM host-loss cases and 3 RPT fatal cases) were debugger-counted at `RemoteProtocolRendererClient::OnReadError`, with exactly one callback each.

### Standalone Exception Evidence

The standalone CLI loss trace for PID 14396 was debugger-confirmed: first-chance and unhandled second-chance C++ exception stacks reached `RpcJsonDispatcherClient::ThrowInjectedExceptionLocked` at `Import/VlppWorkflowLibrary.cpp:4916` (`throw RpcInjectedException(injectedException)`), through `IViewModel::Translate` and text-editor input. The process exited `0xE06D7363`; evidence is retained in `%TEMP%\GacUI-rpWindows-20260921-standalone-exception-14396.log`.

## Local GUI Baselines

| Test App | Launch Shape | Actual Coverage | Result |
| --- | --- | --- | --- |
| `CppTest` | Hosted (`Main.cpp`), `/AsPort:8893` | Shared Input Payload Checks passed; FCT List and editor persistence passed; shortcut labels were exact and Ctrl+Q produced `You pressed Ctrl+Q!`, dismissed via localized `确定`; Exit→`self.Close()` completed normally with retained PID 20016 exit code 0 and listener closed. Ctrl+Alt+Win+Q and Ctrl+Shift+Alt+Win+Q activation remain unverified (native IO key names do not provide Win and desktop input APIs denied access); actual OS global-shortcut activation and screenshot also remain unverified. | 2026-09-21 13:45:25 -07:00 (automation coverage passed; shortcut/input gaps) |
| `CppTest_Metaonly` | Ordinary MultiWindow, `/AsPort:8894` | PID 27980 opened at 13:52:57; Controls title `Complete Control Showcase`, `WindowManagement=MultiWindow`; exact three shortcut labels visible without `osSuper`; initial readout `Alt: 0; Super: 0` and matching theme descriptors confirmed. All five mouse button down/up pairs and 30 move/double-click/vertical and horizontal wheel commands passed across none, Alt, Super, Alt+Super, and Ctrl+Shift+Alt+Super, with exact text/readout assertions after every event and matching label styles. Both TextLists started empty; one Add produced exactly 0–9 in both; Clear removed all items. Exact `CppTestMetaonlyLocalSearch` and `CppTestMetaonlyLocalDocument` markers persisted after Control→List→Control. In the corrected fresh run, PID 34888 produced the exact `You pressed Ctrl+Q!` modal; Win32 confirmed and dismissed its `OK` button, then confirmed the modal HWND was gone before the first post-modal Controls GET. Controls returned 200 with the expected title, MultiWindow mode, Window Manager, Ctrl+Q, `Alt: 0; Super: 0`, and Default labels. Exit→`self.Close()` completed with retained PID exit code 0 and no confirmation modal. Ctrl+Alt+Win+Q and Ctrl+Shift+Alt+Win+Q activation remain unverified because native IO key names do not provide Win; actual OS global-shortcut activation remains unverified due desktop input access. | 2026-09-21 14:11:19 -07:00 (automation coverage passed; shortcut/input gaps) |

Fresh local bracket follow-ups passed in the ribbon Search editor: CppTest at 15:28:27 (PID 20984, port 8893) and CppTest_Metaonly at 15:28:34 (PID 5592, port 8894) both displayed exactly `Hello[Ab]{Cd}` and completed Exit → `self.Close()` with retained exit code 0. This closes the earlier local bracket-text gap.

## Issues Found and Fix

### Windows Automation Shutdown

The browser RVM host-loss check reproduced a Windows automation shutdown deadlock with a pending Controls read. Windows tree reads now queue their responses without blocking HTTP callbacks and cancel queued work during shutdown. The native matrix used the fixed build; the reproduction, debugger diagnosis, and regression results are recorded in [the browser card](TestMatrix_GacJS.md).

### Local GUI Baselines — CppTest Ctrl+Q Procedure

In the hosted local CppTest process (PID 19648, `/AsPort:8893`), CDB inspection established that the first `!KeyPress:Ctrl+Q` opened the expected `GuiMessageDialog::ShowDialog` through `FakeDialogServiceBase::ShowModalDialogAndDelete`. The modal remained active in the nested loop. The operator failed to locate it in the full hosted Controls tree and sent another key command before dismissing it. That invalid sequence reached `iocommands::KeyDown` line 410 and raised `CHECK_ERROR` because `KEY_CONTROL` was already pressed. This explains the subsequent native `abort()` dialog and is a verification procedure error, not an application defect.

For the fresh rerun (PID 20016), the active dialog was found in the hosted Controls tree, its localized `确定` button was clicked, and the modal disappeared before any further input. The erroneous PID 19648 was closed with Abort after CDB detached; its retained process handle reported exit code 3. This was a harness-sequencing error, not an application failure.

### Local GUI Baselines — Metaonly Modal Polling

After opening the expected Ctrl+Q dialog in PID 27980, a `/Controls` GET was issued before the modal was dismissed. The test client timed out and canceled the response; CDB caught `vl::Error: HttpSendHttpResponse failed for responding UTF-8 body.` after the tree dump completed. The Windows testing guide prohibits automation GETs while a native modal is open, so this was a procedure-induced CHECK_ERROR rather than a product defect. The corrected fresh run used Win32 inspection and dismissal, confirmed the modal HWND was gone, and only then queried Controls; it completed normal close with exit code 0. Root's CDB evidence is at `%TEMP%\GacUI-rpWindows-20260921-Metaonly-modal.txt`.

### Local GUI Baselines — CppTest Normal Close

The hosted FCT `Exit` tab's `self.Close()` button closed PID 20016 successfully (exit code 0, no listener remained on 8893). This flow does not ask for confirmation; the `Do you want to exit?` prompt belongs to the RPT test, so its absence here is expected.

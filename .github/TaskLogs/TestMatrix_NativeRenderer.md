# Test Matrix Card 2026-11-03 17:16:17 -08:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] | 2026-11-03 18:01:39 |
| [Windows][CppTest_Rvm][`/Http`] | 2026-11-03 18:01:42 |
| [Windows][CppTest_Rvm][`/MiniHttp`] | 2026-11-03 18:01:50 |
| [Windows][CppTest_Rvm][`/Cli:<path>`] | 2026-11-03 18:01:57 |
| [Windows][`/RPT`][`/Pipe`] | 2026-11-03 18:14:53 |
| [Windows][`/RPT`][`/Http`] | 2026-11-03 18:15:25 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-11-03 18:15:58 |
| [Windows][`/FCT`][`/Pipe`] | 2026-11-03 19:37:39 |
| [Windows][`/FCT`][`/Http`] | 2026-11-03 19:37:51 |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-11-03 19:41:59 |
| [Windows][`/RVMT`][`/Pipe`] | 2026-11-03 18:00:10 |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] | 2026-11-03 18:00:18 |
| [Windows][`/RVMT`][`/Http`] | 2026-11-03 18:00:26 |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] | 2026-11-03 18:00:39 |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-11-03 18:00:46 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] | 2026-11-03 18:00:59 |

## Local Baselines

| Test Item | 1st |
| --- | --- |
| Debug x64 solution build | 2026-11-03 17:16:17 -08:00; 0 warnings, 0 errors |
| CppTest | 2026-11-03 19:22:46 |
| CppTest_Metaonly | 2026-11-03 20:06:48 |

## Observations and Limitations

- A fresh Toolstrip session explicitly rechecked both load and save cancellation against the complete serialized fixture after synthetic point selection 0:0–0:5; both retained identical document content. Queued normal close returned 0. Evidence: CppTest-Toolstrip-file-cancel.json.

- A fresh CppTest_Metaonly session additionally confirmed queued normal close returns exit code 0.

- CppTest_Metaonly full baseline passed, including TouchingList and ordinary list/text touch before/after palette refresh, all responsive modal datasets/sizes and native caption close. Post-palette text momentum was rechecked in an actual 800x600 viewport in both edit modes; fresh contact stopped it and Selection(0,2)-(2,7) remained. The template refresh had expanded the previous viewport to 800x868. Queued normal close passed.

- CDB confirmed standalone /Cli host loss throws rpc_controller::RpcInjectedException from ThrowInjectedExceptionLocked during IViewModel::Translate, with exact message RemotingTest_RvmHost disconnected.; continuing reached an unhandled second-chance C++ exception. Evidence: <GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/CppTestRvm-RpcInjectedException.log.

- Metaonly harness correction: its native Ctrl+Q modal blocks automation Controls. An abandoned diagnostic read triggered the intentional HTTP response fail-fast after dismissal; the CRT Abort prompt was closed. The harness now inspects and dismisses the native dialog before any Controls read. No product change was made.

- CppTest_Metaonly reported its explicit Global shortcut unavailable state while concurrent applications own shortcut registration; local shortcuts remain testable. This is recorded as unavailable OS-global delivery, not a layout defect.

- [Windows][`/FCT`][`/MiniHttp`]: initial lists/clear, exact two-editor retention, full mouse payloads/Ctrl+Q, vertical rail keyboard/five-pixel gap, four palette choices, full responsive dataset/size cycles, Raw Clock updates and point transform/fallback passed. Replacement/takeover retained editors, repeated input and clock, and queued normal close passed. TouchingList and both painting backends are covered by Touching_Remote.ps1. Locked desktop leaves real Win/global input and Direct2D pixels unverified.

- [Windows][`/FCT`][`/Http`]: initial lists/clear, exact two-editor retention, full mouse payloads/Ctrl+Q, vertical rail keyboard/five-pixel gap, four palette choices, full responsive dataset/size cycles, Raw Clock updates and point transform/fallback passed. Replacement/takeover retained editors, repeated input and clock, and queued normal close passed. TouchingList and both painting backends are covered by Touching_Remote.ps1. Locked desktop leaves real Win/global input and Direct2D pixels unverified.

- Pipe FullControlTest completed normal queued exit after full input, layout, palettes, responsive datasets, clock, raw points, renderer replacement/takeover and both editor retention. The readback harness now checks elementDocument for both editors.


- FCT Pipe palette checkpoint: a click used Core coordinates while renderer font measurements were still updating and selected adjacent Refresh List. Returning to Layout showed the exact editor marker retained. The harness now requires current Core and renderer DOM bounds to agree and settle before clicking; no product change.


- CppTest: initial lists/clear, exact two-editor retention, full mouse payloads/Ctrl+Q, vertical rail keyboard/five-pixel gap, four palette choices, full responsive dataset/size cycles, Raw Clock updates and point transform/fallback passed. Queued normal close passed; detailed document and local TouchingList evidence is recorded separately. Locked desktop leaves real Win/global input and Direct2D pixels unverified.

- CppTest local point IO passed TouchingList identity/reversal/cancel/momentum, ordinary-list selection/scroll/reversal, and text point selection/scroll before and after Aurora. Palette refresh grew the local native client to 800x868, leaving no continuation range for the harness flick. Resizing the actual client back to 800x600 restored observed post-Up motion in both Editable and Selectable modes, preserved range 0:2–2:7, and a fresh Down stopped motion. No product change.

- Windows native touch injection was attempted against CppTest. InjectTouchInput returned, but target state did not change on the locked desktop; acquisition remains unavailable. Renderer/Core point IO coverage is recorded separately.

- CppTest: two full responsive grow/shrink cycles, all 24 movie/director records in each appearance, eight publisher menu entries, short/tall account scrolling, OK/Cancel/title close with fresh initial sizes, and Raw Clock change/leave-return passed.

- CppTest Toolstrip persistence passed: private reload retained four images, Consolas/18 selection formatting and hyperlink. RTF/HTML contained Unicode and image data. Point ranges survived Aurora/Default and a 120-pixel content drag at 700x400; actual document bounds moved while endpoints stayed identical. Details: CppTest-Toolstrip-document-results.json. UIA and physical touch are explicitly outside/ unavailable for this job.

- CppTest Toolstrip: synthetic short-press plus 560ms held point selection produced range 0:0–0:5. Holding an existing range opened no menu until release, then exactly one menu with unchanged endpoints. Point Copy/Cut/Paste/Delete, Undo/Redo and Select All passed. After whole-document selection the fixture was reloaded before new point selection, because short presses inside/endpoints intentionally preserve the range. Four effects and all three alignments passed with the point-selected range.

- CppTest Ribbon persistence passed: separate Private Format save/reload retained four images, Unicode, hyperlink, alignment and Deleted style; RTF/HTML exports were nonempty with formatting, Unicode and image data. All eight style effects were independently checked in clipboard HTML at wide and narrow widths, with scroll-to-visible gallery selection. Palette Ember/Default retained range/content; repeated context menus remained usable.

- CppTest document Ribbon: fixture, exact context groups/submenu, selection 0:0–0:5, edge-clamped popup, Copy/Cut/Paste/Delete/Select All and Undo/Redo, four text-effect toggles, three alignments, installed font/size, RGB foreground/background, three hyperlink edit/removal routes and hyperlink preview dialog checked against serialized document state. Keyboard mode checks retain document focus; after hyperlink modal, restore Editable through menu before selecting. Evidence: CppTest-Ribbon-document-results.json.

- Touching_Remote.ps1 passed Pipe/Http/MiniHttp with Direct2D and GDI: renderer/Core typed gesture routing, multi-contact transform/pause/rebase/cancel, fallback, TouchingList identity deletion/reversal/cancel/extra contact, optional momentum, ordinary-list scrolling, text selection/scrolling before and after template replacement, and normal shutdown. GDI PrintWindow showed rectangle/contact pixels on all three transports; Direct2D capture was unavailable (zero matching pixels) on the locked desktop.

- [Windows][`/RPT`][`/MiniHttp`]: button, nine nonempty grid cells/clear, embedded dialog, canonical shortcut labels and Ctrl+Q, full five-button/Alt/Super/move/double-click/wheel payloads before and after replacement/takeover, queued menu close and retained exact fatal error passed. Both Win-key chords are unavailable on the locked desktop: native IO accepts GetKeyName keys, and WindowsInputService exposes no Win key name. Mouse Super payloads passed.

- [Windows][`/RPT`][`/Http`]: button, nine nonempty grid cells/clear, embedded dialog, canonical shortcut labels and Ctrl+Q, full five-button/Alt/Super/move/double-click/wheel payloads before and after replacement/takeover, queued menu close and retained exact fatal error passed. Both Win-key chords are unavailable on the locked desktop: native IO accepts GetKeyName keys, and WindowsInputService exposes no Win key name. Mouse Super payloads passed.

- [Windows][`/RPT`][`/Pipe`]: button, nine nonempty grid cells/clear, embedded dialog, canonical shortcut labels and Ctrl+Q, full five-button/Alt/Super/move/double-click/wheel payloads before and after replacement/takeover, queued menu close and retained exact fatal error passed. Both Win-key chords are unavailable on the locked desktop: native IO accepts GetKeyName keys, and WindowsInputService exposes no Win key name. Mouse Super payloads passed.

- [Windows][CppTest_Rvm][`/Cli:<path>`] delivery: requester terminated nonzero (2147483651) after accepted-host loss in 174 ms; no recovery or retry. Suspended host and pending Controls read established blocked RPC before injection.

- [Windows][CppTest_Rvm][`/Cli:<path>`] idle: requester terminated nonzero (2147483651) after accepted-host loss in 175 ms; no recovery or retry.

- [Windows][CppTest_Rvm][`/MiniHttp`] delivery: requester terminated nonzero (2147483651) after accepted-host loss in 4892 ms; no recovery or retry. Suspended host and pending Controls read established blocked RPC before injection.

- [Windows][CppTest_Rvm][`/MiniHttp`] idle: requester terminated nonzero (2147483651) after accepted-host loss in 186 ms; no recovery or retry.

- [Windows][CppTest_Rvm][`/Http`] delivery: requester terminated nonzero (2147483651) after accepted-host loss in 4860 ms; no recovery or retry. Suspended host and pending Controls read established blocked RPC before injection.

- [Windows][CppTest_Rvm][`/Http`] idle: requester terminated nonzero (2147483651) after accepted-host loss in 184 ms; no recovery or retry.

- [Windows][CppTest_Rvm][`/Pipe`] delivery: requester terminated nonzero (2147483651) after accepted-host loss in 175 ms; no recovery or retry. Suspended host and pending Controls read established blocked RPC before injection.

- [Windows][CppTest_Rvm][`/Pipe`] idle: requester terminated nonzero (2147483651) after accepted-host loss in 206 ms; no recovery or retry.

- [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] delivery: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 257 ms. Host was suspended and a pending Controls read proved the UI RPC was blocked before termination.

- [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 364 ms.

- [Windows][`/RVMT`][`/MiniHttp`] delivery: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 4903 ms. Host was suspended and a pending Controls read proved the UI RPC was blocked before termination.

- [Windows][`/RVMT`][`/MiniHttp`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 485 ms.

- [Windows][`/RVMT`][`/Http /Cli:<path>`] delivery: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 272 ms. Host was suspended and a pending Controls read proved the UI RPC was blocked before termination.

- [Windows][`/RVMT`][`/Http /Cli:<path>`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 377 ms.

- [Windows][`/RVMT`][`/Http`] delivery: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 4913 ms. Host was suspended and a pending Controls read proved the UI RPC was blocked before termination.

- [Windows][`/RVMT`][`/Http`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 482 ms.

- [Windows][`/RVMT`][`/Pipe /Cli:<path>`] delivery: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 285 ms. Host was suspended and a pending Controls read proved the UI RPC was blocked before termination.

- [Windows][`/RVMT`][`/Pipe /Cli:<path>`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 377 ms.

- [Windows][`/RVMT`][`/Pipe`] delivery: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 258 ms. Host was suspended and a pending Controls read proved the UI RPC was blocked before termination.

- [Windows][`/RVMT`][`/Pipe`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 407 ms.

- A second canceled tree-read response reproduced the intentional fail-fast boundary in HttpAutomationService::DumpTreeAsync (CDB: RVMT-Pipe-delivery-renderer.cdb.log). Repository Quality Control requires crashing when a peer disappears and prohibits adding recovery. No GacUI source was changed. The harness now cancels diagnostic reads only after confirmed Core process termination, never while a live native modal blocks the response, and probes until a genuinely pending read establishes the blocked call.

- [Windows][`/RVMT`][`/Pipe`] idle: exact Core-authored host-loss prompt and retained Dom.fatalError, nonzero Core exit, ordinary input rejection, exact !Exit and cleanup passed in 484 ms.

- The first Pipe standalone run passed exact bracket/brace typing, second-host rejection, retained original-host RPC, and requester exit 0. The harness incorrectly required host exit 0: ViewModelHostClient deliberately calls ExitViewModelHostProcess(1) on disconnect. Corrected the harness expectation; no product change.

Fresh CppTest responsive modal resize sequence 240 -> 350 -> 440 -> 680 -> 440 -> 350 -> 240 passed: title-only, detailed rows, icon-only accounts and account-name states changed at the expected widths. RawClock data advanced. Queued self.Close() returned exit code 0. The full baseline completed in the later sessions recorded above.

- The timed-out read was a harness deadline: CDB found live palette work in `WindowsUIAutomationContext::RetireSubtree` with 9,598 bounds hooks. The canceled HTTP read later reached `HttpAutomationService::DumpTreeAsync` and aborted while responding to the disconnected client. A fresh process completed Aurora refresh in 14.2 seconds with a 60-second read bound. No product fix is justified by this single timeout; subsequent palette checks use that bound. Debugger evidence: `<GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/CppTest-palette.cdb.log`.

- CppTest initial list 0–9/clear, exact Search `CppTest_Search[Ab]{Cd}`, rich-editor `CppTest_Document` retention, five-button modifier payloads and Ctrl+Q dialog passed. During Easy Layout choice/palette verification an automation read timed out; no native runtime dialog was present. The original process was inspected with CDB; the resolved harness deadline is recorded above.

- Desktop activation failed with `GetCursorPos failed: Access is denied. (0x80070005)`. Application Controls and IO work. Physical input and real Windows global-hotkey delivery are unavailable; generated input is recorded separately. CppTest reports Hosted mode; CppTest_Metaonly is the native-window variant, following the actual entry points and Project.md.

## Issues Found and Fix

- No native remote-protocol product defect required a source change. Early harness attempts used stale/unsettled bounds, missed the offscreen inline button, attempted an unsupported LWIN key name, or assumed host exit 0. The final native matrix passed with corrected geometry, maximize/scroll, explicit desktop limitations, and the documented host disconnect exit 1.
- Blocked-delivery injection now requires an observed pending Controls read; an earlier completed read did not establish that the RPC was blocked. Live modal reads are never abandoned.

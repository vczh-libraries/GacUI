# TUI Test Matrix Card — Windows — 2026-09-21

Scope: `CppTest_Tui` Debug x64, following `.github/Jobs/DebugTuiControlTestSop.md`. This card tracks only the local TUI showcase; remote protocol matrices are out of scope.

## Test Matrix

| SOP section | Status | Result / evidence |
| --- | --- | --- |
| Windows Terminal identity, profile, and startup at 100x30 / 80x25 | Partial / tool-blocked | `WindowsTerminal.exe` 1.24.11911.0 and the showcase process/title were confirmed from process metadata. sky exposed only ChatGPT; launching the discovered Terminal executable was explicitly rejected by product policy. No viewport/profile or rendered-buffer result observed. |
| Startup lifecycle, 120x40 / 80x25 resize, title, bounds, opaque surfaces | Partial (exec PTY) | The app rendered `Complete Control Showcase (TUI)` and the main List/TextList page in the exec-managed pseudoterminal. PTY viewport and physical Windows Terminal bounds were not measured; resize, no-pre-Show blanking, and opaque-surface checks remain unverified. |
| List / TextList | Partial (exec PTY) | Keyboard `Add 10 items` showed `0` through `9` in both panes. First source rotation emptied only the right pane while the left retained items; the second rotation was followed by a refreshed VT frame showing `0` through `9` in the right pane again. Check/Radio styles, removals, clear/counter, dummy controls, mouse selection, scroll, and revisit retention remain unverified. Input was the PTY stream, not Windows Terminal hardware input. |
| List / ListView | Not run (WT tool policy blocked) | |
| List / TreeView | Not run (WT tool policy blocked) | |
| List / BindableDataGrid | Not run (WT tool policy blocked) | |
| Refresh List / TextList | Not run (WT tool policy blocked) | |
| Refresh List / BindableTextList | Not run (WT tool policy blocked) | |
| Refresh List / ListView | Not run (WT tool policy blocked) | |
| Refresh List / BindableListView | Not run (WT tool policy blocked) | |
| Refresh List / TreeView | Not run (WT tool policy blocked) | |
| Refresh List / BindableTreeView | Not run (WT tool policy blocked) | |
| Refresh List / BindableDataGrid | Not run (WT tool policy blocked) | |
| Layout / Repeat / RepeatStack | Not run (WT tool policy blocked) | |
| Layout / Repeat / RepeatFlow | Not run (WT tool policy blocked) | |
| Layout / Repeat / SharedSize (RepeatFlow) | Not run (WT tool policy blocked) | |
| Layout / Repeat / SharedSize (TextList) | Not run (WT tool policy blocked) | |
| Layout / Responsive | Not run (WT tool policy blocked) | |
| Control / Document Editor (Toolstrip) | Not run (WT tool policy blocked) | |
| Control / TextBox / TextBox | Not run (WT tool policy blocked) | |
| Control / TextBox / TextBox (No Tab) | Not run (WT tool policy blocked) | |
| Control / TextBox / Document | Not run (WT tool policy blocked) | |
| Control / TextBox / Document (No Tab) | Not run (WT tool policy blocked) | |
| Control / Embedded Controls | Not run (WT tool policy blocked) | |
| Misc / Localization | Not run (WT tool policy blocked) | |
| Compact dialog and form layout checks | Not run (WT tool policy blocked) | |
| Misc / Dialogs / MessageDialog | Not run (WT tool policy blocked) | |
| Misc / Dialogs / ColorDialog | Not run (WT tool policy blocked) | |
| Misc / Dialogs / FontDialog | Not run (WT tool policy blocked) | |
| Misc / Dialogs / Open/Save FileDialog | Not run (WT tool policy blocked) | |
| Misc / DatePicker | Not run (WT tool policy blocked) | |
| Window Manager, frame options, local shortcuts, mouse inputs, global shortcut | Partial (exec PTY) | `Ctrl+Q` opened a TUI dialog containing exactly `You pressed Ctrl+Q!` and Chinese `确定`; it was absent from the next refreshed frame after another keyboard action. Child-window/frame, mouse, Ctrl+Alt+Win+Q and global F8 checks were not run. This does not establish Windows Terminal or physical keyboard delivery. |
| Live palette refresh, retained controls, clipping and CJK/wide glyphs | Not run (WT tool policy blocked) | |
| Child captions, borders, dragging/resizing/close and exposed-cell repaint | Not run (WT tool policy blocked) | |
| Exit cancellation, accepted direct/queued Hide/Close and both Stop paths | Not run; forced cleanup | No Exit page/Stop button path was completed. |
| Terminal restoration and usable shell after normal exits | Not run; forced cleanup | Session 15079 was ended after force-stopping its owned child; its exit code was 1. The emitted VT mode-reset sequences do not prove normal restoration or usable shell input. |

## Issues Found and Fix

No application defect was observed. Computer Use explicitly rejected the Windows Terminal target, so no TUI operation was sent to that session.

## Environment and limits

Build: Debug x64 solution build succeeded with 0 warnings / 0 errors (root verified the final Build.log). Windows Terminal 1.24.11911.0 was started through the repository's interactive wrapper and showed process title `Complete Control Showcase (TUI)`; the target window was absent from sky's target inventory and `sky.launch_app` returned the exact rejection `product policy blocks this app: C:\Program Files\WindowsApps\Microsoft.WindowsTerminal_1.24.11911.0_x64__8wekyb3d8bbwe\WindowsTerminal.exe`. The task-owned TUI process (PID 14276), its PowerShell host (PID 36716), and Windows Terminal (PID 28352) were force-stopped after the rejection. This forced cleanup does not establish the SOP's normal-exit/terminal-restoration checks. Profile settings, live viewport, displayed colors/font/cursor, and physical keyboard/mouse input were not observed.

Independent bounded fallback: the documented `copilotExecute.ps1 -Interactive` wrapper ran in an exec-managed pseudoterminal (session 15079). The VT output and keyboard stream supported the List/TextList and local Ctrl+Q checks above. This is console replay evidence only; it is not attributed to Windows Terminal or physical input. The PTY child PID 28872 was force-stopped after no reliable route to the Exit page was available; wrapper session 15079 ended with exit code 1 and emitted `ESC[?9001l`, `ESC[?1004l` and `ESC[m`. That is forced cleanup, not a normal exit.

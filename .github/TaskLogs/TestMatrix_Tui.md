# Test Matrix Card 2026-11-28 04:55:45 -08:00

## Environment

| Field | Value |
| --- | --- |
| Platform / application / commit / build configuration | Windows, CppTest_Tui, GacUI cab13f29a with source fix 358dc4541 and regenerated resources 889978f73, Debug x64 and Win32 |
| Terminal name / version / profile / viewport sizes | Owned Windows pseudoconsole and live console buffer at 120×40, 80×25 and restored 120×40. Installed Windows Terminal 1.25.3131.0. |
| Input methods / desktop availability | The interactive copilotExecute.ps1 wrapper received native console records and replayed terminal input. Physical input and the displayed Windows Terminal profile were unavailable. |

## Test Matrix

| Test Item | 1st |
| --- | --- |
| Build / resource generation / import / sync | 2026-11-28 06:35 -08:00 |
| Unit tests | 2026-11-28 06:51 -08:00 |
| Platform launch and terminal/input capabilities | 2026-11-28 05:05 -08:00 (fixed) |
| Startup, platform services and teardown | 2026-11-28 05:05:00 -08:00 (partial) |
| Follow-up regression checks | 2026-11-28 06:51 -08:00 (fixed) |
| List / TextList | 2026-11-28 05:26:00 -08:00 (partial) |
| List / ListView | 2026-11-28 07:06:17 -08:00 (partial) |
| List / TreeView | 2026-11-28 07:06:17 -08:00 (partial) |
| List / BindableDataGrid | 2026-11-28 07:06:17 -08:00 (partial) |
| Refresh List / TextList | 2026-11-28 06:10 -08:00 (partial) |
| Refresh List / BindableTextList | 2026-11-28 06:10 -08:00 (partial) |
| Refresh List / ListView | 2026-11-28 06:10 -08:00 (partial) |
| Refresh List / BindableListView | 2026-11-28 06:10 -08:00 (partial) |
| Refresh List / TreeView | 2026-11-28 06:10 -08:00 (partial) |
| Refresh List / BindableTreeView | 2026-11-28 06:10 -08:00 (partial) |
| Refresh List / BindableDataGrid | 2026-11-28 06:10 -08:00 (partial) |
| Layout / Repeat / RepeatStack | 2026-11-28 05:45:00 -08:00 (partial) |
| Layout / Repeat / RepeatFlow | 2026-11-28 05:45:00 -08:00 (partial) |
| Layout / Repeat / SharedSize (RepeatFlow) | 2026-11-28 05:45:00 -08:00 (partial) |
| Layout / Repeat / SharedSize (TextList) | 2026-11-28 05:45:00 -08:00 (partial) |
| Layout / Responsive | 2026-11-28 07:06:17 -08:00 (partial) |
| Control / Document Editor (Toolstrip) | 2026-11-28 06:19:15 -08:00 (partial) |
| Control / TextBox / TextBox | 2026-11-28 05:54:03 -08:00 (partial) |
| Control / TextBox / TextBox (No Tab) | 2026-11-28 05:54:11 -08:00 (partial) |
| Control / TextBox / Document | 2026-11-28 05:54:19 -08:00 (partial) |
| Control / TextBox / Document (No Tab) | 2026-11-28 05:54:28 -08:00 (partial) |
| Control / Embedded Controls | 2026-11-28 06:09:28 -08:00 (partial) |
| Misc / Localization | 2026-11-28 06:09:23 -08:00 |
| Compact dialog and form layout checks | 2026-11-28 06:47:11 -08:00 (partial) |
| Misc / Dialogs / MessageDialog | 2026-11-28 05:59:55 -08:00 (partial) |
| Misc / Dialogs / ColorDialog | 2026-11-28 06:00 -08:00 (partial) |
| Misc / Dialogs / FontDialog | 2026-11-28 06:44:50 -08:00 (partial) |
| Misc / Dialogs / Open/Save FileDialog | 2026-11-28 06:06:26 -08:00 (partial) |
| Misc / DatePicker | 2026-11-28 06:09:24 -08:00 (partial) |
| Window Manager | 2026-11-28 06:13:29 -08:00 (partial) |
| Live palette and clipping regression | 2026-11-28 06:57:33 -08:00 (partial) |
| Exit | 2026-11-28 06:26 -08:00 |
| Packaged tutorial checks (when requested) |  |
| Linux/macOS legacy-terminal and Kitty comparison (when applicable) |  |

## Observations and Limitations

- Debug Win32 and x64 built successfully. The resource compiler regenerated both TUI architectures. Both metadata generators and the x64 metadata check exited 0. The x64 unit suite passed 2,032/2,032 cases across 100 files, including all 30 focused TUI and Easy Layout cases. The unmodified binary reproduced the changed file-dialog output and the same rendered content for all 75 changed Ribbon frames. The run retained the existing tracked snapshot baselines.
- The fresh console showed `Complete Control Showcase (TUI)`. Win32 and x64 each ran two simultaneous instances. The first displayed `{Ctrl+Shift+Alt+Win+F8}` and kept its registration. The second displayed `Global shortcut unavailable` and accepted Add 10. All four returned exit code 0, restored the original console modes, cursor visibility and attributes, and read back `RPWINDOWS-INPUT-OK` from the shell.
- Both TextList panes showed 0–9 and then 10–19. The two successive removal operations left odd values, then 1, 5, 9, 13, 17. Clear emptied both panes and the next Add continued at 20. The two Dummy checkboxes changed independently. For TextList, ListView, TreeView and DataGrid the first source rotation removed the right-hand data and the second restored it. The left-hand data remained visible.
- ListView displayed the same Task and Reminder filenames in both Detail panes. Its selector listed BigIcon, SmallIcon, List, Tile, Information and Detail. Selecting SmallIcon preserved the Detail presentation and the visible records. TreeView showed the four original roots and Blue's six arrow children. Green and Orange each showed four. DataGrid showed all five original records and the Name, Gender, Category, Birthday and Website headers. Sorting changed the row order. The edited Name `MarkerName` and Website `https://edited.example` remained visible after committing. The birthday filter showed three records for From only, four for To only and exactly 涼宮 春日 and キョン with both enabled.
- All seven Refresh List pages were visited. TextList displayed `[x] First` and `Read (true)`, then returned to unchecked. BindableTextList switched to titles and retained the updated check state. Both ListViews and the bindable grid changed their cells or headers on column mutations. Both trees showed the updated first parent. The collapsed child views supplied no visible assertion for the child edit.
- Each of the four Repeat layouts displayed zero through nine in words. Successive removal actions left one, three, five, seven, nine and then one, five, nine. Clear emptied each layout. Responsive showed level counts 4/7/4 and current levels 3/6/3 at 120×40, 80×25 and restored 120×40. The editor content survived the resize.
- The TUI editor pasted a unique string set by an independent Windows process. After a suffix was typed and the Windows clipboard was replaced with a different sentinel, Copy published the exact edited text. An independent Windows reader verified it, the guard restored its captured snapshot, and the wrapper exited 0.
- The Toolstrip editor loaded the private Unicode fixture with four paragraphs and image labels 1250×500 and 256×256. Saving and reopening the private file restored the document. RTF contained two picture runs and HTML two embedded image sources. Canceling each export preserved the editor. Both GUI implementations opened the TUI file, and TUI reopened a GUI-produced private file with both images.
- All four TextBox and Document variants retained independent markers through tab switches and resizing. Selection replacement, Undo and Redo matched their inputs. Tab-inserting variants accepted an ensuing `X`. No-Tab variants navigated away and Shift+Tab returned to the field. Increasing and restoring the logical font size kept one-cell line metrics. The embedded combo changed from FullControlTest to RemoteProtocolTest, opened `Pretend to be starting!` and survived the resize and tab round trip.
- Both locales showed the same stored date, time, number and currency with locale-specific formatting and three translated sentences. Dates retained four independent selections, including February 29, 2024 and November 19, 12 and 20, 2026.
- MessageDialog exercised all seven button sets, all applicable default positions and all nine returned selections. ColorDialog accepted #0C2238 and preserved it after canceling #00FF80. FontDialog displayed its simple and full forms, persisted bold, italic, underline and strikeout, and opened the nested color picker. All six file-picker fields were set. Open returned `alpha.txt`, Save appended the configured extension to a new bare name and outer Cancel preserved the selection. File creation was checked separately from selecting a path.
- At 120×40 the captured rectangles measured 56×5 for Message, 52×14 for Color, 46×12 for simple Font, 62×13 for full Font and 74×20 for File. At 80×25, scrolling the form exposed both Open and Save launch buttons and both dialogs opened and canceled. Freshly opened dialogs exposed their actions. A Color dialog already open during a shrink clipped its footer until its title was dragged.
- Window Manager reported Ctrl+Q and left, middle and right button down and up. Child and grandchild windows opened, moved and closed. All six palettes each showed exactly one selected radio at 120×40, 80×25 and restored 120×40. Restoring SkyBlue restored the sampled console attributes. The three sampled cells did not distinguish every palette. Physical primary/extended mouse delivery, the OS-global chord, displayed colors, font and OS IME remain unverified.
- Canceling direct and queued Hide and Close advanced the invocation and closing-query counts together from 1 through 4, with Ready still 0. Six fresh direct and queued Hide, Close and Stop runs then exited normally. Remaining partial items include exhaustive virtualized row comparisons, all dialog validation and locale permutations, direct physical input and displayed terminal styling. Packaged tutorials were outside this job, and Linux/macOS terminal comparisons were inapplicable.

## Issues Found and Fix

- Starting a second TUI repeatedly aborted before its first frame. The debugger stopped in the global-shortcut label when registration was unavailable. The resource now displays the same fallback as the GUI showcase. During unwinding, the physical window also called a destroyed hosted listener. GuiHostedController now uninstalls that listener in its destructor. The new provider regression destroys the physical window afterward and verifies both callbacks on a surviving listener.
- The compact file-picker buttons appeared after scrolling, and live cell reads after palette reconstruction identified the selected radios. A blank pseudoconsole capture and an early font-dialog capture missed these controls. The repeat runs above used the complete visible frame.

## Terminal Comparisons

The Windows observations came from interactive console input and cell-buffer reads. Windows Terminal's visible compositor, profile and physical keyboard/mouse require a desktop session that can operate the terminal. OS IME candidate selection and X1/X2 delivery remain unverified. The Windows console adapter exposed left, middle and right button transitions.

Local evidence is under <GacUI repo>/Test/Output/RpWindows20261128. The paired startup logs, `unit-full.log`, `terminal-root-controls`, `terminal-file-scroll`, `terminal-stable-palettes`, `tui-fixed-final` and `tui-rotation-filter-20261128-b` contain the recorded results.

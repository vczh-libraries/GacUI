# Test Matrix Card 2026-11-03 17:16:17 -08:00

## Environment

| Field | Value |
| --- | --- |
| Platform / application / commit / build configuration | Windows 11 Pro 10.0.26200; CppTest_Tui; base 578a702d5 plus this job; Debug x64 (Win32/x64 verification after fix) |
| Terminal name / version / profile / viewport sizes | Owned Windows ConPTY; conhost 10.0.26100.1; 80x24, 80x25, 100x30, 120x40; displayed Windows Terminal profile unavailable |
| Input methods / desktop availability | Native console-record replay, clipboard transfer, cell-buffer/mode capture and pseudoconsole resize; locked desktop denies physical/desktop input |

## Test Matrix

| Test Item | 1st |
| --- | --- |
| Build / resource generation / import / sync | 2026-11-03 21:12:29; GacUI_Compiler, Debug Win32/x64 builds (0 warnings/errors), both metadata generations and x64 metadata comparison passed |
| Unit tests | 2026-11-03 21:08:45; 21 TUI provider and 8 EasyLayout cases passed (29/29), including both wrapped multiline viewport sizes; no leaks |
| Platform launch and terminal/input capabilities | 2026-11-03 17:41:50; interactive wrapper and owned ConPTY 120x40; desktop-dependent checks unavailable |
| Startup, platform services and teardown | 2026-11-03 20:30:33; 100x30/80x25 initial frames, relayout and six normal teardown paths passed; physical desktop checks unavailable |
| Follow-up regression checks | 2026-11-03 20:37:12; console controls, grid separators/editors, tabs/scalars, clipping and shortcuts checked; unavailable native delivery recorded |
| List / TextList | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| List / ListView | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| List / TreeView | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| List / BindableDataGrid | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Refresh List / TextList | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Refresh List / BindableTextList | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Refresh List / ListView | 2026-11-03 20:32:46; all six view requests, exact mutation/header cycles and Detail fallback passed |
| Refresh List / BindableListView | 2026-11-03 20:34:02; all six view requests, Name/Title/Sub1 and exact mutation/header cycles passed |
| Refresh List / TreeView | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Refresh List / BindableTreeView | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Refresh List / BindableDataGrid | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Layout / Repeat / RepeatStack | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Layout / Repeat / RepeatFlow | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Layout / Repeat / SharedSize (RepeatFlow) | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Layout / Repeat / SharedSize (TextList) | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Layout / Responsive | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Control / Document Editor (Toolstrip) | 2026-11-03 21:12:29; editor actions and four accepted format/path callbacks passed; explanatory-message clipping fixed; persistence remains the documented sample limitation |
| Control / TextBox / TextBox | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Control / TextBox / TextBox (No Tab) | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Control / TextBox / Document | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Control / TextBox / Document (No Tab) | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Control / Embedded Controls | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Misc / Localization | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Compact dialog and form layout checks | 2026-11-03 21:13:13 (fixed); long messages wrap, multiline/blank-line regression passed, all compact dialog/form checks complete |
| Misc / Dialogs / MessageDialog | 2026-11-03 21:13:13 (fixed); all button/output/default/icon cases and 18 long English/Chinese cases across compact/restored sizes passed |
| Misc / Dialogs / ColorDialog | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Misc / Dialogs / FontDialog | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Misc / Dialogs / Open/Save FileDialog | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Misc / DatePicker | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Window Manager | 2026-11-03 20:31:08; passed available console replay and cell-buffer checks; see observations |
| Live palette and clipping regression | 2026-11-03 20:37:12; live presets/state/undo and clipping checks passed; open popup/modal refresh supplemented by provider regressions |
| Exit | 2026-11-03 20:30:33; all four cancellation paths and all six fresh normal exits passed, exit 0 and shell state restored |
| Packaged tutorial checks (when requested) |  |
| Linux/macOS legacy-terminal and Kitty comparison (when applicable) |  |

## Observations and Limitations

- All 18 rebuilt live long-message cases passed for en-US/zh-CN one/two/three-button dialogs at 120x40, 80x25 and restored 120x40, with all content and borders inside the viewport. Final queued Close exited 0 and restored shell input, modes, cursor and colors. CDB confirms wrapping and multiline are both enabled. The regression inspected geometry before the renderer updated wrapped height; it now waits for rendering before asserting viewport bounds and multiline height.

- The new viewport helper initially changed a symbol used by TestControls_EasyLayout. Its original one-argument overload is now retained, and the existing EasyLayout tests are included with TUI provider tests to verify both call paths.

- The corrected GacUI_Compiler run completed with exit 0 and no UI.errors.txt. Generated changes are limited to TuiDialogs and its added template factory; unrelated generated resources are unchanged.

- All four text leaves accepted separately replayed UTF-16 surrogate events: copying each TypedN中文😀Z marker to the Windows clipboard returned the exact Unicode string. Console cell readback represents the supplementary glyph with a replacement character, so displayed emoji/font fidelity remains unverified; this did not corrupt the editor model. Capture/readback: Tui-supplementary-*.

- Document editor File pickers: Load Private plus all three Save commands expose their expected .bin/.rtf/.html filters; cancellation returns to the editor. All four accepted callbacks show the exact selected path and format with the complete explanatory Workflow message, while retaining the edited document. The HTML message remains readable at 80x25 after moving the retained hosted window into the smaller viewport. These callbacks deliberately do not implement file persistence. Evidence: Tui-document-*-callback-fixed*.

- Completed the additional resize retention checks: TextList and BindableTextList kept checked First and true Read across 120x40/80x25/restored120; normal/bindable tree mutations retained selection and updated children after collapse/expand and resizing. ListView filename columns were resized and horizontally scrolled to reach long filenames; both trees retained aligned expansion and selection at compact/restored sizes. Grid captures confirm ascending/descending/source order and stable ties using each source comparator (Category is enum order).

- All six fresh Hide/Close/Stop direct and queued paths exited 0 after opening/dismissing a hosted shortcut modal. Hide/Close each canceled once with 1 invocation/1 query/0 ready before acceptance. Immediate before/after captures matched input/output modes 503/7, visible cursor (25%), and colors 7; prior shell content returned and subsequent shell input worked. Fresh 100x30 and 80x25 frames matched their viewport, with 100x30 -> 80x25 -> 120x40 relayout. Evidence: Tui-exit-*-before/after.jsonl and captures. Fresh even-removal lists showed exactly 0,2,4,6,8.

- All four original Hide/Close actions were canceled once: readouts advanced 1/1/0 through 4/4/0 and the app remained responsive. Clearing cancellation and queued Close returned to the shell; captures Tui-exit-all-four-canceled and Tui-exit-cancellation-readouts.

- All four text leaves additionally retained distinct short markers and long CJK content through 120x40/80x25/restored120. Both document-label outer containers scrolled to their final paragraph; document viewers wrapped, multiline textboxes kept horizontal scroll, and copied text preserved start/end markers. Hidden-page palette refresh retained text and undo/redo.

- Additional fresh 120x40 -> 80x25 -> 120x40 run retained odd-removal list data, source-rotation model, checks/selection, and all four independent thirty-item Repeat models. Compact captures show reflow and reachable twenty-nine; restored counters remain Next: 30; Items: 30.

- Compact dialogs were rechecked at 80x25 in zh-CN and after restoring 120x40: retained RGB12/34/56, simple/full font and nested color cancellation, asynchronous file lists, outer cancellation retention, and a multiline missing-directory prompt. The open nested file prompt and its owner survived resizing. Captures: Tui-*-80-zh.json and Tui-*-restored120-zh.json.

- List/BindableDataGrid at 80x25: exact fresh filter sets (From=3, To=4, both=2) and changed From year 2001=0 passed; disabling filters restored five. All five edited columns survived source rotation, all five sortable headers cycled three states, all six requested views used Detail, and DataGrid editors returned. Name/Website Up/Down kept the caret, while Gender/Category navigation stayed in collapsed/expanded editors until Enter. Captures: Tui-grid-*.json.

- TUI provider regressions passed 20/20, supplementing live checks for viewport startup, close/hide cancellation and notifications, modal interception, cached labels, tabs/scalars, clipping, wide-cell repair, separator backgrounds and live themes. At 120x40 all six palette presets plus return to SkyBlue kept exactly one radio selected with child/grandchild windows retained.

- Mouse4 replay produced no TUI event (readout remained Right button up); extended terminal buttons are unavailable in this backend. Local Ctrl+Q and both Win-extension-bit payloads did produce exact dialogs. Physical and OS-global delivery remain unavailable.

- File dialogs: asynchronous child navigation, txt/xml filters, exact single and multiple paths, outer cancel retention, empty/missing-file/missing-directory validation, create accept/cancel, save default .txt extension, overwrite accept/cancel passed. Pickers returned paths without writing files. Four documented inert options remained checkable.

- FontDialog: simple and full modes retain selectable-only TuiFont/1 fields. All four effects and nested RGB12/34/56 were accepted and retained; canceled Bold edit preserved accepted effects. Cell geometry unchanged; physical font/color fidelity unavailable.

- ColorDialog: RGB tracker Home/End/Left produced 0/255/254. Entered 12,34,56, accepted #0C2238, reopened with that value, changed Red to99 and canceled; page retained 12,34,56. Current implementation has no custom-color palette.

- MessageDialog: seven button sets, all nine outputs, all 16 applicable default positions activated with Enter, and all five icons passed at 120x40. Compact Test / Message 中文 appeared in a borderless message body; buttons were centered.

- Localization: all 12 values changed between zh-CN and en-US; time remained 17:40:34, number/currency remained 2147483647 and 1342177.28, and all three sentence forms retained their literal dollar signs. Wide-cell coordinates use console trailing-byte attributes.

- Responsive: marker Responsive[Ab]{Cd} survived every View migration and was reflected in its label. View/Stack/Group readouts traversed all 4/7/4 levels both ways, stayed bounded after extra end clicks, and survived leave/return. Capture Tui-Responsive-levels.json. These are explicit level-button checks, not automatic resize switching.

- Repeat surrounding controls: horizontal tracker End changed the progress row; horizontal scroll, vertical scroll and vertical tracker changes did not. Returning horizontal tracker to Home restored its progress row. Console character/attribute captures: Tui-Repeat-tracker-end.json and Tui-Repeat-tracker-home.json.

- All four Repeat pages independently passed clear/reset/add zero–nine, both positional-removal survivor sets, clear/add continuing at ten–nineteen, and reset-counter preserving existing rows before appending zero–nine. Shared TextList labels are right-anchored in equal-width left-aligned areas. Captures: Tui-RepeatStack-independent-counter.json and Tui-RepeatFlow/SharedFlow/SharedText-controller.json. Wrapping through twenty-nine, surrounding trackers and 120x40/80x25/restored120 retention subsequently passed.

- Refresh List/BindableDataGrid: Name/Title, Sub1 and both four-step cycles passed; File marker GridFile[Ab]{Cd} survived all six fallback views and return to DataGrid. Every one of 16 cells was activated; all four bottom separator content rows retained their complete character/attribute backgrounds, including intersections and space beyond the last column. The separate outer focus border changed normally. Dragging the following header’s first cell resized the preceding column by five cells. Captures: Tui-DataGrid-separator-states.json and Tui-DataGrid-column-resized.json.

- Refresh List/ListView: first-only MainColumn/SubColumn mutation passed. DataColumns readouts followed [0,1,2] -> [1,1,2] -> [1,1] -> [0,1] -> [0,1,2], and all four header states appeared immediately. Detail uses Columns, while DataColumns projects Tile/Information (knowledge-base listview.md); unchanged Detail cells during DataColumns changes are expected.

- Refresh List/TextList: Check/Uncheck/Read and direct check agreed exactly; Ctrl/Shift row selection preserved First checked. Capture: Tui-RefreshTextList.json. Compact/restored viewport retention subsequently passed.

- List/TreeView: all four roots expanded independently in both panes; Blue showed six arrow children and other roots four. Right-only source rotation/restoration and keyboard Left/Right collapse/expand passed. Expanded hierarchy captured in Tui-TreeView-expanded.json; compact/restored viewport checks subsequently passed.

- List/ListView: original Task/005/16x16/005_Task_16x16_72.png records appeared in both panes; all six requested views retained effective Detail and data. Source rotation emptied/restored only the right pane. End navigation and final visible datasets captured in Tui-ListView-both-end.json; column resizing/horizontal access and compact/restored viewport checks subsequently passed.

- List/TextList console replay: 0–19, odd-position removal, right-only source rotation/restoration, clear/re-add retaining counter at 20, check/radio styles, independent Dummy checkboxes and exclusive Dummy radios passed. Fresh odd/even removal and compact/restored viewport checks subsequently passed.

- Owned ConPTY at 120x40 launched CppTest_Tui through copilotExecute.ps1 -Interactive. Captured all 120x40 cells; the main frame exactly fills that viewport. Evidence and replay helpers are under <GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/. Initial 80x24 interactive run returned exit 0 after canceled Hide then accepted queued Close.

Interactive CLI starts at 80x24. Native console reads and mouse records work: Add 10 items displayed 0 through 9 in both lists. SetConsoleScreenBufferSize returned success but the CLI host restored 80x24, so the remaining viewport checks used the owned pseudoconsole.

Windows locked desktop: physical input and displayed Windows Terminal font/color/profile checks are unavailable (GetCursorPos: Access is denied). Continuing with an interactive CLI, native console-record replay and cell-buffer capture. UIA tests, packaged tutorials and cross-platform comparisons are excluded by this Windows job.

## Issues Found and Fix

- The first post-fix build rejected the authored Workflow variable name template after C++ generation; renamed it labelTemplate and regenerated successfully (no manual generated-source edits).

- Control/Document Editor: private-format callback displayed its documented not-implemented message, but the long path/message exceeded 120 columns; title, both side borders and message ends were clipped. Capture Tui-document-load-callback.json. Reproduced with long MessageDialog text, then enabled wrapping and wrapped-height calculation only on the message label template. Regenerated both architectures; all accepted callbacks and 18 live message cases now pass, alongside the multiline regression.

## Terminal Comparisons


# Test Matrix Card 2026-11-14 / observable skins and document persistence

## Environment

| Field | Value |
| --- | --- |
| Platform / application / commit / build configuration | Windows; CppTest_Tui (packaged TuiSkin pending); task implementation after 7797a0ee8; Debug x64 with Win32 metadata verification |
| Terminal name / version / profile / viewport sizes | Owned Windows ConPTY; 120x40 and 80x25; displayed terminal profile unverified |
| Input methods / desktop availability | Interactive execution wrapper, native console-record replay, pseudoconsole resize and live cell-buffer capture |

## Test Matrix

| Test Item | 1st |
| --- | --- |
| Build / resource generation / import / sync | 2026-11-14; GacUI release flow passed; Release/platform work running |
| Unit tests | 2026-11-14; Debug x64 and Release Win32/x64: 98 files / 1999 cases each; Debug has no leaks |
| Platform launch and terminal/input capabilities | 2026-11-14 01:31; owned 120x40 and 80x25 consoles |
| Startup, platform services and teardown | 2026-11-14 01:31; 120x40 and 80x25 exit 0; input/output modes restored |
| Follow-up regression checks | 2026-11-14; arbitrary items, one action/refresh, image metadata |
| List / TextList |  |
| List / ListView |  |
| List / TreeView |  |
| List / BindableDataGrid |  |
| Refresh List / TextList |  |
| Refresh List / BindableTextList |  |
| Refresh List / ListView |  |
| Refresh List / BindableListView |  |
| Refresh List / TreeView |  |
| Refresh List / BindableTreeView |  |
| Refresh List / BindableDataGrid |  |
| Layout / Repeat / RepeatStack |  |
| Layout / Repeat / RepeatFlow |  |
| Layout / Repeat / SharedSize (RepeatFlow) |  |
| Layout / Repeat / SharedSize (TextList) |  |
| Layout / Responsive |  |
| Control / Document Editor (Toolstrip) | 2026-11-14; private/RTF/HTML, rich images and styles, cross-editor exchange |
| Control / TextBox / TextBox |  |
| Control / TextBox / TextBox (No Tab) |  |
| Control / TextBox / Document |  |
| Control / TextBox / Document (No Tab) |  |
| Control / Embedded Controls |  |
| Misc / Localization |  |
| Compact dialog and form layout checks | 2026-11-14; document Open/Save at 80x25 |
| Misc / Dialogs / MessageDialog |  |
| Misc / Dialogs / ColorDialog |  |
| Misc / Dialogs / FontDialog |  |
| Misc / Dialogs / Open/Save FileDialog | 2026-11-14; document commands, cancel, overwrite accept/cancel |
| Misc / DatePicker |  |
| Window Manager | 2026-11-14; six captions/order/default, mouse and keyboard at both sizes |
| Live palette and clipping regression | 2026-11-14; document identity, selection and undo retained |
| Exit | 2026-11-14; cancel-close and queued Hide retained |
| Packaged tutorial checks (when requested) |  |
| Linux/macOS legacy-terminal and Kitty comparison (when applicable) |  |

## Observations and Limitations

This card covers the skin selector, document persistence, related dialogs, refresh state retention and requested packaged tutorial. Unrelated full-page SOP operations remain unexecuted unless explicitly recorded. Both viewport runs used the rebuilt implementation through the interactive wrapper and ended with exit 0, preserving console modes 503/7. Physical terminal input and displayed terminal profile fidelity remain unverified. A fresh Windows Terminal 1.24.12741.0 session at 120x30 started the interactive wrapper, but the desktop returned no foreground HWND and screen capture failed with an invalid handle. It was closed through the TUI queued Hide path with exit 0 and unchanged modes 503/7; this is console verification only. The separate native GUI remoting session accepted desktop mouse and Tab/Space input with visible rendering; that does not establish terminal-display coverage. Native Linux/macOS and Wasm execution remain unverified. Evidence is kept under <GacUI repo>/Test/Output/ShowcaseVerification.

Private-file verification retained all eight fixture styles and original image payload hashes. A newly inserted 123x456 image retained its file-backed label, then its exact pathless label after reload with the original file unavailable. GUI Ribbon and Toolstrip private copies reopened in the fresh 80x25 TUI process. Both CppTest and native Metaonly subsequently passed the two-editor exchange, preserving edited text/selection through refresh, undoing the edit and saving byte-identical private copies (120 and 124 assertions). HTML and RTF exports contain all three images and Unicode/hyperlink content. Live debugger inspection showed unchanged document identity, selection, undo step and modified state through exports and skin refresh; private save alone updated the saved step. Native operation breakpoints recorded one load/save/RTF/HTML callback each, with canceled dialogs adding none.

## Issues Found and Fix

- The new actual-showcase regression exposed the fake input service’s missing letter/function-key mapping; its shortcut lookup is now implemented and the unfiltered Debug suite passes.
- Early console captures under the release linker load sometimes observed intermediate refresh geometry. Settled captures retain all text and image labels; these transient harness observations are not recorded as product failures.

## Terminal Comparisons

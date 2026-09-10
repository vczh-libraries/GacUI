# TuiControlTest Terminal SOP

Run `CppTest_Tui` in Windows Terminal after building `Test/GacUISrc/GacUISrc.sln`. The title is `Complete Control Showcase (TUI)`. Begin at 120 columns by 40 rows; repeat layout checks at 80 by 25 and after restoring the larger viewport. One GacUI pixel is one terminal cell.

From `Test/GacUISrc` in Windows Terminal, run `& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable CppTest_Tui -Configuration Debug -Platform x64 -Interactive`. Adjust the absolute repository path for another checkout. The interactive flag preserves native console handles. This app has no HTTP automation endpoint, Core process, renderer process, or renderer-replacement procedure. Terminal font and color settings belong to Windows Terminal.

## Linux and macOS launch

On Linux use sibling `wGac`; on macOS use sibling `iGac`. Refresh the owning upstream releases, then run `./import.sh`, `./syncProj.sh`, and `./build.sh` from that platform repository. Start `./test.sh --app:tui` in a foreground interactive terminal. The target is `Test_TuiControlTest` under `WGacTuiControlTest` or `MacTuiControlTest`. Keep stdin/stdout attached; `--unblock`, `--hosted`, and `--port` do not apply. Complete the same page checks below and maintain `TestMatrix_Tui.md` in the platform repository.

On both Linux and macOS, always try the legacy terminal first: use the desktop's ordinary legacy terminal on Linux and Apple Terminal.app on macOS. Record its name, version and relevant profile settings. Only after a check fails there, retry that check in Kitty with the same application build, configuration and inputs. Do not replace a legacy-terminal failure with a Kitty pass. If only Kitty works, write the exact failure and successful retry in the terminal comparison record card below and in the platform's `TestMatrix_Tui.md`. If the legacy terminal or Kitty cannot be operated, record that limitation explicitly; do not infer a result.

Use the host terminal wherever the shared procedures say Windows Terminal, and native paths for file fixtures. The OS modifier label is `Super` on Linux and `Command` on macOS. Validate shortcut labels using those names. The shared showcase currently retains the literal `Win` spelling in its success-message text on all platforms. The Windows-specific console-record and CDB evidence below remains historical Windows evidence.

POSIX terminals supporting the Kitty keyboard protocol can report Super independently of Alt. The backend requests disambiguated keys and restores the prior keyboard mode on exit; legacy terminal Meta remains Alt. SGR mouse input has no Super bit, and the existing wGac input service does not register global hotkeys. Disambiguation mode does not report standalone modifier keys, so the current POSIX adapter cannot show access-key overlays from Alt alone; use mouse/arrow menu navigation and record that limitation. The existing POSIX locale implementation also keeps en-US date/number formatting while translated resource strings follow the selected locale. Record these unavailable paths explicitly. Injected CSI/SGR bytes prove decoding and application behavior, while real terminal-generated input is required to establish host delivery. A passed decoder test does not establish macOS verification.

On macOS, `Mac/TUI/TuiCocoaController` supplies Cocoa clipboard/image services and Carbon global registration to the shared controller. The plain terminal executable pumps Cocoa events and clipboard changes on the TUI owner thread without opening or activating a Cocoa window. Verify external text using another application or `pbcopy`/`pbpaste`. Carbon registration and actual global chord delivery are separate checks; a locked desktop prevents native activation and displayed appearance verification. Record terminal-generated test keys and raw replay separately from physical input.

Record the Cocoa configuration's `overrideAlt` and `overrideOsSuper` values. Both default to false, and the local macOS showcase enables both. They independently supplement terminal Alt/Super from current local Option/Command state, preserving modifiers already reported by the terminal. Verify the disabled settings and all enabled combinations separately. Current state can differ from queued event-time state, and no override can recover a chord that the terminal consumes without delivering a key. Treat such a failure as a legacy-terminal result before trying Kitty.

On Linux, the TUI clipboard uses an unmapped X11 selection owner because a separate Wayland client cannot borrow the terminal's focus serial. With `DISPLAY` available, verify text with another desktop app through X11/XWayland. Without an X display, document the process-local clipboard limitation and use the terminal's ordinary paste action for external text. Rich document/image clipboard objects are retained inside the app; external transfer is UTF-8 text. Keep the app running while another client reads its selection.

## Rules for every operation

Locate the current visible control after every transition. Perform one action, inspect its immediate result, and then continue. Exercise mouse and keyboard access and inspect focus, selection and checked state separately. Leave each page and return to verify retained state. Use fresh runs wherever a procedure specifies exact initial values. Check CJK and supplementary text at both viewport sizes; a width-two scalar must not leave half a glyph when clipped or overwritten.

Keep these baseline limitations explicit: the four private-format/RTF/HTML callbacks show explanatory dialogs and do not persist documents. ReadOnlyCheckBox, DereferenceLinks, NetworkButton and AddToRecent remain inert fake-file-dialog options. The backend may not expose OS Super or all five mouse buttons. Do not record an unobservable chord or button as passed.

## Startup, platform services and teardown

Start fresh at 100x30 and 80x25 before manually resizing. The first visible main layout and physical/hosted bounds must match the terminal, despite the resource declaring 120x40. Before Show, terminal takeover must not paint application cells. After Show, verify title, cell geometry, one physical terminal window, hosted popups/dialogs, and opaque surfaces. Programmatic main-window sizing records the requested dimensions but does not resize Windows Terminal. Resizing the terminal replaces the stored dimensions, relayouts, and clips to the physical viewport.

Open child and modal windows, overlap them, move/resize them, and close them. Exposed cells must repaint. Check clipboard copy/paste with another app, asynchronous file enumeration, caret blinking and global shortcuts. All owner-thread work must continue during modal pumping.

For child title bars, inspect short, long, empty and CJK captions with SizeBox enabled/disabled and TitleBar toggled. The double/thick top border must continue from the caption's immediate trailing space to the close button. An empty caption must not erase a blank strip. Long captions must ellipsize without covering the button; dragging the title/border region and clicking Close must still work. Caption buttons have no spacer between them when multiple commands are supported; the current hosted manager supports Close only.

Exercise each direct/queued Hide and Close button and both Stop buttons on separate fresh runs. For Hide/Close, first enable cancellation and require one query with no ready notification or stop, then disable cancellation and retry. Accepted requests must return normally from GuiApplication::Run through WindowService()->Run. Closing with a hosted modal open must preserve its existing focus/dismiss interception and keep the main app running; closing an ordinary child must also leave it responsive. After each normal exit, require restoration of terminal contents, cursor visibility, colors and input modes; type into the shell to verify usable input. Repeat after opening/closing dialogs. Terminal-tab close and forced termination do not satisfy this check.

## Follow-up regression checks

Inspect borderless ordinary popups, tooltips and combo/date content dropdowns, retaining actual menu and content-control borders. Ordinary/date combos stay one row. Scrollbar/tracker handles paint uniformly and disabled handles blend into their tracks. Header sorting glyphs precede the title with its text color; the following header's first cell resizes the preceding column. Focus uses bold plus underline for buttons, check/radio, combos, menus and tab headers; ALT labels have opaque white backgrounds and black text. Recheck all of these after shrinking and restoring the viewport.

In DataGrid view, hover/select every cell and change rows: both the row highlight and cell fill must preserve the entire bottom separator's normal background and glyph, including intersections and space beyond the last column. Repeat editing, column resize and scrolling/recycling. Detail/TextList/TreeView keep their full one-row selection. Name/Website editors retain caret Left/Right and Shift selection, with Up/Down a no-op. Gender/Category combos keep arrow/Home/End/Page navigation, boundaries and repeats inside the editor with the popup collapsed and expanded; highlight changes commit only through the existing Enter/mouse path. Dismiss the editor and verify grid navigation returns. Compare CppTest for shared control behavior.

Type and paste literal TAB in each tab-enabled editor; require exactly one U+0009 without focus traversal. At default interval four, X in tab-X, a-tab-X, abc-tab-X and abcd-tab-X starts at columns 4, 4, 4 and 8. Repeat with interval eight, wide characters, wrapping, alignment, horizontal scrolling, selection, undo and clipboard. No Tab pages retain TAB/Shift+TAB traversal without insertion. The provider regression separately checks paragraph/label geometry and caching; injected events do not establish native terminal typing.

Repeat the Window Manager local/global shortcut checks independently. The Windows TUI controller owns its service window, clipboard/input/image services and owner-thread pump; deterministic service/label-cache tests supplement actual startup, clipboard, caret and global-hotkey checks. Returning from document pages must not leave hidden document shortcuts intercepting a dialog or another page. Keep native input failures and unobserved font/cursor/color appearance explicitly unverified.

## List / TextList

`TextListTabPage.xml` and `MyTextItem` in `Resource.xml`. Preserve the two parallel direct/bindable lists, top `Text / Check / Radio` selector, central `Operations` group, source-rotation command, and two Dummy checkboxes plus two grouped Dummy radio buttons.

Follow FCT `Add and Clear Both Lists`: fresh `Add 10 items` shows `0` through `9` in both panes; `Clear` removes them from both. In a fresh run, two additions produce `0` through `19`. Exercise all three list styles and their check/radio behavior. On separate fresh ten-item datasets, `Remove odd items` leaves `1,3,5,7,9`, and `Remove even items` leaves `0,2,4,6,8`; preserve the actual positional-removal handlers despite their potentially misleading captions. Rotate the source twice: only the right list empties, then its model reappears. Clear and add again without restarting to check the counter continues. Verify independent Dummy checks, exclusive Dummy radios, selection, scrolling, and retained data after tab changes.

## List / ListView

`ListViewTabPage.xml` and `ListViewData.xml`. Preserve the top view selector/source-rotation command, two corresponding panes, `Id / Category / Size / File` columns, and original sample records, including `Task / 005 / 16x16 / 005_Task_16x16_72.png`. These are text records; the TUI showcase bundles no images.

Compare the complete datasets in both panes by scrolling. Select rows by mouse and keyboard, request each of BigIcon, SmallIcon, List, Tile, Information, and Detail, and require effective Detail with unchanged data and no unsupported renderer. Rotate the source twice: only the right pane empties then restores. Resize columns and the terminal and verify headers remain aligned with their values and all filenames remain accessible.

## List / TreeView

`TreeViewTabPage.xml`. Preserve the two side-by-side trees, source-rotation command, and original `Blue+ / Green+ / Grey+ / Orange+` hierarchy and arrow labels.

Expand and collapse all four roots in both panes. Blue contains six children (`<--`, `-->`, `V`, `^`, `<`, `>`); the other roots contain four (`V`, `^`, `<`, `>`). Exercise keyboard tree navigation, mouse expansion, selection, and scrolling. Rotate twice and require only the right tree to empty then restore. Verify ordinary repainting preserves selection/expansion and resizing leaves correctly aligned glyphs and text.

## List / BindableDataGrid

`DataGridTabPage.xml`, `DataGridComponents.xml`, and models in `Resource.xml`. Preserve the view selector/source-rotation command, five original Japanese/Chinese records, and `Name / Gender / Category / Birthday / Website` columns, including the Birthday header's `From:`/`To:` filter popup.

Verify all five records and edit every column with its appropriate editor; leave/reopen the cell and require the value to persist. The existing editors submit through `CellValue` bindings, so do not assume Escape rolls back an already submitted value. Cycle each sortable header through its source-supported sort states, checking order and ties. On fresh records, default From-only filtering leaves three rows, default To-only leaves four, both leave exactly `涼宮 春日` and `キョン`, and disabling both restores five. Change the date bounds and verify filtering follows them. Rotate the source twice and require edited records to return. Request BigIcon, SmallIcon, List, Tile, Information, and Detail in turn; require effective Detail each time, then choose DataGrid and require the editors to return. Resize columns, scroll, and inspect popup placement and CJK cell alignment.

## Refresh List / TextList

`RefreshListTabPages.xml`, `RefreshTextListTabPage`. Keep `Check First / Uncheck First / Read (false)` above the six checkable rows `First` through `Sixth`, initially unchecked.

`Check First`, then `Read`, must show a checked First item and `Read (true)`; `Uncheck First`, then `Read`, reverses both. Toggle First directly and require Read to report the model value. Exercise Ctrl/Shift selection independently of checking. Repeat after scrolling and shrinking/growing the terminal to ensure recycled rows show current check state.

## Refresh List / BindableTextList

`RefreshListTabPages.xml`, `RefreshBindableTextListTabPage` and `RefreshItem`. Preserve `Use Name / Use Title / Check First / Uncheck First / Read` above the three checkable rows, initially unchecked.

`Use Title` changes `First / Second / Third` to `1st / 2nd / 3rd`; `Use Name` restores the names. Under both mappings, exercise programmatic check/uncheck, Read, and direct check interaction and require agreement with the model. Ctrl/Shift selection remains separate. Switching text mappings, scrolling, and returning to the page must not discard checked state or leave stale labels.

## Refresh List / ListView

`RefreshListTabPages.xml`, `RefreshListViewTabPage`. Retain the selector, `*MainColumn / *SubColumn / *DataColumn / *Column`, four columns, `First` through `Fourth`, and `One / Two / Three` subitems with Task/Reminder/Tip identities.

Toggle `*MainColumn`: only First becomes MainColumn and back. Toggle `*SubColumn`: only its first subitem becomes SubColumn and back. Four `*DataColumn` clicks must expose `[0,1,2] -> [1,1,2] -> [1,1] -> [0,1] -> [0,1,2]`. Four `*Column` clicks must show `[Id,Category,Size,File] -> [Id,What?,Wait?,File] -> [Id,Wait?,File] -> [Id,Size,File] -> [Id,Category,Size,File]`. Verify each intermediate result immediately, including duplicate projection indexes. Repeat view requests, selection, scrolling, and resizing without having to reopen the page to see updates.

## Refresh List / BindableListView

`RefreshListTabPages.xml`, `RefreshBindableListViewTabPage` and `RefreshItem`. Preserve the corresponding table and `Use Name / Use Title / *Sub1 / *DataColumn / *Column` commands.

Switch names `First` through `Fourth` to titles `1st` through `4th` and back. `*Sub1` changes only the first row's One to SubColumn and back immediately. Run the exact four-click DataColumns and header cycles specified for Refresh List / ListView. After Category is reinserted, confirm it still reads Sub1 and follows later changes. Repeat under each requested view and after scrolling modified items out of view and back.

## Refresh List / TreeView

`RefreshListTabPages.xml`, `RefreshTreeViewTabPage`. Preserve `*First / *First/Second` above the four-root tree, the children of First and Second, and Task/Reminder/Tip associations.

Expand First; `*First` toggles its label First/One without losing its children. `*First/Second` toggles its second child Second (1)/Two (1), leaving sibling branches unchanged. Repeat the child mutation with First collapsed, then expand and require the latest name. Select another node before mutating the target and check coherent selection/expansion. Scroll and resize to verify visible and recycled node templates refresh correctly.

## Refresh List / BindableTreeView

`RefreshListTabPages.xml`, `RefreshBindableTreeViewTabPage` and `RefreshItem`. Preserve the same root/child arrangement and `Use Name / Use Title / *First / *First/Second` commands.

Switch names to ordinal titles throughout expanded branches and back. `*First` toggles Name First/One and Title 1st/One; `*First/Second` toggles Name Second (1)/Two (1) and Title 2nd (1)/Two (1). Verify both mappings, mutation while collapsed followed by expansion, unchanged siblings, and current labels after scrolling. Renaming must remain separate from selection and expand/collapse actions.

## Refresh List / BindableDataGrid

`RefreshListTabPages.xml`, `RefreshBindableDataGridTabPage`, and `TextEditor` in `DataGridComponents.xml`. Preserve four original records, the selector and `Use Name / Use Title / *Sub1 / *DataColumn / *Column`, and `Id / Category / Size / File` columns.

Verify Name/Title switching, first-row Sub1 mutation, and both exact four-click collection/header cycles from Refresh List / ListView. Edit File and leave/reopen the cell. Request BigIcon, SmallIcon, List, Tile, Information, and Detail in turn, requiring effective Detail, then return to DataGrid and verify its editor and edited value survive alongside text-mapping changes. Reinsert Category and require its binding to follow subsequent Sub1 changes. Exercise cell selection, header resizing, horizontal scrolling, and repainting at every intermediate column count.

## Layout / Repeat / RepeatStack

`RepeatTabPage.xml`, `RepeatComponents.xml`. Keep the four inner Repeat tabs, this page's vertical English-number buttons, and the right-hand `Operations` group with all five commands. Preserve the surrounding horizontal scroll/tracker/progress controls below and vertical scroll/tracker controls at the right.

Clear, Reset Counter, Add 10 must produce zero through nine. Remove odd items must leave one/three/five/seven/nine. On a reset ten-item dataset, Remove even items must leave zero/two/four/six/eight. After a reset/add, Clear/Add without resetting must produce ten through nineteen; Reset Counter alone leaves existing rows, and the next Add appends zero through nine. Add enough items to scroll, check order and tab-state retention, then exercise the surrounding controls over their ranges; progress follows only the horizontal tracker. The other three Repeat pages must run this same controller sequence on their independent models.

## Layout / Repeat / RepeatFlow

`RepeatTabPage.xml`, `RepeatComponents.xml`. Keep the wrapping field of English-number buttons beside the same Operations group and surrounding scroll/tracker controls.

Run the complete RepeatStack controller sequence on this tab. Add several batches, narrow and widen the terminal, and require wrapping without missing or reordered items; scroll to the last item. Confirm button widths follow their text lengths and remain different where labels have different lengths. Check the surrounding tracker/progress relationship and independent state when switching Repeat tabs.

## Layout / Repeat / SharedSize (RepeatFlow)

`RepeatTabPage.xml`, `RepeatComponents.xml`, retaining `SharedSizeRoot` and the `EnglishNumber` width group. Keep the same English-number buttons and Operations placement as RepeatFlow.

Run the complete RepeatStack controller sequence. Add batches containing longer names and require participating buttons to share the widest measured width. Resize and verify uniform widths, wrapping, and item order. Clear/reset/add short labels again and verify stale content or an obsolete long-label width does not survive. Test independent counters and the surrounding tracker/progress controls.

## Layout / Repeat / SharedSize (TextList)

`RepeatTabPage.xml`, `RepeatComponents.xml`, and `SharedSizeTextItemTemplate`. Preserve the selectable text-list counterpart with right-anchored, equal-width English-number label areas and the same Operations group.

Run the complete RepeatStack controller sequence. Select with mouse/keyboard, scroll through longer names, and require equal-width, right-anchored label areas with left-aligned text. Remove rows and clear; require exact surviving items, no stale labels, and no invalid selection appearance. Switch among all four Repeat tabs and verify their independent contents/counters and the surrounding tracker/progress behavior.

## Layout / Responsive

`ResponsiveTabPage.xml` and its three helper controls. Preserve the vertically ordered `GuiResponsiveViewComposition`, `GuiResponsiveStackComposition`, and `GuiResponsiveGroupComposition` groups, `LevelUp();`/`LevelDown();`, level readouts, and familiar Pen/Pineapple/Apple labels.

Enter a marker into `Edit me!`, traverse View levels both ways, and require the same editor content to survive migration and appear in the label. Verify View has four levels, Stack seven, and Group four; step to both boundaries and require bounded readouts. Stack changes participating children incrementally, while Group advances eligible children together. Resize and switch away/back without state loss. This source demonstrates explicit level buttons and has no ResponsiveContainer, so resizing must not be falsely described as an automatic level-switching test.

## Control / Document Editor (Toolstrip)

`DocumentEditorToolstrip.xml`, `DocumentEditorBase.xml`, `DocumentComponents.xml`, and their dependencies. Preserve the exact page title, `File / Edit / View` menu bar, familiar submenu labels (including the source's `Paragram Alignment`), and large central document area.

Type a unique marker, select/replace, copy/paste, cut/delete, undo/redo, and verify exact content and command availability. Apply/remove each style and foreground/background color; cancel dialogs and require no change. Exercise default/left/center/right alignment and Preview/Selectable/Editable modes, requiring each mode's actual editing/selection behavior. Create/edit/cancel/remove a hyperlink and activate it to check `You Clicked a Hyperlink!` and the recorded URL. Cancel image selection, then insert/select/delete/undo a textual image object. Open every File picker and verify its filters, navigation, and cancellation. For FCT's two-editor continuity check, enter a second distinct marker in a retained TextBox page, visit List, and return to both editors; require unchanged markers with no invented Search field.

The four file-format callbacks in the generated FullControlTest C++ currently throw `You should implement this function.`, while their Workflow bodies show not-implemented dialogs. Retain the commands and document this baseline limitation; do not promise successful private-format/RTF/HTML file round trips. When rewriting TUI XML, prefer retaining those explanatory Workflow bodies as generated behavior instead of copying throwing UserImpl stubs, and verify the named format/path in the resulting TUI message. This does not implement persistence. `CancelWindowClose()` is not wired by FullControlTest's `Resource.xml`, so do not invent a baseline save-on-exit requirement.

## Control / TextBox / TextBox

`TextBoxTabPage.xml`, `TextBoxSubTabPage` with tab input enabled, and its Text resource. Preserve `Archer` in the single-line field above the multiline poem beginning `I am the bone of my sword`, the four child-tab captions, and `Make Font Larger / Make Font Smaller` below their shared editor group.

Verify both original texts; enter independent markers, navigate arrows/Home/End, select/replace, copy/paste, and undo/redo. Tab inside an editor inserts tab content without moving focus; use the mouse/access key to leave it. Add multiline content and scroll. Run Larger then Smaller and require the requested size to change and return across all four leaves without changing glyph height. Switch away/back and require both markers. Repeat with CJK and supplementary characters to expose cell/caret mismatches.

## Control / TextBox / TextBox (No Tab)

The second `TextBoxSubTabPage` instance in `TextBoxTabPage.xml`, with `TextBoxAcceptTabInput=false`. Keep the exact `(No Tab)` title and the same Archer-over-poem arrangement as its sibling.

Repeat the preceding page's editing, selection, clipboard, undo/redo, and scrolling checks with different markers. Tab/Shift+Tab must move focus without inserting a tab character. Compare directly with the tab-accepting sibling. Shared size requests must update the binding/readout without changing cell metrics or either page's text. Switch away/back and verify independent retained content.

## Control / TextBox / Document

`TextBoxTabPage.xml`, `DocumentBoxSubTabPage`, `DocFixed`, and `DocRelative`, with tab input enabled. Preserve the top Archer DocumentTextBox and the two document panes below: `Fixed Size Title` left, `Relative Size Title` right, with their original poem.

Check all three original texts, enter distinct markers in each editor, select/replace and undo/redo, and require Tab insertion without focus movement. Exercise access-key/mouse focus. Resize to wrap the documents, verify viewer scrolling versus label growth, and reach all text before restoring the viewport. Shared Larger/Smaller must preserve the fixed/relative metadata and text while glyph height remains one row. Visit another top-level tab and return to require all three markers unchanged.

## Control / TextBox / Document (No Tab)

The second `DocumentBoxSubTabPage` instance and the same document resources in `TextBoxTabPage.xml`, with tab input disabled. Keep the exact `(No Tab)` title, top Archer field, and matching fixed-title/relative-title panes.

Repeat its three-editor content, selection, clipboard, undo/redo, scrolling/growth, and resize checks with different markers. Tab/Shift+Tab must move focus and leave all three document texts unchanged. Verify access keys and mouse focus, shared size requests without cell-metric changes, and persistent independent text/metadata after switching pages.

## Control / Embedded Controls

`EmbeddedControlsSubTabPage.xml` and linked `SideDocuments.xml`. Preserve the selectable document headed `RemotingTest_Core.exe`, explanatory text, `Argument / Description / Required` table with `/Http`, `/Pipe`, `/FCT`, `/RPT`, and final `Run [combo]? Start [RIGHT NOW]!` sentence.

Verify the heading, complete four-row table, and initial combo selection. Scroll to the inline controls, choose RemoteProtocolTest, and require persistence after scrolling, resizing, and returning to the page. Click RIGHT NOW and require exactly `Pretend to be starting!` in a dismissible TUI dialog; dismiss OK and require a responsive document with the combo state retained. Select surrounding text and navigate around every embedded composition, checking valid caret/selection boundaries, inline relayout after wrapping, and no stale cells after dropdown/dialog dismissal.

## Misc / Localization

`LocalizedStringsTabPage.xml` and `LocalizedComponents.xml`. Preserve the locale selector above the formatted-value list, the localized page title `Localization / 本地化`, all twelve entries, and all three sentence-binding forms including escaped dollar characters.

Switch en-US to zh-CN and back. Require the title, selector label, date/time/number/currency strings, and all three sentence entries to update. The number value remains 2147483647 and currency value 1342177.28 under their locale formatting; changing locale alone must not change the stored date/time. Check the literal dollar characters in sentence results and correct CJK cell boundaries. Visit Dialogs and require its shared locale selector to reflect the application locale.

## Compact dialog and form layout checks

At 120x40, 80x25 and after restoring the larger viewport, inspect all Message/Color/simple Font/full Font/Open/Save dialogs and their launch forms. Require zero blank rows between compact fields and exactly one explicit blank row before final actions; keep textbox, group and list borders. First-column labels must align vertically with bordered textbox content while retaining left alignment, and labels beside one-row combos must remain one row. Keep results adjacent to their label/list and preserve scrolling, keyboard focus and returned values.

Message content must be a readable borderless label, preserving short, multiline and CJK content and explicit blank lines. Exercise one-, two- and three-button English/Chinese messages with/without icons: the natural-width buttons remain centered across the dialog at every localized width. Color, simple/full Font and Open/Save File dialog actions align to the right, including the full-font Pick a Color action, with one explicit blank row above. Include multiline file-validation prompts.

Count each RGB tracker as exactly one rendered cell row centered beside its textbox, with vertically centered labels. Drag it, use keyboard steps and reach 0/255; type RGB 12,34,56, accept/reopen, then change/cancel and require preserved color semantics. Recheck font effects/nested color and file enumeration/selection/nested prompts.

## Misc / Dialogs / MessageDialog

`LocalizedDialogsTabPage.xml`, `LocalizedComponents.xml`, and the localized dialog-string injection. Keep the shared locale selector and `Title / Text / Input / DefaultButton / Icon / Output` form with `Show Dialog`.

Edit the title/message and verify both in the hosted modal. Exercise all seven button sets, all nine possible returned outputs, all five icon choices, and every applicable default-button position. Keyboard activation of the default must return the corresponding Select output and update only to the actual choice. Repeat representative cases in en-US/zh-CN and inspect localized buttons. Close every dialog, verify it disappears, and require focus to return to the page with correct output.

## Misc / Dialogs / ColorDialog

`LocalizedDialogsTabPage.xml`. Keep the color preview above `Show Dialog`, together with the shared locale selector.

Open with the current preview color, select/edit another color, accept, and require the swatch and numeric readout to agree. Reopen and require that accepted color as the initial value. Change then cancel and require the previous page color to remain. Exercise custom colors, repeat under zh-CN, and verify the dialog and its controls remain usable after resizing.

## Misc / Dialogs / FontDialog

`LocalizedDialogsTabPage.xml`. Preserve `Sample Text`, the `Effect` checkbox, and `Show Dialog` in their original order with the shared locale selector.

Open with Effect off, then on, and require the appropriate dialog controls. With effects enabled, exercise bold, italic, underline, strikeline, and color; accept and verify the sample in Windows Terminal. Reopen to check retained settings, change/cancel, and require the prior sample style/color. Repeat in both locales and confirm font controls cannot change terminal cell metrics.

## Misc / Dialogs / Open/Save FileDialog

`LocalizedDialogsTabPage.xml`. Preserve the six fields `Title / Directory / FileName / Filter / FilterIndex / DefaultExtension`, all nine option checkboxes, `Selected Files`, and both launch buttons. Retain fields left, options right, and results below at comfortable widths.

Prepare a known directory with several file types. Set all six fields and verify initial title/location/name/filter selection; navigate folders, wait for asynchronous rows, change filters, and select one or multiple files as allowed. Require exact Selected Files results. Exercise empty/invalid selection, missing file/directory, create confirmation, save default-extension behavior, and overwrite confirmation, accepting and canceling nested prompts. Canceling the outer dialog preserves prior page results. Repeat representative validation in Chinese and resize while the dialog is open. These pickers select paths; do not infer that they read/write file contents.

## Misc / DatePicker

`DatePickerTabPage.xml`. Preserve two calendars side by side, their two date combo boxes below, and the recognizable `DP1 / DP2 / DC1 / DC2` readouts. Keep four independent selected-date values.

Change month/year and select distinct dates in both calendars; only the corresponding DP readout changes. Choose separate dates through each combo's popup; its text/DC readout changes without altering the other three values. Test leap-year February and month/year boundaries, date-grid alignment, keyboard/mouse selection, popup dismissal, resizing, and persistence after visiting another page.

## Window Manager

`Resource.xml`, `WindowManagerContentStyles`, `SubWindowResource`, and the main Window Manager tab. Preserve the vertical frame-option checklist, `Open New Window` button, three shortcut labels, mouse-result label, and separate `Alt: 0; Super: 0` readout. Subwindows repeat the same checklist, matching the original demonstration.

Adapt FCT `Verify Shortcuts and Mouse Buttons`. Open a child and then a grandchild; move, resize, activate, overlap, and close them, requiring correct ownership, focus, clipping, and exposed-background repaint. Toggle each enabled frame option and verify the corresponding child's appearance; disabled main-window options remain unavailable and do not change the terminal frame. Require the Windows labels `Ctrl+Q`, `Ctrl+Alt+Win+Q`, and `{Ctrl+Shift+Alt+Win+F8}`. Press `Ctrl+Q` and require `You pressed Ctrl+Q!` in a dismissible TUI dialog. In a focused Windows Terminal, press `Ctrl+Alt+Win+Q` with left and right Win and different modifier release orders; require exactly `You pressed Ctrl+Alt+Win+Q!`, then dismiss it and verify ordinary input. VlppOS decodes Windows Terminal's right/left Win extension bits (`0x0200`/`0x0400`) from the actual console record. A host omitting these bits remains unverified for Win; failed native input and direct payload injection do not establish delivery. Test the global chord through real OS registration and require `You pressed Ctrl+Shift+Alt+Win+F8!`, separately from terminal key payloads, including concurrent CppTest startup with its distinct global Q chord. For every delivered mouse button require exact `<button> button down!`/`<button> button up!`, and verify movement, double-click, both wheel axes, and modifier readouts. The Alt/Super readout describes mouse events, not keyboard delivery. Mark unobservable terminal inputs explicitly instead of silently passing the native renderer's five-button/Super matrix. Finish with another successful ordinary interaction to prove the page remains live.

## Live palette and clipping regression

The Window Manager page has equal left/right columns, with its existing controls on the left and Color Theme on the right. Start with only SkyBlue (default) selected. Select Pink, Orange, Grass, Emerald, SkyBlue and Purple, then return to SkyBlue. Repeat using mouse and keyboard at 120x40, 80x25 and after restoring 120x40. Exactly one radio remains selected; the original SkyBlue appearance returns. Inspect normal, disabled, focused, hovered, pressed, highlighted and selected states for readable text.

Before switching, open children and a grandchild, edit two independent document fields, select text, retain undo/redo history, select a list row, expand a tree, resize/sort a grid column and scroll editors/lists away from the origin. Revisit hidden pages and recycle rows after switching. Compare existing and newly created controls, including column hover colors and filter/date dropdowns. Repeatedly refresh menus and combos, then activate each once and require one action. Retained neutral content-popup frames and explicit borderless grid editor templates keep their intended appearance. Arbitrary captured element colors and caller-supplied templates are not automatically reactive.

Queue a refresh while a popup or fake dialog is open when the main radios cannot be reached. Verify that the popup remains usable, date-picker replacement does not dereference destroyed month/year popups, keyboard focus still receives text, and edits/selection/history/scroll positions survive after layout settles. Use undo and redo after another switch, then close child windows and exit normally.

Overlap, move, resize and close child/grandchild windows at both viewport sizes. Exposed cells repaint, a clipped rectangle never acquires new clip-edge corners, and CJK/supplementary text leaves no half-glyphs. Record console-buffer replay, physical input and displayed-color observations separately, including any unavailable desktop access. Finish by checking normal console restoration and usable shell input.

## Exit

The `Exit` tab in `Resource.xml`. Preserve the four vertically arranged, exactly named `self.Hide()` / `self.Close()` buttons, with and without `(InvokeInMainThread)`, so the page is immediately recognizable.

Adapt FCT `Close the Application`. With `Cancel Hide/Close requests` selected, click each original button and require the app to remain responsive, its invocation/query readouts to advance once and its ready count to remain zero. With cancellation cleared, test each button on a fresh run: one WindowClosing and one WindowReadyToClose must precede normal loop exit. Queued operations must reach the same path. Include canceled-then-accepted retry, reentrant close and listener removal in provider regressions. On separate fresh runs, use each Stop button and require normal teardown and restored terminal contents, cursor, colors, input mode, and usable shell input. Repeat after opening/closing hosted dialogs and changing pages. Do not substitute terminal-tab close, force termination, or a remote renderer's Force Exit for normal application stopping.

## Linux/macOS terminal comparison record card

Append a completed card here for each legacy-terminal failure that prompts a Kitty retry, and retain the corresponding evidence in the platform's `TestMatrix_Tui.md`. If only Kitty works, the card must say so explicitly and preserve the legacy failure details. Identify physical keyboard/mouse input, generated native events, terminal test-key commands and raw byte replay separately; record the input method for each result.

| Field | Required evidence |
| --- | --- |
| Host / build / configuration | Date, Linux/macOS version, application commit/build, viewport and relevant adapter options. |
| Legacy terminal attempted first | Terminal name/version, profile, Option/Meta settings and relevant shortcut bindings. |
| Operation and expected result | Page/control, exact chord or action, modifier press/release order and expected response. |
| Legacy result | Exact observed behavior, delivered bytes/modifiers if captured, alert or interception, and input method. |
| Kitty retry after failure | Kitty version/settings, the same operation/build/configuration, input method and exact observed result. |
| Conclusion | Works in both, fails in both, only Kitty works, or verification unavailable; scope the conclusion to the checks actually performed. |
| Evidence / cleanup | Capture/log locations, remaining physical-input limits and process/terminal cleanup. |

### macOS Alt/Command retry, 2026-09-09

| Field | Observed result |
| --- | --- |
| Host / build / configuration | macOS 26.5.2 arm64; iGac `3f7caf6`, with `overrideAlt=true` and `overrideOsSuper=true`; 120x40. `./build.sh` and 2,880 controlled Cocoa input callbacks pass. |
| Legacy terminal attempted first | Apple Terminal.app 2.15, Basic profile. Its profile has no explicit Option-as-Meta setting. Targeted native Quartz key events are delivered through Terminal's normal input handling. |
| Operation and expected result | Ctrl+Q should open `You pressed Ctrl+Q!`; Ctrl+Alt+Command+Q should open the shared `You pressed Ctrl+Alt+Win+Q!` dialog. Generated key-down/up events carry the chord's flags; this does not exercise physical modifier holds or release orders. |
| Legacy result | Raw input: Q=`71`, Ctrl+Q=`11`, Ctrl+Alt+Q=`11`, Ctrl+Alt+Command+Q/A=no bytes. In the rebuilt showcase, Ctrl+Q succeeds before and after the failed Command chord. Alt is lost in this profile; the Command chord produces no event for either override to supplement. |
| Kitty retry after failure | Kitty 0.48.2 with built-in configuration, socket control and the same viewport/build/options. The native Ctrl+Q attempt is unavailable: the locked desktop leaves Kitty without a key window, reported explicitly in its debug log. Kitty `send-key` generation separately passes Ctrl+Q and Ctrl+Alt+Super+Q; Ctrl+Alt+Q does not match Ctrl+Q. Both success dialogs dismiss and later Ctrl+Q still works. |
| Conclusion | The Command shortcut works through Kitty-generated protocol input after failing through Terminal.app's generated native input. A claim that only Kitty works for physical/native input remains unverified because the Kitty native retry is blocked by desktop state. Controlled callback tests verify Alt supplementation and option independence; they do not establish physical held-state timing. |
| Evidence / cleanup | See [iGac's verification matrix](../../../iGac/TestMatrix_Tui.md#independent-altcommand-overrides-2026-09-09) and `/tmp/tui-alt-*` captures/logs. The raw probe and legacy showcase are stopped and the separate legacy window is closed. Pseudo-terminal and Kitty replay Stop return 0 and restore the alternate screen and shell input. Kitty's transient Darwin `PENDIN` bit clears on the first shell input, after which stty matches exactly. The Kitty shell/window and owned idle process are closed. |

## Verification record (historical, before follow-up 3)

Record the build, terminal version, viewport, page/function, exact observed result and any unobservable input. A successful build or deterministic buffer test does not establish a visual result.

| Check | Result |
| --- | --- |
| Debug Win32/x64 provider, TuiSkin, TUI dialogs, showcase and deterministic tests compile | Final builds passed, 0 warnings / 0 errors. Metadata_Generate passed in both architectures; Metadata_Test passed in x64 |
| Full showcase generation and launch | Both architecture resources generated successfully. The final unchanged-input repeat preserved all 72 TUI output hashes across Debug x64 and Release Win32 compiler hosts. The final image-free Debug x64 showcase launched in Windows Terminal |
| Release Win32/x64 build and tests | Both final solution builds passed with 0 warnings / 0 errors. Both metadata-generation/validation stages passed. Win32's pipeline suite passed; the full x64 diagnostic replay passed 90/90 files and 1,737/1,737 cases after a failed pipeline attempt. The investigation records the failed attempts and retries |
| Existing UnitTestSnapshots unchanged | Passed: all snapshot contents match HEAD and Git status is clean. Seven stale status entries were confirmed by matching blob hashes; a stale empty index lock was removed and the index refreshed. No snapshot content was edited or restored |
| Final publication and consumer | Release-GacUI completed GacGen, both architecture skins and both CodePack passes. A separate Debug x64 consumer rebuilt from the published GacUI/Windows/TuiSkin sources with 0 warnings / 0 errors, found its expected label in the live TUI buffer, stopped normally and returned 0 with a failing-on-leaks heap check |
| TextList and buttons | Both lists Add 10 and remove odd positions produced the expected items; check/radio selection and exclusivity worked |
| ListView | Detail rows render at one cell; scrolling changes only the targeted list; requesting BigIcon reports effective Detail and retains the data. The final image-free build retains Task/005/16x16 and corresponding records in both panes; rotating the bound source clears/restores its rows |
| TreeView | The final image-free build expands Blue+ to all six text children, including the corrected circumflex. Left and right trees retain independent expansion |
| Refresh text/list/tree pages | Checkbox readback, title/name switching, subcolumn changes, all five DataColumns states and headers observed; expanded and collapsed tree edits retained their data |
| Repeat pages | All four populate/remove correctly; flow/shared-size variants also clear/reset and retain items on counter reset. Fresh RepeatStack run confirms Items counts 10, 5 and 0 after mutations |
| Responsive page | All three groups visible; View cycles through all four levels; Stack and Group level changes update independently |
| Calendars | DP1, DP2, DC1 and DC2 selected independently; DP1 month/year changed to February 2024 and February 29 selected; dropdowns dismiss and underlying dates remain |
| Viewport | Resized 120x40 to 80x25, 60x20 and back; top-left clipping and restored content preserve selected dates |
| Hosted windows | Opened children/grandchild, dragged title, resized corner, overlapped, toggled SizeBox double/thick border and TitleBar, then closed all windows with exposed background restored; local Ctrl+Q dialog accepted |
| Document editing | Unicode marker including CJK and emoji survives native clipboard copy/paste and undo/redo; clipboard UTF-16 pair checked directly |
| Toolstrip document operations | All four style commands check their menu states; undo clears the last style. Preview and Selectable reject typing, and Selectable copies the exact Unicode text. Center alignment moves the paragraph. Preview hyperlink activation displays the exact URL. Image insertion produces the path/dimensions placeholder; deleting it and undoing restores it as one object, and insertion undo/redo restores the replaced text/object |
| ALT menu navigation | Alt exposes access keys; F opens File and S opens its Save as submenu with all three original format commands |
| BindableDataGrid | All five column editors save values. Birthday filtering gives 3 / 4 / 2 / 5 rows for From / To / both / neither. Birthday sorting orders dates, preserving equal-date rows. Source rotation empties/restores the edited records. All six standard view choices report Detail; DataGrid restores the cell editors. The final category popup displays Black and White without clipping |
| Four TextBox leaves | Replaced the independent top/viewer/label fields on all four leaves. Tab inserts on the tab-accepting TextBox page and moves focus on No Tab. Forty-line clipboard paste, Ctrl+Home/End, undo and redo preserve content. A fresh first paste now immediately shows lines 19 through 40, including the caret's final line |
| Spacing ASCII | The upstream width correction renders the circumflex in all forty pasted lines; the VlppOS Release Win32/x64 pipeline passes the printable-ASCII width coverage |
| Embedded controls | All four rows and wrapped descriptions render; command combo selects RemoteProtocolTest and RIGHT NOW opens the expected dismissible message |
| TextBox font requests | Larger changes requested size 1 to 6; Smaller returns to 1; rendered row height remains one cell |
| Localization and initial dialogs | Chinese page contains all twelve values and literal-dollar sentences; localized message default Enter returns SelectOK; color cancel and simple font acceptance work |
| Message button sets | All seven button sets return the expected first default result; Cancel/TryAgain/Continue also returns SelectTryAgain and SelectContinue for the second and third defaults |
| Color dialog | Typed RGB 12,34,56; accepted result updates the page to #0C2238 and RGB: 12,34,56. Reopening retains it; changing red to 200 and canceling preserves the previous page result |
| Full font dialog | All four effects toggle, accept and persist when reopened; canceling a changed checkbox preserves prior settings. Nested color picker accepts a new component. The rebuilt dialog now displays the exact ABCxyz 你好 preview; visible terminal font styling remains unverified |
| File dialogs | Asynchronous folder enumeration and text filtering show Child, alpha.txt and gamma.txt. Chinese empty-selection prompt dismisses. Single/multiple selection return exact paths. Save newfile returns newfile.txt without creating it. The overwrite prompt keeps its Chinese question and alpha.txt on separate lines. Canceling overwrite and then the picker preserves the prior result |
| Main Hide/Close operations | All four remain live, including queued calls; completion counter reaches four |
| Both normal Stop paths | Separate Debug x64 sessions exit code 0 under CDB, without leak dumps; original 120x30 console buffer restored. Queued-stop shell accepted input afterward |
| Terminal observation limits | Windows Terminal 1.24.11911.0: live console buffers and input were used at 120x30/120x40. PrintWindow returned stale compositor images; final displayed font styles, cursor and RGB fidelity are not established by those captures |

The records above describe performed checks, not a blanket pass of every procedure. Classic ReadConsoleOutputCharacterW exposes supplementary glyph cells as replacement characters in this environment; native clipboard inspection confirms the underlying document text retains the complete surrogate pair.

Inherited fake-service limitation: GuiFakeDialogServiceBase_ColorDialog.cpp does not use customColorOptions or customColors. The TUI dialog retains the same current-color/accept/cancel behavior; a custom-color palette is not implemented by this shared service.

## Follow-up 3 verification (2026-09-09)

This record predates the continuation that corrects non-message actions to right alignment. Its layout results remain historical; fresh continuation results are recorded separately below.

| Check | Fresh result |
| --- | --- |
| Generation/build/metadata | Both resource architectures completed without UI errors. Debug Win32/x64 builds have 0 warnings/errors; both Metadata_Generate runs and x64 Metadata_Test pass |
| Unit coverage | 90/90 files, 1,748/1,748 cases; no leak dump. Existing GUI snapshots unchanged. New provider tests cover first-layout bounds, both close flags, all direct/queued hosted paths, cancellation, notification order, reentrancy, detachment and modal interception |
| Startup/resize | Rebuilt showcase initially fills 100x30 and 80x25 without manual resize. A separate dialog run shrinks 120x40 to 80x25 and restores 120x40; pages, scrolling and hit positions remain usable |
| Message dialogs | Seven button sets return expected first defaults in Chinese at 120x40 and English at 80x25. Second/third defaults return TryAgain/Continue. All icon choices preserve CJK text. Message content has no inner border; action groups are naturally centered with one gap. Chinese overwrite question and filename retain separate lines |
| Color dialogs | RGB label/content alignment observed; CDB measures all three actual trackers as 38x1. Arrow, Home/End and drag changes reach/clamp endpoints. Typed 12,34,56 returns #0C2238, reopening retains it, and later cancel preserves it |
| Font/file dialogs and forms | English/Chinese font previews, compact effects and Pick a Color are retained. Nested color closes back to full font. File labels are centered; results are adjacent and one blank row precedes actions. At 80x25 the form scrolls to every field/action, and the picker retains useful lists. Existing-path selection, empty-selection and Chinese overwrite prompts work; cancel preserves results |
| Ordinary close and Stop | Four vetoed direct/queued Hide/Close calls increment invocation/query counts once each, with ready zero. Fresh accepted direct Hide, queued Hide, direct Close, queued Close and both Stop paths all exit 0. Closing after dialog use also exits 0 |
| Console restoration | Every accepted exit restores shell input/output modes 484/7, attributes 7 and cursor size/visibility 25/true; the restored shell accepts a command and reports exit 0 |
| Observation limits | Live console buffers, console event replay and CDB were used. OpenInputDesktop fails with error 5; physical input and displayed font/cursor/RGB fidelity remain unverified. These results do not relabel the preceding historical runs |

## Packaged tutorial checks (follow-up 3)

The Release tutorial lives in `../Release/Tutorial/GacUI_ControlTemplate/TuiSkin`. Run its interactive wrapper from that project directory so `../UIRes/TuiSkin.bin` resolves correctly:

```powershell
& C:\Code\VczhLibraries\Release\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable TuiSkin -Configuration Debug -Platform x64 -Interactive
```

Use the actual checkout's absolute wrapper path. The wrapper must support `-Interactive` and walk parent directories to find `GacUI_ControlTemplate.sln`; the owning templates are in `../Tools/Copilot/Scripts`. Keep console handles inherited and application output unredirected. Repeat the startup, resize, document/dialog and shutdown checks above against the packaged executable and binary resource.

| Check | Packaged result, 2026-09-09 |
| --- | --- |
| Aggregate release | Build.ps1 -Project Release completed successfully: 88 resources in both architectures, Workflow generation/rebuild, all six GacUI tutorial solutions in Win32 Debug/Release and the executable inventory |
| ControlTemplate builds | Debug/Release x64 solution builds passed with zero warnings/errors. Individual Debug x64 builds passed in TuiSkin → BlackSkin → TuiSkin → WindowSkin order; their PE subsystems and CRT entry points retain the expected Console/GUI selection |
| Repeat packaging | UpdateRelease and tutorial generation completed again for both architectures. All five generated TUI C++ hashes are unchanged; the four skin files match upstream. Project/configuration mappings and separate binary-resource settings survive; no compiler error files |
| Startup and packaged resources | Debug x64 starts at 100x30; Debug Win32 and Release x64 start at 80x25. Dialog checks resize to 120x40, shrink to 80x25 and restore the larger viewport. Embedded document text loads from TuiSkin.bin |
| Startup selection | CDB resolves wmain and DefaultTuiSkinPlugin::Load in TuiSkin, with no WinMain/DefaultSkinPlugin::Load. BlackSkin and WindowSkin resolve the GUI entry/plugin, create their expected native windows and exit 0 through WM_CLOSE |
| Localized dialogs | Chinese three-action message preserves CJK text and returns default SelectCancel; actions are centered with one gap. RGB 12/34/56 accepts as #0C2238 and survives reopen/cancel. English simple/full font previews and nested color picker work. File enumeration and the borderless validation prompt return to their owner |
| Closing and restoration | Four vetoed Hide/Close requests produce invocation/query counts 4/4 and ready 0. Fresh direct/queued Hide/Close and both Stop paths all exit 0. Direct Close after dialogs exits 0 under CDB without a leak dump. Settled shell modes 484/7, attributes 7 and cursor 25/visible are restored; the shell accepts commands |
| Final regenerated package | Win32 Release starts at 100x30, loads embedded document text/tables and shows a borderless Chinese message with centered action and one gap. Enter returns SelectOK. A vetoed Close stays live, then the accepted retry returns 0 through the wrapper and restores the usable shell |
| Observation limits | Console event replay, stable live-buffer inspection and CDB establish these results. Physical input and final displayed font/cursor/RGB fidelity remain unverified |

## Follow-up 3 continuation verification (2026-09-09)

This continuation corrects the preceding non-message action alignment and adds the missing child title border. The preceding records remain historical.

| Check | Continuation result |
| --- | --- |
| Generation/build/metadata | Full GacUI_Compiler completed both architectures and merging with exit 0; no UI error files. Debug Win32/x64 builds passed with zero warnings/errors, both metadata generators passed and x64 metadata validation passed |
| Full unit suite | Finished Debug x64 log reports 90/90 files and 1,748/1,748 cases with no leak dump. Existing GUI snapshots are unchanged |
| Child captions | Debug x64 PID 12120 at 120x40 preserves the thick border after short/CJK captions; empty captions leave the full border and long captions ellipsize before Close. At 80x25 border dragging and clicking the moved Close target work. Window Manager children retain double/thick borders across SizeBox changes and restore their caption after TitleBar is toggled |
| Dialog actions | Chinese at 120x40 and English at 80x25: Color, simple/full Font, Open and Save groups align right, including Pick a Color. Exactly one blank row precedes actions; message groups remain centered |
| Dialog behavior | RGB 12/34/56 accepts as #0C2238 and survives reopen/cancel. Both font previews retain ABCxyz 你好; nested Color returns to full Font. File enumeration, nested empty-selection validation and an exact existing-path result work. Save cancellation returns to its form |
| Resize/normal close | 120x40 → 80x25 → 120x40 retains usable forms, scrolling and hit targets. Main Close after all dialogs returns 0 through the wrapper and restores usable shell input, modes 484/7, attributes 7 and visible 25-percent cursor |
| Win32 smoke | A separate Debug Win32 process starts directly at 80x25, preserves the CJK message caption border/centered action and right-aligned Chinese Color actions, then returns 0 through normal Close with the console state restored |
| Release refresh | Release-GacUI, UpdateRelease and both-architecture tutorial generation completed without compiler errors. Packaged GacUI.cpp, all four skin files and both metadata binaries match upstream. Generated showcase sources/resources remain unchanged; Debug x64 ControlTemplate builds with zero warnings/errors |
| Packaged smoke | Debug x64 starts at 80x25, then at 120x40 shows the repaired message border and right-aligned Chinese Color/simple Font/full Font/Open/Save groups. Nested Color and Save caption Close return to their owners. Main Close reports exit 0 and restores the usable shell/console state; the owned terminal then closes normally. Release is pushed as a4ccbc2f |
| Observation limits | Stable console buffers and console-event replay establish these checks. Physical keyboard/mouse and final displayed font/cursor/RGB fidelity remain unverified |

## Follow-up 4 verification (2026-09-09)

Windows Terminal launch was rejected by the computer-use tool's product policy. The checks below use the interactive repository wrapper in a controlled Windows pseudoterminal, native console-event replay, live console buffers and captured VT output. They do not establish physical input delivery or displayed Windows Terminal font/cursor/RGB fidelity.

| Check | Result |
| --- | --- |
| Upstream clipping | VlppOS Debug x64 build has zero warnings/errors. Full wrapper run passes 16/16 files and 306/306 cases without a leak dump. Coverage includes every overload, omitted/null clips, original edges, empty clips, invalid arguments and CJK/supplementary wide-pair repair at buffer edges |
| Initial clipping process | Debug x64 PID 7832 starts at 120x40 and resizes to 80x25. A child and grandchild open; moving the grandchild beyond the right/bottom viewport clips its existing edges without inventing corners. Restoring 120x40 exposes the original frame, resizing its bottom-right corner works, and closing each window repaints the parent. Queued main Close returns normally with exit 0 |
| Regenerated Win32 selector | PID 2332: 120x40 has equal columns, all six radios and SkyBlue alone initially. TAB/Space cycles Pink through Purple; at 80x25 mouse replay cycles all six and returns to SkyBlue. Each settled capture has exactly one selected radio and the expected emitted accent RGB. Ctrl+Q opens one Chinese fake message and Enter dismisses it. Queued Close exits 0; input/output modes 503/7, attributes 7, visible 25-percent cursor and original 120x40 viewport are restored |
| Final focused lifecycle replay | Debug x64 CDB passes 2/2 files and 22/22 cases. Deferred checks retain a (5,10) list offset through all palettes, header identity/width/sort/filter and new hover/pressed colors, tree expansion, content/date dropdowns and an open modal window. GUI-theme checks retain document identity, edits, selection, undo/redo, modified flag, nonzero scroll, focus and TAB/Shift+TAB navigation |
| Final architecture builds | Debug x64 and Win32 wrapper builds after the list lifecycle correction both have zero warnings/errors |
| x64 palette/input replay | PID 24528: all six presets and return to SkyBlue pass both mouse and keyboard replay at 120x40 and 80x25, with one selected radio and matching accent RGB in emitted VT. Two existing child windows retain caption positions across all 12 per-preset captures at 120x40 and close normally |
| x64 retained grid and list | A grid cell edited to Palette edit, descending birthday sort and enabled From filter survive refresh and hidden-page revisit. The nested date-filter calendar is text-identical before/after Purple. At 80x25, End scrolls the left detail list to its last rows while the right list stays at the origin; both viewports are text-identical after Purple then SkyBlue. Home/End still recycles rows afterward |
| x64 retained document and menus | A 45-paragraph edited document remains scrolled to line 30 with Marker on its final visible row after Pink, Emerald and SkyBlue. The entire visible buffer matches. Mouse Undo/Redo, Ctrl+Z/Ctrl+Y and Alt/E1/U then Alt/E1/R each remove/restore exactly one marker character. Menu navigation does not duplicate actions |
| x64 fake dialogs and clipping | Color opens before and after a confirmed Orange change with identical dialog/underlying document text. Shrinking with Color open clips its original right/bottom frame; dragging the caption brings the whole dialog into 80x25, and caption Close repaints the editor. The final Ctrl+Q opens one fake message; Enter dismisses it |
| x64 normal restoration | After returning to SkyBlue, queued main Close exits 0. Input/output modes 503/7, attributes 7, visible 25-percent cursor and original 120x40 viewport exactly match the pre-run state |
| Final document-initialization correction | The first 1,755-case run passed assertions but GUI snapshot review exposed opaque backgrounds in initial plain-text document controls. Keep complete baseline formatting on first template installation and use the preserving font/baseline refresh for later installs. The added initial-transparency assertion and all 22 focused cases pass |
| Post-correction smoke and leak capture | Debug x64 PID 21476 retains newly typed document text through Pink/SkyBlue and applies one Undo via keyboard menu navigation. Debug Win32 PID 14440 starts at 80x25, switches Purple/SkyBlue and opens/dismisses the shortcut message. Both run under attached architecture-matched CDB through normal shutdown, exit 0, emit no leak dump and restore their original console state (120x40 and 80x25 respectively) |
| Final complete suite and snapshots | After the initialization correction, the completed Debug x64 wrapper log passes 90/90 files and 1,755/1,755 cases with no leak dump. Existing GUI snapshot content is unchanged: the only differences are frame IDs/intermediate trace timing in ScrollResetOnNavigation. Exact JSON comparison excluding frameId confirms identical saved content; those seven timing-only files were restored, and new theme regression snapshots retained |

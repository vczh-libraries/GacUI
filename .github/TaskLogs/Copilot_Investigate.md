# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

## TuiSkin

- In DataGrid's DataGrid view, when a row/cell is selected, the background color changing should not cover the last line, as it is for the border.

### DETAILS

- The last line means the bottom terminal-cell row reserved for the horizontal separator in each data row. Keep its normal background and border color while the selected cell uses `ItemBackgroundSelected` and the other cells in the selected row use `ItemBackgroundHighlighted`.
- Inspect both background layers: `tuiskin::TuiItemBackgroundTemplate` in `Test/Resources/App/TuiSkin/Template_List.xml` paints the entire row, and `TuiUpdateGridCellColors` in `Source/Controls/ListControlPackage/TuiItemTemplates.cpp` paints selected visualizers. Restricting only one layer can still leave the separator with a selection background.
- `CellBorderVisualizerTemplate` in `Source/Controls/ListControlPackage/GuiDataGridExtensions.cpp` reserves bottom/right separators outside its content container. Preserve those separators and the editor insets maintained by `DefaultDataGridItemTemplate` in `GuiDataGridControls.cpp`. Keep the existing compact row height; do not add an empty row to hide the color issue.
- Scope the inset to the editable DataGrid view. `TuiInitializeItemBackground` currently marks every `GuiVirtualDataGrid` as `GridRow`, including when it displays Detail view; this flag alone does not establish that a bottom separator row exists. Do not cut away the single content row of Detail, TextList, or TreeView items.
- During implementation, clarify `GacUILayout.md` under the DataGrid color/layout rules: selection backgrounds cover content and preserve the separator row's normal background. The current document specifies separator foreground colors and reserved space but does not state this background rule.

### VERIFICATION

- In `CppTest_Tui`, open List / BindableDataGrid and explicitly select DataGrid view. Inspect the bottom separator across the entire row before selection, with each cell selected in turn, and after moving selection to another row. Require both the glyph and its background to retain their normal colors; the old selection must repaint correctly.
- Repeat while hovering, focusing/unfocusing, editing Name/Gender/Category/Birthday/Website, opening and dismissing dropdowns, resizing columns, and scrolling rows out of view and back. Check bottom/right intersections and clipped rows without restoring the removed blank row.
- Compare at 120x40 and 80x25 terminal cells and after restoring the larger viewport. Switch DataGrid to Detail and back; also inspect ordinary ListView, TextList, and TreeView selection so their full content height and colors remain intact. Compare `CppTest` to confirm the GUI skin is unaffected.
- Extend the relevant deterministic TUI cell-buffer checks in `Test/GacUISrc/UnitTest/TestTuiProvider.cpp`, or the existing grid test fixture where it owns the setup, to assert selected content and unchanged separator backgrounds across selection changes. Test the actual row/cell composition together so the underlying row fill cannot hide a faulty cell-only fix.

## Control Bugs

For every bug, you need to verify if this is a TUI only issue or a general GacUI issue that happens in both `CppTest_Tui` and `CppTest`.

- Known to be TUI specifc bugs:
  - In any text box with configuration to allow typing TAB key, pressing TAB doesn't actually insert or render a tab character.
    - Excluding `TextBox (No Tab)` and `Document (No Tab)` as in those pages text boxes do not accept TAB as typing.
    - A TAB key is defined to be aligning to 4 spaces.
    - There should be a configuration in `Tui` but the default value would be 4. You can add a struct with default value initialized in its constructor as an optional argument put in `SetupTuiWindowsRenderer`.
    - When drawing text, the paragraph (and label is using paragraph) should do the alignment by itself as when the text begins matters. Remember that in case of scrolling, the starting point should still in the real first character in each row.
  - In `TODO_Task_TUI_FollowUp_1.md` the globa shortcut key is changed to `ctrl+shift+alt+win+f8` I guess is due to fixing the concurrent startup of `CppTest`, which is good. But the `ctrl+win+alt+q` is still not working. You might need to simulate the key sequences directly to the CLI window so that you can observe the issue.
- Known to be GUI and TUI bugs:
  - In `TODO_Task_TUI_FollowUp_1.md` the left/right issue on DataGrid is fixed. But I found a new bug, when a combo box dropdown editor is opened, pressing up/down is supposed to select different item in the combo box, but it actually closes the editor and select another row. Meanwhile pressing up/down when the text editor is opened is no-op, this is expected and correct.

### DETAILS

#### TAB input and configurable stops

- Separate insertion from display during reproduction. `TuiGraphicsParagraph` already lays out and paints literal U+0009 characters at hard-coded four-column stops in `Source/PlatformProviders/TUI/TuiTextLayout.cpp`; `TuiEllipsizeText` has a separate four-column calculation there. The existing tab geometry assertion in `Test/GacUISrc/UnitTest/TestTuiProvider.cpp` does not establish that pressing TAB reaches an editor.
- Trace terminal character delivery through `TuiControllerBase::Char` in `Source/PlatformProviders/TUI/TuiController.cpp`, TAB focus/suppression handling in `Source/Application/GraphicsHost/GuiGraphicsHost_Tab.cpp`, and `GuiDocumentCommonInterface::OnCharInput` in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`. KeyDown and Char are separate events; avoid inserting a duplicate tab by synthesizing text unconditionally from KeyDown.
- Insert one literal U+0009 through the existing editing path when `AcceptTabInput` permits it. Tab expansion changes display width, not stored text, caret offsets, clipboard content, or undo semantics. Preserve No Tab focus traversal and the existing read-only/modifier rules.
- Add a portable TUI configuration type with a constructor-initialized tab interval of 4 and an optional argument to `SetupTuiWindowsRenderer`, declared in `Source/GacUI.h` and implemented in `Source/PlatformProviders/Windows/TUI/TuiWindowsController.Windows.cpp`. Keep the no-argument call valid, require a positive interval, and retain the configuration by value for the application lifetime. Startup configuration is sufficient; no runtime setter is required.
- Use the same configured interval for all paragraph layout, label rendering, natural minimum-size measurement, and ellipsis calculations. In particular, `TuiLabelRenderer::OnElementStateChanged` in `Source/PlatformProviders/TUI/TuiGraphicsRenderers.cpp` creates a separate layout provider for natural measurements; changing only the display paragraph would leave labels inconsistent.
- For interval `N`, advance from row-local terminal-cell column `x` by `N - x % N`. A tab at a stop advances a full interval. Count wide characters and inline objects by their layout widths, not string length. Keep the existing wrapping semantics and measure from each full laid-out row's origin before alignment translation, clipping, or scrolling; the first visible character is not a new origin.

#### Local Win-key shortcut

- Preserve all three distinct commands in `Test/Resources/App/TuiControlTest/Resource.xml`: local `Ctrl+Q`, local `Ctrl+Alt+Win+Q`, and global `Ctrl+Shift+Alt+Win+F8`. `Test/Resources/App/FullControlTest/Resource.xml` retains global `Ctrl+Shift+Alt+Win+Q`. Fixing the local chord must not turn it into another global registration.
- The current Windows TUI decoder fills Ctrl/Shift/Alt/Caps Lock but leaves `osSuper` false; `TuiControllerBase` forwards that payload unchanged, and `GuiShortcutKeyItem::CanActivate` in `Source/Application/GraphicsHost/GuiGraphicsHost_ShortcutKey.cpp` compares every modifier. Trace the actual console records and modifier delivery before choosing the fix. The global hotkey path uses OS registration and does not prove that this local input path works.
- If the fix belongs in the decoder, its source is `../VlppOS/Source/TUI/TUI.Windows.cpp`; follow that repository's instructions, test there, regenerate its release, and import the generated result. Do not hand-edit `Import/VlppOS.Windows.cpp`. Preserve independent Alt and OS Super and key-down/key-up ordering.
- The existing TUI knowledge base and `.github/Jobs/DebugTuiControlTestSop.md` describe a Super-delivery limitation. That historical limitation does not resolve this follow-up. Update affected guidance when the actual behavior changes, and distinguish native delivery evidence from injected payload tests. The Window Manager `Alt: ...; Super: ...` readout is updated by mouse events, so it cannot establish keyboard modifier delivery.

#### DataGrid combo-editor navigation

- Reproduce Gender and Category editors in both applications, distinguishing an active in-place editor with a collapsed combo from an expanded dropdown. `GuiComboBoxListControl` in `Source/Controls/ListControlPackage/GuiComboControls.cpp` retains focus on the combo and delegates navigation to its contained list.
- Inspect event consumption at that shared control boundary. `GuiComboBoxListControl::OnKeyDown` currently calls `SelectItemsByKey` without consuming the result, allowing the parent list's handler in `GuiListControls.cpp` to navigate the grid row and close the editor. Handle navigation owned by the combo, including boundary/no-change cases and repeated input, without swallowing unrelated editor keys or adding a TUI-only workaround.
- Preserve the distinction between the dropdown's highlighted item and the committed combo value. Existing Enter/mouse acceptance copies the list selection into the combo and its `CellValue` binding. Arrow navigation must remain in the same grid cell with the editor active; do not introduce a new commit-on-arrow rule or make Escape roll back an already submitted value.
- Preserve the prior mouse-focus and Left/Right grid fixes, text-editor caret movement and Shift selection, and text-editor Up/Down no-op. After normal editor dismissal, grid navigation must work again.

### VERIFICATION

#### Execution and evidence

- These are verification requirements for implementation, not claims that this review ran the applications. Record a fresh before/after result for every bug in both `CppTest_Tui` and `CppTest`, identifying the selected page, editor, focused control, and whether a combo popup is expanded. Use matching interactions to determine TUI-only versus shared scope.
- Follow `.github/Jobs/DebugTuiControlTestSop.md`: launch `CppTest_Tui` in actual Windows Terminal through the absolute `.github/Scripts/copilotExecute.ps1` path with `-Mode CLI -Executable CppTest_Tui -Configuration Debug -Platform x64 -Interactive`. It has no HTTP automation endpoint. Use `.github/Guidelines/Running-GacUI.md` for `CppTest`; native shortcut checks still require input through its real focused window.
- Build `Test/GacUISrc/GacUISrc.sln` using `.github/Scripts/copilotBuild.ps1`. For source changes, run `UnitTest` using `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest` and inspect the completed logs, selected test files, and Debug leak report. Respect existing filters while ensuring affected tests are included.
- If skin/showcase XML changes, run `GacUI_Compiler`, inspect generated changes and any `*.UI.errors.txt`, follow the Debug Win32/x64 metadata-generation and x64 metadata-test sequence in `Project.md`, and rebuild before application checks. Generate protected outputs through their owning tools. If a dependency changes, verify it upstream and then build/test the downstream import.

#### TAB

- On the tab-enabled TextBox and Document pages, test all five controls: SinglelineTextBox, MultilineTextBox, DocumentTextBox, editable DocumentViewer, and editable DocumentLabel. Type TAB between distinguishable characters and verify exactly one U+0009 in the text, unchanged focus, and the next character at the correct stop. Compare physical typing with pasting text containing a tab to isolate input loss from layout errors.
- Check caret movement across a tab, deletion, selection/replacement, clipboard round trips, and undo/redo. Repeat the No Tab counterparts with TAB and Shift+TAB: focus must move and text must remain unchanged. Include the same input checks in `CppTest` without imposing terminal-cell metrics on its GUI renderer.
- Extend `TestTuiProvider.cpp` for intervals 4 and 8. At interval 4, `\tX`, `a\tX`, `abc\tX`, and `abcd\tX` place X at zero-based columns 4, 4, 4, and 8; `\t\tX` places it at 8. Cover wide CJK/supplementary characters, inline objects, CRLF, wrapping, and styled/selected tab spans. Assert caret bounds and hit testing along with rendered cells.
- Check left/center/right alignment, a nonzero paragraph origin, and horizontal scrolling/clipping through a tab span whose preceding text is hidden. The visible suffix must agree with the original row layout. Verify labels' natural minimum sizes, ellipsis decisions, and rendered positions use the same interval, including the nondefault value.

#### Shortcuts

- Foreground the real Windows Terminal Window Manager page. Send Ctrl/Alt/Win down, Q down/up, and modifier releases through successful native keyboard input, checking the input API result and the received events. Use the documented CDB workflow if needed to trace the console record, decoded payload, hosted forwarding, and command activation. A failed input call or direct callback injection is not a successful terminal reproduction.
- Require exactly `You pressed Ctrl+Alt+Win+Q!`, dismiss it, and confirm ordinary input still works. Use local `Ctrl+Q` as a control case; repeat with different modifier press/release orders and check for stuck modifiers. Compare the local commands in `CppTest`.
- Separately verify the real OS global F8/Q chords with both apps running together and after restarting either app. Require their existing exact success dialogs and independent registrations. Posting `WM_HOTKEY` alone only tests message forwarding.
- Add focused modifier-delivery tests at the changed layer, including Super with and without Alt and modifier releases. An upstream decoder fix needs upstream tests; injected provider tests supplement the real terminal check. Record any unobservable native input as unverified, not passed.

#### DataGrid editors

- Extend `ComboEditor` and `TextEditorCaretAndGridFocus` in `Test/GacUISrc/UnitTest/TestControls_List_DataGrid_CellEditor.cpp`; the current combo case uses mouse selection and does not cover this regression. Reuse grid property/navigation tests in `TestControls_List_DataGrid_Properties.cpp` for the prior focus/boundary behavior.
- In both applications, activate Gender and Category editors by supported mouse/keyboard paths and test Up/Down with the popup collapsed and expanded. Assert the contained list's highlighted item moves one step, the grid row/column does not change, and the editor remains active. At first/last items and during held-key repeats, no event may escape into grid navigation. Accept with Enter and mouse in separate runs and reopen the cell to verify the submitted value.
- Repeat Name/Website text editors: Up/Down stays a no-op, Left/Right moves the caret, Shift+Left/Right selects text, and typing changes text at the expected caret without moving cells. Preserve existing Enter handling (the single-line editor remains active) and Escape dismissal, then verify restored grid navigation, including first/last column and row boundaries. Also check an ordinary combo outside the grid and Birthday's date editor for regressions from shared key-handling changes.

## REVIEW COMMENTS

, pay more attention to the ctrl+alt+win+q issue, since I can't activate the handler by this shortcut key in multiple Windows PCs, you should find a way to repro it otherwise you won't be able to observe your fix. It is still suspicious that is alt really possible to be part of a shortcut key in a TUI application, but if you can't repro it first, you won't find out.

# UPDATES

# TEST

Native shortcut reproduction is the first priority. Establish successful native input and trace console records, decoded modifiers, hosted forwarding and command matching before changing shortcut behavior. Synthetic payload tests are supplementary evidence only. Reproduce and verify the grid separator, TAB insertion/configuration and shared combo navigation issues using the task's full acceptance criteria.


## Combo navigation [CONFIRMED]

Baseline Debug x64 solution builds passed with zero warnings/errors. The unfiltered UnitTest run failed in the extended `ComboEditor` case on the first Down while the dropdown was expanded: `dataGrid->GetOpenedEditor() && dataGrid->GetSelectedCell() == GridPos(0, 3)`. The preceding text-editor caret/Up/Down regression passed. In the real TUI showcase, injected console input on List / BindableDataGrid / DataGrid reproduced both collapsed and expanded Gender editors closing and selection moving from the first to the second row. GUI HTTP input reproduced the collapsed Gender editor closing and the focus rectangle moving to row two. These are control-path reproductions, not native shortcut evidence.

## Native shortcut prerequisite [UNVERIFIED]

Fresh Windows Terminal PID 4636 and CppTest_Tui PID 3384 were launched through the prescribed CLI wrapper at 120x40. The computer-use service listed only Codex. The shell process is in session 2, window station WinSta0, thread desktop Default. OpenInputDesktop returned null/error 5; SetForegroundWindow returned false; GetForegroundWindow returned null. Correctly sized x64 INPUT (40 bytes) SendInput Ctrl-down/up both returned zero/error 5. No successful native keyboard event was delivered. An interactive-desktop request is pending; no shortcut fix is claimed or implemented from this failed probe.
# PROPOSALS

- No.1 Consume navigation at the combo-list boundary [CONFIRMED]
- No.2 Configure terminal tab stops consistently [CONFIRMED]
- No.3 Preserve DataGrid separator backgrounds [CONFIRMED]

## No.1 Consume navigation at the combo-list boundary [CONFIRMED]

`GuiComboBoxListControl::OnKeyDown` delegates arrows/Home/End/PageUp/PageDown to its contained list but discards its result and ignores repeated key-down events. Own these navigation keys at the combo boundary, including no-change/boundary cases, and process held-key repeats. Keep Enter acceptance separate, retain the committed selected index while only moving the list highlight, and let unrelated keys continue to the parent. Preserve OS Super handling and existing text-editor behavior.

### CODE CHANGE

Extend the existing ComboEditor regression for expanded/collapsed arrow navigation, first/last boundaries, held-key repeats, Enter and mouse acceptance, and value persistence after reopening. Extend TextEditorCaretAndGridFocus for Up/Down no-op. Replace the combo handler's outer repeat gate with explicit navigation-key handling; retain its nonrepeat Enter behavior.

### CONFIRMED

The baseline expanded Down assertion failed. The final full x64 suite passes ComboEditor, its related property/navigation cases and TextEditorCaretAndGridFocus. This covers repeats, boundaries, highlight versus commit, Enter/mouse acceptance and reopening. In both real showcase applications, synthetic input confirms collapsed/expanded Gender and Category navigation retains the same first-row editor; Category changes from Lime to Blue only on Enter. The shared boundary fix is required independently of the TUI rendering changes.

## No.2 Configure terminal tab stops consistently [CONFIRMED]

Add portable `TuiConfiguration` with constructor default `tabInterval = 4`, passed as an optional startup argument and copied into the controller and layout provider. Reject nonpositive values. Use that same interval in paragraph layout, label natural measurements and ellipsis. Keep row-local measurement before alignment/clipping and preserve existing wrapping. Test intervals 4/8, tabs at stops, wide text, objects, caret/hit testing and label rendering. The synthetic console TAB character already inserts and renders on the tab-enabled single-line page; native insertion remains unverified, so do not synthesize another Char from KeyDown.

### CODE CHANGE

Update TuiApplication, startup/controller/resource/layout ownership and label measurement; extend TestTuiProvider. Keep configuration immutable during a renderer lifetime.

### CONFIRMED

The final suite passes interval 4/8 geometry, wide scalars, inline objects, CRLF/wrapping, alignment, nonzero origin, clipping, styled tab backgrounds, caret navigation/hit testing, label minimum size/ellipsis and invalid-interval checks. In CppTest_Tui, console KeyDown/Char/KeyUp records retain focus and clipboard-copy exactly one U+0009 in all five editors. Singleline copied code points are [65,114,99,104,101,114,65,9,66]; each document control and MultilineTextBox copied A\tB. Deleting the singleline tab produced ArcherAB and Undo restored ArcherA\tB. DocumentLabel paste/copy round-tripped P\tQ. No Tab pages traversed forward/backward and copied ArcherAC with no tab. This confirms configuration and the existing supplied-character path; physical TAB insertion remains unverified and no input synthesis was added.



## Additional shortcut evidence

The local Windows Terminal settings contain no custom Ctrl+Alt+Win+Q binding (only Ctrl+C, Ctrl+V, Ctrl+Shift+F and Alt+Shift+D overrides). Microsoft documents independent left/right Alt flags in KEY_EVENT_RECORD and special handling for a lone Alt press/release, so Alt chords are representable; the documented control-state flags have no Win bit. Reference: https://learn.microsoft.com/en-us/windows/console/key-event-record-str . This is API/source evidence only. Successful native input, actual Win delivery and shortcut activation remain unverified; the application decoder has not been changed.

## Separator reproduction [CONFIRMED]

After loading the real application resources, the composed-row regression fails at its separator-background assertion. CDB first-chance trace identifies TestTuiProvider.cpp line 248: selected=0, row=0, x=0. The pixel is Mergeable with foreground #808080 and background #87CEFA instead of #000000. The unselected iteration passes. The test renders the generated TuiItemBackgroundTemplate and real CellBorderVisualizerTemplate together.

## No.3 Preserve DataGrid separator backgrounds [CONFIRMED]

Give the generated row template a normal base fill and a separate selection/highlight fill inset by one cell at the bottom only when GridRow is true. Classify GridRow from DefaultDataGridItemTemplate, so Detail items in a GuiVirtualDataGrid retain their full height. Paint CellBorderVisualizerTemplate selection on its inset content container, leaving its separator compositions outside the fill. Preserve row heights and existing editor insets.

### CODE CHANGE

Update Template_List.xml and TuiItemTemplates.cpp; regenerate through GacUI_Compiler and run the required metadata sequence. Clarify GacUILayout.md. Confirm both background layers through selection changes in the composed-row test. Add the generated TuiSkin inventory to the Unix UnitTest vmake (excluding x86 sources), matching the existing Windows test library dependency required by this portable composed-row fixture.

### CONFIRMED

The final composed-row test passes exact RGB/glyph checks through hover, every selected cell, row changes and deselection, alongside a selected ordinary row at full content height. Real Windows Terminal checks retain normal separator background attributes while editing Name/Gender/Category/Birthday/Website. Detail view has one-cell rows with full selection background. Ordinary TextList, ListView Detail and expanded TreeView also retain full one-cell selected backgrounds (legacy background nibble B across their content). Dragging the Name header boundary from column 21 to 25 moves the cell separator and following intersections four cells; all 114 bottom-row cells retain background nibble 0 while editing Name. Birthday's calendar opens and dismisses after Up/Down without changing its displayed date. These interactions use console input records, not native physical input. Native SetWindowPos resized the same terminal process from 120x40 to 80x25 and back to 120x40; the clipped right edge and bottom separator background remained correct. An additional 80x11 clipping observation was made, but it does not establish scrolling/recycling. Unix build inventory was updated; a Unix build was not run on this Windows host.

## TUI startup diagnostic

The first post-build TUI launch exited with 0xC0000005. CDB in its real Windows Terminal places the null call in CRT __scrt_initialize_type_info/pre_c_initialization, before wmain and SetupTuiWindowsRenderer. Removing only the generated CppTest_Tui x64 .ilk and executable forced a full relink in the next prescribed solution build. The relinked application started successfully and completed the TUI checks.

## GUI application checks after combo change

CppTest Debug x64 on List / BindableDataGrid / DataGrid: collapsed and expanded Gender and Category editors retain their first-row editor bounds after Up/Down (Gender 146,188..226,217; Category 226,188..306,217), unlike the baseline row-two focus transition. Expanded popup remains open during navigation. Category remained Lime while highlighting Blue, and changed to Blue only after Enter; popup then closed with the editor retained.

GUI Control / TextBox: the supplied Char path accepts literal tabs in SinglelineTextBox, MultilineTextBox, DocumentTextBox, editable DocumentViewer and editable DocumentLabel. Plain text serializes U+0009; rich document runs serialize <tab/> between A and B. HTTP KeyPress emits KeyDown/KeyUp only, so KeyPress:TAB without Type:\t is not a native insertion reproduction. On Document (No Tab), KeyPress:TAB followed by the tab character suppresses insertion and moves typing from DocumentTextBox to DocumentViewer. Native physical typing remains unverified.

## Resource generation

GacUI_Compiler Debug x64 completed with exit code 0. Both x86/x64 TuiSkin outputs contain the additional inset row background; no *.UI.errors.txt or deleted generated outputs remain. Debug Win32 and x64 solution builds both passed with zero warnings/errors. Metadata_Generate passed in Win32 and x64, followed by Metadata_Test x64 (exit 0). The unfiltered Debug x64 UnitTest run passed 90/90 files and 1745/1745 cases; both affected test files were selected, none were skipped, and no memory-leak dump followed the summary.

## Outstanding acceptance [UNVERIFIED]

This request is not fully resolved. A final native-input prerequisite check still returns OpenInputDesktop=null/error 5 and GetForegroundWindow=null, despite successful window resizing. Ctrl+Alt+Win+Q has not been reproduced with successful native input, so no keyboard decoder or shortcut fix has been made. Local/global shortcut activation, modifier orders/releases, concurrent global registrations and physical TAB-versus-paste comparisons remain unverified. The interactive-desktop request is still unanswered. Alt is documented in console keyboard records, but native Win delivery must be observed before choosing an upstream fix. Exhaustive scrolling/recycling checks were not completed; the verified viewport-clipping observations must not be read as those checks passing.

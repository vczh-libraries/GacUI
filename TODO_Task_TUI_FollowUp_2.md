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

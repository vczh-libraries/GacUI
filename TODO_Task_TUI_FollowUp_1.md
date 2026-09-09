`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

## TuiSkin

- Normal popup template doesn't need to have a border.
  - But menu popup should have that border.
  - GacUILayout.md requests this but this does not look good, fix this document.
- The first pixel of either vertical or horizontal scroll's dragging handler has a different color than the rest, they should have consistent color.
  - When the scroll bar is disabled, the first pixel is visible, which is confusing. These two issues might be connected.
- Combo box need to only take one line, make it a button style with dropdown arrow, instead to have a border making it 3 pixels height.
- In ListView's header, the sorting triangle should be put before the column text using the text color.
  - Currently it is put at the very last, which is confusing when the column also has a dropdown triangle.
- `ALT` label should use white background and black text so that when it it put on top of a control it looks obvious instead of missing the shortcut key sequenceand the control text together.
- When a button or similar looking controls is focused, changing the text to bold doesn't seem obvious enough, change that to use bold+underline instead.

## TuiControlTest

- TextList/ListView/TreeView/BindableDataGrid
  - Extra lines around top/bottom of the list is unnecessary, as list boxes have a border, they would be clear.
  - TextBox/TextBox, Misc/Localization has a similar issue, remove all unnecessary empty lines between controls.
- Empty line between data cell bottom border and content, should remove the gap.
  - Editor specific theme name mighy need to add so that the text box editor won't have border.

## TUI Platform Provider

- `TuiController` accepting an `INativeController` is not a good design:
  - e.g. in Windows implementation, it should make a new class inheriting from `TuiController`, and fill Windows specific services and other stuff.
  - `INativeInputService` and `INativeResourceService` does not need to be its base class, the Windows implementation could just make two service classesinheriting from original Windows ones.
- Verify how clipper applies when drawing elements.
- `TuiLabelRenderer` should not use paragraph, it is too heavy.
  - Cache the paragraph instead of creating one on rendering.
- `TuiElementRenderer` is better to split into 3 different classes instead of having 3 `if constexpr` branches.

## Control Bugs

For every bug, you need to verify if this is a TUI only issue or a general GacUI issue that happens in both `CppTest_Tui` and `CppTest`.

- Starting `CppTest_Tui` and `CppTest` together always crash the later one. `CppTest_Tui` does not start the automation service so there might be a different reason.
  - Starting other GUI test apps together is expected to fail when the http port is taken by the started automation service though.
- ListView/DataGrid's non-last column can't drag to resize. The first pixel of the column is reserved for dragging to resize the previous column.
- DataGrid:
  - When a cell is selected, pressing [LEFT]/[RIGHT] can't move the selected cell. Need to verify if this is a TUI only issue or a general GacUI issue.
  - When the text editor is opened, pressing [LEFT]/[RIGHT] select another cell instead of moving the caret.
  - Data grid combo box editor does not render the select value, even though the cell value is still updated. This is probably a TUI only issue.
- Shortcut key `ctrl+win+alt+q` not working, this is a TUI only issue, make sure it is the limitation of CLI preventing you from using `ALT`:
  - If so change it to `ctrl+win+shift+q`.
  - Otherwise this would be a bug.

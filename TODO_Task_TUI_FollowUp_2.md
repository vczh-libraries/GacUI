`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

## TuiSkin

- In DataGrid's DataGrid view, when a row/cell is selected, the background color changing should not cover the last line, as it is for the border.

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

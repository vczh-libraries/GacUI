When running on Windows:
  - The target folder will be `GacUI/Screenshots`.
  - `FullControlTest` will be the `CppTest` test project.
  - `TuiControlTest` will be the `CppTest_Tui` test project.
When running on Linux:
  - The target folder will be `wGac/Screenshots`.
  - `FullControlTest` will can be started with `wGac/test.sh --app:fct`.
  - `TuiControlTest` will can be started with `wGac/test.sh --app:tui`.
When running on macOS:
  - The target folder will be `iGac/Screenshots`.
  - `FullControlTest` will can be started with `iGac/test.sh --app:fct`.
  - `TuiControlTest` will can be started with `iGac/test.sh --app:tui`.

You are going to override screenshot png files in the target folder.

## FullControlTest

The following steps need to be done to prepare the UI for screenshot capturing:
- Turn off customized frame when not running on Windows:
  - Open `Window Manager` tab and uncheck `Customized Frame`.
- Open `Control/Document Editor (Ribbon)` page.
- In the text box, type two paragraph `This is a header` and `Some randome text`. `[ENTER]` would create a new paragraph.
- Select the first line, click `Style`, choose `Header 1`.

Now the UI is ready for screenshot capturing. To change the theme, follow these steps:
- Remember the current window size.
- Go to `Window Manager` tab.
- In `Color Theme` choose a theme.
- Go back to `Control/Document Editor (Ribbon)` page.
- Resize the window to the previously remembered size.

The screenshot should contain the whole window with its window frame.

For each theme you should capture the whole window and save the screenshot to the target folder:
- The file name is `FCT_<theme>.png`, where `<theme>` is the text of the chosen theme.

## TuiControlTest

The following steps need to be done to prepare the UI for screenshot capturing:
- Open `List/BindableDataGrid`.
- There will be a datagrid, select the third row third cell, the text is `White`.
- Click the cell to open the editor, which is a combo box, and click it again to open the dropdown.

Now the UI is ready for screenshot capturing. To change the theme, follow these steps:
- Go to `Window Manager` tab.
- In `Color Theme` choose a theme.
- Go back to `List/BindableDataGrid` page.
- You need to click the cell twice to make the dropdown appear again.

The screenshot should contain the whole CLI window with its native OS window frame.
Sometimes you might need another number of clicks to show the dropdown, the screenshot should be captured when the dropdown is showing.

For each theme you should capture the whole window and save the screenshot to the target folder:
- The file name is `TUI_<theme>.png`, where `<theme>` is the text of the chosen theme.

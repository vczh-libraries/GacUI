This job can't be done when the screen is locked.

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

## Tools for Windows

Build `Test/GacUISrc/GacUISrc.sln` with `copilotBuild.ps1` before launching either showcase. Use the wrappers described in the running guidelines.

### FullControlTest automation

- Launch `CppTest` with `copilotExecute.ps1 -Mode CLI -Executable CppTest -Configuration Debug -Platform x64` from `Test/GacUISrc`.
- Read `http://localhost:8888/Automation/CppTest/Controls` to locate visible labels and their bounds. Send commands to `/IO` without a window id, using exactly `Content-Type: application/json; charset=utf8`.
- Navigate and click through `!LeftClick:X,Y`. After focusing the document editor, use `!Type:This is a header`, `!KeyPress:Enter`, and `!Type:Some randome text`. Select the first paragraph with `!KeyPress:Ctrl+Home` followed by `!KeyPress:Shift+End`, then choose `Style` / `Header 1`. Clear the selection with `!KeyPress:Ctrl+End`.
- Verify the document through `elementDocument` in `/Controls`: the first paragraph should contain a `div` with `style="Header 1"`, and the second should contain `Some randome text`.
- `Queued` only means input was accepted. Read the control tree again after transitions and wait for popup bounds or refreshed templates to settle before the next click. Theme refresh can enlarge the window; return to the document page and restore the remembered dimensions before saving.
- Read theme names from `Window Manager` / `Color Theme`. The current names are `Default`, `Aurora`, `Ember`, `Moonstone`, `Lagoon`, and `Rosewood`.
- Close the app with `!Exit` after capturing.

### TuiControlTest console input and inspection

Launch a separate Windows Terminal window at 120 columns by 40 rows. For this checkout:

```powershell
& wt.exe -w new --size '120,40' new-tab -d 'C:\Code\VczhLibraries\GacUI\Test\GacUISrc' pwsh.exe -NoProfile -Command '& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable CppTest_Tui -Configuration Debug -Platform x64 -Interactive'
```

- Identify the running `CppTest_Tui` process. From a separate helper process, call `FreeConsole` and `AttachConsole` with that process id. Open `CONOUT$` to read `GetConsoleScreenBufferInfo` and `ReadConsoleOutputCharacterW`, and open `CONIN$` to send `WriteConsoleInputW` records. Close the handles and detach when finished. This app has no HTTP endpoint and does not need UI Automation.
- Read the visible viewport one row at a time. Respect the returned character count when converting each native buffer to text; do not read past it. Coordinates for input records are console cells, starting at zero.
- A click consists of three `MOUSE_EVENT` input records at the same cell: mouse move, left button down, then button up. With the current 120x40 layout, `List` is at `(5,0)`, `BindableDataGrid` at `(40,1)`, `Window Manager` at `(50,0)`, and the third row's category dropdown at `(43,9)`. Re-read the console after each transition instead of assuming these positions for another layout.
- On `Window Manager`, the current palette radios are at column 65, rows 2 through 7: `Pink`, `Orange`, `Grass`, `Emerald`, `SkyBlue (default)`, and `Purple`. Wait until the selected radio shows `(•)` beside the requested label before returning to `List`. Theme refresh is asynchronous.
- Returning to the grid can retain its editor. Click the category cell, read the console, and click again only if the editor is visible but the dropdown is not. Confirm the popup contains `Black`, `Red`, `Lime`, `Blue`, and `White` before capturing.
- Use the complete displayed palette label in the filename, including `TUI_SkyBlue (default).png`.
- After capturing, open `Exit` and choose `self.Close() (InvokeInMainThread)` with cancellation unchecked. Require normal process exit.

### Saving native window images

Use screenshots for the final artifact and visual verification; the HTTP control tree and console buffer provide the state needed for navigation. Capture the whole native window, including its frame. A window capture API can save FCT directly. For Windows Terminal, identify its showcase window, bring it to the foreground, obtain `DWMWA_EXTENDED_FRAME_BOUNDS` with `DwmGetWindowAttribute`, and use `System.Drawing.Graphics.CopyFromScreen` for that rectangle. Move the pointer outside the rectangle first and keep the window unobscured. Save PNG without rescaling. Verify equal dimensions within each showcase's theme set and inspect every final image.

## Tools for Linux

<!-- Update here when you find correct, stable and efficient way to interact with the OS for this task -->

## Tools for macOS

<!-- Update here when you find correct, stable and efficient way to interact with the OS for this task -->

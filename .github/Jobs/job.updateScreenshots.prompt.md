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

## Tools for the Task

- Update sections below when you find correct, stable and efficient way to interact with the OS for this task.
- You can't trust actual coordination, since in different computer the size and DPI is going to be different.

### for Windows

Build `Test/GacUISrc/GacUISrc.sln` with `copilotBuild.ps1` before launching either showcase. Use the wrappers described in the running guidelines.

#### FullControlTest automation

- Launch `CppTest` with `copilotExecute.ps1 -Mode CLI -Executable CppTest -Configuration Debug -Platform x64` from `Test/GacUISrc`.
- Read `http://localhost:8888/Automation/CppTest/Controls` to locate visible labels and their bounds. Send commands to `/IO` without a window id, using exactly `Content-Type: application/json; charset=utf8`.
- Navigate and click through `!LeftClick:X,Y`. After focusing the document editor, use `!Type:This is a header`, `!KeyPress:Enter`, and `!Type:Some randome text`. Select the first paragraph with `!KeyPress:Ctrl+Home` followed by `!KeyPress:Shift+End`, then choose `Style` / `Header 1`. Clear the selection with `!KeyPress:Ctrl+End`.
- Verify the document through `elementDocument` in `/Controls`: the first paragraph should contain a `div` with `style="Header 1"`, and the second should contain `Some randome text`.
- `Queued` only means input was accepted. Read the control tree again after transitions and wait for popup bounds or refreshed templates to settle before the next click. Theme refresh can enlarge the window; return to the document page and restore the remembered dimensions before saving.
- Read theme names from `Window Manager` / `Color Theme`. The current names are `Default`, `Aurora`, `Ember`, `Moonstone`, `Lagoon`, and `Rosewood`.
- Close the app with `!Exit` after capturing.

#### TuiControlTest console input and inspection

Launch a separate Windows Terminal window, using 120 columns by 40 rows as a starting size. Query the actual viewport after launch and after resizing; font settings and DPI can change the resulting window geometry. For this checkout:

```powershell
& wt.exe -w new --size '120,40' new-tab -d 'C:\Code\VczhLibraries\GacUI\Test\GacUISrc' pwsh.exe -NoProfile -Command '& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable CppTest_Tui -Configuration Debug -Platform x64 -Interactive'
```

- Identify the running `CppTest_Tui` process. From a separate helper process, call `FreeConsole` and `AttachConsole` with that process id. Open `CONOUT$` to read `GetConsoleScreenBufferInfo` and `ReadConsoleOutputCharacterW`, and open `CONIN$` to send `WriteConsoleInputW` records. Close the handles and detach when finished. This app has no HTTP endpoint and does not need UI Automation.
- Read the visible viewport one row at a time. Respect the returned character count when converting each native buffer to text; do not read past it. Preserve the mapping to console cells, including whitespace and wide-character occupancy. Input records use zero-based console buffer coordinates; add the current viewport's left and top offsets when converting viewport-relative positions. Do not derive input coordinates from screen pixels, font size, or saved screenshots.
- Locate `List`, `BindableDataGrid`, `Window Manager`, and `Exit` by their labels in the current tab rows. A click consists of three `MOUSE_EVENT` input records at a cell inside the matched label: mouse move, left button down, then button up. Re-read the console after every navigation, resize, or theme refresh and locate the next target again. Do not reuse row or column numbers from an earlier run.
- On the grid page, locate the `Category` header and the surrounding column boundaries. Count data rows, excluding the header and separator rows, to find the third row. Confirm its category is `White`, then click inside that cell. After the editor appears, locate its dropdown arrow within the same cell from a fresh console read.
- On `Window Manager`, find the `Color Theme` group and enumerate its visible radio labels. Locate each requested theme by its label and adjacent radio marker. Wait until the selected radio shows `(•)` beside that label before returning to `List`; theme refresh is asynchronous. If a target is clipped, resize or scroll to reveal it and read the viewport again before acting.
- Returning to the grid can retain its editor. Locate the third row's category cell again, click it, read the console, and click its dropdown arrow only if the editor is visible but the dropdown is not. Confirm the popup contains `Black`, `Red`, `Lime`, `Blue`, and `White` before capturing.
- Use the complete displayed palette label in the filename, including `TUI_SkyBlue (default).png`.
- After capturing, open `Exit` and choose `self.Close() (InvokeInMainThread)` with cancellation unchecked. Require normal process exit.

#### Saving native window images

Use screenshots for the final artifact and visual verification; the HTTP control tree and console buffer provide the state needed for navigation. Capture the whole native window, including its frame. A window capture API can save FCT directly. For Windows Terminal, identify its showcase window, bring it to the foreground, obtain `DWMWA_EXTENDED_FRAME_BOUNDS` with `DwmGetWindowAttribute`, and use `System.Drawing.Graphics.CopyFromScreen` for that rectangle. Move the pointer outside the rectangle first and keep the window unobscured. Save PNG without rescaling. Verify equal dimensions within each showcase's theme set and inspect every final image.

### for Linux

Build with `wGac/build.sh` before launching either showcase. Keep `WAYLAND_DISPLAY`, `XDG_RUNTIME_DIR`, and `DISPLAY` from the unlocked desktop session. On GNOME, check `org.gnome.ScreenSaver.GetActive` before capturing.

#### FullControlTest automation

- Launch `wGac/test.sh --app:fct`. Use `/Automation/Test_FullControlTest/Controls` and `/IO` on port 8888 with the same content type and commands described for Windows. Native popups have their own window ids; use the id containing the selected label when sending a popup click.
- Locate `Window Manager` and `Customized Frame` in the current control tree. Uncheck the latter and require the main composition to report `SystemFrameWindow`.
- Open `Control` / `Document Editor (Ribbon)`. Locate the `DocumentViewer` bounds, focus its content, type the two paragraphs, and select the first paragraph with `Ctrl+Home`, then `Shift+End`.
- Choose `Header 1` from `Style`. At wider sizes, Style is an expanded gallery and `Header 1` can be clicked directly. Refocus the document before sending `Ctrl+End`: with the gallery focused, that key can select another style and apply it to the selected paragraph. Require `<div style="Header 1">` around the first paragraph and the unchanged second paragraph in `elementDocument`.
- Enumerate the six palette labels from `Color Theme`. After every theme change, wait for the refreshed controls, return to the document page, and restore the remembered client size. Theme refresh can increase the window height even when the document is unchanged.
- GNOME's native `Alt+F8` resize operation works for the libdecor frame. The first arrow selects an edge; subsequent arrows move it by ten pixels, or one pixel with Ctrl held. Finish with Enter and clear any GacUI access-key overlay with Escape. Read `/Controls` again to verify the exact client dimensions. Desktop key injection can use a temporary `org.gnome.Mutter.RemoteDesktop` session and `NotifyKeyboardKeysym`; stop that session afterward.
- Close with `!Exit` after capturing.

#### TuiControlTest terminal input and inspection

- Launch a separate GNOME Terminal window with `--hide-menubar --geometry=120x40` and run `wGac/test.sh --app:tui` there. Check the actual terminal dimensions; the geometry option specifies cells. GNOME Terminal 3.52 / VTE 0.76 was used for this procedure.
- For repeatable inspection, a temporary PTY relay can start the repository launcher, forward its output unchanged to the visible terminal, and feed a copy to an ANSI terminal parser such as `pyte`. Initialize both the child PTY and parser from the terminal's actual dimensions and propagate `SIGWINCH`. Preserve terminal modes and record the child's exit status. The app itself has no HTTP automation endpoint.
- Read a settled cell buffer before each action. Locate labels and count grid data rows, excluding separators. Preserve wide-character occupancy when translating text offsets to columns. Do not derive input coordinates from screenshot pixels or a remembered font size.
- Inject SGR mouse reports through the relay's PTY master. Coordinates are one-based terminal cells: `ESC[<35;C;RM` moves, `ESC[<0;C;RM` presses the left button, and `ESC[<0;C;Rm` releases it. Click the third row's Category cell, inspect the editor, and click its dropdown arrow if the popup is not already showing.
- Enumerate `Pink`, `Orange`, `Grass`, `Emerald`, `SkyBlue (default)`, and `Purple` from `Color Theme`. Wait for `(•)` beside the chosen label and for the tab row to finish repainting before navigating back. A selected radio can appear before the rest of the refresh finishes.
- After returning to the grid, locate the cell again; its editor may persist. Capture only after the popup visibly contains `Black`, `Red`, `Lime`, `Blue`, and `White`. Use the full palette label in each filename.
- After capturing, dismiss the popup, open `Exit`, and choose `self.Close() (InvokeInMainThread)` with cancellation unchecked. Require exit status zero and restored terminal modes, then close the owned terminal and relay.

#### Saving native window images on GNOME Wayland

Use the live desktop for final PNGs. GNOME's `org.gnome.Shell.Screenshot` methods may reject ordinary D-Bus callers. The compositor's `org.gnome.Mutter.ScreenCast` interface provides a working capture path:

1. Keep one session-bus connection open, call `CreateSession({})`, and call `RecordMonitor` on the returned session to inspect the current desktop. Determine the showcase's complete native frame rectangle from this fresh image.
2. For the final image, create a session with `RecordArea(x, y, width, height, {"cursor-mode": uint32(0)})`. Subscribe to the returned stream's `PipeWireStreamAdded` signal before calling the session's `Start` method.
3. Use its PipeWire node id in a GStreamer pipeline: `pipewiresrc path=NODE num-buffers=1 ! videoconvert ! pngenc ! filesink location=PATH`. Wait for EOS, set the pipeline to NULL, and call the session's `Stop` method. Python's system `gi` bindings for Gio, GLib, and Gst can drive this without a command-line GStreamer executable.

Do not use an unprocessed `RecordWindow` frame for FCT: on the tested GNOME version it excludes the libdecor title bar and pads the image to monitor dimensions. `RecordArea` captures the actual native frame. Reinspect the desktop after moving or resizing a window; never reuse another run's rectangle. Keep the window unobscured, wait for notifications to disappear, and restore any temporary notification setting afterward. Save without rescaling, require equal dimensions within each showcase's theme set, and inspect every final PNG.

### for macOS

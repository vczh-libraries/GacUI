# !!!INVESTIGATE!!!
# PROBLEM DESCRIPTION
- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

You are going to implement `DumpWindowClientArea` and `RunIOCommandOnNativeWindow` following the incremental strategy in tasks.
You are required to always attach a debugger to GacUI applications, so that you can handle automation requests crashing or IO operations not properly implemented.
Use `!Exit` to close the application.
- This has already been implemented in `RunIOCommandOnNativeWindow`, `/IO/windowId` could trigger it.
- When remote protocol is used, `!Exit` should be sent to the renderer.

## Task 1

Implement `DumpWindowClientArea` following the comment.
- Ignore document so far.
To verify:
- Start `CppTest` and request for `/Controls`.
- See if it the returned JSON correct describes everything in the default UI state.

## Task 2

Implement `RunIOCommandOnNativeWindow`.
- Focus on mouse move or mouse button.
To verify:
- Start `CppTest`.
- Call `!MouseMove` on `Add 10 Items` and see if the button is highlighed.
- Click this button and see if 10 items appears.
- Click `Text` and you will see a new sub window representing the combo box drop down:
  - Switch to `Check` and see if items marked with a rectangle.
  - Switch to `Radio` add see if teism marked with a circle.

## Task 3

Ensure `DumpWindowClientArea` correctly reporting all kinds of elements.
- Ignore document so far.
To verify:
- Start `CppTest`.
- Switch to `Misc` tab and under `Elements` it enumerates all kinds of elements.
- Verify if element properties are properly reported.

## Task 4

Implement `RunIOCommandOnNativeWindow`.
- Focus on document element.
Implement `RunIOCommandOnNativeWindow`
- Focus on `!Type:Text`, you should ignore all `\r` characters and send `\r\n` for any `\n`.
To verify:
- Start `CppTest`.
- Switch to `Control` tab, click the document element, and begin typing.
- In this control, CRLF is interpreted as new paragraph.
- Type several lines of text and see if document element report attributes correctly.

## Task 5

Implement `RunIOCommandOnNativeWindow`
- Focus on keyboard operations.
To verify:
- Start `CppTest`.
- Press `ALT` and you can find that the UI adds additional labels to guide you how to navigate and eventually focus or activate a control.
- You are going to do pure keyboard navigation and do this:
  - Swtich to `Control` tab and focus on the document element.
  - Type `ABCDEFG`.
  - Select `CDE`.
  - Activate the toolbar button to open a color dialog, choose green (red:0 green:255 blue:0).
  - Focus on the document element again and press `END` therefore the selection is gone.
  - See if the document is properly edited.
You need to carefully reads the knowledge base, the UI definition and the source code of document control to figure out how to done all these without mouse.

## Task 6

Implement `RunIOCommandOnNativeWindow`
- Focus on wheel operations.
- Unfortunately you are only able to test up/down, but you should make sure left/right is propertly implemented.
To verify:
- Start `CppTest` and click `Add 10 items` multiple times.
- Wheel the left list box up and down and see if it actually scrolls.

## Task 7

Start `RemotingTest_Core /Http /FCT` and `RemotingTest_Rendering_Win32`, you are able to read UI and perform IO operations by both two processes.
Perform `Task 5` verification steps on `RemotingTest_Core` and see if both process reports proper UI changes.
Perform `Task 5` verification steps on `RemotingTest_Rendering_Win32` and see if both process reports proper UI changes.
Real system message box will popup up from renderer when core exception raised, the current automation service cannot handle this currently, you will need to kill both processes to exit when it happens.
# UPDATES
## UPDATE
Your fix was basically good but there is one problem. When handling /IO you call InvokeInMainThread in windows::HttpAutomationService, this will make it unable to respond syntax error.

Please fix it to call automationService->RunIOCommand directly and return the result, instead when in RunIOCommandOnNativeWindow, do this:
- Change all return Executed to Queued
- Instead of anonymous namespace change it to "iocommands", with one struct as parsing result for one command.
- Define a Variant of all commands, define a ParseIOCommand function to return the variant
- define a ExecuteIOCommand function to run Variant::Apply in InvokeInMainThread

In this way you are able to return the parsing error meanwhile queue the command avoiding it being blocked by blocking operations like waiting for a dialog.

## UPDATE
Read [TODO_Task.md](TODO_Task.md) which is the original task definition, you are going to rerun Task 5 validation to make sure your change actually works

# TEST
Manual verification uses the HTTP automation service with debugger-attached GacUI applications as requested. Unit verification will build and run `UnitTest` through `.github/Scripts/copilotExecute.ps1`; targeted coverage should exercise automation IO parsing and event dispatch when feasible.

Success criteria:
- `/Controls` returns schema-compliant JSON with window, composition, layout, element, text, document, and control fields as applicable.
- `/IO` accepts mouse, keyboard, text, and wheel commands and dispatches native listener callbacks with correct coordinates, button/key state, modifiers, and CRLF text normalization.
- `CppTest` and the remote protocol pair can be driven through the requested automation scenarios without automation crashes.
- `UnitTest` passes with no memory leak report.
# PROPOSALS
- No.1 Implement automation dumping and IO dispatch in the shared automation service
- No.2 Split IO parsing from queued native execution [CONFIRMED]

## No.1 Implement automation dumping and IO dispatch in the shared automation service
Implement `/Controls` by recursively walking visible compositions from each window's bounds composition, reporting cached global bounds, layout tags, supported graphics element properties, and associated control theme names. Implement `/IO` by reusing the unit-test protocol state model for pressing keys/buttons and dispatching native listener callbacks directly on the selected native window. Confirm the proposal incrementally through unit tests plus debugger-attached `CppTest` and remote protocol HTTP automation checks.

### CODE CHANGE
- Implemented `/Controls` client-area dumping by walking visible compositions recursively from each window bounds composition.
  - Each composition now reports bounds and, when available, layout (`Table`, `Cell`, `Stack`, `Flow`, splitters), element description, label text, document XML, associated control theme, and child compositions.
  - Element detection is intentionally kept local and direct with `dynamic_cast` checks for the supported GacUI element types.
  - Hosted control dumps now report the actual window management mode (`Hosted` or `HostedRemoteProtocol`) instead of hardcoding `MultiWindow`.
- Implemented `/IO` command execution for native windows.
  - `!Exit` hides the main window and returns `Executed`.
  - `!Type:<TEXT>` emits character events, ignores `\r`, and emits `\r\n` for every `\n`.
  - `!KeyDown`, `!KeyUp`, and `!KeyPress` resolve keys via `INativeInputService::GetKey`/`GetKeyName`.
  - Mouse move, button down/up/click/double-click, vertical wheel, and horizontal wheel commands maintain IO state and dispatch native listener callbacks with modifier state.

Build and test:
- `C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1` from `Test\GacUISrc`: succeeded with 0 warnings and 0 errors.
- `C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: passed `84/84` test files and `1686/1686` test cases.

Debugger-attached manual verification:
- `CppTest` under `cdb`:
  - `/Automation/CppTest/Controls` returned `WindowManagement: Hosted`, title `Complete Control Showcase`, 266 dumped nodes, 32 controls, and default UI labels/buttons including `Add 10 items`.
  - `!MouseMove` over `Add 10 items` hit the button, and `!LeftClick` added visible items `0..9`.
  - Combo box dropdown subwindow was dumped; selecting `Check` reported rectangle marker elements, and selecting `Radio` reported ellipse marker elements.
  - `Misc` tab element samples reported `Background`, `Border`, `FocusRectangle`, `Gradient`, `Image`, `InnerShadow`, `Label`, and `Polygon` data.
  - Document typing with `!Type:Alpha\nBeta\r\nGamma` ignored `\r`, sent CRLF for line feeds, and `/Controls` reported document XML containing the typed text.
  - Wheel commands moved the left list box viewport down and back up after adding multiple pages of items.
- `RemotingTest_Core /Http /FCT` and `RemotingTest_Rendering_Win32` under separate `cdb` sessions:
  - Core `/Controls` returned `WindowManagement: HostedRemoteProtocol`; renderer `/Dom` returned the remote rendering tree.
  - IO through `RemotingTest_Core` switched to the `Control` tab, focused the document, typed `COREABCDEFG`, selected text with `Shift+Left`, and collapsed the selection with `End`; both core `/Controls` and renderer `/Dom` reported the text.
  - IO through `RemotingTest_Rendering_Win32` typed `RENDERXYZ` into the same document; both processes reported the UI update.
  - `!Exit` was sent to the renderer; the already-invalid core window returned `!Invalid window` as expected after renderer exit.

### FOLLOW-UP CODE CHANGE
- Changed HTTP `/IO` dispatch to queue the command with `INativeAsyncService::InvokeInMainThread` and return `Queued`.
  - The HTTP response now only confirms that the command was accepted by the automation endpoint.
  - It no longer means the input operation has finished, because listener callbacks may run later and may open modal UI.
  - The comments on `RunIOCommandOnNativeWindow` and the `/Controls` schema were updated to document this behavior.
- Fixed POST matching for `/IO/<windowId>` by checking the `/IO` prefix instead of requiring the path to equal `/IO`.
- Passed the actual target native window into `RunIOCommandOnNativeWindow`.
  - `!Exit` and activation now operate on the native window that owns the listener list, not merely the controller's current main window.
  - This matters for hosted and remote protocol scenarios where the controller and target listener window can differ.
- Adjusted hosted keyboard dispatch so the main hosted window is activated before forwarding key or char input when no hosted child is active yet.
  - This lets pure keyboard automation begin from a freshly launched hosted/remote window without requiring a priming mouse click.

### FINAL VERIFICATION
- Rebuilt after the follow-up changes:
  - `C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1` from `Test\GacUISrc`: succeeded with 0 warnings and 0 errors.
- Ran the required unit tests after the final source changes:
  - `C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: passed `84/84` test files and `1686/1686` test cases.
- Reverified remote protocol under debugger-attached `RemotingTest_Core /Http /FCT` and `RemotingTest_Rendering_Win32 /Http`.
  - Fresh core `/IO` pure keyboard navigation:
    - `!KeyPress:Alt` produced Alt labels such as `[L]3`.
    - `!KeyPress:L`, `!KeyPress:3` switched to the `Control` tab.
    - `/Controls` then reported 8 document elements.
  - Fresh renderer `/IO` pure keyboard navigation:
    - `!KeyPress:Alt` also produced Alt labels in core `/Controls`.
    - `!KeyPress:L`, `!KeyPress:3` switched to the `Control` tab.
    - Core `/Controls` and renderer `/Dom` stayed synchronized.
  - Full Task 5 through core `/IO`:
    - Switched to `Control`, focused the document, typed `ABCDEFG`, selected `CDE`, opened the color dialog, set RGB to `0,255,0`, confirmed, refocused the document, and pressed `End`.
    - Final core `/Controls` reported `<nop>AB</nop><font color="#00FF00"><nop>CDE</nop></font><nop>FG</nop>`.
    - Renderer `/Dom` contained both the text and the green color data.
    - Dumping core `/Controls` while the modal color dialog is open can time out because `/IO` is queued and the modal command is still active; final verification avoids treating `/IO` completion as operation completion and dumps only after the dialog closes.
  - Full Task 5 through renderer `/IO`:
    - Used the same pure keyboard workflow with explicit synchronization before opening the modal dialog.
    - Final core `/Controls` again reported green `CDE` between `AB` and `FG`.
    - Renderer `/Dom` also contained `ABCDEFG` and `00FF00`.

## No.2 Split IO parsing from queued native execution
Keep `/IO` syntax parsing synchronous in `windows::HttpAutomationService` by calling `INativeAutomationService::RunIOCommand` directly. Move the asynchronous boundary into `RunIOCommandOnNativeWindow`: parse commands into a `Variant` under an `iocommands` namespace, return syntax errors immediately, and queue only the parsed command execution with `INativeAsyncService::InvokeInMainThread`. This keeps modal or blocking UI work off the HTTP response path while still reporting malformed command text correctly.

### CODE CHANGE
- Replace the old immediate execution path in `RunIOCommandOnNativeWindow` with `iocommands::ParseIOCommand` and `iocommands::ExecuteIOCommand`.
- Represent each command as its own parsing-result struct and store them in an `iocommands::IOCommand` `Variant`.
- Queue parsed command execution with `INativeAsyncService::InvokeInMainThread`, where `Variant::Apply` dispatches to the existing mouse, keyboard, text, wheel, and exit event helpers.
- Store the parsed `Variant` in a reference-counted command holder before queuing, so the async closure copies only the holder pointer instead of copying the `Variant` through the function object.
- Change successful command responses from `Executed` to `Queued`.
- Change Windows HTTP `/IO` handling back to directly returning `automationService->RunIOCommand(...)`, so syntax errors can be returned synchronously.
- Update comments to say `/IO` synchronously parses and returns either syntax text or `Queued`.

### CONFIRMED
- Rebuilt after splitting parsing from queued execution:
  - `C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1` from `Test\GacUISrc`: succeeded with 0 warnings and 0 errors.
- Ran the required unit tests after the final source changes:
  - `C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: passed `84/84` test files and `1686/1686` test cases.
- Rechecked `/IO` response behavior against debugger-attached `CppTest`:
  - Malformed `!MouseMove:nope` returned HTTP 200 with `Syntax Error! ...`, confirming parsing errors are reported synchronously by the HTTP handler.
  - Valid `!KeyPress:Alt` returned HTTP 200 with `Queued`, confirming accepted commands now cross the async boundary inside `RunIOCommandOnNativeWindow`.
  - The first live keyboard check exposed a `Variant` copy assertion in the queued closure; storing the command in `IOCommandHolder` fixed it, and the same `!KeyPress:Alt` then returned `Queued`.
- Reran Task 5 validation on debugger-attached `CppTest`:
  - Started from `/Controls` reporting document text `ABCDEFG` with the caret at the end.
  - Selected `CDE` using only keyboard IO: two `Left` presses, `Shift` down, three `Left` presses, then `Shift` up. `/Controls` reported document selection `(0,5)-(0,2)`.
  - Used Alt labels to activate the ribbon text group (`Alt`, `T`, `1`) and color command (`C`), opening the color dialog as a subwindow titled `选择颜色`.
  - Used only keyboard IO in the dialog: Alt labels focused the red, green, and blue text boxes; the text boxes reported RGB values `0`, `255`, and `0`; the color preview reported `#00FF00FF`.
  - Confirmed with `Alt`, `O`; after the dialog closed, `/Controls` reported `<nop>AB</nop><font color="#00FF00"><nop>CDE</nop></font><nop>FG</nop>`.
  - Refocused the document with `Alt`, `D`, `2`, pressed `End`, and `/Controls` reported `Document:Selection(0,7)-(0,7),WrapLine`, confirming the selection collapsed after the edit.

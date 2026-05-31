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

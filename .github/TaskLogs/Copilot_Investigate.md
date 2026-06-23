# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

`Copilot_Investigate.md` in `Tools` repo should not be touched.
You are going to complete `REPO-ROOT\.github\Guidelines\Running-ComputerUse.md`.
You are going to sync the completed document to `REPO-ROOT\..\Tools\Copilot\Guidelines\Running-ComputerUse.md`.
You are going to commit and push all changes in both repos.

UIA does not work when the screen is locked, so the safest way is to taking screenshots and/or using Powershell with embedded C# code calling Win32 API to deal with native dialogs.
Win32 API could include enumerating windows, sending messages, or anything you need.
You are going to figure out how to actually operate on those dialogs to complete the document.
You can find those dialogs by running `CppTest_Metaonly`:
- Message
  - Pressing `CTRL+Q` and a native message box would show.
  - You should figure out how to click OK or close the message box.
  - The document should contain extra instructions about, if the message box has other combination of buttons, what to do.
- Color
  - Switch to `Control` tab and in the `Document Editor (Ribbon)` tab there is a text box.
  - Type something and select all
  - Click the text color button on the ribbon
  - A native color dialog would show
  - You should figure out how to change color, how to click OK and Cancel
  - You will need to check the text box and see if the expected color applies, you should unselect the text to reveal its color.
- Font
  - Switch to `Control` tab and in the `Document Editor (Ribbon)` tab there is a text box.
  - Type something and select all
  - Click the text font button on the ribbon
  - A native font dialog would show
  - You should figure out how to change all components of the font, how to click OK and Cancel
  - You will need to check the text box and see if the expected font applies.
- File
  - Switch to `Control` tab and in the `Document Editor (Ribbon)` tab there is a text box.
  - Switch to `Insert` tab and click `Insert Image ...` button.
  - A native file dialog would show
  - You should figure out how to navigate between folders, select a file by either clicking it or typing the file name, how to click OK and Cancel.
  - You can select `C:\5900.png` and see if the image is inserted to the text box.

- General
  - Something like a vcruntime error dialog is not a standard dialog, but it is still native, provide hints about how to read the window, find information, click buttons, etc.
- Just to say again, UIA should not be involved in this document, as it does not work when the screen is locked, calling UIA will just fail.
- The test project has no bug editing, so if you can't see it, you are operating the native dialog wrong.

The document should contain only general information, e.g. anything about `CppTest_Metaonly` is `GacUI` repo only so it should not be mentioned.
If any useful code snippet is helpful, especially `using Powershell with embedded C# code calling Win32 API` or whatever, you should include it in the document.
Quality expectation of the document: it would become an instruction so that a future you can handle native windows correctly, without causing any panic or block yourself forever. A coding agent (like you codex) dead locking with the application under test is funny and unacceptable, this document helps a future you avoid being into such situation.

# TEST [CONFIRMED]

Verify the native-window handling guidance by running the GUI application that exposes standard native dialogs and operating each dialog without UI Automation:

- Open and dismiss a native message box by locating the window and button handles through Win32 enumeration.
- Open a native color dialog, change the selected color, confirm and cancel it, and verify the edited text changes only after confirmation.
- Open a native font dialog, change font components, confirm and cancel it, and verify the edited text changes only after confirmation.
- Open a native file dialog, navigate/select a file by Win32-accessible controls or filename entry, confirm and cancel it, and verify the selected image is inserted only after confirmation.
- Inspect at least one general native window using Win32 enumeration so the final document can explain how to avoid deadlock even for non-standard crash/error dialogs.

Success criteria:

- No UIA APIs or UIA tools are used.
- Every opened native dialog can be closed deterministically.
- The final `Running-ComputerUse.md` contains general reusable instructions only, with no repository-specific sample application names.
- The completed document is synced to the Tools repository guideline path without touching `Copilot_Investigate.md` in the Tools repository.

# PROPOSALS

- No.1 Document Win32-native dialog handling [CONFIRMED]

## No.1 Document Win32-native dialog handling

Document a general workflow for native windows that avoids UIA and prevents modal deadlocks:

- Use screenshots or Win32 enumeration to find top-level native dialogs and child controls.
- Prefer `EnumWindows`, `EnumChildWindows`, `GetWindowText`, `GetClassName`, `GetDlgCtrlID`, `SendMessage`, `PostMessage`, and standard button/control messages.
- Include reusable PowerShell embedded-C# snippets for enumerating windows, clicking buttons, setting edit text, selecting combo-box items, and closing dialogs.
- Cover standard message, color, font, and file dialogs, plus general non-standard native error windows.

### CODE CHANGE

Completed `REPO-ROOT\.github\Guidelines\Running-ComputerUse.md` with the Win32-native dialog handling workflow, control IDs, and reusable PowerShell/C# snippets. The document will be synced to the Tools repository guideline path after local review.

### CONFIRMED

Confirmed with a running GacUI application that:

- A standard message box is a `#32770` top-level dialog with a `Static` text child and `Button` child; clicking `OK` by `BM_CLICK` unblocks the app.
- A standard color dialog exposes `OK` id `1`, `Cancel` id `2`, RGB edit ids `706`, `707`, `708`, and other common color controls; setting RGB edits and clicking `OK` closes the dialog, and `Cancel` also unblocks without applying.
- A standard font dialog exposes combo-box ids `1136` for font, `1137` for style, `1138` for size, `1140` for script, plus `OK` id `1` and `Cancel` id `2`; `CB_SELECTSTRING` selects values and `BM_CLICK` confirms or cancels.
- A standard file-open dialog exposes filename edit id `1148`, `Open` button id `1`, and `Cancel` button id `2`; setting a full path in the filename edit and clicking `Open` selects the file, and `Cancel` unblocks without applying.
- The app's HTTP automation endpoint blocks while native modal dialogs are open, so native dialog handling must use Win32 enumeration/messages from another process and must not wait indefinitely on app-level automation.

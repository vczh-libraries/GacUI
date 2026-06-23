- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

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
- The test project has no bug editing, so if you can't see it, you are operating the native dialog wrong.

The document should contain only general information, e.g. anything about `CppTest_Metaonly` is `GacUI` repo only so it should not be mentioned.
If any useful code snippet is helpful, especially `using Powershell with embedded C# code calling Win32 API` or whatever, you should include it in the document.

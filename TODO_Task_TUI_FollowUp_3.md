`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

Extra requirements for making commits:
- `## Release` should be in the last step.
- Before beginning `## Release`, all local changes in affected repos should be committed and pushed first.

## FakeDialogService TUI Dialogs

- Message/Color/Font/File dialog no need to have empty lines, except one above buttons.
- The message in the message box should not have a border, just put a label there it is fine.
- Message box buttons should align to center horizontally. (other dialogs have button aligned left, that is correct)
- Color dialog scroll bars are too big:
  - Both label and scroll bar should align to center vertically in their cell.
  - Scroll bars should have 1 pixel in height.

## TuiControlTest

- After startup the main window does not resized to fit the CLI window.
  - This should also be added to `DebugTuiControlTestSop.md`.
- Although TUI's `INativeWindow::Hide` is currently no-op, but it should:
  - Query window close and give a chance for the app to prepare. You can check out how Windows native GUI implementation does that to invoke necessary callbacks.
  - Call directly or behave like current `GetTuiApplication()->Stop` so that to quit `GetCurrentController()->Run`.
  - Update the `Exit` tab in `TuiControlTest` according to this.
- In `Misc/Dialogs/*`, there are tables where the first column are labels and the second columns are text boxes.
  - Those label should align to center vertically inside their cell.
  - No need to keep empty lines, except one above buttons in the last line.

## Documentation

- Scan `TODO_Task_TUI_FollowUp_[123].md` and see if any knowledge base page or sop need to udpate.

## Release

- Refer to `REPO-ROOT/../AGENTS.md` for understanding the mono repo.
- Just like how `DarkSkin` and `FullControlTest` is done, `TuiSkin` should also be published to `Release` repo, and `TestControlTest` should become its tutorial.
  - Check out how `Build.ps1 -Project Release` currently handle `DarkSkin` and `FullControlTest`.
  - `TuiSkin` should go to `Release/Import/Skins/TuiSkin`.
  - `TuiControlTest` should go to `Release/Tutorial/GacUI_ControlTemplate/TuiSkin.vcxproj` with a new project `TuiSkin`.
  - `TuiControlTest` resource files will be put in its `UI/TuiControlTest` sub folder, the same structure to the sibling `BlackSkin` for `FullControlTest.
- To implement the new `TuiControlTest` tutorial, necessary new files should go to `GacUILite` library.
  - In `GacUILite` there is a `WinMain.cpp` to start the Direct2D renderer.
  - I would like you to add a `TuiMain.cpp`, but both files will be in every projects.
  - In order not to break existing apps, you should declare a global macro in `TuiSkin.vcxproj` say `GACUILITE_TUI_APPLICATION`.
    - `WinMain.cpp` will `#ifndef GACUILITE_TUI_APPLICATION` the whole file.
    - `TuiMain.cpp` will `#ifdef GACUILITE_TUI_APPLICATION` the whole file.
    - Therefore no other projects need to change, since not defining `GACUILITE_TUI_APPLICATION` causes `WinMain.cpp` to use and ignore `TuiMain.cpp`.
- Update the website to:
  - Introduce GacUI on TUI and how to start.
  - Remember that TUI does not work with remote protocol, so no need to mention TUI in remote protocol specific pages or sections.
  - Scan the demo list and see if any tutorials are missing, fix the list.
  - DO NOT publish the website.

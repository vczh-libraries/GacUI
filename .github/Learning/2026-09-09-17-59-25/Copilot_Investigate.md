# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- Based on what OS you are on:
  - On Linux, you are working on VlppOS, GacUI and wGac.
    - When porting to Linux, Windows verification should have been done.
  - On macOS, you are working on VlppOS, GacUI and iGac.
    - When porting to macOS, Windows and Linux verification should have been done.
  - VlppOS and GacUI are already verified on windows, but if you find any issue in the current OS during verification, fix them.
  - wGac/iGac is updated but never verified, you are going to take care of any issue in them.
- TUI has been completed and verified on Windows:
  - `TODO_Task_TUI.md`
  - `TODO_Task_TUI_FollowUp_1.md`.
  - `TODO_Task_TUI_FollowUp_2.md`.
  - `TODO_Task_TUI_FollowUp_3.md`.
- The goal of this request is to make sure it also works on Linux/macOS:
  - Port the new test app, make sure it could be synced in the future in `syncProj.sh`, and started by `test.sh --app:tui`.
  - Update `GacUI/.github/Jobs/job.rpXPlat.prompt.md` for the new test app. The SOP is in `GacUI/.github/Jobs/DebugTuiControlTestSop.md`.
    - If it has already been updated, verify this document again.
  - Make sure the ported test app passed tests following `GacUI/.github/Jobs/DebugTuiControlTestSop.md`.
  - Check out `TuiWindowsController.Windows.(h|cpp)` to understand how to use the shared `TuiControllerBase`.
- TUI in VlppOS did not respond to `osSuper` in IO structs, but it has been done on Windows. Verify if the behavior works cross platform and fix it if not.
- commit and push once finishing.

# TEST

Build and run the VlppOS and GacUI unit suites on macOS. Import and synchronize iGac, build all targets, and exercise Test_TuiControlTest through test.sh following DebugTuiControlTestSop.md. Record each supported operation and terminal limitations in iGac/TestMatrix_Tui.md; verify all six normal shutdown paths and terminal restoration. Repeating synchronization must reproduce the generated TUI snapshot.

# PROPOSALS

- No.1 Supply the macOS terminal controller, reusable Cocoa service ownership, and reproducible app wiring.

## No.1 Supply the macOS terminal controller, reusable Cocoa service ownership, and reproducible app wiring

The iGac checkout has no TUI adapter, target, imports or sync/launcher entry. Implement the adapter over TuiControllerBase and supply Cocoa clipboard, image, key names and global shortcuts without a Cocoa window controller. Keep terminal timers on the TUI owner thread. Verify the existing POSIX Kitty/Super decoder on macOS.

### CODE CHANGE

The iGac adapter, app target, import/sync/launcher wiring and explicit Cocoa callback ownership are implemented. Apple Clang required explicit algorithm includes in the shared TUI renderer/layout and provider tests. VlppOS passes 270 cases and GacUI passes 1748 cases on macOS.

A native GUI clipboard regression check exposed an inherited CoreText UTF-16/glyph versus UTF-32 indexing crash on supplementary text (crash report CoreTextParagraph::GenerateFormatData -> ObjectString<wchar_t>::operator[]). The paragraph adapter now maps storage ranges and returns native text offsets; standard and hosted FCT regression checks now pass Unicode clipboard, scalar caret navigation, combining/ligature and multiline text without the crash. The iGac fix is committed separately from the TUI integration.

The macOS desktop is locked, so native keyboard/mouse and displayed appearance cannot be verified. Kitty terminal inspection and replay remain available.

### RESULT

Proposal No.1 is implemented and verified on macOS: all iGac targets build, repeated TUI synchronization reproduces 28 files, 270 VlppOS and 1748 GacUI cases pass. The terminal showcase runs from test.sh --app:tui; all SOP pages have scoped terminal-replay evidence in iGac/TestMatrix_Tui.md. Kitty-generated Ctrl+Alt+Super+Q reaches the application independently of Alt. All six direct/queued Hide/Close/Stop paths exit 0 and restore terminal modes and usable shell input. Physical keyboard/mouse, Carbon global activation and displayed appearance cannot be certified while the desktop is locked. The shared long unwrapped document placeholder dialog exceeds the terminal viewport and is recorded as a baseline layout limitation.

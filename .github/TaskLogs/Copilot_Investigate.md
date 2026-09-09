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

Build VlppOS and GacUI UnitTest on Linux and run their full suites. Build wGac after refreshing upstream release/import snapshots and synchronizing resources. Exercise the TUI showcase through test.sh --app:tui using the complete applicable DebugTuiControlTestSop.md procedures, recording terminal capabilities and unavailable native inputs explicitly. Verify repeat synchronization, startup at physical terminal sizes, input including OS Super, dialogs, and all normal shutdown/restoration paths.

# PROPOSALS

- No.1 Complete the Linux TUI provider and showcase integration.

## No.1 Complete the Linux TUI provider and showcase integration

Compare the existing native service adapters with TuiWindowsController and compose the shared TuiControllerBase with Linux services. Synchronize the showcase and skin from GacUI, add its build target and foreground terminal launcher, fix cross-platform defects in their owning repositories, and update the shared job and Linux verification record.

### CODE CHANGE

The baseline wGac tree has no TUI target or launcher; POSIX input has no CSI-u or distinct Super decoding. Normalize the TuiSkin vcxitems entry to separate XML lines so the Linux inventory includes TuiSkinConfig.cpp. Share the showcase GuiMain through platform-guarded includes and wmain. Synchronize TuiControlTest with an explicit TuiSkin include and import the released skin. Compose terminal resource/input/image services, title handling, and an X11/XWayland UTF-8 clipboard because terminal focus belongs to another Wayland client. Preserve existing wGac global-hotkey and terminal SGR mouse-Super limitations. Add Kitty disambiguation-mode entry/exit, capability response and CSI-u decoding, plus byte-boundary and malformed-input regressions.

Baseline VlppOS passes 268 cases; the new regression passes with all 269 cases. First wGac build confirmed the missing generated TuiSkin include, now fixed in the synchronization owner.

The Linux GacUI suite completed 89/89 files and 1,747/1,747 cases. One asynchronous file-dialog recording changed its element-allocation IDs. The focused file-dialog rerun passes 27/27; resolving IDs and comparing the entire rendered tree shows identical element properties, geometry, clipping and hierarchy. Preserve the existing golden snapshots and discard run-generated recording/log differences. The native Kitty 0.32.2 showcase is now running at 120x40 with XTest-generated keyboard/mouse input and rendered screen captures.

Live Kitty verification delivered Ctrl+Alt+Super+Q using both Super keys and varied modifier release orders, with the expected shortcut dialog. The new clipboard initially missed external ownership notifications, leaving document Paste disabled; add XFixes notifications and retest from a non-text selection to UTF-8 text. The native file picker also exposed C-locale filename conversion (a Unicode file became an empty name); initialize LC_CTYPE before constructing the new TUI services. The rebuilt provider now passes external-to-app/app-to-external Unicode, supplementary-scalar and literal-TAB clipboard checks. All four canceled Hide/Close paths advance invocation/query counts once; accepted Close returns 0 and restores identical termios plus usable shell input.

Native editing exposed POSIX DEL Backspace leaking U+007F through Char after KeyDown had already deleted the previous character. Normalize DEL/LF to backspace/carriage return in the shared POSIX queue and cover raw/CSI-u forms; the rebuilt app verifies exact clipboard text and single-step Undo. Typing absolute paths in the real picker also exposed Linux ConcatPath prefixing an absolute operand; absolute POSIX operands now replace the base, with root/non-root/relative normalization regressions. Final VlppOS suite passes 270/270. The final wGac build passes all targets; the final GacUI full suite passes 89/89 files and 1748/1748 cases, including the new caret regression and all file-dialog cases. Run-generated async allocation/frame-ID differences resolve to identical rendered trees; original snapshots are retained.

The native SOP record is maintained in ../wGac/TestMatrix_Tui.md. Real Kitty/XTest input verified both Super keys and release orders, three mouse buttons, both wheel axes, list/grid refresh and layout pages, Unicode clipboard/editors, dialogs, calendars, image loading and file callbacks. Known inherited limits are explicit: global hotkeys, standalone Alt overlays in the requested terminal mode, extra mouse/Super fields, POSIX en-US formatting, and the source-defined non-persisting document callbacks/custom-color options.

Final native caret sampling found that the block caret at the end of a minimum-width document was clipped; reserve its trailing terminal cell and add a rendered ASCII/empty/CJK/supplementary/TAB regression. A first paste of long lines exposed a separate ordering issue: rendering reveals the horizontal scrollbar after the initial caret scroll, then reduces the vertical page. The focused document viewer now repeats caret visibility on scrollbar page-size changes. Fresh 40-line and 1,100-line wide pastes show their last line immediately, with Home/End and undo/redo verified; all four editor leaves pass the final caret/TAB/paste/resize recheck. Temporary diagnostics were removed.

All six final-build normal exits pass, including each Hide/Close veto and terminal-state restoration; local Unicode clipboard exchange also passes without DISPLAY or WAYLAND_DISPLAY. The final Linux SOP card records supported passes and platform limitations. A remote GacUI fast-forward added only two TODO-file commits and did not overlap this implementation.

Final verification: all supported Linux SOP checks pass, repeat synchronization reproduces all 27 generated TUI files, and owning release/import pairs match. Original unit-test snapshots are retained after comparing all six asynchronous file-dialog frames. VlppOS is committed and pushed; GacUI and wGac contain the completed provider, synchronization, regression fixes and verification record for the final commits.

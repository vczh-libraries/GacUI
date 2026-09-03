# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

there is one bug. in iGac, `./test.sh --app:fct` is running well, shortcut keys works, this is a native macOS Cocoa app. But when I run `./test_core.sh --app:fct --protocol:minihttp` and `./test.sh --app:renderer`, Ctrl+Q and Ctrl+Alt+Command+Q does not work, meanwhile Ctrl+Shift+Alt+Command+Q is working. Both shortcut keys not working is in-app shortcut key, meanwhile the working one is global shortcut key. Figure out what happens, fix it, commit and push all local changes.

# UPDATES

# TEST [CONFIRMED]

Reproduce with the native macOS renderer connected to `RemotingTest_Core /FCT /MiniHttp`. Activate the three FullControlTest shortcuts with physical keyboard events and observe their dialog results:

- `Ctrl+Q` must activate the in-app shortcut.
- `Ctrl+Alt+Command+Q` must activate the in-app shortcut with all three distinct modifiers.
- `Ctrl+Shift+Alt+Command+Q` must continue to activate the global shortcut.

Use protocol tracing or a debugger to compare the renderer-side key event with the Core-side `NativeWindowKeyInfo` and shortcut lookup. The test confirms the bug when the first two combinations fail remotely while the global combination succeeds. Success requires all three remote combinations and the standalone Cocoa FullControlTest equivalents to work, with no regression in the relevant unit tests.

On macOS, real keyboard events sent to the focused `RemotingTest_Rendering_macOS` window reproduced the issue: `Ctrl+Q` and `Ctrl+Alt+Command+Q` produced no dialog, while `Ctrl+Shift+Alt+Command+Q` produced `You pressed Ctrl+Shift+Alt+Win+Q!` through the global-hot-key path.

# PROPOSALS

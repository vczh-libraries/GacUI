# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Previous [TODO\_Task\_MouseEvent.md](TODO_Task_MouseEvent.md) has been executed but there is a few bugs:
```markdown
  - "Click here with any mouse button" has wrong color and font like other labels. The most simplist way is to put a label and wire event handlers to its `<BoundsComposition-set/>` property, so you don't worry about the color and font.
  - Under remote protocol, both `FullControlTest` and `RemoteProtocolTest` displays "Ctrl+osSuper+Q" and "{Ctrl+Shift+Alt+osSuper+Q}" in native renderer or GacJS, which is incorrect. Meanwhile non-remote protocol native app (e.g. `CppTest`) displays "Ctrl+Win+Q" and "{Ctrl+Shift+Alt+Win+Q}" which is the expected result.
    - This localization feature is important, add it to .github/Jobs/DebugRemoteProtocolSop.md to verify the text of these lables if there is no such step.
  - Ctrl+Win+Q seems to be conflict a windows predefined shortcut key, change it to Ctrl+Alt+Win+Q for testing.
```

fix it, follow the verification steps in the origin task [TODO\_Task\_MouseEvent.md](TODO_Task_MouseEvent.md) to make sure your change is good.

# UPDATES

# TEST [CONFIRMED]

- Reproduce initial remote connection with shortcuts created before the renderer connects. Verify actual bound label text, including a command installed in a nested control, and verify label refresh after renderer replacement.
- In FullControlTest Window Manager and RemoteProtocolTest Home, require themed mouse labels and exact Windows shortcut labels: `Ctrl+Q`, `Ctrl+Alt+Win+Q`, and `{Ctrl+Shift+Alt+Win+Q}`.
- Follow the original task's generation, complete unit-test, native input, remote transport/replacement, and release verification requirements. Check GacJS against the updated core as well.

The existing Debug x64 native renderer over named pipe reproduces RemoteProtocolTest's `Ctrl+osSuper+Q` and `{Ctrl+Shift+Alt+osSuper+Q}`. Both applications report the click target as black Lucida Console 12, while adjacent labels use light Segoe UI 15. FullControlTest's current executable already refreshes its direct-window shortcuts to `Win`. The new regression compiles with zero warnings/errors and exercises window, nested-control, and template-owned commands before initial connection and across `Win`, `Command`, and `Super` renderer configurations.

# PROPOSALS

- No.1 Notify components throughout the window composition tree and use themed labels

## No.1 Notify components throughout the window composition tree and use themed labels

The renderer sends the correct canonical name, and `GuiRemoteController::OnControllerConnect` publishes the environment change. `GuiWindow::EnvironmentChanged` currently visits only its own components. RemoteProtocolTest installs its commands on the nested Home tab, so their `DescriptionChanged` notifications never reach the bound labels. Extend the existing ownership-based notification to every instance root in the window's composition tree, including associated controls and templates. Detached commands remain unnotified.

### CODE CHANGE

- Move component notification into `GuiInstanceRootObject::InvokeEnvironmentChanged`, and traverse the composition tree from `GuiWindow::EnvironmentChanged` after updating the display font.
- Replace both raw mouse `SolidLabel` elements with themed `Label` controls and attach the unified handlers to `att.BoundsComposition-set`.
- Change the local Super shortcut and dialog text to `Ctrl+Alt+Win+Q` in both resources. Strengthen SOP checks for exact renderer-localized labels, themed mouse labels, and renderer replacement.
- Retain protocol schema and reflected API signatures; regenerate application resources and required metadata using the repository tools.

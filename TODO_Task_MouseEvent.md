- Remake mouse message to accept mouse 4 and 5 buttons.
  - Same events for different buttons merge into one with extra field in argument (to the non-native argument type), e.g., `button` for mouse buttons.
  - Update remote protocol to respond to mouse 4 and 5.
- Allow WIN/COMMAND/SUPER in mouse/keyboard events
  - Fix all mapping code (including remote protocol)
    - Try search `.shift` and `->shift` to locate all places to modify.
    - For example `NativeWindowKeyInfo` has `bool ctrl\shift\alt`, `bool osSuper` will be added here.
    - Everywhere where ctrl\shift\alt is listed together there will be needed `osSuper` if related to this task.
  - Converting shortcut key to different label per OS, but any OS should accept names from other platform.
    - Displayed shortcut key would use COMMAND on macOS, WIN on Windows, SUPER on Linux for the same key.
    - Setting COMMAND/WIN/SUPER works on all platforms.
- Update the shortcut key test tab in `FullControlTest` to reflect key changes and mouse changes.
  - Adding a label for clicking.
  - Use `WIN` and `COMMAND` on the local/global shortcut keys.
  - Update this also to `RemoteProtocolTest`.
  - Update `DebugRemoteProtocolSop.md`.
- Test cases might be affected.
- Release GacUI to Release, call `../Tools/Tools/Build.ps1 -Project Release` and fix all build breaks, mostly xml files are affected.
- Fill the `Breaking Changes` in ToDo/1.4.1.1.md with bullet points using very simple words to describe the change, following the style in Release repo's release notes. You will find many examples of new features, breaking changes, fixed bugs, etc.

## API Design and Breaking Changes

### Mouse4 and Mouse5

- Representing mouse buttons:
  - There are two layers of event arguments, one is `WindowMouseInfo_` which must be frozen.
  - Add an enum class `NativeMouseButton` above `WindowMouseInfo_`
    - It has values `Left`, `Middle`, `Right`, `Mouse4`, `Mouse5`.
    - Use it as a field named `button` in `GuiMouseEventArgs`, the default value is `Left`.
- `mouseDown` and other events:
  - The complete list would be `mouseDown`, `mouseUp`, `mouseDoubleClick`.
  - Remove `(left|middle|right)Button(Down|Up|DoubleClick)` from `GuiGraphicsEventReceiver`, add the simplified events above instead.
  - Note that existing members in `INativeWindowListener` should be updated to `Mouse(Down|Up|DoubleClick)`, adding `NativeMouseButton` as the first argument, because `NativeWindowMouseInfo` is not changed.
- Reflection and XML Compiler
  - Event are registered in `BEGIN_CLASS_MEMBER(GuiGraphicsComposition)`.

### FullControlTest

The work is about the `Window Manager` tab
- `Ctrl+Q` is kept, add a `Ctrl+Win+Q`, and `Ctrl+Shift+Alt+Q` will be replaced by `Ctrl+Shift+Alt+Command+Q`:
- But when rendering on Windows, all needs to show `Win`. This relies on the resource service to translate key names, where the windows implementation always return `Win`.
- Below the 3 labels add another one, when button(Down|Up) happens, shows "(Left|Middle|Right|Mouse4|Mouse5) button (down|up)!"

### RemoteProtocolTest

The same change applied to `FullControlTest` will be applied to the main tab.

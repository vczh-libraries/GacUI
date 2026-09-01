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

## DETAILS

### Event model and propagation

- Keep `WindowMouseInfo_` binary- and source-compatible: do not add a button state or `osSuper` field to it.
- Add `NativeMouseButton` to the basic reflected type list and register all five enum values. Add `button = NativeMouseButton::Left` and `osSuper = false` to `GuiMouseEventArgs`; `GuiItemMouseEventArgs` and `GuiNodeMouseEventArgs` inherit both fields.
- Replace the nine button-specific callbacks on `INativeWindowListener` with `MouseDown`, `MouseUp`, and `MouseDoubleClick`. The button is the first argument. Carry `osSuper` separately beside the frozen `NativeWindowMouseInfo` for these callbacks and for `MouseMoving`, `HorizontalWheel`, and `VerticalWheel` so every emitted `GuiMouseEventArgs` receives the modifier state captured with the native event.
- Do not query `INativeInputService` from `GuiGraphicsHost` for every mouse event. That would add a synchronous renderer round trip in remote mode and could observe a later key state. Capture and transport the state with the event instead.
- Preserve the existing double-click sequence: the second press raises `mouseDown` immediately before `mouseDoubleClick`, followed later by `mouseUp`.
- Update every listener implementation, adapter, test listener, and call site, including the default implementation, `GuiGraphicsHost`, `GuiHostedController`, the Windows provider, remote core and renderer, shared automation services, GacGen stubs, and unit-test protocol helpers.
- When migrating handlers that previously subscribed to only a left-, middle-, or right-button event, explicitly filter `arguments.button`. Buttons, splitters, scrollbars, text selection, hyperlinks, menus, list/data-grid behaviors, and tree expansion must not begin responding to unrelated buttons.
- Keep the existing button-specific `GuiListControl` Item events and `GuiVirtualTreeListControl` Node events. The explicit API-design section limits event removal to `GuiGraphicsEventReceiver`; adapt the item/node bridges by filtering the unified composition event back to Left/Middle/Right. Mouse4/Mouse5 remain available from the underlying composition event without adding 18 more breaking event removals.

### Native Windows behavior and capture

- Map `WM_XBUTTONDOWN`, `WM_XBUTTONUP`, and `WM_XBUTTONDBLCLK`: use the low word as client key flags and `GET_XBUTTON_WPARAM` for `XBUTTON1`/`XBUTTON2`, mapping them to `Mouse4`/`Mouse5`. For `WM_NCXBUTTON*`, decode the low-word hit-test value separately from the high-word X button instead of passing the whole `wParam` through the existing non-client conversion. Add client and non-client X-button downs to the same popup activation/closing paths as other button downs, keep caption min/max/close actions left-button-only, and explicitly return `TRUE` for a handled X-button message.
- Populate `osSuper` for mouse, key, and char events from the left/right Windows keys. Add it to `NativeWindowKeyInfo` and `NativeWindowCharInfo` with a default value of `false`.
- Stop relying only on `NativeWindowMouseInfo.left`, `.middle`, and `.right` for capture. Track all five buttons in `GuiGraphicsHost`: Mouse4/Mouse5 must start capture, a double-click must count as its second down, and capture must remain until every held button is released. Preserve the existing rule that non-client input does not start client capture. Reset tracking when the captured composition is disconnected/deleted and when the host is detached if that path owns capture cleanup; a remote controller disconnect must continue preserving logical capture for replay on reconnect.

### Modifier and shortcut APIs

- Add `osSuper` after `alt` and before the non-modifier key throughout the shortcut stack: `IGuiShortcutKeyManager`, `GuiShortcutKeyManager`, `GuiShortcutKeyItem`, `GuiToolstripCommand::ShortcutBuilder`, application global-shortcut management, `INativeInputService::RegisterGlobalShortcutKey`, all implementations/stubs, reflection signatures, and remote hot-key identity and payloads.
- Include `osSuper` in shortcut equality, duplicate/collision checks, matching, reconnect replay, and global shortcut registration. On Windows, map it to `MOD_WIN`.
- Audit modifier-sensitive behavior, not just structs and signatures. Searches for `.shift`, `->shift`, and groups of `ctrl`/`shift`/`alt` should find guards where `osSuper` must be considered so a Super chord is not treated as an unmodified shortcut/navigation key.
- The `SHORTCUT` parser accepts `Win`, `Command`, and `Super` as aliases for one `osSuper` flag on every platform. The aliases affect parsing only; display always uses the active platform's canonical label.
- Add a resource-service API dedicated to the canonical Super-key label. Windows returns `Win`, macOS implementations return `Command`, and Linux implementations return `Super`. Implement or delegate it in Windows, Hosted, GacGen, Remote, test substitutions, and reflection.
- Add the canonical label to `ControllerGlobalConfig`. A remote renderer fills it from its resource service, and the remote core uses it when `GuiShortcutKeyItem::GetName()` renders shortcut text. This ensures the displayed name follows the renderer OS instead of the core build OS.
- Give the remote resource service a nonempty platform fallback before the first renderer connects. On initial connection and renderer replacement, invalidate already-rendered shortcut names through the existing environment-change path and raise the command/description notification needed by bound labels and toolstrip items. A changed canonical label must refresh existing UI without reconstructing the shortcut objects.

### Remote protocol, reflection, and generated artifacts

- In `Protocol_IO.txt`, map `IOMouseButton` to `NativeMouseButton`, add `Mouse4` and `Mouse5`, add `osSuper` to `IOKeyInfo`, `IOCharInfo`, and `GlobalShortcutKey`, and carry mouse `osSuper` in non-native protocol wrappers for button, move, and wheel events.
- Forward the unified callbacks directly in `GuiRemoteRendererSingle` and `GuiRemoteEvents`; remove the Left/Middle/Right dispatch switches. Include `osSuper` in renderer-side global registration and in the core's hot-key storage/replay.
- Update `Protocol_Controller.txt` for the canonical Super-key label. Treat all protocol changes as wire-breaking. Downstream native-renderer and GacJS ports scheduled in `ToDo/1.4.1.1.md` must update to the same schema later; compatibility with their old schema is not a completion requirement for this GacUI task.
- Run `Metadata_UpdateProtocol` to regenerate `Source/PlatformProviders/Remote/Protocol/Generated/GuiRemoteProtocolSchema.h`, `.cpp`, and `Protocol/Metadata/Protocols.json`; never edit generated protocol files directly.
- Register `NativeMouseButton`, the new event-argument and key/char fields, the unified events, resource-service method, and all changed shortcut/input signatures in reflection. Update the reflected composition events while keeping the existing item/node event registrations.
- Update XML source resources first, then use `GacUI_Compiler` to regenerate protected FullControlTest, RemoteProtocolTest, fake-dialog, snapshot-viewer, and other affected outputs. Do not directly edit any protected generated folder listed in `Project.md`.
- Update maintained manuals and knowledge-base pages that teach the removed event names or old native/remote input flow.

### Automation, tests, samples, and release notes

- Extend `GuiSharedAutomationService` with `Mouse4`/`Mouse5` state and the same Down/Up/Click/DbClick command forms as the first three buttons. Mouse and wheel modifier syntax accepts the Super aliases and forwards the captured `osSuper` state.
- Extend `UnitTestRemoteProtocol_IOCommands` in `GuiUnitTestProtocol_IOCommands.h` with Mouse4/Mouse5 helpers and held-button state, and add `osSuper` to key, char, mouse, and wheel helpers.
- In both test applications, create two local shortcuts (`Ctrl+Q` and `Ctrl+Win+Q`) and one global shortcut (`Ctrl+Shift+Alt+Command+Q`) with separate result dialogs. Bind all three visible labels to the shortcut objects' rendered names rather than hard-coding platform text.
- On Windows the two Super shortcuts display `Win` even though one builder uses `Win` and the other uses `Command`. Add the fourth label and update it from `mouseDown`/`mouseUp` using `arguments.button` and the exact text required by the task.
- Add the equivalent controls and handlers to the RemoteProtocolTest main tab. Expand the `/FCT` Window Manager and `/RPT` Home sections in `DebugRemoteProtocolSop.md` with the exact shortcut-label, activation, dialog, and five-button down/up expectations.
- Add simple bullets under `## Breaking Changes` in `ToDo/1.4.1.1.md` for the consolidated mouse events and `button` field, changed native-listener callbacks, new `osSuper` parameters/fields, and the lockstep remote-protocol update.
- Regenerate GacUI's release files before updating the sibling Release repository. Run the Tools build with `-Project GacUI`, then run the requested `-Project Release`; fix every compilation, XML, protocol, and release verification failure. Commit and push both repositories when both contain intended changes.

## VERIFICATION

### Focused unit and protocol coverage

- Preserve the existing Left/Middle/Right cases in `TestRemote_GraphicsHost_Mouse.cpp`, add Mouse4/Mouse5 down/up/double-click routing, and assert the exact `arguments.button`. Assert bubbling/order once for the consolidated event and verify the full `down`, `up`, `down`, `double-click`, `up` sequence. `StartRemoteControllerTest` already wraps these events with `JsonProtocol`, so these cases also cover serialization.
- Add one extended-button-only capture case and one ordinary/extended mixed case. Verify capture begins on an extended-button down, obeys the existing non-client guard, and is not released until the final held button is up.
- Update list/tree bridge tests to verify their existing Left/Middle/Right Item/Node events remain unchanged after the composition-event consolidation, while Mouse4/Mouse5 remains observable at the composition layer.
- Add focused `osSuper = true` cases at each distinct conversion boundary: Windows/native construction, Hosted forwarding, shared automation, unit-test helpers, and remote JSON forwarding. Existing cases cover the false/default path; include representative button, move, and wheel events without creating a full Cartesian matrix.
- Extend `TestRemote_GraphicsHost_KeyShortcut.cpp` and parser/toolstrip tests for positive and negative Super matching, duplicate identity, all three parser aliases, canonical Windows display, global-shortcut payloads, and disconnect/reconnect replay. Simulate canonical-label changes across renderer connections and verify existing labels/commands refresh without recreation.
- Use existing reflection/metadata assertions to verify `NativeMouseButton`, `button`, every `osSuper` field, the resource-service and shortcut signatures, and the consolidated composition events. Add bespoke reflection cases only if the existing metadata checks do not expose a changed surface.
- Run automation command tests for Mouse4/Mouse5 Down, Up, Click, and DbClick, and for `Win`, `Command`, and `Super` modifier spellings.

### Generation and repository tests

- Run `Metadata_UpdateProtocol` after changing the protocol sources and inspect all regenerated schema/metadata diffs for the intended enum values and fields.
- Generate current reflection metadata before compiling changed XML: build Debug Win32 and run `Metadata_Generate` Debug Win32; build Debug x64, run `Metadata_Generate` Debug x64, and run `Metadata_Test` Debug x64.
- Run `GacUI_Compiler` after updating the XML resources. Confirm both x86 and x64 FullControlTest/RemoteProtocolTest outputs are regenerated and that `git status` contains no `*.UI.errors.txt`. Because executing the compiler triggers the metadata workflow in `Project.md`, repeat the required Win32/x64 metadata generation and x64 metadata test after generation.
- Finish with a Debug x64 solution build and the complete `UnitTest` project through the repository PowerShell wrappers. Require `Build.log` to report success with zero warnings/errors, `Execute.log` to report every selected file/case passed, and no memory-leak dump after the summary.
- Search maintained source/XML/docs for obsolete consolidated native/composition event names and for modifier groups that still omit `osSuper`. Inspect the matches rather than changing historical or intentionally retained APIs mechanically.

### End-to-end Windows behavior

- Run FullControlTest locally with `GacUI_Host` so input travels through the Windows native provider. Run RemoteProtocolTest through `RemotingTest_Core` plus `RemotingTest_Rendering_Win32`, following the native-renderer guide and the updated SOP. Exercise each transport required by that guide so the schema works through every channel.
- In each updated tab, require three shortcut labels and the mouse result label. On Windows require both Super-containing shortcuts to render `Win`, activate `Ctrl+Q` and `Ctrl+Win+Q`, and verify their distinct local dialogs.
- Activate `Ctrl+Shift+Alt+Win+Q` through the real Windows global-hot-key path and verify the global dialog. Directly injecting a key event into a listener is not sufficient to validate `RegisterHotKey`/`MOD_WIN`.
- Send native XBUTTON1/XBUTTON2 input (or use a physical extended-button mouse) over the new label in both the local app and the Windows renderer. Require exact Mouse4/Mouse5 down/up text, and also confirm Left/Middle/Right remain correct. Automation-only injection is not sufficient to validate the Windows `WM_XBUTTON*` mapping.
- Re-run the relevant SOP operations after renderer replacement and confirm shortcut registration/display and input handling remain stable without a fatal error or disconnect.

### Release verification

- Set `UseMultiToolTask=true`. From the expected sibling-repository layout, run `C:/Code/VczhLibraries/Tools/Tools/Build.ps1 -Project GacUI` and then `C:/Code/VczhLibraries/Tools/Tools/Build.ps1 -Project Release` using the required PowerShell invocation form. Fix every compilation, XML, and release-verification failure.
- Inspect GacUI's regenerated `Release/GacUI*` files and the Release repository's copied GacUI sources, FullControlTest tutorial resources, remote-protocol TXT/JSON metadata, and build outputs. The diffs must contain the new API/protocol and no unrelated generated churn.
- `-Project Release` ends with a repository audit that will report the intentional uncommitted generated changes. Do not mistake those dirty-status messages for build failures. Confirm the breaking-change bullets are simple and complete, run `git diff --check`, commit and push each affected repository, then rerun a clean audit if needed.

## REVIEW COMMENTS

In `GuiPointSequenceStartedEventArgs` there must be two additional fields default to true:
- `generateLeftButtonClick`
- `generateRightButtonClick`
If event subscriber doesn't explicitly set them to false, the left/right button click gesture will be activated like what they are now.
`IGuiPointGesture::OnPointSequenceStarted` should accept `GuiPointSequenceStartedEventArgs` instead of just `NativeWindowPointInfo`.
The other two functions also receive dedicated event args to keep them aligned, also making `IGuiPointGesture::OnPointAction` no need to have two arguments.
Add two remote protocol based unit test to verify each.

Also address a bug. When a tooltip is displaying and its parent window is hidden or closed, the tooltip should be closed first. It applies to all popup showing when the parent window/control is specified, it trackes when the specified window or the window of the specified control.

## DETAILS

### Point gesture arguments and click suppression

- The event arguments are declared in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h`. Add both fields as `bool` with in-class defaults of `true`, and register them as writable fields in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp`. Preserve Started's inheritance, borrowed `Points` list and lack of reflected constructors.
- Use the flags from the completed `pointSequenceStarted` dispatch, including ancestor handlers, when initializing recognizers for an accepted sequence. Latch the resulting options for that sequence and reset them from fresh arguments on the next sequence. Preserve the first-selected-composition rule and traversal through handled queries.
- Disable only the corresponding click recognizer. This does not reject point input, disable raw Action/End delivery, or suppress scrolling/manipulation. Physical mouse input and rejected-sequence mouse compatibility remain unchanged; these options govern accepted-sequence deferred clicks.
- Preserve existing recognizer priority and eligibility. Right click requires at least 500000 microseconds; left click has no duration minimum. Thus, for an otherwise eligible successful single-contact sequence:

  | Left enabled | Right enabled | Short tap | Long press |
  | --- | --- | --- | --- |
  | true | true | Left click | Right click |
  | false | true | No click | Right click |
  | true | false | Left click | Left click |
  | false | false | No click | No click |

  Disabling right click does not introduce a new maximum duration for left click. Existing movement, extra-contact, cancellation and selected-gesture exclusions still apply.
- In `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.h`, use these recognizer signatures and update every implementation/call site in the paired `.cpp` file:

  ```cpp
  virtual void OnPointSequenceStarted(const GuiPointSequenceStartedEventArgs& arguments) = 0;
  virtual void OnPointAction(const GuiPointEventArgs& arguments) = 0;
  virtual void OnPointSequenceEnded(const GuiPointSequenceEndedEventArgs& arguments) = 0;
  ```

  `OnPointAction` obtains its action kind from `arguments.eventType`. Reuse existing event-argument types; `IGuiPointGesture` remains internal, and native listener signatures and remote wire messages need no change.
- Adapt `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp` and `GuiPointActionManager` without losing their native samples. Event-argument positions are logical and composition-relative; they cannot be passed to helpers expecting native client coordinates. Started finishes bubbling with coordinates relative to the last receiver, so prepare recognizer arguments relative to the accepted composition. Retain canonical screen baselines and the final Up sample even though Up has already emptied its contact slot. Reproject with current DPI/window/composition offsets as before.
- Preserve callback order: establish capture before recognizer Started, deliver raw Action before recognition, and run recognizer End before public End. Construct the End arguments early enough for recognition without moving public End ahead of it. `handled` remains independent of recognition. Borrowed arguments/lists must not be retained past callbacks; capture is cleared before synthesized mouse input.
- During implementation, update the maintained point-input guidance, canonically `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_PointInput.md`, and propagate it through instruction synchronization.

### Popups follow their owner window's lifetime

- Implement the ownership rule for `GuiPopup` and all its derived popups, including `GuiTooltip` and `GuiMenu`. This includes context menus, menu-bar dropdowns and cascading submenus. The relevant shared code is `<GacUI repo>/Source/Application/Controls/GuiWindowControls.h`, `<GacUI repo>/Source/Application/Controls/GuiWindowControls.cpp` and `<GacUI repo>/Source/Application/Controls/GuiApplication.cpp`; menu behavior lives in `<GacUI repo>/Source/Controls/ToolstripPackage/GuiMenuControls.h` and `<GacUI repo>/Source/Controls/ToolstripPackage/GuiMenuControls.cpp`.
- Reuse the existing owner/child relationships before introducing another ownership registry. In `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedWindowManager.h`, `hosted_window_manager::Window<T>` has `parent` and `children`; `GuiHostedWindow::SetParent` maintains that tree. The Windows implementation in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp` has `parentWindow`, `childWindows` and the existing `ClosePopupsOf` traversal. Both modal windows and control-relative popups set native parent relationships. `GuiWindow::ShowModalRecord` stores the modal chain's origin/current endpoints, not the complete popup inventory; `GuiApplication::openingPopups` is a flat list. Keep these roles distinct while reusing their lifecycle hooks.
- Hosted visibility changes currently affect only the selected window, and `WindowManager<T>::UnregisterWindow` reparents surviving children to the removed window's parent. Dismiss owned popups before hiding the owner or losing these relationships, so they cannot remain visible or be promoted to the main window. Select popup, tooltip and menu window modes; preserve existing normal/modal-window behavior rather than indiscriminately closing every child. Hosted parenting also attaches otherwise unowned windows to the main window, so structural ancestry alone must not invent a control-relative owner.
- All three `ShowPopup(GuiControl*, ...)` overloads identify an owner host/window. Passing a `GuiWindow` uses that same control API. Track the owner resolved for each successful show, replace the association when reused with another owner, and remove it when the popup closes or either side is destroyed. The screen-position overload `ShowPopup(NativePoint, INativeScreen*)` has no specified owner and must not retain an association from an earlier show.
- Owner hide or an accepted close must dismiss associated popups before the owner finishes hiding/closing. Include nested popups, closing descendants before their popup owner. Keep unrelated windows' popups open. A canceled owner close must not dismiss its popups; perform mandatory dismissal after cancellation has been resolved, not from an arbitrary `WindowClosing` subscriber.
- Preserve ordering across native, hosted and remote providers. `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerSetup.cpp` wraps the remote controller in `GuiHostedController`; remote subwindows therefore use the hosted parent/children tree. In `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedWindowProxy_NonMain.cpp`, both hide and close reach `SetVisible(false)`. Fixing only the underlying `GuiRemoteWindow` shutdown path would miss these owners. Windows `Hide` posts `WM_CLOSE`, so merely calling a child's `Close()` first is insufficient evidence that it has closed first. Ensure popup visibility/closure and detachment complete before the owner's corresponding completion notification; do not rely on a later idle frame or application shutdown to remove it.
- Route menu dismissal through the normal closure notifications so `IGuiMenuService::openingMenu`, menu-button `SubMenuOpening` state and ALT-host state are released. `GuiMenu::OnWindowClosed` already attempts to hide its opening submenu; coordinate with that path to avoid duplicate closure and to enforce descendant-first ordering. Dismissal must not execute a menu item. When the owner is a modal subwindow, preserve its existing completion callback, owner re-enabling and focus restoration.
- `GuiApplication::CloseTooltip()` intentionally defers closure while hovering over the tooltip. Owner-driven dismissal must bypass that deferral, clear the temporary tooltip content and invalidate owner-relative positioning before the owner can disappear. Preserve the existing owner-control-deletion fix in `GuiTooltip::SetTemporaryContentControl` and subsequent tooltip reuse.
- Maintain owner associations on the UI thread, using the existing application/window lifecycle machinery. Closing callbacks can mutate popup lists or detach handlers; traversal must tolerate that without stale pointers, duplicate closure or dependence on handler ordering. Do not add polling or make unrelated normal/modal windows part of popup dismissal.

## VERIFICATION

These are requirements for implementation; this review does not execute the task.

### Point gesture coverage

- Add the requested two remote-protocol rendering tests, one for each flag, using the existing resource/snapshot pattern in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp` and `PointProtocolInput` from `<GacUI repo>/Test/GacUISrc/UnitTest/TestPointInput.h`. Feed protocol point messages rather than calling recognizers directly, and specify durations with input timestamps instead of sleeps.
- In the left-flag test, establish the default tap activation, disable left generation in Started and verify no left mouse Down/Up or activation, then verify a long press still produces right activation. In the right-flag test, establish default long-press right activation, disable right generation and verify no right Down/Up while the enabled left recognizer still activates. Include both-disabled and a subsequent fresh default sequence within these two cases; assert raw Action/End and capture cleanup still occur.
- Use visible selection/toggle state or an actual context popup to make enabled/disabled behavior inspectable in snapshots, alongside exact event counts. Each case must record useful rendering frames. Group no-change negative assertions with the next meaningful action rather than adding empty frames or unrelated title changes.
- Exercise setting both fields from an interpreted XML/Workflow Started handler. Extend the ordinary reflection checks in `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` for both constructor defaults and writable-field readback; metadata-only assertions do not need snapshots.
- Reuse the existing low-level point tests in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp` to verify coordinate conversion, movement thresholds, final Up, recognizer priority, cancellation, extra contacts, rejected-sequence fallback and independent physical mouse state. Ensure the new dispatch passes ancestor-modified flags and resets options between sequences.

### Popup lifetime coverage

- Add a small set of remote-protocol rendering scenarios for a tooltip, an ordinary popup and a menu with a cascading submenu, each exercised with both `Hide()` and `Close()` on a secondary owner window. Reuse `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp`, `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Windows.cpp`, `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_Toolstrip_GuiToolstripMenu.cpp` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_Toolstrip_GuiToolstripMenuBar_Cascade.cpp` for their resource and interaction patterns. Keep the separate main window alive through the result frame, so final application teardown cannot conceal an orphan popup.
- For each scenario, render the subwindow and its visible tooltip/popup/menu first, invoke the subwindow's `Hide()` or `Close()` directly from the frame callback, then record the surviving main window with the subwindow and all its popup descendants absent. Do not click outside the popup or on the owner's close button for this regression: ordinary outside-click dismissal could close it before the owner-lifetime bug is exercised. Do not manually hide/delete the popup during setup or before the result assertions. Use the native-close smoke check separately.
- Include hovering over the tooltip. Record popup/owner completion order and assert the tooltip has closed, its temporary content is detached and `GetTooltipOwner()` is null by owner completion. The later rendered frame proves the visual result; callback-time assertions establish that closure happened before the owner completed.
- Cover an ordinary popup through each control-relative overload, a window passed as owner, and a nested popup. For menus, render both a top-level context/menu-bar popup and its open submenu; assert both close, menu buttons report `GetSubMenuOpening() == false`, and no menu command executes. Reopen the owner and menu to verify normal interaction without stale menu-service or ALT state.
- Verify descendants close before their owner and an unrelated owner's popup survives. Include a canceled owner close as a negative control, and one modal subwindow owner to confirm popup dismissal preserves its existing modal callback and re-enables the original owner. Fold these checks into the focused scenarios rather than creating duplicate fixtures for each assertion.
- Verify reopening with a different owner, returning to the unowned screen-position overload, and destroying the popup before its former owner. Closing the old owner must not affect the reassigned popup or invoke a stale callback. Retain the existing visible-tooltip-owner-deletion and surviving-owner-reuse regression.
- Check ordinary Windows and hosted windows through the repository's application execution/automation procedure, including a native window close request. Remote snapshots alone cannot prove Windows posted-message ordering. Record these results separately from the protocol tests.

### Build and generated artifacts

- Follow `<GacUI repo>/.github/Guidelines/Building.md` and `<GacUI repo>/.github/Guidelines/Running-UnitTest.md`. Build the solution with the repository wrapper; run the affected point, reflection, tooltip, window and menu test files through `copilotExecute.ps1`, preserving relevant existing filters. If the shared hosted window manager changes, also run `<GacUI repo>/Test/GacUISrc/UnitTest/TestHostedWindowManager.cpp` for ownership, visibility and ordering regressions. Require passing summaries and no Debug memory-leak dump.
- Because reflection registration changes, follow `<GacUI repo>/Project.md`: build Debug Win32 and run `Metadata_Generate`, then build Debug x64 and run `Metadata_Generate` and `Metadata_Test`. Run `GacUI_Compiler` only when its authored resource inputs change, with the prescribed subsequent metadata regeneration. Never hand-edit generated files.
- Inspect generated snapshots for the intended visible changes and closure order. Keep authored changes, generated artifacts and unit-test snapshots in their required separate commits when implementing the task.

## REVIEW COMMENTS

No unresolved review comments. The implementation choices and verification requirements are recorded above.

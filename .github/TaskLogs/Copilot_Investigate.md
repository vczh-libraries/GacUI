# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

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

# UPDATES

## UPDATE

The previous work needs a few improvements:

- I think there is no need to let `LeftClickGesture` inherits from `RightClickGesture`, using `minimumDuration` to distinguish them does not look good although it is technically accurate. I would like you to split them, aka making `LeftClickGesture` repeat the implementation instead of inheriting from `RightClickGesture`, and let `RightClickGesture` no need to implement both. The implementation is very short, not sharing code between them is totally OK.
- The tooltip issue is a general GacUI issue, I don't think it is Windows specific, I would like you to reconsider the change to [WinNativeWindow.cpp](Source/PlatformProviders/Windows/WinNativeWindow.cpp) , and see if there is any replacement implementation that could be done in GacUI. It is totally find if the answer is still no, and in this case you might need to take a look at `wGac` and `iGac` and see if any patch needs to applied to them too.
  - I am also questioning the reason to fix `WindoesForm::Hide` in this way, what is the issue of sending `WM_CLOSE` to a invisible window?
  - I am also questioning the reason to fix `WindowsForm::ClosePopupsOf` as in GuiWindow there is also storing parent-child popup relationship, if the patch could be applied to GacUI shared code directly, then it would actually simplify any native platform provider implementation.
  - Another reason is that, if the fix could make remote protocol based unit test pass, why it is needed to fix [WinNativeWindow.cpp](Source/PlatformProviders/Windows/WinNativeWindow.cpp) ?
  - Please consider above question carefully before making your decision.
- In `UnitTest.vcxproj` there is a `LinuxSkin` solution explorer folder, there are 6 files that only available for linux
  - You need to delete that solution explorer folder as well as detach those files from the `UnitTest` project
  - These files could be added to the dedicated `vmake` files instead of changing `UnitTest` itself.
# TEST [CONFIRMED]

- Add protocol rendering coverage for independently suppressing left and right deferred clicks, reflected defaults and writes, and owner-driven tooltip/popup/menu dismissal. Preserve the detailed success criteria in the problem description above.
- Reproduce owner dismissal on a secondary window while the main window remains visible; assert descendant closure and tooltip detachment at owner completion.

## Reproduction result

- Debug x64 solution build passed with zero warnings and errors. The new HideTooltipWindowOwner protocol case fails at the owner WindowClosed callback: closed == 1 is false. The hovered tooltip has not sent its closure notification before its secondary owner closes. The main window is still alive.

# PROPOSALS

- No.1 Latch Started options and dismiss explicitly owned popups at accepted lifecycle boundaries [DENIED]
- No.2 Separate click recognizers and platform inventories; verify native popup lifecycle boundaries

## No.1 Latch Started options and dismiss explicitly owned popups at accepted lifecycle boundaries [DENIED]

Use the completed Started query to initialize internal recognizers with dedicated event arguments relative to the accepted composition. Keep canonical native screen samples in the manager, including the current sample independently of the sparse contact list, so final Up and current DPI/layout reprojection retain existing behavior. Only the respective click candidate uses its flag; recognizer priority and raw event ordering remain unchanged.

Reuse native parent relationships and the application's existing opening-popup list. Record the explicitly resolved owner on each popup, clearing it on close, destruction and unowned show. Dismiss matching popups recursively from the accepted AfterClosing path before WindowReadyToClose, and before native detachment or owner destruction. Popup closure uses ordinary notifications, including menu service cleanup and tooltip content detachment. Windows popup dismissal must complete synchronously, while normal window close requests keep their posted-message behavior. Audit the existing Windows popup traversal for mutations caused by synchronous closure. No second ownership registry, polling, or modal-chain repurposing is needed.

### CODE CHANGE

Implemented: add reflected Started flags; pass dedicated arguments to recognizers after the completed Started query; retain native screen samples independently of logical event coordinates and contact slots. Add protocol and reflection tests for both suppression flags and the existing raw-input/capture contract.

Use the existing opening-popup list and each popup's native parent, with one boolean distinguishing explicitly owned shows from hosted structural parenting. Dismiss descendants from accepted close and destruction/detachment hooks. Complete Windows popup Hide synchronously, snapshot native child traversal with disposal flags, and clear popup positioning and tooltip content before public closure callbacks. Normal/modal window requests retain their existing behavior.

Add secondary-owner tooltip, ordinary/nested popup, context menu and menu-bar cascade scenarios, including canceled close, modal completion, reassignment, unowned reuse and destruction. Keep the native/hosted Playground fixture in XML/Workflow for manual verification. Move context-menu lifetime snapshots to the sibling GuiToolstripMenuOwner directory: the original GuiToolstripMenu snapshot writer removes non-frame files from its own frame directory.

Updated canonical point-input and owner-lifetime guidance and propagated it through instruction synchronization. Generated metadata and unit-test snapshots are retained separately from authored changes.

### DENIED BY USER

The continuation rejects using right-click inheritance and a duration sentinel for left clicks, and requests an audit of the native popup changes and Linux-only inventory. The validation below is historical evidence for the previous implementation, not acceptance of these design choices.

Owner dismissal is mandatory once the owner has accepted closing. A per-popup transient owner-dismissal flag is set before Hide and cleared before public Closed; the popup BeforeClosing override runs normal subscribers but clears cancellation for this path. Cancellation subscribers on both ordinary and nested test popups prove they cannot outlive an accepted owner close. Independent popup closes remain cancellable.

- Debug x64 solution build passed with zero warnings/errors. The ten selected test files passed all 158 cases, with no Debug leak dump. The existing point-coordinate, final-Up, cancellation, fallback and physical-mouse regressions are included.
- Ordinary Windows Playground checks passed all nine combinations of tooltip, ordinary popup and cascading menu with owner Hide, Close and native WM_CLOSE. Automation Controls confirmed visible setup and surviving main-window-only results; callback-time Workflow assertions required child closure and tooltip detachment before owner WindowReadyToClose. UIA Invoke avoided outside-click dismissal. Tooltip hover used automation MouseMove on the tooltip window.
- Hosted Playground checks passed the same nine combinations, using UIA Window.Close for the logical owner's close-request path. Both applications exited normally with code zero after UIA closed the main window. Native desktop coordinate input was unavailable in the locked session; automation, UIA and native messages remained functional.
- Snapshot render-tree inspection confirms tooltip/owner labels disappear while main-window labels remain; nested/owned popup labels disappear while the unrelated main-window popup remains; menu cascade items disappear, then ALT shortcut labels and the executed command appear after reuse. Point snapshots add the expected target/right/suppression checkmarks and return to the initial unchecked state after fresh defaults.
- After adding mandatory dismissal, the final Debug Win32 solution build passed with zero warnings/errors and Metadata_Generate exited zero. Repeated all eighteen native/hosted checks using the final Win32 Playground, now with a popup WindowClosing handler that always requests cancellation. All passed and both application wrappers exited zero. Instruction synchronization copied the canonical guidance from the Tools repository.
- The final Debug Win32 affected-file run passed 10/10 files and 158/158 cases with no Debug leak dump, including independent popup cancellation and mandatory dismissal of both popup levels. The renamed context-menu snapshots retain their JSON, command, diff and architecture files; render-tree inspection confirms cascade dismissal and normal command execution after reuse.
- The final Debug x64 solution build passed with zero warnings/errors. Metadata_Generate x64 and Metadata_Test x64 both exited zero, validating the core and complete metadata for both architectures after the prescribed Win32-then-x64 generation order.
- The final Debug x64 affected-file run passed 10/10 files and 158/158 cases with no Debug leak dump. Both architectures include the point, reflection, tooltip, window, menu, theme, gallery and remote-startup filters. Final snapshot outputs contain both architecture logs and the complete new context-menu cases. No shared hosted-window-manager or GacUI_Compiler input change was needed.

At the previous completion, No.1 was the only proposal and was retained as the final implementation; the continuation above supersedes that selection. Code review and authored-diff whitespace checks passed; generated allocation logs retain the snapshot writer's standard trailing blank line. The tests establish callback-time ordering as well as rendered results. After recording confirmation, the investigation baseline was restored and the selected proposal reapplied; the tracked patch hash remained identical to the verified patch.

## No.2 Separate click recognizers and platform inventories; verify native popup lifecycle boundaries

Implement independent left/right click recognizers. Each uses its own Started flag and fixed fallback button; only the right recognizer stores the starting timestamp and requires 500000 microseconds. Preserve native coordinate reprojection, priority and eligibility.

Move the six Linux skin adapter compile items from the Windows project/filter into `CPP_ADDS` in `<GacUI repo>/Test/Linux/UnitTest/vmake`. Keep the physical source files and architecture-selecting adapters unchanged.

Audit shared popup ownership against native lifecycle boundaries before deciding on platform changes. The existing shared AfterClosing hook owns descendant dismissal and tooltip/menu cleanup. INativeWindow currently exposes only Hide for hiding a reusable native popup; Windows queues WM_CLOSE while hosted subwindows complete synchronously. A shared-only implementation would need an additional synchronous native API, an asynchronous accepted-close continuation contract, or premature synthetic closure/destruction. Evaluate those costs against retaining the small synchronous popup implementation. Independently audit hidden-window notifications and mutation of native outside-click traversal, and inspect the Wayland and Cocoa equivalents.

### CODE CHANGE

Implemented independent click recognizers in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.cpp`; removed the six skin adapters and LinuxSkin filter from the UnitTest project and registered them explicitly in its Linux `vmake`. Retained the existing shared owner-lifetime implementation and Windows native fixes after auditing the notification and message paths. Patched `<wGac repo>/WGac/WGacNativeWindow.h` and `.cpp` to unify accepted hide/close and forced compositor dismissal, and `<iGac repo>/Mac/NativeWindow/OSX/CocoaWindow.h` and `.mm` to guard repeated popup hide and snapshot mutable native children with disposal tokens. Updated both Wayland README languages and the Cocoa window-provider guide.
### Native provider audit and implementation decision

- Keep ownership and mandatory dismissal in shared GuiApplication/GuiControlHost/GuiPopup. GuiWindow::ShowModalRecord is a modal-chain record, not a popup-child registry; openingPopups plus the native parent relationship already supplies the reusable popup inventory. There is no native visibility setter or synchronous reusable-popup operation besides Hide. Delaying an accepted owner close would require a new continuation/cancellation protocol across every provider; destroying and recreating a popup would break its reusable native-window lifecycle. Neither is a simpler replacement for synchronous native popup Hide.
- Keep the Windows synchronous popup branch. A posted WM_CLOSE returns before BeforeClosing, AfterClosing, visibility change and Closed. Consequently the owner's AfterClosing would immediately publish WindowReadyToClose while its tooltip is still visible. Remote subwindows use GuiHostedWindow::Hide and NonMainProxy, which complete SetVisible(false) synchronously, so those tests cannot exercise this Windows queue boundary.
- Sending WM_CLOSE to an invisible window is legal. The visibility guard is about GacUI callbacks: the WM_CLOSE handler always invokes BeforeClosing and accepted AfterClosing even when the window was already hidden, but Closed comes from a visibility transition. Reentrant menu/ALT dismissal can therefore repeat Closing/Ready notifications without another actual close. Keep the guard scoped to popup modes; normal-window behavior is unchanged.
- Windows ClosePopupsOf implements native outside-click/application-deactivation dismissal, including native users without GuiApplication. It is distinct from explicit owner-lifetime dismissal. Shared Closed now detaches the native parent synchronously, invalidating the old live-index traversal and potentially skipping siblings. Descendant-first snapshots with disposal flags keep this native traversal valid; moving it into GuiWindow would introduce a provider-to-application dependency and lose raw-native behavior.
- Cocoa already hides synchronously, but its live child traversal has the same mutation problem and repeated hidden-popup notifications. Mirror the guarded, descendant-first traversal there using a disposal token.
- Wayland Hide(false) currently tears down roles and emits Closed without BeforeClosing/AfterClosing. Route requested hide and close through one accepted-close path, keeping low-level role teardown separate for native forced dismissal. Compositor popup_done and ancestor role teardown are mandatory and must still emit AfterClosing before hiding. Preserve descendant-first xdg role destruction and avoid invoking requested-close callbacks twice.

The current Windows host has no installed WSL distribution and no macOS runtime. Port changes can be source-reviewed here; their official import/sync/build and native smoke checks remain platform verification requirements, separate from the Windows result.
### Verification

- The Debug x64 solution build through the repository wrapper passed with zero warnings and zero errors.
- The unchanged ten-file regression filter passed 158/158 cases and 10/10 files, covering point suppression, priority, coordinates, final Up, cancellation, fallback, reflection, popup/tooltip ownership, menus, gallery, themes and remote startup. No Debug memory-leak dump followed the summary. The run produced no tracked snapshot differences.
- The native Windows Playground fixture passed all nine tooltip/ordinary-popup/cascading-menu combinations with owner Hide, Close and native WM_CLOSE. Automation Controls established visible setup and surviving-main-only output; UIA Invoke triggered actions without an outside click. Tooltip hover used automation IO on the tooltip. Fixture assertions executed before owner WindowReadyToClose, including mandatory popup cancellation override and descendant/menu completion ordering. The native application wrapper exited zero.
- Hosted Windows Playground passed the same nine combinations using UIA WindowPattern.Close for the logical owner close-request path, with automation Controls and hovered-tooltip IO. The hosted wrapper exited zero. The temporary test driver was corrected to read hosted subWindowsInZOrder; that initial driver-schema mismatch was not an application failure. Playground debugger arguments were restored.
- Static inventory validation confirms that all six adapter files exist, appear exactly once in CPP_ADDS, and are absent from both MSBuild files. Both XML files parse, and LinuxSkin no longer exists. The Linux UnitTest vmake is the only consumer of UnitTest.vcxproj. Existing generated vmake.txt already lists those same six adapters; generated files were not edited manually.
- Both native port build wrappers were attempted through Git Bash: wGac stopped at missing cmake; iGac stopped at missing cmake/make. WSL is not installed and no macOS SDK/runtime is available. The import/sync/build sequence and native Linux/macOS verification cannot be completed in this environment. Port patches are source-reviewed and retained for backup, not marked runtime-verified. Current release/import snapshots also predate CloseOwnedPopups; the normal GacUI release regeneration and port import/sync steps are required before validating the complete combined fix on each target.
- No reflected declaration or registration changed in this continuation, so metadata regeneration is not required.

### Remaining platform verification

On Linux and macOS, regenerate the owning GacUI release, then run the port's import.sh, syncProj.sh and build.sh in that order. Verify native and hosted owner Hide/Close/native-close with hovered tooltip, ordinary/nested popup and menu cascade while an independent main window remains alive. Assert descendant Closed precedes owner Ready/Closed, preserve canceled independent closes, test mandatory compositor/ancestor dismissal and ensure unrelated popups survive. Exercise popup reuse and outside-click/app-deactivation closure with multiple children, including callback-driven child deletion. Rebuild Linux UnitTest through its wrapper to regenerate inventories from the new CPP_ADDS registration.

No.2 remains without a global CONFIRMED marker until the native port changes receive their target-platform builds and tests. Windows implementation and regression results above are confirmed separately.
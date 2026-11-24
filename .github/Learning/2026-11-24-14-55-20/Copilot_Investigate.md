# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

# Remove `GuiControl::PrepareForPropertyChange` and reject recursive window theme refreshes

Remove the control-wide pre-change callback introduced in `dc55be928`. Keep the Windows UI Automation (UIA) behavior of the window and its live descendants when a theme is rebuilt.

## Why the hook can go

When `BeforeRebuildingTheme` runs, the provider exposes the window as having no children and raises `ChildrenInvalidated` for structure listeners. It discards pending descendant notifications from the old tree. A destroyed template control has finished its lifetime. A client listening only to that element should receive no further `Name` changes from it. A replacement gets a new identity and its initial name is part of its initial state. There is no old-to-new name transition between two elements. A logical control that survives the rebuild in the same window can keep its identity. [Microsoft’s provider disconnection contract](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiadisconnectprovider)

A client can also subscribe to `Name` on the window with `TreeScope_Subtree`. That registration survives the rebuild. If a new label later changes from “Original label” to “New label”, the client should receive that change from the new label, even before its first tree walk. For a label whose accessible name is its text, `TextChanged` already signifies a real change. The provider can observe that event and report the current name. Controls with an explicit accessible name or a name supplied by another label must follow their actual name source. The native client callback takes the sender, property ID and new value. The provider-side `UiaRaiseAutomationPropertyChangedEvent` accepts old and new values. The provider can maintain its own snapshots for that call. [UIA event subscription scope](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-eventsforclients) · [Property-changed callback](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nf-uiautomationclient-iuiautomationpropertychangedeventhandler-handlepropertychangedevent)

The current implementation uses the hook to attach the provider at the last moment and take its first snapshot. Disabling it in the existing native theme test changed the label text but produced zero UIA `Name` events. Address this during removal. Give the provider a way to receive the first ordinary change on a live descendant covered by a window subscription. Use control events, structural attachment or event dispatch as appropriate. Seeding a snapshot from the new text and then comparing it with the same text loses the first event. Preserve the original performance requirement of zero per-edit subtree scans during rebuild and no eager full-tree scan at completion.

## Delete the API and all 39 calls

The table inventories the 35 calls in portable controls and the four calls in Windows UIA metadata setters at `3fc12ad1d`. Delete the calls, including receiver-qualified calls and conditions whose only body calls the hook. Preserve each setter’s existing value comparison, mutation order and ordinary event dispatch.

| File | Methods containing calls | Calls |
| --- | --- | ---: |
| `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp` | `GuiControl::UpdateVisuallyEnabled`, `OnGotFocus`, `OnLostFocus`, `GetNotifyEventArguments`, `SetVisible`, `SetText` | 6 |
| `<GacUI repo>/Source/Application/Controls/GuiWindowControls.cpp` | `GuiControlHost::SetText` before the native title write | 1 |
| `<GacUI repo>/Source/Controls/GuiButtonControls.cpp` | `GuiSelectableButton::SetGroupController`, `SetAutoSelection`, `SetSelected` | 3 |
| `<GacUI repo>/Source/Controls/GuiContainerControls.cpp` | `GuiTab::SetSelectedPage` in the empty-tab branch and on the tab, previous page and next page in the selection branch | 4 |
| `<GacUI repo>/Source/Controls/GuiDateTimeControls.cpp` | `GuiDatePicker::CommandExecutor::NotifyDateChanged`, `GuiDatePicker::SetDate` | 2 |
| `<GacUI repo>/Source/Controls/GuiScrollControls.cpp` | `GuiScroll::SetTotalSize`, `SetPageSize`, `SetPosition`, `SetSmallMove`, `SetBigMove` | 5 |
| `<GacUI repo>/Source/Controls/ListControlPackage/GuiDataGridControls.cpp` | `GuiVirtualDataGrid::NotifySelectCell` | 1 |
| `<GacUI repo>/Source/Controls/ListControlPackage/GuiListControls.cpp` | `GuiListControl::SetItemTemplate`, `SetArranger` and `GuiSelectableListControl::SetMultiSelect`, `SetSelected`, `SelectItemsByClick`, `ClearSelection` | 6 |
| `<GacUI repo>/Source/Controls/ListControlPackage/GuiListViewControls.cpp` | `GuiVirtualListView::SetView` | 1 |
| `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` | `GuiDocumentCommonInterface::EditTextInternal`, `SetDocument`, `SetEditMode` | 3 |
| `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp` | `GuiSinglelineTextBox::SetPasswordChar` | 1 |
| `<GacUI repo>/Source/Controls/ToolstripPackage/GuiMenuControls.cpp` | `GuiMenuButton::SetShortcutText`, `SetSubMenuOpening` | 2 |
| `<GacUI repo>/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.cpp` | `SetWindowsUIAutomationName`, `SetWindowsUIAutomationLabel`, `SetWindowsUIAutomationId`, `SetWindowsUIAutomationText` | 4 |

- [ ] Delete the public declaration and its documentation from `<GacUI repo>/Source/Application/Controls/GuiBasicControls.h` and the definition from `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp`. Remove the observer-installation comment in `GetNotifyEventArguments` and restore its simple construction of `GuiEventArgs`.
- [ ] Delete the `GuiWindow::controlPropertyChangeObserver` field (the actual spelling of the proposed `preparePropertyChangeObserver`), `SetControlPropertyChangeObserver`, its documentation, and the `friend class GuiControl` added for direct field access. The declaration and definition are in `<GacUI repo>/Source/Application/Controls/GuiWindowControls.h` and `<GacUI repo>/Source/Application/Controls/GuiWindowControls.cpp`.
- [ ] Delete the inline `GuiGraphicsHost::GetControlHost()` accessor from `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.h`. The deleted helper is its sole caller. The adjacent `SetCompositionBoundsObserver` is used by layout.

## Remove the Windows provider code that exists for this hook

Use `<GacUI repo>/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.h` and `<GacUI repo>/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp` for these changes.

- [ ] Delete `WindowsUIAutomationWindowState::observingChanges`. In `WindowsUIAutomationContext::ObserveWindowChanges`, remove the matching early return, flag assignment, callback installation and removal, and the lambda that calls `Control(control)`, `Observe(node)` and `UpdateProperties(node, false)` to seed an empty root snapshot. Remove its comments about observing a control just before mutation, including the stale wording in `EndRebuild`. Keep ordinary observation and snapshot comparison where they have live callers.
- [ ] Remove the corresponding property-observer cleanup in `WindowsUIAutomationNode::Retire` and the `observingChanges` predicate in `WindowsUIAutomationContext::Collect`. Give the remaining bounds teardown its own `observingBounds` check so the graphics host releases its callback on unsubscribe or retirement.
- [ ] Narrow `ObserveWindowChanges` to the bounds work it still performs. Account for each caller in `BindWindows`, `BeginRebuild`, `EndRebuild`, `RemoveSubscriptions`, `RefreshObservation` and `Collect`. Preserve geometry subscription changes, bounds capture and cleanup. `lazyDiscovery`, `observingBounds` and `boundsPending` have independent uses.
- [ ] Delete `WindowsUIAutomationNode::updatingProperties` and its check, set, reset and explanatory comment in `UpdateProperties`. They guard the recursion introduced by the callback when reading a native window title synchronizes `GuiControl::Text`. Exercise a title change on the surviving window.
- [ ] Keep provider-side handling of dependent properties on live controls. Examples are `UiaLabelTextChanged` and its reverse label registrations, `UpdateSelection` for virtual items, scroll-view notifications and `CaptureBounds` for geometry. The theme events, rebuild generations, retained-node retirement and the GDI listener factory used by the native and hosted tests also have independent consumers.

## Keep `GuiMenu::GetOwnerControl`

Git attributes this accessor to the same commit, `dc55be928`. Its sole production caller is `UiaParentWindow` in `<GacUI repo>/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp`. A closed menu has no native parent to follow. The accessor lets the provider find its owning window while that window rethemes the menu. Removing it would break that ancestry lookup even with all property observation disabled.

Keep the getter, its disposed-owner check and `GuiMenu::UpdateMenuService` in `<GacUI repo>/Source/Controls/ToolstripPackage/GuiMenuControls.h` and `<GacUI repo>/Source/Controls/ToolstripPackage/GuiMenuControls.cpp`. Keep the `GuiRibbonGroupMenu` override, its stored group and constructor argument in `<GacUI repo>/Source/Controls/ToolstripPackage/GuiRibbonControls.cpp`. The group remains the theme owner when its responsive dropdown button detaches.

## Reject recursive calls to `GuiWindow::RefreshThemes()`

A call to `RefreshThemes()` on the same window from inside its own refresh is a programming error. This replaces the existing rule that silently coalesces nested calls. Each window has its own guard. During `AfterRebuildingTheme`, the rebuilt controls are already available to UIA, and the outer call still owns the guard.

- [ ] In `<GacUI repo>/Source/Application/Controls/GuiWindowControls.h` and `<GacUI repo>/Source/Application/Controls/GuiWindowControls.cpp`, delete `GuiWindow::ThemeRebuildPhase`, its `themeRebuildPhase` field and `GetRebuildingTheme()`. Add a private per-window `bool refreshingTheme = false`. At entry, use `CHECK_ERROR(!refreshingTheme, L"GuiWindow::RefreshThemes() cannot be called recursively.")`, then set the flag before dispatching `BeforeRebuildingTheme`. Keep it set through template reconstruction and every `AfterRebuildingTheme` handler. Clear it after dispatch returns if the window still exists. Preserve the disposal checks around callbacks and update the method's API comment to document the error.
- [ ] Have the Windows UIA listener mark the affected window's subtree empty **before** the first public `BeforeRebuildingTheme` handler runs. Reopen it after template reconstruction and **before** the first public `AfterRebuildingTheme` handler runs. Record these boundaries even when the window has no element provider yet, without constructing one or walking the tree. A provider first acquired in a Before handler must see an empty root. An After handler must be able to query and change a rebuilt child while the reentry guard is still set.
- [ ] Use that listener-owned state for late attachment in `WindowsUIAutomationContext::Observe` and for the owning-window checks in `UiaRebuildingWindow` and `WindowsUIAutomationWindowState::ClosedWindow`. These reads are in `<GacUI repo>/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp` and `<GacUI repo>/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp`. Keep the existing window scope for owned menus and independent popups, and discard any outstanding entry when the window is destroyed.
- [ ] In `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Theme.cpp`, change the nested calls in both boundary handlers to `TEST_ERROR(base->RefreshThemes())`. Verify one ordered Before/After pair for the outer call, then verify a later refresh of the same window succeeds. Replace assertions on the deleted getter with observable behavior. In `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Theme_Windows.cpp`, keep the first-provider navigation check inside Before and the rebuilt label change inside After.

## Retire implementation-only assertions and verify the behavior

- [ ] No whole unit-test case calls the hook, installs the window callback or tests only that mechanism. In `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Theme_Windows.cpp`, retire the five assertions that inspect the private `GacUI.Windows.UIAutomation` property for absence (lines 117, 205, 221, 222 and 247 at `3fc12ad1d`). Keep the actual no-client-tree-walk setup and the event assertions around them. Keep `ThemeObservation` and its event probes for the mixed native fixture.
- [ ] Run the native theme case and hosted submenu case. Check that a retired child and text range become unavailable, a surviving window/control keeps its identity, and current controls produce the expected events. Cover the first label change, unchanged text, the native title, a name supplied by another label, a change from an `AfterRebuildingTheme` handler, document text and bounds. Keep the portable refresh-order tests and the hosted popup isolation checks in `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Theme.cpp` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Windows.cpp`.
- [ ] Use `-Scenario ThemeProperty` in `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1`. The external client in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` subscribes on the window, switches the palette through HTTP, and expects the first `ToggleState.On` event when the user clicks the rebuilt checkbox. Also keep the earlier two-client coverage of unvisited descendants in `<GacUI repo>/Test/UIA_Lifetime.cs`, which predates the hook. Run the existing `RawClockHttp` and `RawClockPassive` comparisons for the 15-second responsiveness requirement and the Windows unit suite as described in `<GacUI repo>/Project.md`.
- [ ] Verify that a source search finds no remaining references to `PrepareForPropertyChange`, `controlPropertyChangeObserver`, `SetControlPropertyChangeObserver`, the hook-only accessor and flags, `ThemeRebuildPhase`, `themeRebuildPhase` or `GetRebuildingTheme`. The removed APIs have no reflection registrations. The metadata from commit `00305bc1f` describes the two retained window theme events.

This task is about removing many constructions just added in a previous work. By doing this, my point is I don't want what those constructions benefits me, so do not invent anything to replace it.

# UPDATES

## UPDATE

let's do this following change

`GuiWindow::RefreshThemes` does not need to do `GetDisposedFlag`. I don't think a window will be disposed during a `RefreshThemes` call to itself. A window might dispose other owned windows but definitely not during their `RefreshThemes`.&#x20;

In `GuiGraphicsHost` turn `compositionBoundsObserver` and `SetCompositionBoundsObserver` into a public `GuiNotifyEvent CompositionCachedBoundsChanged`. It fires after a composition's `CachedBoundsChanged`, its sender will be the trigger so `ExecuteWithNewSender` is preferred. A public `GuiGraphicsHost::RaiseCompositionCachedBoundsChanged(sender)` will be used as a helper function. Make sure UIA no longer use `CachedBoundsChanged`.

# TEST [CONFIRMED]

Count the pre-change calls in the source, and exercise recursive refresh from both public theme handlers. The outer call must produce one ordered pair and accept a later refresh. Verify the native window and hosted submenu lifetime and event cases, both ThemeProperty clients, both five-switch RawClockHttp and RawClockPassive comparisons with the existing 15-second deadline, and the configured Windows unit suite.

The baseline has 39 calls across 13 implementation files. The Debug x64 solution built with zero warnings and errors. Its portable refresh-order case stopped at the recursive call in Before with "Expect an error but nothing occurred: base->RefreshThemes()".

For the host event, check that the local handler and two host listeners receive the changed composition and see the committed rectangle in that order. An unchanged layout must deliver zero additional events. For UIA, move a label's parent while keeping the label's local rectangle fixed. A client subscribed to the window must receive one bounding rectangle change before the first refresh and one after the refresh when the rebuilt parent moves.

# PROPOSALS

- No.1 Remove the pre-change API and attach ordinary UIA events during theme refresh [CONFIRMED]
- No.2 Publish composition bounds changes on the graphics host and simplify window refresh [CONFIRMED]

## No.1 Remove the pre-change API and attach ordinary UIA events during theme refresh

Delete the hook and its callers. Notify the Windows listener directly at the two window theme boundaries. The existing theme traversal visits controls with both default and explicit templates. Use those visits to attach ordinary UIA control events for existing window subscriptions, preserving the actual name source and bounds notifications. Have the window reject recursion until its public After handlers finish.

### CODE CHANGE

Remove the portable API, calls in setters and metadata helpers, and Windows bookkeeping used solely for pre-change snapshots. The per-window recursion guard spans both public events. The Windows listener closes and reopens UIA separately, including when no root provider exists yet. UIA discards old notifications by generation and uses the controls identified by the theme traversal to establish the rebuilt controls' initial values. Test recursive calls in each handler and keep native and hosted assertions on observable behavior.

The Windows renderer installs the theme-only `IGuiThemeRefreshObserver` declared in `<GacUI repo>/Source/Application/Controls/GuiBasicControls.h`. Its window notification runs before each public boundary. Its control notification uses the work already performed by `GuiControl::RefreshThemes` in `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp`. The Windows context records a control only when its window is already observed and closed for rebuilding. At completion it retires stale records, attaches ordinary handlers and seeds the requested properties on the surviving records. Hidden popups first bound during a rebuild defer their own discovery until opening. The insertion, removal and completion paths perform no recursive composition discovery for the closed subtree.

The native fixture exercises first edits on a default label and on a label inside a detached, explicitly templated control. It checks name precedence, old and new values, duplicate suppression, the native title, document notifications, bounds, identity and retirement. Both public handlers reject recursion, and a second refresh of the same window completes.

### CONFIRMED

The Debug x64 build completed with zero warnings and errors. All 550 configured cases in 22 files passed, including the native UIA case and hosted submenu isolation. The leak report was empty. In both hosted and native ThemeProperty runs the separate subtree subscriber received the first ToggleState.On from the rebuilt Momentum Scrolling checkbox, and each application shut down normally.

The full elapsed times for the first Controls GET that showed the finished palette are in seconds. Each row used the same running target for all five switches.

| Debug x64 configuration | Aurora 1 | Default 2 | Aurora 3 | Default 4 | Aurora 5 | Driver GETs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Hosted HTTP | 0.797 | 0.820 | 0.920 | 0.828 | 0.820 | 115 |
| Hosted with passive UIA client | 1.120 | 1.147 | 1.173 | 1.979 | 1.158 | 436 |
| Native HTTP | 1.065 | 1.109 | 1.099 | 1.036 | 1.044 | 115 |
| Native with passive UIA client | 1.454 | 1.403 | 1.367 | 1.459 | 1.439 | 451 |

All 1,117 driver GETs completed within the 15-second deadline. The scenarios verified the selected palette and reopened the ticking Raw Clock after each switch. The 13 x64 generated Workflow listings differed only in the order of existing declarations and assignments. The recorded renderer frames were identical, so the original snapshots were retained.

The two-client Lifetime fixture passed 140 assertions in the native host and 139 in the hosted configuration. Each run checked the unvisited descendant's first name change, logical focus and continued notifications after the first client released its subscription. The target was in the background, so the fixture skipped the desktop foreground-focus event check in both runs. The existing UIA Review resource was selected temporarily for Playground and restored byte-for-byte afterwards.

The Debug Win32 build also completed with zero warnings and errors. Its unfiltered run passed all 2,022 cases in 100 files and produced an empty CRT leak report. The file-dialog and ribbon snapshots matched in all 88 rendered frame trees and 283 intermediate command frames after resolving element identities. The three ribbon allocation counts also matched the prior investigation's unmodified executable.

A repository-wide search of C++ sources, headers and C# found zero references to the removed API, accessor or flags. Both theme events still have their original reflection registrations in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp`. The closed-menu ownership lookup and ribbon group menu override remain in use.

## No.2 Publish composition bounds changes on the graphics host and simplify window refresh

Treat the current window as live throughout its synchronous theme refresh. Keep the disposal checks on pending entries in the application and child traversal. Publish each changed composition's committed bounds on its current graphics host after the local event. Subscribe Windows UIA at the window and use the provider's existing property snapshots and queued comparisons for all observed layout changes.

### CODE CHANGE

`GuiWindow::RefreshThemes` now clears its recursion guard after the public After event returns. `GuiGraphicsComposition::Layout_SetCachedBounds` commits the new value, dispatches its local event and then raises `GuiGraphicsHost::CompositionCachedBoundsChanged` on its current host. `RaiseCompositionCachedBoundsChanged` supplies the changed composition as the sender and source.

Windows UIA attaches the event through its existing window handler lifetime and queues the window's normal property comparison. The per-composition handlers, lifetime markers and pre-change bounds capture were removed. Composition insertion still discovers controls, and window movement still updates screen positions.

### CONFIRMED

The Debug x64 solution built with zero warnings and errors. All 557 configured cases, including the graphics-host startup file added to this run, passed across 23 files. The test log ended with the passing summary and no memory leaks. The host case verified local-before-host ordering, sender and argument sources, committed bounds, both host listeners and silence on an unchanged layout. The Windows fixture observed exactly one UIA change on each parent move while the label's cached rectangle stayed fixed, and also observed the label's direct move. The existing recursive-refresh, owned-window disposal and hosted submenu cases passed.

The hosted and native ThemeProperty clients both received the first checkbox change after a theme refresh. Across five palette switches per Raw Clock scenario, all 1,072 requests completed within 15 seconds. Hosted runs made 111 requests with HTTP alone and 409 with a passive UIA subscriber. Native runs made 113 and 439 respectively. All six showcase runs shut down normally. The 13 regenerated x64 Workflow listings contained only reordered declarations and assignments, and all rendered snapshot files matched.

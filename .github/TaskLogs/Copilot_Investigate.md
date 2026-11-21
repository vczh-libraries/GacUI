# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Purpose of the work:

  Keep the Windows HTTP automation service responsive during a color-theme switch with UI Automation
  (UIA) listeners attached. A `Controls` request needs the UI thread to read the finished GacUI tree. It
  should complete within the existing 15-second client deadline, including the normal cost of rebuilding
  templates.

  In the xgSkins FullControlTest replay, the same switch waited 2.18–2.83 seconds in the UI queue without
  listeners and 31.577 seconds with a passive listener. In a separate 37.32-second refresh, 83 of 97
  UI-thread samples were in `WindowsUIAutomationContext::RetireSubtree`. Retirement repeatedly searched
  all bounds hooks and, for compositions with an associated control, all automation nodes. Use GacUI's
  built-in palettes to exercise the same refresh path.
  [Original repro and timings](https://github.com/vczh-libraries/xgSkins/blob/bdd997337496561ce4b22ecd41dccc3b1414b1c3/suggestions/GacUI/AutomationResponseFailure.md#root-cause)

Implementation:

  - Add `BeforeRebuildingTheme` and `AfterRebuildingTheme` as `GuiNotifyEvent` members on `GuiWindow`.
    Associate them with its bounds composition and register both alongside `BoundsChanged` in reflection.
    `GuiPopup` inherits the pair. [Window events](Source/Application/Controls/GuiWindowControls.h) ·
    [Reflection](Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp)
  - Put the pair around one whole window refresh. Today `GuiWindow` inherits the **nonvirtual**
    `GuiControl::RefreshThemes()`. Make the entry point virtual. In the window override, raise Before,
    run the existing recursive base implementation, then raise After on the surviving window. This covers
    the application loop, direct popup calls and calls through a `GuiControl*`. Keep disposal checks,
    explicit-template handling, hidden tooltip content and the application's shared-tooltip reset.
    Coalesce reentrant refreshes of the same window so an inner refresh cannot resume UIA early.
    [Application loop](Source/Application/Controls/GuiApplication.cpp) ·
    [Control recursion](Source/Application/Controls/GuiBasicControls.cpp)
  - On Before, mark the affected window empty **before** calling
    `UiaRaiseStructureChangedEvent(windowProvider, StructureChangeType_ChildrenInvalidated, nullptr, 0)`
    for structure listeners. A reentrant query must already see the empty state. Use the exposed root of
    the affected subtree. One event covers its descendants. A native popup can have its own HWND, while
    a hosted popup appears under its owner in a shared fragment. A popup-only refresh leaves its owner
    and siblings usable. Keep the window provider and its subscriptions alive across the switch.
    [UIA tree relationships](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp) ·
    [Microsoft event API](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraisestructurechangedevent)
  - Discard that window's old template hooks, snapshots and pending descendant notifications in bulk
    while their owners are safe to detach. Retire genuinely removed providers and text ranges so retained
    references return `UIA_E_ELEMENTNOTAVAILABLE`. Preserve the identity of surviving logical controls.
    During the interval, child navigation returns no children, and focus, hit testing, pattern calls and
    queued actions cannot expose or act on descendants. This also applies to providers first requested
    during the rebuild. Bypass per-composition `RetireSubtree`, insertion `Scan` and repeated collection
    for that window.
    [Observation and retirement](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp) ·
    [Provider entry points](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp)
  - On After, make the finished window queryable and discover descendants on demand. If a query observed
    the empty tree, raise one completion invalidation for structure listeners after reopening the root. A
    client that cached the empty result then knows to enumerate again. Use a direct root notification for
    these transitions. The existing `Notify(node, true)` scans the subtree before raising the event.
    Preserve event-only subscriptions too. A subsequent real change on an unvisited control must reach an
    interested listener, for example by routing the change through the window and materializing its
    source on demand. With no UIA client, both boundaries should require no provider construction or tree
    walk.

Verification:

  - Reproduce on the unmodified GacUI build. Use Windows Debug x64 FullControlTest. Start from the Raw
    Clock scenario in the hosted wrapper
    `<GacUI repo>/Test/UIA_CppTest.ps1 -Scenario RawClock -AsPort 8888`. Its existing test skips the
    broad UIA registrations. Extend the shared harness to run the same inputs with and without a separate
    passive UIA client. Before the click, wait for it to register subtree handlers on the root for
    structure changes, changes to Name, IsSelected and ToggleState, and the TextChanged and
    TextSelectionChanged events. Keep them registered through the first completed post-switch tree. Event
    handlers should only record deliveries during this timing run.
    [Raw Clock inputs and existing registrations](Test/UIA_CppTest_Shared.cs)
  - Start on **Misc → Raw Clock** under **Default**. Through HTTP IO click **Window Manager → Aurora**,
    then immediately GET `http://localhost:8888/Automation/CppTest/Controls` and reopen Raw Clock using
    the existing polling sequence. The palette click queues the refresh, so an early response can still
    describe Default. Confirm Aurora is selected on the rebuilt Window Manager and
    `FullControlTest.Clock` is present and ticking when reopened. Alternate Aurora and Default for at
    least five switches per listener configuration. Use the same fixture, window size and build
    configuration for each comparison. [Palette switch](Test/Resources/App/FullControlTest/Resource.xml)
  - Measure HTTP latency. Give each individual GET a 15-second deadline and record every outcome,
    including timeouts. Record refresh duration and separate the GET's UI-queue wait, tree serialization
    and send time. Compare unmodified and fixed builds with listeners on and off. On the fixed build
    every GET succeeds within 15 seconds, and a trace shows zero per-edit retirement or insertion scans
    for the rebuilding window and no eager completion scan ahead of the queued GET. Record the baseline
    even if the built-in palette stays under 15 seconds on the test machine.
  - Verify the observable UIA lifecycle. Extend the theme tests with a window and a directly refreshed
    popup. Assert one ordered Before/After pair around all template edits and exercise a nested call. Use
    an in-process probe during the interval for deterministic empty-root, focus and hit-test checks. An
    external query may wait behind the UI thread and see only the finished state. Hold the root, a
    surviving control and a replaced template child across the switch. The first two retain their
    identity. The old child is unavailable. Verify one root invalidation at entry, none for intermediate
    template edits, and one completion notification when the probe observed the empty tree. Change a
    control after refresh with only a property listener attached and verify delivery before any client
    tree walk. [Theme tests](Test/GacUISrc/UnitTest/TestApplication_Theme.cpp) ·
    [Tooltip tests](Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp)
  - Run the Windows unit tests and the extended FullControlTest scenario in both hosted `CppTest` and
    native-window `CppTest_Metaonly` using their existing wrappers. For a popup-only refresh, check that
    a retained sibling remains navigable, that only the affected subtree reports the transition, and that
    reopening or destroying the popup leaves no stale callbacks. [Hosted wrapper](Test/UIA_CppTest.ps1) ·
    [Native-window wrapper](Test/UIA_CppTest_Metaonly.ps1)

## DETAILS

- Keep the rebuild state on each logical `GuiWindow`. Set it before dispatching
  `BeforeRebuildingTheme` so even the first `WM_GETOBJECT` or event subscription from inside a Before
  handler sees the closed subtree. `BindWindows` and `RefreshObservation` can both initiate scans.
  Honor the state at both entry points while continuing to count subscriptions. Reopen the tree before
  notifying completion and keep same-window recursion coalesced through the last After handler.
- Track each observed attachment's window before template edits begin. A surviving control can
  temporarily lose its host during template replacement. Keep it eligible to resume under the same
  identity. Using temporary invisibility as the result of `WindowsUIAutomationNode::IsLive` would let
  `Collect` permanently retire it. Check visibility in the text range's separate read path as well as
  in provider calls. A destroyed control, replaced document or cross-window move must still invalidate
  the corresponding old reference.
- Account for work already posted to the dispatcher. A pending root `Notify` reads and clears mutable
  flags on delivery. Record the window's generation when posting work and check it before touching
  those flags, so an old callback cannot scan the reopened tree or consume a later change. Revalidate
  queued actions against their original attachment when they run. Keep cleanup and notification state
  independent for sibling windows.
- Preserve the first subscribed change on a rebuilt control that the client has never visited.
  `UpdateProperties` initializes a missing snapshot silently. If observation starts after a change,
  capture the transition at its source so the first delivery identifies the control, property and
  changed value. An unchanged assignment should produce no property-change event.

## VERIFICATION

- Run the HTTP-only comparison in a fresh target before `AutomationElement.FromHandle`, which the
  current Raw Clock harness calls during startup. On the fixed build, verify that UIA constructs no
  providers and walks no subtrees at either rebuild boundary. Use the same HTTP driver for the passive
  listener run. Take the endpoint from the wrapper (`CppTest` on 8888 and `CppTest_Metaonly` on
  8890 by default).
- Time each `ControlsSnapshot` GET through the last response byte. The shared helper uses
  `WebClient.DownloadString`, and the 120-second Raw Clock polling loop cannot enforce a deadline on
  an individual GET. Record each timeout before retrying. If the unmodified target terminates when
  sending the late response, keep the failed sample and restart it for the remaining trials.
- `SelectSkinItem` updates the selected item before it queues the palette change. Correlate each
  request's UI-thread tree capture with the end of that switch's refresh. Report the immediate GET
  and the first GET that captured the rebuilt tree, even when those are different requests. Confirm
  the requested palette and ticking clock for each direction.
- Use the portable theme tests for callback order and same-window reentry from Before and After
  through a `GuiControl*`. Exercise the Windows provider on the UI thread in a Windows fixture.
  Request the first provider from an early Before handler and verify empty child navigation. In a
  separate run, subscribe before the refresh and count invalidations on the affected root. Expect
  two when the probe reads the empty tree and one when no query observes it. For the hosted popup,
  refresh a menu button's open submenu and hit-test both the menu and a sibling.
- Queue a structure notification before the switch. Immediately after reopening, change an unvisited
  control with only the corresponding property subscription active. Drain the dispatcher and check
  the event's source and payload and the absence of a stale scan. Check a retained replaced child's
  bounds and a retired range's text for `UIA_E_ELEMENTNOTAVAILABLE`. `GetRuntimeId` remains callable
  during provider disconnection.
- For the new reflected events, build Debug Win32 and run `Metadata_Generate`, then build Debug x64
  and run `Metadata_Generate` and `Metadata_Test` as specified in `<GacUI repo>/Project.md`.

## REVIEW COMMENTS

# UPDATES

# TEST [CONFIRMED]

Run Raw Clock in fresh Debug x64 hosted and native FullControlTest processes. Use HTTP to open the clock, alternate Aurora and Default five times in each listener configuration, and verify the selected palette and changing clock data. In the listener process register all four requested handlers before switching. Time every GET through its final byte with a 15-second deadline. Correlate UI-thread capture with refresh completion and record UI queue, serialization and send times, provider creation, insertion scans and retirement work.

Exercise window and popup refresh through a control pointer with reentry from both events. Probe native UIA interfaces on the UI thread during a rebuild, before the provider has ever been requested and with subscriptions already established. Check child navigation, focus, hit tests, pending actions, retained identities and text ranges, sibling access, entry and completion notifications, stale dispatcher work, and the first property change on an unvisited rebuilt control. Verify both metadata architectures and the Windows unit tests.

The original Debug x64 hosted Raw Clock passed 115 assertions. In a fresh hosted process with the passive subtree listener, immediate GET 11 returned the old palette. GET 12 timed out at 15.026 seconds. The UI thread finished the refresh in 17.027 seconds, after 93,218 scan calls and 97,695 retirements. Sending the late response opened the target's native C++ runtime error dialog. The same HTTP driver completed all five switches in its HTTP-only warmup, with 70 successful GETs.
# PROPOSALS

- No.1 Suspend UI Automation observation for each window while its templates rebuild.

## No.1 Suspend UI Automation observation for each window while its templates rebuild

Bracket the existing recursive refresh with two reflected events. Close the logical window before entry dispatch, reopen before completion dispatch, and coalesce recursion until the last completion callback returns. Track the window that owns each observed attachment. Detach observation once at entry, keep surviving identities, suppress visibility independently of lifetime, and reject notifications and queued actions from an earlier attachment generation. Invalidate the exposed subtree directly at entry and, if its empty state was observed, at completion. Resume observation on demand when a client asks for a descendant or a subscribed control changes.

### CODE CHANGE

Make the existing control refresh virtual and bracket its window override with reflected events. Track the original logical window on automation nodes, reject requests to hidden descendants, and discard stale dispatcher work by generation. Drop bounds hooks and control handlers in bulk, compare surviving attachments after the rebuild, and observe rebuilt controls through their window when they first change. Probe the production COM interfaces from Windows fixtures and compare both showcase window modes using the same HTTP driver.
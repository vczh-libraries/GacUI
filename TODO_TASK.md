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

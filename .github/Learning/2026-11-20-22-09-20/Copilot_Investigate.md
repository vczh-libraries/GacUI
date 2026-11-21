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

### Built-in palette baseline

The hosted fixture measured 656 × 519 and the native fixture measured 640 × 516. Each comparison used its own unchanged window size and Debug x64 binary. Each series began on Default and alternated Aurora and Default. After a late response terminated a passive target, the harness launched a fresh target and prepared its prior palette before starting the listener. The passive callbacks recorded deliveries without reading the tree.

| Baseline measurement in seconds | Aurora 1 | Default 2 | Aurora 3 | Default 4 | Aurora 5 |
|---|---:|---:|---:|---:|---:|
| Hosted HTTP only, first finished-tree GET | 0.928 | 0.790 | 0.857 | 0.807 | 0.799 |
| Native HTTP only, first finished-tree GET | 1.250 | 1.110 | 1.232 | 1.253 | 1.613 |
| Hosted with listener, UI-thread refresh | 16.109 | 17.191 | 17.362 | 16.579 | 15.723 |
| Native with listener, UI-thread refresh | 16.488 | 17.111 | 16.621 | 17.068 | 17.692 |

All ten passive trials missed the 15-second deadline on the finished tree. Their immediate GETs returned the previous tree in 28–46 ms. The late sends terminated the targets before the client could reopen the clock. Both HTTP-only series verified the selected palette and the reopened, ticking clock after all five switches. The portable [request records](./ThemeRefreshRequests.csv) include each request's client outcome, timing through the last byte, UI queue wait, serialization, send and refresh markers. A missing send-completion timestamp means the process stopped inside the send path.

# PROPOSALS

- No.1 Suspend UI Automation observation for each window while its templates rebuild. [CONFIRMED]

## No.1 Suspend UI Automation observation for each window while its templates rebuild

Bracket the existing recursive refresh with two reflected events. Close the logical window before entry dispatch, reopen before completion dispatch, and coalesce recursion until the last completion callback returns. Track the window that owns each observed attachment. Detach observation once at entry, keep surviving identities, suppress visibility independently of lifetime, and reject notifications and queued actions from an earlier attachment generation. Invalidate the exposed subtree directly at entry and, if its empty state was observed, at completion. Resume observation on demand when a client asks for a descendant or a subscribed control changes.

### CODE CHANGE

Make the existing control refresh virtual and bracket its window override with reflected events. Track the original logical window on automation nodes, reject requests to hidden descendants, and discard stale dispatcher work by generation. Drop bounds hooks and control handlers in bulk, compare surviving attachments after the rebuild, and observe rebuilt controls through their window when they first change. Probe the production COM interfaces from Windows fixtures and compare both showcase window modes using the same HTTP driver.

The portable fixtures refresh a window through a control pointer and refresh a popup directly. The Windows fixtures retain a control, a document range and nested menu providers, then inspect navigation, hit testing, queued actions and root invalidations inside the refresh. They also exercise the first Name, TextChanged and bounding-rectangle change after reopening. A separate process subscribes only to ToggleState and checks the cached source and value for the first click on the rebuilt Momentum Scrolling checkbox.

An intermediate hosted build returned the first finished tree in 2.192 seconds with a passive listener. The trace still counted 7,795 scans and 8,013 retirements during that refresh. Debugger stacks showed menu buttons applying inherited templates to their closed submenus while the owner refreshed. The trace after refresh and before the queued capture was already clear.

The provider now follows a closed menu's logical owner for rebuild observation. An open menu still uses its native parent, and a ribbon group retains its popup's theme ownership even when its dropdown button is detached. An unexposed, unrequested menu needs no transition provider. A retained command keeps its ancestor window observers alive through collection. Explicit label registrations also retain the previous label text so an unvisited control whose accessible name comes from that label can report its first change.

The unfiltered Win32 test run also exposed an independent file-dialog fixture issue. Its child-window lookup could select a tooltip belonging to the same owner as the expected modal dialog. The saved baseline failed in the same test case. Restricting the lookup to an open modal with the expected native owner preserved the original assertions, and the corrected file passed under the Win32 debugger.

A separate trace-free rerun exposed competing UIA contexts in the native test fixture. Sending an early desktop `WM_GETOBJECT` reproduced the failure under the debugger. The renderer's listener owned the subscription while the test counted events on its private context. The two Windows fixtures now supply their observation context through the renderer's single listener. The native Before handler verifies the root is still unbound and acquires its first provider directly on the UI thread. Repeating the forced early request passed all 12 cases across the two x64 test files with an empty CRT leak log. The external subscriber exercises Windows' provider-acquisition transport in both hosts.

### Fixed-build measurements

The first GET that captured the finished tree completed in the following times, measured through the final response byte. Each column is one switch in a single running process for that row.

| First finished-tree GET in seconds | Aurora 1 | Default 2 | Aurora 3 | Default 4 | Aurora 5 |
|---|---:|---:|---:|---:|---:|
| Hosted HTTP only | 0.829 | 0.825 | 0.849 | 0.806 | 0.790 |
| Hosted with passive listener | 0.888 | 0.919 | 1.364 | 0.984 | 0.928 |
| Native HTTP only | 1.102 | 1.092 | 1.097 | 1.130 | 1.065 |
| Native with passive listener | 1.163 | 1.162 | 1.236 | 1.179 | 1.175 |

The four runs completed all 856 timed driver GETs within the 15-second deadline. Each trial verified the requested palette and reopened the ticking Raw Clock. The slowest response took 1.364 seconds. In all 20 switches the immediate GET captured the preceding tree. The [request records](./ThemeRefreshRequests.csv) distinguish it from the first UI capture after refresh and record the constituent timings and observation counters. The 1,350 rows include the baseline, the intermediate hosted diagnostic, the four final series and the wrappers' readiness probes.

For the first finished capture with listeners, the UI queue wait was 859–1,336 ms hosted and 1,134–1,213 ms native. Serialization took 9.6–13.4 ms and sending took 10.6–14.0 ms across those ten responses. The passive baseline's UI queue wait alone exceeded the client deadline.

Every refresh recorded zero calls to `Scan`, `RetireSubtree` and `Collect`. Both HTTP-only runs also constructed zero providers during refresh and before the first finished capture. With a passive listener, later switches constructed providers in response to Windows' own UIA queries. A hosted debugger stack showed UIAutomationCore constructing an event source while handling the direct root invalidation. It synchronously navigated to a combo box in the main window and requested that ancestor's provider.

Six passive trials also recorded scans after reopening and before the queued capture. In switch order the hosted counts were 0, 972, 1,852, 0 and 1,652. The native counts were 0, 0, 1,888, 1,165 and 752. Debugger stacks in both hosts started with deferred scroll-view layout. The list arranger realized a row, inserted its style and triggered the ordinary child-insertion observer on that new composition. The same work could occur after the HTTP capture when scheduling differed. The completion notification itself raised the root event directly and performed no discovery scan.

### CONFIRMED

The ten original passive trials exceeded the 15-second tree deadline. The ten fixed passive trials completed in 0.888–1.364 seconds, and the ten fixed HTTP-only trials completed in 0.790–1.130 seconds. Every fixed trial verified the requested palette and a ticking clock. In both hosts a separate ToggleState-only subscriber identified the rebuilt checkbox and its new value before any client tree walk. The original Raw Clock scenario also passed in both hosts.

The Windows probes verified empty descendants at entry, retained root and control identity, unavailable replaced providers and text ranges, queued-action cancellation, and isolation of hosted siblings. They observed one entry invalidation with no query, and a second invalidation when a query saw the empty tree.

The unfiltered Win32 run passed all 2,022 cases across 100 files. After isolating the two Windows fixture listeners, the final x64 run passed all 550 configured cases across 22 files and the focused Win32 run passed all 30 cases across four files. Each run had a clean CRT leak report. Both trace-free Debug builds completed with zero warnings and errors. Win32 and x64 metadata generation and x64 metadata testing succeeded. The four core/full text descriptions loaded from the generated binaries matched the generated originals byte for byte.

The file-dialog and ribbon snapshot differences preserved every rendered tree across 88 captured frames and 283 intermediate renderer frames. The saved unmodified executable independently reproduced the ribbon cases' current allocation counts of 275, 389 and 476 elements. The checked-in snapshots were retained.

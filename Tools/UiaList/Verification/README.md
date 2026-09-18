# UiaList verification

The independent fixture uses Windows controls and implements UIA providers directly. It does not import the inspector's adapters, catalogs, model services, or GacUI. Its native window supplies ordinary button, checkbox, edit, combo, list, and slider behavior. Its synthetic window deliberately combines patterns for adapter coverage; these combinations are not presented as standards-conforming real controls.

The synthetic window exposes the 35 standard patterns across capability/document nodes, 41 raw-only control roles, an unknown numeric role, typed references, and text ranges. **Open large/deep raw tree** creates 10,000 siblings and a separate 1,000-level chain. Mutation/query logs contain stable target keys and typed arguments; strings are recorded as UTF-16 units. Native button notifications provide an independent exactly-once Invoke counter.

## Reproduce

Build `Verification.sln` through `copilotBuild.ps1` from this directory, then run `UiaFixture` through `copilotExecute.ps1 -Mode CLI`, selecting Debug and x64 or Win32. The fixture writes `UiaFixture-PID.calls.txt`, `.synthetic.txt`, and optional `.stress.txt` in its working directory.

Run the following from any directory, adjusting the checkout prefix as needed:

```powershell
& C:\Code\VczhLibraries\GacUI\Tools\UiaList\Verification\CheckCatalog.ps1
& C:\Code\VczhLibraries\GacUI\Tools\UiaList\Verification\Smoke.ps1 -Platform x64 -Cycles 25
```

`CheckCatalog.ps1` compares named SDK IDs, synchronized translation keys, and explicit C++ project/filter inventories. `Smoke.ps1` launches the inspector through the repository wrapper, selects the fixture process by PID, single-clicks its window row, uses Nodes' context-menu Inspect, drives both modal levels, verifies exactly one native Invoke notification, refreshes, and checks normal exit. Supply `-FixtureProcessId` when multiple fixtures run; this selects both the process and independent log. All drivers accept `-AsPort`; use 8891 beside showcase ports 8888 and 8890.

`DeepTree.ps1 -Cycles 25` operates an already-running Debug inspector after the fixture's **Open large/deep raw tree** button has been clicked. It repeats complete traversal and replacement of the deep tree, checking the observed 11,053/51 node counts. `-Cycles 0 -CloseWithDeepTree` loads the large tree and closes the inspector. Confirm the process exit and leak output in CDB; use `sxd eh` to preserve second-chance C++ exception breaks during error investigation.

Add `-NavigationRegression` to Smoke to open the fixture's optional navigation windows: duplicate titles, untitled, owned, hidden, cloaked, minimized, and empty bounds. It compares every listed HWND with an independent native enumeration, browses the fixture's headless launcher, exercises show/hide/close and repeated Refresh, and checks that a pruned target clears inspection. Navigation smoke also covers column/background clicks, selected-row reactivation, Enter, node double-click, menu cancellation/invalidation, and collapsed-parent inspection.

`Inspector.ps1` can inspect captions, issue IO commands, select `-SelectProcessId`, and click a caption. Use `-InspectNode` for a node's right-click/Inspect workflow, `-RightClick` for menu/cancellation testing, and `-Button` for an action. `-Scroll` uses current composition/viewport bounds, including dropdowns and reduced windows. Property details and returned references still use `-DoubleClick`. A queued IO response is not an assertion of completion. Read Controls and the fixture log after each operation. Wait for the inspector's busy state to finish before the next mutation.

```powershell
& C:\Code\VczhLibraries\GacUI\Tools\UiaList\Verification\Inspector.ps1 -Find '*'
& C:\Code\VczhLibraries\GacUI\Tools\UiaList\Verification\Inspector.ps1 -Window 1 -ClickLabel Invoke -Button -Scroll
```

All HTTP IO uses the exact `application/json; charset=utf8` content type and actual modal window IDs. The driver checks for a native runtime error dialog before each request and after timeouts. It never uses UIA to drive GacUI. Give Controls requests sufficient time: canceling an in-flight request can trigger the existing Debug HTTP service's fatal send-error policy.

## Recorded environment and evidence

The following original implementation and integration results are historical. Their old interactions, translation counts and script names do not define the current UI. Current acceptance includes the process dropdown/own-window list, node context menu, unsupported-property filtering, persistent groups and measured compact spacing in [Planning.md](../Planning.md).

Implementation verification used Windows 11 Pro 10.0.26200, SDK 10.0.26100.0, MSVC 14.51.36231/v145, and a 96-DPI desktop. The product build matrix completed with zero warnings and zero errors. GacBuild compiled both architecture paths; an authored-string perturbation reached both, restoring the string reproduced every generated production-file hash, and a no-change generation preserved those hashes. Catalog checks found 175 properties, 35 patterns, 44 text attributes, 41 control types, one metadata ID, and 43 matching translation keys per locale.

The existing GacUI regression suite passed 90/90 files and 1,768/1,768 cases with no appended leak report. A native-fixture Debug session under CDB exercised a Unicode Value mutation, document-range acquisition, the 44-attribute workspace, and Clone; normal process exit was zero and no CRT leak report appeared. Further run results are recorded in `Results.md`.

The synthetic fixture exposed all 35 pattern cards and their typed current readouts. Positive GUI evidence includes Invoke, selection, scrolling, grid/header references, view names, transforms, legacy accessibility, ItemContainer, Realize, SynchronizedInput start/cancel, opaque ObjectModel/IAccessible results, Styles, spreadsheet references, zoom, TextChild, Drag, and CustomNavigation. Individual action evidence is not a claim that every enum value, provider failure mode, or version-only combination in A00 has been covered.

One repeat run stopped on cycle 18 because its startup probe canceled a Controls request after one second. CDB found `HttpAutomationService::OnHttpRequestReceived` terminating on `vl::Error("HttpSendHttpResponse failed for responding UTF-8 body.")`; UIA and capture workers were idle. The harness now allows 15 seconds for startup Controls and restarts the consecutive-run count. This preserves the repository endpoint's existing fatal policy.

## Acceptance limits

The full acceptance matrix remains in [Planning.md](../Planning.md). Treat cases without explicit recorded results as unfinished verification. In particular, this environment has not established the mixed-monitor 150%/200% and negative-origin cases, all installed OS-language combinations, protected/secure-desktop boundaries, every fault-injection/timing permutation, real drag/composition/input behavior, or every parameter combination and version-only pattern scenario. The synthetic fixture's text selection and context-menu operations record calls; they do not substitute for all standards-conforming behavioral scenarios required by R01-R10/A00. Real-application and SDK Inspect comparisons are also separate from adapter coverage.

Build outputs, volatile call logs, screenshots, deployment copies, and full debugger/automation dumps are ignored. Keep source, reproducible scripts, and concise results in version control.

## Windows provider integration regression (2026-09-18)

`Smoke.ps1` also requires the parameterless `GetClickablePoint` result to appear as a readout without a duplicate button, and verifies that inspecting getters does not increment the independent Invoke counter. The current Debug x64 and Win32 runs passed with exactly one deliberate invocation and normal exit. All drivers accept `-AsPort` for concurrent inspection.

Add `-GetterRegression` to test invalid and committed getter parameters, rapid edits, returned-element inspection, detail replacement while a query is pending, explicit StartListening/Cancel, and opening a returned document range to execute Clone. Each deliberate operation is checked against the independent fixture log. The synthetic document exposes a TextEdit interface whose SupportedTextSelection query can return `UIA_E_ELEMENTNOTAVAILABLE` while the element remains readable; that section displays the operation and HRESULT, and other text sections/range workspaces remain available.

The synthetic provider also supplies false, zero, empty string/array and null properties. HelpText becomes Unsupported at Toggle state 1 and returns at state 2. Getter regression selects that row before mutation, checks disappearance on action readback/Refresh and reappearance after the next Toggle, and restores the initial toggle state. The three explicit Toggle calls are counted independently.

To probe the real Windows MSAA bridge, run another fixture through `copilotExecute.ps1 -Mode CLI` with `/LegacyProbe:<native-fixture-pid>` in its matching `.vcxproj.user` configuration. This does not start an automation service. The probe logs native target 101 (button) and 108 (static label), including default-action text, state, and the actual `DoDefaultAction` HRESULT. The observed button returned `Press` and state `0x00100000`; the label returned empty text and state `0x00000040`. Both calls returned `S_OK`; an empty default action still disables the inspector command. The original unspecified failing target could not be retested.

The current local GacUI provider suites are `Test/UIA_CppTest.ps1` and `Test/UIA_CppTest_Metaonly.ps1`, backed by `UIA_CppTest_Shared.ps1` and `.cs`. They use windowless MTA clients and resolve paths from their script directory. Run them sequentially from outside `Test` too: opening another ordinary application can dismiss a target's popup through normal activation handling. The former AutomationPorts suite was removed; its recorded results below are historical, not current run instructions. The shared parser is `Test/GacUISrc/SharedArguments.h`.

The earlier integration runs passed 4,433 hosted and 4,445 ordinary Direct2D assertions, both with normal shutdown and released endpoints. The complete `-GetterRegression` workflow passed on Debug x64 and Win32. Two concurrent three-application inspection/restart cycles passed, as did the Release no-endpoint check. Exact commands, GDI smoke coverage, build/unit/metadata results and verification limits are recorded in [the archived investigation report](../../../.github/Learning/2026-09-18-15-13-48/Copilot_Investigate.md).

## Process browsing and inspector layout regression (2026-09-18)

The current change passed GacBuild for both architecture paths, all four UiaList build configurations, both GacUISrc Debug architectures, and 1,769/1,769 configured UnitTest cases (90/90 files; no appended leak report). CheckCatalog now reports 42 matching localization keys in each locale; its SDK inventories are unchanged. Every one of the 18 relocated-header project/filter references resolves.

Use the expanded regression with a specific fixture PID:

```powershell
& C:\Code\VczhLibraries\GacUI\Tools\UiaList\Verification\Smoke.ps1 -Platform x64 -AsPort 8891 -FixtureProcessId 16360 -GetterRegression -NavigationRegression
```

Replace the example PID with the running fixture's PID and use `-Platform Win32` for the other inspector architecture. Two concurrent fixtures with identical executable names and captions were used to verify PID selection. Current observations and measured layout bounds are recorded in [the investigation report](../../../.github/TaskLogs/Copilot_Investigate.md).

Both expanded smoke runs passed with exactly one native Invoke and normal exit. The renamed hosted/metadata-only suites passed 4,434/4,444 assertions and released their endpoints. The final deep-tree run passed 25 replacements (11,053 to 51 nodes each), then closed with the full tree loaded; CDB confirmed exit zero and no CRT leak report. Default/custom ports, invalid argument rejection and Release's absent endpoint were also checked.

Default-size rendered screenshots confirmed the persistent provider groups. Final Controls measurements verified five-pixel row/list gaps and reachable commands at 900×650 and 660×480. Capture later became unavailable on the desktop, so final reduced-size/References screenshots remain unverified; the report distinguishes measured bounds from actual screenshot observations.

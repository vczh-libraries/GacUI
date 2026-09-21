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

All HTTP IO uses the exact `application/json; charset=utf8` content type and actual modal window IDs. The driver checks for a native runtime error dialog before each request and after timeouts. These drivers operate the inspector through its automation service; Windows UIA verification is an additional pass required by [Running-GacUI.md](../../../.github/Guidelines/Running-GacUI.md#windows-specific). Give Controls requests sufficient time: canceling an in-flight request can trigger the existing Debug HTTP service's fatal send-error policy.

## Recorded environment and evidence

The following original implementation and integration results are historical. Their old interactions, translation counts and script names do not define the current UI. Current acceptance includes the process dropdown/own-window list, node context menu, unsupported-property filtering, persistent groups and measured compact spacing in the [acceptance matrix below](#acceptance-matrix).

Implementation verification used Windows 11 Pro 10.0.26200, SDK 10.0.26100.0, MSVC 14.51.36231/v145, and a 96-DPI desktop. The product build matrix completed with zero warnings and zero errors. GacBuild compiled both architecture paths; an authored-string perturbation reached both, restoring the string reproduced every generated production-file hash, and a no-change generation preserved those hashes. Catalog checks found 175 properties, 35 patterns, 44 text attributes, 41 control types, one metadata ID, and 43 matching translation keys per locale.

The existing GacUI regression suite passed 90/90 files and 1,768/1,768 cases with no appended leak report. A native-fixture Debug session under CDB exercised a Unicode Value mutation, document-range acquisition, the 44-attribute workspace, and Clone; normal process exit was zero and no CRT leak report appeared. Further run results are recorded in `Results.md`.

The synthetic fixture exposed all 35 pattern cards and their typed current readouts. Positive GUI evidence includes Invoke, selection, scrolling, grid/header references, view names, transforms, legacy accessibility, ItemContainer, Realize, SynchronizedInput start/cancel, opaque ObjectModel/IAccessible results, Styles, spreadsheet references, zoom, TextChild, Drag, and CustomNavigation. Individual action evidence is not a claim that every enum value, provider failure mode, or version-only combination in A00 has been covered.

One repeat run stopped on cycle 18 because its startup probe canceled a Controls request after one second. CDB found `HttpAutomationService::OnHttpRequestReceived` terminating on `vl::Error("HttpSendHttpResponse failed for responding UTF-8 body.")`; UIA and capture workers were idle. The harness now allows 15 seconds for startup Controls and restarts the consecutive-run count. This preserves the repository endpoint's existing fatal policy.

## Acceptance limits

The [acceptance matrix below](#acceptance-matrix) preserves the inspector requirements and case IDs used by the recorded results. Treat cases without explicit recorded results as unfinished verification. In particular, this environment has not established the mixed-monitor 150%/200% and negative-origin cases, all installed OS-language combinations, protected/secure-desktop boundaries, every fault-injection/timing permutation, real drag/composition/input behavior, or every parameter combination and version-only pattern scenario. The synthetic fixture's text selection and context-menu operations record calls; they do not substitute for all standards-conforming behavioral scenarios required by R01-R10/A00. Real-application and SDK Inspect comparisons are also separate from adapter coverage.

Build outputs, volatile call logs, screenshots, deployment copies, and full debugger/automation dumps are ignored. Keep source, reproducible scripts, and concise results in version control.

## Windows provider integration regression (2026-09-18)

`Smoke.ps1` also requires the parameterless `GetClickablePoint` result to appear as a readout without a duplicate button, and verifies that inspecting getters does not increment the independent Invoke counter. The current Debug x64 and Win32 runs passed with exactly one deliberate invocation and normal exit. All drivers accept `-AsPort` for concurrent inspection.

Add `-GetterRegression` to test invalid and committed getter parameters, rapid edits, returned-element inspection, detail replacement while a query is pending, explicit StartListening/Cancel, and opening a returned document range to execute Clone. Each deliberate operation is checked against the independent fixture log. The synthetic document exposes a TextEdit interface whose SupportedTextSelection query can return `UIA_E_ELEMENTNOTAVAILABLE` while the element remains readable; that section displays the operation and HRESULT, and other text sections/range workspaces remain available.

The same regression edits the numeric RangeValue property, scrolls its active editor out of view and back, verifies the retained draft, cancels with Escape without a setter, rejects an invalid number, then commits one valid value and checks readback. Read-only rows create no inline editors; drafts belong to the row view model and survive visualizer recycling. Run the HTTP driver in PowerShell 7; Smoke uses Windows PowerShell for the child execution wrapper and retains its process handle to verify the exit code.

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

## Acceptance matrix

### Verification status and evidence

The following criteria preserve the inspector acceptance requirements and stable case IDs used by the historical results. They are requirements, not claims about current implementation or completed verification. Use the dated evidence above and in [Results.md](Results.md) to identify observed coverage; every other case or parameter combination remains unfinished. This documentation migration ran no application checks. Full acceptance requires a result for every applicable row and every operation in the pattern/range tables.

For each run record: case ID, commit, Windows/SDK versions, inspector and target configuration/architecture, locale/DPI/monitor arrangement, fixture scenario, initial state, exact input sequence, expected result, actual result, pass/fail, and evidence paths. Evidence consists of inspector Controls dumps, screenshots for layout/geometry, and the fixture's independent state and exact call records for mutations. A button rendering, a zero exit code without checking logs, or HTTP `Queued` alone never proves the feature.

### Controlled fixture and independent comparison

Use the independent native Windows fixture in this directory and extend its scenarios only when a verification task requires it; keep Win32/x64 coverage. It is a development target that exposes ordinary Win32 controls and synthetic UIA providers, not a test mode or CLI edition of UiaList. It must use independent expected data and call logging, not import UiaList's adapters/catalog code to generate its oracle. Launch UiaList and fixture under the documented repository execution/debugging procedures on an unlocked interactive desktop.

| Fixture ID | Required scenarios |
| --- | --- |
| F01 Process topology | Visible parent/child, headless parent with visible child, pruned headless sibling under visible branch, wholly headless branch, duplicate executable names, missing parent, multiple visible windows, hidden/cloaked windows, owned dialogs, minimized windows, exit between enumeration/activation. |
| F02 Tree/geometry | All 41 standard roles across valid fixtures; unnamed/Unicode/multiline labels; non-control/content RawView nodes; duplicate AutomationIds/names; cross-process subtree; deep nesting, overlapping/empty/offscreen bounds, child outside parent bounds; mutable add/remove/reorder; 10,000 siblings and a 1,000-level hierarchy. |
| F03 Values/patterns | Every known standard property descriptor and all value-format families, optional/unsupported/mixed values, read-only/writable/capability changes, all 35 patterns across appropriate scenario nodes, and a separate fault scenario with deterministic HRESULTs. Record exact target key, method, typed arguments, and call count. |
| F04 Text/references | Empty and rich documents; wrapped/multiline/Unicode text, mixed attributes, embedded child, annotation, caret, offscreen span, no/single/multiple selection, Text2, TextRange2/3, TextEdit composition/conversion; null/external references and opaque objects. |
| F05 Capture | Labeled/color-coded target extents, standard/custom frames, oversized target, occlusion, minimize/restore, move/resize, mixed-DPI/negative-origin monitors, optional capture exclusion. |

Synthetic capability combinations used for adapter coverage must be labeled as such; keep separate standards-conforming scenarios for behavior checks. No single real control is expected to implement all patterns. Obtain TextRange3 client extension coverage on a supported OS/client; absence is a tested capability case, but the extension's success paths still require a supported environment before declaring them verified.

Use the Windows SDK Inspect utility as an independent spot check of raw tree, properties, patterns, and bounds. Include real-app smoke runs against a native editor/dialog, a browser or other virtualized custom UIA application, and an MSAA-backed application; record their versions. These smoke runs complement the controlled oracle. UiaList's Debug GacUI automation controls the inspector UI but does not establish the inspected target's UIA correctness.

### Build, packaging, and catalog checks

| ID | Procedure and required result |
| --- | --- |
| B01 Generation | Run GacBuild from the absolute driver path. Inspect console output, both architecture logs/cache outputs and generated production files. No `Errors.txt` or resource compile errors. Edit an authored string, regenerate, verify both architecture paths feed the merged output; a no-change generation leaves tracked Source unchanged. |
| B02 Build matrix | Build Debug/x64, Debug/Win32, Release/x64, Release/Win32 through copilotBuild; wait for completion and inspect Build.log each time. Check v145/latest SDK/C++20, includes, explicit file/filter inventories and consistent library settings. |
| B03 Embedded deployment | Run each app build from a clean temporary deployment directory containing no Resource.xml, generated sources, or resource `.bin`. All windows, translations, templates, and images load. A link that drops the compressed-resource plugin fails this test. |
| B04 Debug service | Debug Controls and IO work at default port 8888 and custom `/AsPort` values with the exact prefix and Content-Type. Open both modal levels and target their real window IDs. Normal exit releases the port; restart succeeds. Occupied-port startup reports failure. Release owns no listening socket and does not start the service. |
| B05 Regression/audit | Run the existing GacUI UnitTest when implementing C++ as required by Project.md. Inspect Debug leak reports. Audit no handwritten generated code, XML `-eval` events, C++ implementations, no view/control access from VM, no runtime-reflection dependency, and no app dependency on test helpers. |
| C01 SDK completeness | Compare named pattern/property/attribute/control-type/metadata identifiers in the actual selected SDK with the handwritten descriptor sets. Baseline: 35/175/44/41 plus known metadata. Verify unique IDs, expected types, IID/provider/client spellings, and one implemented adapter per pattern. Record SDK changes explicitly instead of silently preserving old counts. |
| C02 Node types | Cover all 41 roles and a fixture-supplied unknown numeric role; preserve IDs and canonical names. Probe client Element versions available on the OS; a newer client version never implies a different semantic control role or remote provider inheritance. |
| C03 Client additions | Exercise Element SetFocus, clickable-point true/false, Element3 context menu, and Element7 metadata supported/unsupported states. Exercise property additions through the complete catalog. |
| C04 Text catalog | Compare all named text attributes against descriptors; exercise every attribute's expected value conversion, Mixed/NotSupported handling, and the TextRange3 batch results where supported. |

The 41-role checklist is: AppBar, Button, Calendar, CheckBox, ComboBox, Custom, DataGrid, DataItem, Document, Edit, Group, Header, HeaderItem, Hyperlink, Image, List, ListItem, Menu, MenuBar, MenuItem, Pane, ProgressBar, RadioButton, ScrollBar, SemanticZoom, Separator, Slider, Spinner, SplitButton, StatusBar, Tab, TabItem, Table, Text, Thumb, TitleBar, ToolBar, ToolTip, Tree, TreeItem, Window. This checks inspector presentation, not whether every arbitrary target meets Microsoft's per-role provider obligations.

Use absolute script paths. Example generation and build commands for this checkout (execute separately, from `C:\Code\VczhLibraries\GacUI\Tools\UiaList` for builds):

```powershell
& C:\Code\VczhLibraries\Tools\Tools\GacBuild.ps1 -FileName C:\Code\VczhLibraries\GacUI\Tools\UiaList\GacUI.xml
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform Win32
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Release -Platform x64
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Release -Platform Win32
```

For a normal GUI launch use the repository wrapper's existing `-Mode CLI -Executable UiaListApp -Configuration Debug -Platform x64` from the same solution directory, following the GUI automation guideline. CLI is the wrapper's launch mode, not a proposed UiaList CLI frontend; there is no `-Mode GacUI`. For errors or packaged deployments, run CDB in a PTY as documented in Debugging.md. Inspect native crash dialogs after timeouts; a blocked `Microsoft Visual C++ Runtime Library` dialog is a failed run, not evidence the app remains healthy. Run the existing UnitTest from `Test/GacUISrc` with its documented wrapper, not the UiaList solution directory.

### Processes, tree, and preview checks

| ID | Procedure and required result |
| --- | --- |
| D01 Startup | Start with F01 running. Processes populates once; UI/Nodes show no selection and are disabled. Reopening/activation notifications do not launch duplicate initial refresh jobs. |
| D02 Pruning | Verify the exact bottom-up HasVisibleUI result: retain headless ancestors with visible descendants, prune headless helper/sibling branches and wholly headless roots. The combo has only processes. Each selected PID lists only its own qualifying windows; headless ancestors list none. Cover minimized, hidden/cloaked, untitled, owned and duplicate-title windows. |
| D03 Identity/order | Duplicate executable/title cases remain distinct by PID/HWND; owned dialog parentage is correct; missing parents do not invent rows; sorting is deterministic; process and window keys are not truncated on Win32/x64. |
| D04 Manual refresh | Add/close/hide/show windows and processes externally. Tree stays unchanged until Refresh, then exactly matches fixture. Rapid repeated refresh cannot publish an older result last. A surviving selected window remains selected; an absent one clears dependent state. |
| D05 Activation | Navigate/select the process hierarchy without activating a window. Single-click a window, return from UI, click the same row again, and test Enter. Background/header clicks never activate prior selection. Verify full-width combo and details columns. |
| D06 Switching | Delay A's provider response, select A then B, finish A. Only B's results/preview/dialog state may publish. No accepted mutation is silently retargeted or repeated. |
| N01 Raw tree | Compare the complete tree and sibling order with F02. Non-control/non-content and cross-process descendants remain; there is no ControlView/PID filter or desktop-wide traversal. |
| N02 Labels | Check Name present/empty, Unicode, multiline normalization, highest acquired client interface, semantic role secondary text, exact provider aliases and order. Unsupported patterns produce no fake provider suffix. |
| N03 Expansion/selection | Collapse, refresh, reorder, and reopen; expansion/selection remap by identity. Duplicate names/AutomationIds do not swap selection. Missing selected node falls back to surviving ancestor/root. |
| N04 Large/deep | Traverse all 10,000 siblings and 1,000 levels, cancel/switch during traversal, and scroll once ready. No stack overflow, silent cap, repeated COM reads caused by painting, or UI-thread blocking. Record timings for diagnosis without arbitrary machine-dependent pass thresholds. |
| N05 Virtualization | Find an unrealized item through ItemContainer. It is inspectable as a returned reference and only appears in the raw tree when exposed. Realize affects exactly that item; inspector row recycling never changes its target identity. |
| U01 Image fidelity | Compare F05's labeled frame/client corners and dimensions with the preview. Oversized images get both scrollbars; resizing inspector does not stretch the image. No XML/bin disk dependency supplies the preview. |
| U02 Coordinates | At 100%, 150%, and 200% DPI, including monitor origins left/above primary, scroll both axes and hover known descendants. Outline matches expected physical bounds transformed once; verify clipping and border offsets. |
| U03 Hit rules | Exercise overlaps, parent/child depth, child outside parent, tied depth, zero/invalid/offscreen rectangles, root background, and mouse leave. First choose the deepest visible containing Window (ties use preorder), then the deepest eligible node in that window subtree (ties also use preorder). A deeper background page must not win over a shallower hosted child window. Raw preorder does not establish sibling-window desktop z-order. With no containing Window, search the whole snapshot. No stale hover on click. |
| U04 Reveal | Collapse ancestors, click a deep node in preview. Nodes activates, ancestor chain expands, selected row is visible and is the same identity. Target input/focus/action counters remain unchanged. |
| U05 Occlusion/unavailable | Cover target with inspector/another app, then minimize/hide/cloak or exclude capture. No unrelated desktop node is selected and no unavailable image is presented as a valid empty/black snapshot. Where Windows returns indistinguishable protected black pixels, record the platform limitation; do not claim pixel heuristics detect protection. |
| U06 Snapshot consistency | Move/resize between extent samples and delay frame callbacks across a target switch. Mismatched geometry or generation is rejected. After target settles and explicit Refresh, image/nodes share the new generation and mapping. |

### Properties, modal, and presentation checks

| ID | Procedure and required result |
| --- | --- |
| P01 Modal binding | Right-click an unselected node and choose Inspect: exactly one modal for that clicked node, with owner disabled and focus restored on close. Cover leaves/parents, cancellation, empty space, tree refresh with menu open and Enter. Double-click only expands/collapses. A stale command never inspects another selection. |
| P02 Typed values | F03 covers BSTR/empty/Unicode/control characters, signed/unsigned numbers, double, Boolean, enum/unknown enum, null reference, empty array, element/range arrays, point/rect/runtime ID, nonzero SAFEARRAY lower bounds and Mixed. Only Unsupported rows disappear from Properties, on initial inspection and refresh/action readback; restore selection by key or clear it when absent. Unsupported action results and text-range attributes remain visible. |
| P03 Read-only | Click properties without mapped setters and read-only/capability-disabled pattern values. No writable inline editor or fake setter appears. Toggle/selection changes only occur through their explicit actions. |
| P04 Inline commit | Edit each whitelisted inline kind, including the positively verified native single-line Edit fixture. Confirm other/unknown text widgets select the modal path. Type several characters, then Enter: exactly one setter receives the typed value, actual normalized value is reread, and displayed properties/cards agree. |
| P05 Draft lifecycle | Escape cancels with zero calls. Invalid numbers/enums show validation. Change row while valid/invalid; verify defined commit/block behavior. Scroll/recycle while editing, return to row: draft persists and no setter was triggered by recycling or initialization. |
| P06 Multiline | Selected row displays `...` even when a different cell in that row is selected; other rows hide it. No multiline inline editor exists. Cancel, Escape and title-bar close perform zero writes. OK writes one full Unicode/multiline value, preserving blank/final lines. |
| P07 Read-only text | Long read-only and Text-only multiline values open a viewer with no write command. Password fixtures reveal only the provider's allowed data. No fallback text retrieval bypass is attempted. |
| P08 Changed capability | Toggle target read-only/remove support after BeginEdit. Commit checks current state, makes no invalid optimistic write, and reports capability loss or the specified fatal HRESULT if the provider fails. |
| L01 Languages | Launch under English, Simplified/Traditional Chinese variants, Japanese, unsupported preferred language with supported fallback, and differing display/regional locales. Verify exact normalization/fallback; compare translation key/signature sets. Raw target and canonical API data remain intact. |
| L02 Layout | In each shipped locale and high DPI, inspect main minimum size, long tree labels, grid details, all provider cards, and both modal levels. Essential controls remain reachable, provider cards fill width, text wraps/scrolls without overlap. |
| L03 Input/modal ownership | Test Tab/Shift+Tab, tree arrows, Enter activation, grid draft keys, nested modal OK/Cancel/close, and focus restoration. Node double-click only expands/collapses; Inspect and Enter do not create duplicate modals; asynchronous completion cannot access a destroyed window. |

#### Property-editor acceptance contract

The setter whitelist is shared with Actions:

| Editable value | Client method | Editor and validation |
| --- | --- | --- |
| Value.Value | `IUIAutomationValuePattern::SetValue` | Text; only when not read-only. Inline only for the verified native single-line Edit predicate below. All other text values use the modal editor. |
| RangeValue.Value | `IUIAutomationRangeValuePattern::SetValue` | Finite number inside current min/max, not read-only. Do not silently clamp or invent a step restriction. |
| Window.WindowVisualState | `IUIAutomationWindowPattern::SetWindowVisualState` | Enum; disable unsupported minimize/maximize choices using capabilities. |
| Dock.DockPosition | `IUIAutomationDockPattern::SetDockPosition` | Defined DockPosition choice. |
| MultipleView.CurrentView | `IUIAutomationMultipleViewPattern::SetCurrentView` | Choice from actual supported IDs/names. |
| Transform2.ZoomLevel | `IUIAutomationTransformPattern2::Zoom` | Finite numeric percentage inside reported limits and CanZoom. |
| Scroll percentages | `IUIAutomationScrollPattern::SetScrollPercent` | Numeric 0..100 for supported axis; the unchanged/unsupported axis uses `UIA_ScrollPatternNoScroll`. |
| LegacyIAccessible.Value | `IUIAutomationLegacyIAccessiblePattern::SetValue` | Keep the grid row read-only and offer only the explicit legacy SetValue action. This version does not infer a generic writable legacy-property contract. |

ToggleState, selection state, bounding rectangles, focus, Name, AutomationId, and other readouts do not acquire guessed setters. Their proper action, if any, remains in Actions.

The single-line predicate requires ControlType Edit, a nonzero NativeWindowHandle whose `ElementFromHandle` identity matches this node, native window class `Edit`, no `ES_MULTILINE` in `GetWindowLongPtrW(GWL_STYLE)`, and a current value without CR/LF. Any unmet condition selects the modal editor; an empty value alone proves nothing. Native `ES_MULTILINE` semantics are defined by [Edit control styles](https://learn.microsoft.com/en-us/windows/win32/controls/edit-control-styles). The pattern's IsReadOnly still governs whether editing is allowed. Do not use native messages to read or write target text as a substitute for UIA.

### Standard-pattern acceptance inventory

These operation and result requirements apply to the inspector, which can inspect arbitrary Windows providers. They do not claim that GacUI itself implements all these patterns. Parameterless pure getters are readouts; parameterized pure getters run on valid commits; mutations, listening and range-workspace operations require explicit commands. Check the current native catalogs linked in [source ownership](../README.md#source-ownership) and compare them with the selected SDK before changing this inventory.

The registry below is the required SDK baseline. The local Windows SDK `10.0.26100.0` declares 35 patterns (IDs 10000 through 10034). The [Microsoft pattern-ID reference](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpattern-ids) is useful for links, but the SDK plus individual interface documentation governs omissions such as Selection2. Except for the explicit version rows, a pattern named X maps to client `IUIAutomationXPattern` and provider label `IXProvider`.

| Pattern ID / name | Readouts and exact client operations | Parameter UI and acceptance behavior |
| --- | --- | --- |
| 10000 Invoke | `Invoke()` | One Invoke button. Fixture action counter increases once, including when the action opens a target modal. |
| 10001 Selection | CanSelectMultiple, IsSelectionRequired, `GetCurrentSelection()` | Automatic parameterless getter readout on inspection/refresh, with inspectable element links. Cover none/one/multiple. |
| 10002 Value | Value, IsReadOnly, `SetValue()` | Draft text and Set; use the same multiline/read-only contract as Properties. Read back provider-normalized text. |
| 10003 RangeValue | Value, IsReadOnly, Minimum, Maximum, SmallChange, LargeChange; `SetValue()` | Finite numeric draft and Set; test min/mid/max and prevent writes outside known bounds. Preserve unsupported/NaN readouts where documented rather than forcing zero. |
| 10004 Scroll | Horizontal/Vertical ScrollPercent, ViewSize, Scrollable; `Scroll()`, `SetScrollPercent()` | Two ScrollAmount choices; two percentage drafts with explicit NoScroll choices. Test each amount and each axis; a disabled axis receives the documented no-scroll value. |
| 10005 ExpandCollapse | ExpandCollapseState; `Expand()`, `Collapse()` | Two buttons enabled for appropriate states; LeafNode is read-only, PartiallyExpanded is displayed accurately. Refresh topology after changes. |
| 10006 Grid | RowCount, ColumnCount; `GetItem(row,column)` | Zero-based integer inputs validated against counts; returned cell link. Test corners, merged-cell references, and bounds rejection. |
| 10007 GridItem | Row, Column, RowSpan, ColumnSpan, ContainingGrid | Read-only positions/spans and grid link. Verify merged spans and actual containing-grid identity. |
| 10008 MultipleView | CurrentView; `GetCurrentSupportedViews()`, `GetViewName(id)`, `SetCurrentView(id)` | ID/name choice plus query/set. No assumptions that IDs are contiguous. Each view refreshes affected data. |
| 10009 Window | CanMaximize, CanMinimize, IsModal, IsTopmost, WindowVisualState, WindowInteractionState; `SetWindowVisualState()`, `WaitForInputIdle(ms)`, `Close()` | Supported-state choice, bounded nonnegative idle timeout, Boolean idle result, explicit Close button. Run Close last on a disposable target. Client spelling is not the provider's `SetVisualState`. |
| 10010 SelectionItem | IsSelected, SelectionContainer; `Select()`, `AddToSelection()`, `RemoveFromSelection()` | Three commands with known container capability/required-selection constraints; inspect container and verify actual selection changes. |
| 10011 Dock | DockPosition; `SetDockPosition()` | Enum choice and Set; test supported positions and None. Do not infer permitted docking from ControlType. |
| 10012 Table | RowOrColumnMajor; `GetCurrentRowHeaders()`, `GetCurrentColumnHeaders()` | Header lists with links, including empty lists. Grid is discovered separately. |
| 10013 TableItem | `GetCurrentRowHeaderItems()`, `GetCurrentColumnHeaderItems()` | Cell-specific header lists; verify their relationship to the selected cell. GridItem is separate. |
| 10014 Text | SupportedTextSelection, DocumentRange; `GetSelection()`, `GetVisibleRanges()`, `RangeFromPoint()`, `RangeFromChild()` | Range browser; physical screen X/Y and a child-element picker. Every returned range exposes the full range workspace below. |
| 10015 Toggle | ToggleState; `Toggle()` | One Toggle button; test fixture state cycle. Do not implement `SetToggleState` or silently toggle repeatedly to reach a requested state. |
| 10016 Transform | CanMove, CanResize, CanRotate; `Move(x,y)`, `Resize(width,height)`, `Rotate(degrees)` | Labeled physical coordinates/dimensions and angle, finite values, positive sizes; disable unsupported commands and verify target geometry. |
| 10017 ScrollItem | `ScrollIntoView()` | One command; verify item becomes visible without claiming a selection change. |
| 10018 LegacyIAccessible | ChildId, Name, Value, Description, Role, State, Help, KeyboardShortcut, DefaultAction; `GetCurrentSelection()`, `Select(flags)`, `DoDefaultAction()`, `SetValue()`, `GetIAccessible()` | Named legal selection flags with a mask preview; text action; selection links; opaque IAccessible result. Include this bridge for arbitrary inspected apps. |
| 10019 ItemContainer | `FindItemByProperty(startAfter, propertyId, value)` | Nullable start-after element, known property descriptor and typed value. ID 0 provides next-item enumeration semantics; value ignored as specified. Return element or explicit no result. Do not convert enumeration order to a row number. |
| 10020 VirtualizedItem | `Realize()` | Explicit button; refresh to materialized state without automatically realizing other placeholders. |
| 10021 SynchronizedInput | `StartListening(inputType)`, `Cancel()` | Named SynchronizedInputType flags with legal combinations and mask preview, Start and Cancel. This does not inject input. Show listening request status, not invented received/discarded event outcomes. Keep its worker-side listening handle across content refreshes; Start/Cancel do not invalidate content generations. Disable repeat Start until Cancel, since this app does not subscribe to completion events. Cancel before normal session disposal if a request remains tracked. |
| 10022 ObjectModel | `GetUnderlyingObjectModel()` | Automatic parameterless getter readout; null or opaque COM reference description. No arbitrary IDispatch crawling/editor or application-specific object-model operations. |
| 10023 Annotation | AnnotationTypeId, AnnotationTypeName, Author, DateTime, Target | Read-only fields and target link; preserve provider date text rather than inventing timezone conversion. |
| 10024 Text2 | `UIA_TextPattern2Id`, client `IUIAutomationTextPattern2`, provider `ITextProvider2`; inherited Text plus `RangeFromAnnotation()`, `GetCaretRange()` | Annotation-element picker; caret range and active flag. Include inherited Text when base acquisition is absent. |
| 10025 Styles | StyleId, StyleName, FillColor, FillPatternStyle, Shape, FillPatternColor, ExtendedProperties; `GetCurrentExtendedPropertiesAsArray()` | Readouts with numeric/color identity and an inspectable extended-property array. Unknown IDs remain visible. |
| 10026 Spreadsheet | `GetItemByName(name)` | Text name input and query; existing/missing cell results. |
| 10027 SpreadsheetItem | Formula; `GetCurrentAnnotationObjects()`, `GetCurrentAnnotationTypes()` | Read-only formula and aligned annotation values/references. No inferred formula setter. |
| 10028 Transform2 | `UIA_TransformPattern2Id`, client `IUIAutomationTransformPattern2`, provider `ITransformProvider2`; inherited Transform, CanZoom, ZoomLevel/Minimum/Maximum; `Zoom()`, `ZoomByUnit()` | Percentage within reported limits and ZoomUnit choice, guarded by CanZoom; cover all units and inherited Transform. |
| 10029 TextChild | TextContainer, TextRange | Container link and returned-range workspace; verify embedded-child mapping. |
| 10030 Drag | IsGrabbed, DropEffect, DropEffects; `GetCurrentGrabbedItems()` | Readouts and grabbed-item links on Refresh; actual dragging is performed in the fixture. No invented StartDrag. |
| 10031 DropTarget | DropTargetEffect, DropTargetEffects | Read-only effects refreshed before/during/after fixture drag. No invented Drop method. |
| 10032 TextEdit | Inherited Text; `GetActiveComposition()`, `GetConversionTarget()` | Composition/conversion range results; include base Text operations. No insertion, deletion, replace, or paste command is implied. |
| 10033 CustomNavigation | `Navigate(direction)` | NavigateDirection choice, query, and nullable element link; keep distinct from inspector tree expansion. |
| 10034 Selection2 | `UIA_SelectionPattern2Id`, client `IUIAutomationSelectionPattern2`, provider `ISelectionProvider2`; inherited Selection, FirstSelectedItem, LastSelectedItem, CurrentSelectedItem, ItemCount | Readouts and links for no/one/multiple selection. ItemCount is selected-item count, not total container count. |

Before any command is enabled, validate typed arguments in C++: full-string numeric parsing, finite required numbers, documented bounds/enums, correct element/range ownership, and legal optional values. Command identifiers, UI labels, and formatted readout strings must not be used to reconstruct typed COM arguments. The worker repeats capability checks where the target can change; an unexpected HRESULT reports the actual operation failure, not success or an optimistic local update.

Use invariant decimal syntax with a period and no thousands separators for technical numeric drafts, and format their initial values the same way in every locale; localize the explanatory labels. Reject embedded NUL in arguments of null-terminated UIA string methods rather than silently submitting only a prefix. Preserve embedded NUL when displaying a BSTR result. A command rejected before acceptance clears its busy/pending state and returns a concrete validation/rejection result.

Every element-valued result supports Inspect. A result in the selected raw subtree also supports Reveal in Nodes. An external result opens its own property VM with a reference label but no fabricated tree ancestry; Reveal is disabled with an explanation. An action that needs an element parameter uses the current root's nodes and retained returned references, preserving identity even for duplicate names. Cross-root inspection does not silently change the selected top-level window.

After a target mutation, reread the affected element/properties/pattern capabilities and rebuild the selected root snapshot/capture once. Publish the post-action generation as one UI update; preserve the issuing dialog by verified element identity, update its generation, and then allow further commands. Old text ranges are discarded after a target mutation or explicit refresh and must be reacquired. Range-workspace operations that only change a range's endpoints are local COM range operations and do not rebuild the root or destroy that workspace. Pure queries do not refresh the whole tree.

After a local range endpoint/Find/Clone operation, refresh that result range's text preview, attributes, and rectangles while retaining the other workspace operands. Range selection/scroll/context-menu actions change the target: verify their results with fresh queries, then clear/reacquire entry ranges in the successor generation. SynchronizedInput's explicitly retained listening request follows its own lifecycle in the registry instead of this content-mutation rule.

For `Window.Close`, successful return is followed by closing affected inspector dialogs and clearing a selected root that has closed; do not try to read that destroyed window as ordinary post-action refresh. Closing an external result window refreshes the selected root only if still relevant. Unexpected target disappearance during a different read/action follows the error policy.

#### Returned text-range workspace

Ranges are returned objects, not raw-tree nodes or independently discoverable element patterns. Name them locally (Document, Selection 1, Visible 1, Result 1, Clone 1) and show their owning document. Establish document identity from the originating TextPattern element, or TextChild.TextContainer for TextChild results; propagate it unchanged through Clone and Find results. The inspector node returning a range is not necessarily its document. Keep two selectable named range operands from the same document. Offer all methods in [IUIAutomationTextRange](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nn-uiautomationclient-iuiautomationtextrange), with typed inputs and visible results:

| Method group | Controls and result contract |
| --- | --- |
| `Clone`, `Compare`, `CompareEndpoints` | Clone adds a separate range; comparisons select a second range and endpoints and display Boolean/signed comparison. Never compare range pointer addresses. |
| `ExpandToEnclosingUnit` | TextUnit choice: Character, Format, Word, Line, Paragraph, Page, Document. Show resulting text/range and documented provider fallback to larger units. |
| `Move`, `MoveEndpointByUnit` | Signed count, TextUnit, optional endpoint; display actual moved count as well as the changed span. |
| `MoveEndpointByRange` | Source endpoint, second range, target endpoint; show normalized result, including degenerate spans. |
| `FindText` | Text, backward, ignore-case; return named range or no match. |
| `FindAttribute`, `GetAttributeValue` | Catalog attribute choice, typed search value and backward for Find; typed result distinguishing Mixed and Not supported for Get. |
| `GetText` | Max length accepts `-1` or a nonnegative integer. Full text is available on demand; bounded preview shows truncation explicitly. |
| `GetBoundingRectangles` | Display each physical rectangle and optional preview overlay when it belongs to the published root snapshot. Empty results do not imply a fabricated caret rectangle. |
| `GetEnclosingElement`, `GetChildren` | Inspectable element/list results using normal reference navigation. |
| `Select`, `AddToSelection`, `RemoveFromSelection` | Respect SupportedTextSelection, execute one command, verify actual target selection; these are target mutations. |
| `ScrollIntoView(alignToTop)` | Boolean alignment input; refresh after target scroll. |
| `IUIAutomationTextRange2::ShowContextMenu` | Expose only after QI succeeds; opens target menu for that span. |
| `IUIAutomationTextRange3` | Probe returned range for this client-only extension. `GetEnclosingElementBuildCache`, `GetChildrenBuildCache`, and `GetAttributeValues` expose cached-reference and selected-attribute batch queries using the app's fixed cache recipe. Compare with ordinary getter results. There is no inferred `ITextRangeProvider3`. |

Include all named SDK text attributes in the attribute selector and a Read all attributes operation. The SDK baseline has 44 attributes (40000 through 40043); use named descriptors rather than assuming numeric contiguity forever. Each descriptor supplies exact expected argument/result type and enum/value formatting. TextRange3 is a client extension documented [here](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nn-uiautomationclient-iuiautomationtextrange3).

#### Element-level operations and completeness boundary

Add an Element section before pattern cards: base `SetFocus`, `GetClickablePoint` with availability flag, and `IUIAutomationElement3::ShowContextMenu` if acquired. `SetFocus` is an explicit action; ordinary selection/hover never calls it. Query supported Element7 metadata through `GetCurrentMetadataValue`, with a known target PROPERTYID and metadata descriptor (baseline `UIA_SayAsInterpretAsMetadataId`). Text attributes use range APIs instead. The remaining Element2-9 property additions are covered by the property catalog. Cache/search/version helper APIs underpin inspection; they do not need duplicate arbitrary low-level COM-call panels.

The current catalog baseline is **41 control types, 35 patterns, 175 element/pattern property IDs, 44 text attributes, and the named metadata IDs in the selected SDK**. Check it against the actual build SDK before implementation acceptance. Custom/vendor IDs not known to the SDK, provider implementation internals (`IRawElementProviderFragment`, event advice, HWND override), and arbitrary returned object methods are not generically discoverable. This boundary must be explicit in Help; an unsupported formatter must never be used to excuse omission of a known standard descriptor.

### Pattern/action and range acceptance

**A00 applies to every one of the 35 registry rows:** test pattern absent and present, exact provider/client naming, all readouts, every parameter editor, all commands, null/empty returned results where valid, dynamic capability changes, and error diagnostic paths. The fixture must assert actual target key, client operation's provider counterpart, typed arguments, call count, and resulting state. Read-only pattern cards pass by correct complete readings, not invented commands. Versioned patterns are tested alone and with their base pattern.

Cases **A01 through A35** correspond respectively to numeric pattern IDs **10000 through 10034** in the [standard-pattern acceptance inventory](#standard-pattern-acceptance-inventory). Each row's acceptance behavior is mandatory. For methods with enumerated inputs (scroll amounts, dock/visual states, navigate direction, input type, zoom unit), exercise every documented value supported by its fixture scenario plus local rejection of invalid values. Use separate disposable fixtures for Window.Close and expected failures so one action cannot invalidate later evidence.

Additional required results: ScrollItem does not select; Selection2 counts selected items; ItemContainer accepts null start-after and ID zero and returns null at the end; TextEdit shows live fixture composition/conversion ranges when explicitly refreshed; Drag/DropTarget are read during a real fixture drag, with no invented input action; SynchronizedInput receives real input through the fixture UI and Cancel is observed in its call/state log. ObjectModel and GetIAccessible stay opaque. Every element result is inspected, and a cross-root result has no fake Reveal path.

| ID | Text-range procedure and required result |
| --- | --- |
| R01 Acquisition | Obtain Document, selection, visible, child, point, annotation, caret, composition and conversion ranges in F04. Cover empty/null/degenerate/multiple results and active caret flag. |
| R02 Identity/comparison | Clone A to B; compare equal, then move B and verify A is unchanged. Compare TextChild-derived and TextPattern-derived ranges from the same document. Test all start/end pairings in CompareEndpoints; wrong-document operands are rejected before dispatch. |
| R03 Units/movement | Expand each TextUnit; Move and MoveEndpointByUnit with negative, zero, positive and excessive counts. Report actual moved counts and provider fallback/endpoint normalization accurately. |
| R04 Endpoint transfer | MoveEndpointByRange for each endpoint pairing, including crossing/collapse to a degenerate span; result matches fixture. |
| R05 Searching | FindText forward/backward and case modes with Unicode/no match. FindAttribute uses typed values and returns the intended range/no result. |
| R06 Attribute reads | All 44 named descriptors render; uniform/mixed/unsupported values remain distinct. TextRange3 GetAttributeValues returns results in requested order matching individual reads. |
| R07 Text/geometry | GetText with -1, 0, small limit and large text; preserve returned control characters. Wrapped text yields multiple physical rectangles; empty geometry does not fabricate a caret. |
| R08 Relationships/cache | GetChildren and GetEnclosingElement point to correct identities; TextRange3 BuildCache variants agree and use the fixed cache request. All links support the defined Inspect/Reveal behavior. |
| R09 Mutations | Select/Add/Remove obey no/single/multiple selection; ScrollIntoView supports both alignments. Verify real target state, then stale workspace is cleared/reacquired according to generation rules. |
| R10 Context/lifetime | TextRange2 menu opens for the right range; unsupported QI hides the command. Close dialog/refresh/switch target with ranges retained: no wrong-session action, leak, or stale completion. |

### Lifecycle, failures, and final acceptance

| ID | Procedure and required result |
| --- | --- |
| S01 Ownership | Instrument/debug one run to verify every UIA COM call/ref release on its MTA owner and every bound-collection/GacUI update on UI thread. Capture callbacks publish only owned data. |
| S02 In-flight close | Close inspector and both modal levels with queued reads, capture callback, and accepted action. No worker waits on a UI callback while UI waits for worker; normal close drains/releases, endpoint stops, and process exits cleanly. |
| S03 Refresh/action ordering | Delay old reads, accept one action, switch target before it starts/completes, then release the delay. Accepted action executes once on its retained original target and cannot update the new UI; rejected-before-acceptance commands clear busy state. Separately keep the issuing target current and verify the base-to-successor handoff publishes the action's own post-state instead of discarding it. |
| S04 Expected absence | Empty processes/search/selection, null references, no patterns, unsupported attributes, documented optional getter `UIA_E_NOTSUPPORTED` (including Styles), and absent interface extensions remain distinguishable normal states, with no retry loops. |
| S05 Error exits | Separate runs destroy a target during read/action and inject provider/access/transport failures. Diagnostic includes operation/ID/HRESULT and process exits unsuccessfully. No false success, swallowed exception, stale update, or indefinite retry. Invalid local input makes no COM call. |
| S06 Capture/service failures | Unsupported capture, Closed before first frame, and a no-frame deadline complete as unavailable exactly once. Race deadline/Closed/FrameArrived/cancel and verify later callbacks cannot publish. Unexpected device failure and occupied automation port follow the defined fatal policy. Check runtime dialogs on any stalled execution. |
| S07 Architecture/lifetime | Run x64 inspector against x64 and Win32 fixture, then Win32 inspector against both. Repeat selection, modal, refresh and normal-close cycles; inspect Debug leak reports for COM/native and Vlpp ownership errors. If instability appears, apply the repository's 25 consecutive successful rerun rule after fixing it. |

Implementation is ready when every applicable case has reproducible passing evidence, all 35 patterns and every listed method have actual handlers, SDK catalogs are complete, every translation is synchronized, generated files reproduce from authored inputs, all four configurations build/run, and normal Debug shutdown is leak-free. A platform capability absence can justify a negative-case result, but it cannot stand in for the corresponding success-path verification on a supported fixture/OS. Record any unresolved coverage as unfinished work.

### Provider-group layout acceptance

Actions keeps Providers, Results, References and Text ranges. All provider and range sections use full-width titled group boxes without expansion state. Parameterless getters remain readouts, parameterized getters run only on valid commits, and explicit mutations/listening/workspace actions execute once. No display/initialization path starts one full inspection per group. Use independent fixture logs for call counts.

Measure approximately five pixels between visible controls and around groups, including skin insets. Empty validation/status/reference rows have zero minimum size and no padding; actual text wraps. Check controls with/without parameters, readouts, returned references and short/long text-range groups through Controls and screenshots at default/reduced sizes. Verify the last control remains reachable after scrolling. XML padding equality alone is not acceptance evidence.

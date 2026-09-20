# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Two work to improve `<ez:EasyLayout/>` and UIA supports.

## EazyLayout

Here are code snippets from `Release/Tutorial/GacUI_Controls/AddressBook/UI/Resource.xml`. I don't mean here is the only defect, you should take that as an example for better understanding my request:

```XML
<ez:Layout Padding="5" Border="false">
  <ez:Top>
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <ToolstripToolBar>
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
        <ToolstripButton Command-ref="commandNewFolder"/>
        <ToolstripButton Command-ref="commandDeleteFolder"/>
      </ToolstripToolBar>
    </Bounds>
  </ez:Top>
  <ez:Fill Percentage="1.0" Direction="Vertical">
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <BindableTreeView ref.Name="treeViewFolders" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false" env.ItemType="demo::ICategory^">
        <att.ItemSource-eval>ViewModel.GetRootCategory()</att.ItemSource-eval>
        <att.TextProperty>Name</att.TextProperty>
        <att.ImageProperty>Image</att.ImageProperty>
        <att.ChildrenProperty>Folders</att.ChildrenProperty>
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
      </BindableTreeView>
    </Bounds>
  </ez:Fill>
</ez:Layout>
```

```XML
<ez:Row CellOption="composeType:Percentage percentage:1.0">
  <ez:Column CellOption="composeType:MinSize"/>
  <ez:Column CellOption="composeType:Percentage percentage:1.0"/>
  <ez:Column CellOption="composeType:MinSize"/>
  <ez:Column CellOption="composeType:MinSize"/>
</ez:Row>
<ez:Row CellOption="composeType:MinSize">
  <ez:Column CellOption="composeType:MinSize"/>
  <ez:Column CellOption="composeType:Percentage percentage:1.0"/>
  <ez:Column CellOption="composeType:MinSize">
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <Button Text="OK">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0" PreferredMinSize="x:60"/>
        <ev.Clicked-eval><![CDATA[{ self.Ready = true; self.Close(); }]]></ev.Clicked-eval>
      </Button>
    </Bounds>
  </ez:Column>
  <ez:Column CellOption="composeType:MinSize">
    <Bounds MinSizeLimitation="LimitToElementAndChildren">
      <Button Text="Cancel">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0" PreferredMinSize="x:60"/>
        <ev.Clicked-eval><![CDATA[{ self.Ready = false; self.Close(); }]]></ev.Clicked-eval>
      </Button>
    </Bounds>
  </ez:Column>
</ez:Row>
```

In the original spec `TODO_Task_EazyLayout.md` there are two rules:
- ez:Fill@Direction is only needed when the layout has only ez:Fill at a certain level and only one is needed. Here we have ez:Top as its sibling node, so Direction="Vertical" should be inferred.
- When rows over columns (vice vesa), for every column position, only one ez:Column needs to specify `CellOption`. Here we repeated everything. By the way, since the default value is MinSize, it means when multiple ez:Column on the same column position conflicts, all MinSize will be ignored.

Basically the XML is technically correct, but I would like you to omit whatever you can, so that it could be used as a test to see if omitting actually works or not. And tutorials in Release folder are samples, people are going to learn from them, keep them simple would be one of the goal. Here my request is that, update all XML UI in `GacUI_Controls.sln` to omit ez:* properties that are not needed, rebuild the solution, starts every affected apps, and make sure the visible layouts are not changed.

By the way, in newly added test cases running in remote protocol based unit test framework, there are some tests to see if `BuildLayout` crashes. I don't think it is appropriate to put them here because the goal for remote protocol based unit test framework is to see how UI actually layouted in each frame. To test the error, we should move them to `TestCompositions_*.cpp`, it is better to create a dedicated file for such cases `TestCompositions_EazyLayoutFailures.cpp`.

### DETAILS

- The implemented XML element is `ez:Layout`. The tutorial solution is in the sibling Release repository at `../Release/Tutorial/GacUI_Controls/GacUI_Controls.sln`, not this repository's generated `Release` directory. The requested edits apply to authored tutorial XML, including linked resources, templates, and shared components used by that solution. Regenerate their C++ outputs through the existing resource tools.
- Use `TODO_Task_EazyLayout.md`, `Source/GraphicsComposition/EazyLayout/GuiEasyLayout.h/.cpp`, and `Source/Compiler/InstanceLoaders/GuiInstanceLoader_EasyLayout.cpp` as the contract and implementation references. Current defaults are `Padding="5"`, `Border="true"`, `Percentage="1"`, `Direction="Inherited"`, `CellOption="composeType:MinSize"`, and `CellSpan="1"`. Remove redundant attribute and property-element forms while preserving effective values and any binding or script behavior. Equal fill weights can also become omitted defaults collectively: the `0.5:0.5` pair in `ListControls/UI/TreeViewTabPage.xml` has the same ratio as `1:1`; verify rounding and do not change one weight independently. In the first example, omit `Padding`, `Percentage`, and `Direction`, but keep `Border="false"` and the payload's bounds/alignment settings.
- Infer each descriptor group's direction from its docking siblings. For a fills-only group, retain one explicit direction only when the intended axis cannot be inherited; the outermost fallback is horizontal. Remove duplicate declarations only within that group. A nested `ez:Layout` starts a separate layout root, so direction does not inherit through arbitrary payload compositions into that root.
- Deduplicate `CellOption` only among inner descriptors representing the same shared track: columns in rows, or rows in columns. Outer row/column options describe separate tracks. `MinSize` contributes no conflicting nondefault option; agreeing nondefault options can be reduced to one declaration. Only inner descriptors with `CellSpan == 1` contribute track options. Preserve empty descriptors, nondefault spans, and the single-span descriptors required to define tracks; do not remove structural cells while simplifying their properties. Retain one percentage option on the second shared column in the second example, and retain the first outer row's percentage option.
- The current test placement differs from the reported description. Runtime `BuildLayout` rejection cases already use direct C++ in `Test/GacUISrc/UnitTest/TestCompositions_EasyLayout.cpp`. Consolidate those failure cases, including invalid branches mixed into acceptance matrices, in the requested `TestCompositions_EazyLayoutFailures.cpp`; preserve positive layout/rebuild coverage in the existing files. Assert the expected rejection without starting a window or remote-protocol frame loop, and preserve descriptor/payload ownership when cleaning up failed builds.
- The negative cases in `TestControls_EasyLayout.cpp` call `PrecompileResource` to test forbidden bindings, runtime expressions in constant fields, and invalid child grammar; they do not instantiate a window or call `BuildLayout`. Move this compiler-only coverage to the existing non-frame compiler harness in `TestResource.cpp`, following its `SetGuiMainProxy` / `SetupGacGenNativeController` lifecycle. Keep all diagnostics and source-position assertions. The namespace serialization-only case can use that harness too; instantiated XML and visible-frame tests remain in `TestControls_EasyLayout.cpp`.
- Register the new failure-test file in `UnitTest.vcxproj` and its filters according to `.github/Guidelines/SourceFileManagement.md`. If omission exposes a runtime/compiler defect, fix the owning implementation to satisfy the original specification rather than restoring redundant tutorial declarations.

### VERIFICATION

1. Before editing tutorial XML, map every affected resource to its runnable consumers and capture baseline screenshots and control/composition bounds with fixed DPI, skin, window size, and application state. Include XML linked by other resources. `DocumentEditor` is a shared library: changes there must be checked in both `DocumentEditorToolstrip` and `DocumentEditorRibbon`. Do not assume the historical `EasyLayoutVerification` helper files are still available.
2. Regenerate tutorial resources with the sibling Release repository's `Tools/GacBuild.ps1` and `Tutorial/GacUI.xml` driver, using absolute paths. Inspect both architecture results and resource error artifacts; the script continuing to its end does not prove every resource compiled. Rebuild `GacUI_Controls.sln` in Debug Win32 and x64 through the documented build wrapper from that solution directory. If GacUI runtime/compiler sources change, publish them through the existing release workflow before testing the tutorial consumers, so the tests use the changed implementation.
3. Start every affected runnable tutorial through the execution wrapper and compare its visible layout and bounds with the baseline. Cover initial, smaller, larger, maximized, and restored sizes, and every affected tab, dialog, popup, item template, or shared component. Exercise AddressBook splitters/dialogs and ColorPicker2's repeated palette where affected. Preserve native layouts such as CalculatorAndStateMachine's table. Check actual spacing, sizing, clipping, and alignment, not merely successful startup. Use the HTTP automation surface and screenshots first, followed by UIA checks as documented in `.github/Guidelines/Running-GacUI.md`.
4. Verify omission coverage for both docking axes, fills-only inherited/explicit directions, a nested independent layout root, rows-over-columns and columns-over-rows, and shared-track options mixed with omitted `MinSize` defaults. Preserve rejection of conflicting nondefault options, invalid spans, values, grammar, and cyclic/reused descriptor structures. Retain or extend focused existing tests only where these behaviors are not already asserted.
5. Build `Test/GacUISrc/GacUISrc.sln` and run its configured `UnitTest` through `.github/Scripts/copilotBuild.ps1` and `copilotExecute.ps1`. Ensure the new failure file, affected composition/frame tests, and destination compiler tests are not skipped; preserve unrelated configured filters. Verify that rejection cases produce no remote UI frames, positive frame results remain equivalent, and no cases disappear during relocation. Inspect compiler diagnostics, test totals, snapshot diffs, and the Debug leak report after the test summary. Generate any snapshot changes through their owning tests.

## UI Automation

### Responding to WM_GETOBJECT

The current implementation is too proactive. In order to improve the performance when no UIA client is asking for it:
- Any UIA interface implementation (UIA object) should be created when it is requested, do not create any UIA object ahead of time.
- COM is based on reference counting so it should be easy to do: when any references from UIA client is lost, those objects should be released directly, as well as detaching all listening events. Since GuiWindow exposes events to tell you when a composition is removed from the control, verify did the code already handle these event to release those objects and detecthing all listening events.
  - This is important, as currently I found that when a sub window is created and then deleted, closing the app causing a crash in UIA. I have not debugged into it, but I doubt this could be a reason.
- When UIA is running the GacUI itself seems to be very slow, I will come back and check this later to see if the lazy UIA creation resolves this issue or not.
  - The most easy observation is the progress bar tutoail in `GacUI_Controls`, when the document is completed downloaded after clicking the button and waiting for the progress bar to reach the end, maximizing the window takes a lot of time in debug mode, it did not happen before implementing UIA.

Improving UiaListApp tool:
- In `UiaListApp`, right click UIA node to popup a context menu for "Inspect" is technically working, but you have to pass the cursor position to show the context menu in a correct position, currently it is popup at the middle of the top border, which is not good in terms of user experience.

### DETAILS

- The provider implementation is under `Source/PlatformProviders/Windows/UIAutomation`; its current design is documented in `.github/KnowledgeBase/KB_GacUI_Design_UIAutomation.md`. Audit more than the `WM_GETOBJECT` factory: `NativeWindowCreated` queues `BindWindows`, which recursively scans compositions, attaches control/model hooks, and collects snapshots. Snapshot collection and event raising can themselves call `Provider()`. Merely delaying the first root provider will not remove this eager work.
- Treat returning an interface to a UIA request and delivering a subscribed UIA event as demand. Create root/child providers, patterns, and text ranges only as needed by those operations. Window creation, ordinary layout changes, and accessibility metadata alone must not instantiate COM providers or start full-tree property observation. Preserve semantic navigation independently of discovery order: radio groups and combo/popup ownership currently consult collections populated by eager scanning.
- Remove owning caches and cycles that prevent final-reference cleanup. `WindowsUIAutomationNode::provider` currently owns a COM provider, while that provider owns its node; snapshots can also retain provider-valued variants. Audit these along with text ranges and queued work. Keep identity consistent while an element is represented, and make release followed by reacquisition safe. A naive non-owning cache also needs synchronization so acquisition cannot race the final release or resurrect a retiring object.
- Interpret the requested last-reference release as the final outstanding COM reference, including references held by UIA infrastructure, other clients, patterns, and ranges. COM does not identify the disappearance of a particular external client, and UIA's raised-event map can retain providers. Release the implementation's own unnecessary retention promptly and preserve the required HWND-map cleanup. See Microsoft's [IUnknown::Release contract](https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release) and [UiaReturnRawElementProvider cleanup](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiareturnrawelementprovider).
- Give event/model observation an explicit, detachable lifetime. Current control/composition `AttachLambda` calls discard handler tokens, and model callbacks detach in `WindowsUIAutomationLifetime` destruction. Retain enough information to detach every owned hook when its last consumer disappears, clear stale internal-property/cache records, and reattach without duplication. Active subscriptions or retained ranges may still require shared observation. Perform UI-object access, detachment, and cache mutation on the UI thread even when final COM release occurs on another thread; queued cleanup must remain safe during shutdown.
- Handle `GuiCompositionUpdateEventArgs` removal information synchronously while the child is available. The current `ChildCompositionUpdated` handler ignores the update kind/parent/child and queues a rescan, which cannot rediscover an already removed subtree. Invalidate the old attachment and detach its observation without keeping raw event pointers for later work. Define removal/reinsertion and cross-window reparenting so retained old providers cannot access a disposed or detached host, while a newly attached control remains discoverable. Include template replacement and hosted child windows.
- Preserve event-only clients: a subscription may require an event from a previously unvisited descendant. Do not require an already-created provider before sending requested window, menu, focus, selection, or text events. Retain synchronous `WindowClosed` before modal deletion. `UiaClientsAreListening` is not an ownership test; subscription tracking, if needed, must handle matching additions/removals and multiple clients. See Microsoft's [IRawElementProviderAdviseEvents contract](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovideradviseevents).
- Retire providers before disconnecting them and preserve deferred `UiaDisconnectProvider`: it must not execute in a `SendMessage` response, and reentrant `WM_GETOBJECT` must not return the disconnecting provider. Preserve per-HWND `UiaReturnRawElementProvider(hwnd, 0, 0, nullptr)` cleanup and application shutdown ordering. Retained providers/ranges must return their unavailable result after retirement without dereferencing GacUI objects. See Microsoft's [UiaDisconnectProvider restrictions](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiadisconnectprovider).
- Treat the reported shutdown crash and `ProgressAndAsync` slowdown as symptoms to reproduce, not proven consequences of the ownership findings. Capture the crash with the debugger and compare equivalent Debug runs before attributing the delay to UIA. The performance scope is removal of eager UIA overhead; broader active-client optimization remains follow-up work as stated above.
- UiaList already calls `nodeMenu.ShowPopup(nodeTree, { x:arguments.x y:arguments.y } of Point)` in `Tools/UiaList/UiaList/UI/MainWindow.xml`. The likely issue is coordinate conversion: `GuiListControl::OnItemMouseEvent` forwards item-style mouse coordinates, and `GuiVirtualTreeListControl::OnItemMouseEvent` forwards them again, whereas `GuiPopup::ShowPopup(GuiControl*, Point)` expects coordinates relative to its owner control. Trace and verify this path, then convert the click to the owner's coordinates while preserving popup ownership and screen-edge adjustment. Keep the handler in authored XML per `Tools/UiaList/AGENTS.md`; retain clicked-node capture, stale-node checks, Enter-to-inspect, and double-click expansion behavior. Regenerate `UiaList/Source` through `../Tools/Tools/GacBuild.ps1` with `Tools/UiaList/GacUI.xml`, then rebuild the UiaList solution.
- Update the UIA knowledge-base lifecycle description after implementation so it reflects the final ownership, subscription, and retirement behavior.

### VERIFICATION

1. Establish a no-client baseline using native window operations or the HTTP automation surface without inspecting the target through UIA. Open, mutate, resize, and close windows, including secondary windows. Use debugger observations or focused instrumentation to distinguish COM providers/ranges, semantic nodes, attached hooks, snapshots, and queued notification work. After the change, these operations must create no UIA COM objects and must not trigger eager UIA property snapshots or whole-tree observation. Avoid accidentally creating demand with the verification tool itself.
2. With a native out-of-process client, request the root, selected children, patterns, provider-valued properties, and text ranges. Verify selective materialization, consistent identity, and order-independent radio/combo/popup relationships. Balance COM references explicitly instead of relying on managed garbage collection. Exercise release/reacquisition and confirm hooks do not accumulate. Account for infrastructure-held references when checking final destruction.
3. Retain provider, pattern, and text-range references across subtree removal, model replacement, template replacement, child-window deletion, and application shutdown. While the application remains alive, verify unavailable responses from retired elements; after process exit, require a bounded client-side UIA/RPC disconnection or unavailable failure rather than one exact provider HRESULT. Verify no stale object access, deadlock, or leaked provider/callback cycles. Exercise removal/reinsertion and cross-window moves. Repeat the reported create/delete-child-then-close-app sequence for 25 consecutive successful runs with debugger crash/leak checks, including pending notifications and a client releasing from another thread.
4. Use two clients so one can release/unsubscribe while the other continues querying/listening. Subscribe before traversing newly created descendants and verify their requested events still arrive, especially focus and modal `WindowClosed`. Remove subscriptions and references and verify observation returns to the expected idle state. Check matching subscription lifetimes, concurrent queries/releases/removals, and queued work during shutdown.
5. Reuse relevant scenarios in `Test/UIA_CppTest_Shared.ps1` and its native/managed client checks, including the existing Review3 modal-deletion loop. Run the Playground UIA review fixture in ordinary and `-HostedFixture` modes, and exercise both GDI and Direct2D consumers. Preserve tree, property, pattern, text, focus, selection, and event contracts; passing existing tests alone does not prove lazy construction or final-reference cleanup. Build GacUISrc and run configured UnitTest checks with the documented wrappers and post-summary leak inspection. If reflection changes, perform the required metadata generation/testing in `Project.md`.
6. In `ProgressAndAsync`, complete the same download/document load and measure repeated maximize/restore in equivalent Debug builds: first with no UIA client, then with active traversal/subscriptions, then after client references/subscriptions are released. Record elapsed times and UIA work/object counts at fixed DPI, window sizes, and document contents. Report the change in eager overhead without inventing an absolute timing threshold. Ensure the tutorial actually links the updated Windows provider through the release workflow.
7. Regenerate UiaList resources for both architectures, inspect resource diagnostics, and build `Tools/UiaList/UiaList.sln` through the wrapper. In UiaListApp, right-click nodes at different row heights and indentation levels, after scrolling, and after moving/resizing the window. Verify the menu opens at the actual click except for normal screen-edge clamping, and Inspect opens the clicked node even when another was selected. Check Enter, double-click, dismissal, and stale-node handling. Repeat with nondefault DPI and a second monitor when available; record unavailable display configurations explicitly. Inspect popup bounds/screenshots and close modal windows and the application normally without a crash.

## REVIEW COMMENTS

# UPDATES

## UPDATE

FYI it is accurate to capture the layout via automation service, actual screenshot will be your last choice

## UPDATE

by the way, when you make your Release repo commit, the first commit would be all updated xml files together, and the rest in the second commit.

# TEST [CONFIRMED]

Verification follows both matrices in the problem description. Establish tutorial geometry and screenshot baselines before simplifying XML; preserve all positive and rejection coverage while relocating tests. Establish no-client UIA construction and observation counts, reproduce lifetime and popup-coordinate behavior, then verify demand-driven ownership, subscriptions, retirement, shutdown and native clients. Use repository build/execution wrappers and inspect final logs for test totals, compiler errors and memory leaks. Record unavailable configurations explicitly.

Baseline GacUISrc Debug x64 build completed successfully on 2026-09-19; final Build.log reports zero warnings and zero errors.

The relocated test sources compile in Debug Win32 with zero warnings and errors. The complete baseline x64 UnitTest suite passed before rebuilding it. Confirmation here refers to the eager UIA work and popup-coordinate error; the reported shutdown crash was not reproduced.

No-client UIA reproduction: launched Debug x64 Playground under CDB with `/AsPort:8960`, without UIA inspection. Breakpoints counted 41 provider constructor calls and 716 UpdateProperties calls after the first snapshot breakpoint (at least 717 snapshots). The startup stacks are `NativeWindowCreated` queued callback -> `BindWindows` -> `Control` -> `UpdateProperties`; querying WindowVisualState during that snapshot enters `Node::Provider` and constructs a COM provider. This confirms eager startup work independently of any external client. It does not yet establish the reported shutdown crash or quantify ProgressAndAsync maximize latency. Screenshot capture was tested only after these counters were collected and works in this session.


## Recorded verification evidence

All raw captures, scripts and logs are in `C:/Users/vczh/AppData/Local/Temp/GacUIRefactor-20260919`. Automation control/composition bounds are the primary layout evidence, following the update above. Screenshots were captured earlier and used only to resolve visual ambiguities.

The resource map covers 32 authored XML files for GacUI_Controls, including the two external links: FullControlTest/ToolbarImages/Images.xml and DocumentEditorToolstrip.xml. Neither external linked file contains ez descriptors. Twenty authored XML files contain the 256 redundant declarations removed. Descriptor structure, nondefault spans/options, payload geometry, bindings and scripts are preserved. Both DocumentEditor consumers were checked.

Eighteen runnable tutorials were checked: ContainersAndButtons, ColorPicker, AddressBook, ProgressAndAsync, ListControls, DataGrid, CalculatorAndStateMachine, Animation, MenuVisibility, Localization, DocumentEditorToolstrip, DocumentEditorRibbon, TriplePhaseImageButton, QueryService, Win11ToolstripMenu, ColorPicker2, Dialogs and GlobalHotKey. The matrix includes initial/restored, smaller, larger, maximized and restored sizes; affected tabs, dialogs, menus and shared components; AddressBook splitters and folder/contact dialogs; and the repeated ColorPicker2 palette. CalculatorAndStateMachine's native table was preserved.

`geometry-reviewed.json` compares 190 matching states, with 185 exactly equal window/control/composition geometries. Five raw captures differ: moving particles in Animation/complex-tab, transient tooltips in GlobalHotKey/menu-bar and ListControls/treeview-tab/treeview-expanded, and pre-paint scrollbar/viewport measurements in ListControls/listview-Detail. The stable main layout matches in the tooltip cases; the settled detail-view images match. These five raw captures are not claimed to be byte-for-byte equal. One unmatched exploratory DocumentEditorToolstrip/current-edit baseline is excluded from the comparison.

Source GacUISrc builds pass Debug x64 and Win32 with zero warnings and errors. Completed full suites before and after relocation report 92/92 files and 1784/1784 cases, then 93/93 files and 1786/1786 cases. `UnitTest-reviewed-x64.log` is a further complete 93/93, 1786/1786 pass without an appended memory leak report. The final source suite, UnitTest-final-demand-x64.log, also passes 93/93 files and 1786/1786 cases without leaks. The dedicated runtime rejection file creates no remote frames. All 30 compiler-negative inputs and their diagnostics/source-position checks remain in TestResource; positive geometry/rebuild/frame coverage remains in its original harness. Unrelated generated file-dialog and theme snapshots varied by enumeration/selection timing; those incidental outputs were reviewed and restored. No affected positive-layout snapshot changes remain.

The updated Windows provider was CodePacked and published through Release-Project/Build-Release-Update. Tutorial resources compile with Release/Tools/GacBuild.ps1 and Tutorial/GacUI.xml in both architectures, with no Errors.txt artifacts. An earlier attempt using Tools/Tools/GacBuild.ps1 selected core-only metadata and failed in two unrelated control-template resources; using the required Release tool bundle resolves those failures. The final GacUI_Controls Debug x64 and Win32 builds both report zero warnings and errors. No reflected interface changed, so metadata generation was not required.

Original no-client Playground startup under CDB created 41 COM providers and at least 717 property snapshots, through NativeWindowCreated -> BindWindows -> Control -> UpdateProperties -> Provider. After the demand refactor, the private-desktop no-client operations run creates zero providers, ranges, semantic Control calls, Observe calls, snapshot values or Notify calls. It opens/mutates/detaches/reinserts controls, replaces model/template, moves across windows and back, deletes a modal child and closes normally. At Stop, nodes and bounds hooks have zero count (nodes capacity is also zero). Actual maximize/restore transitions are verified separately in the isolated ProgressAndAsync run below. The private desktops were never switched into view and prevent inspection tools from creating accidental demand.

Previous complete UIA regression runs passed 3387 assertions in the ordinary showcase and 3383 in hosted mode; Review3 passed 847/839, each including 25 modal deletion/subscription cycles. GDI Review2 passed 130 assertions and Transitions 71 assertions in each ordinary/hosted mode. The Review2 busy-thread check now runs before global subscription/traversal and uses the Busy button's HTTP layout bounds: the previous managed UIA traversal waited on a pending GetSelection call behind the deliberately busy UI thread. Its original 60 ms idle-probe and <1000 ms deadline assertions remain. The final ordinary/hosted showcases pass 3384/3380 assertions and close normally. Two earlier hosted ribbon screen-point failures occurred while the lifetime test opened overlapping windows; the isolated full rerun passes both hit tests and all remaining contracts.

The final provider passes 25 consecutive fresh processes under CDB, alternating native/hosted windows and GDI/Direct2D (2638 native-client assertions in total). Each includes retained providers/patterns/ranges through removal/reinsertion, cross-window moves, model/template replacement, deleting a modal child and application shutdown, with explicit COM balancing and off-thread client release. Every debugger exits normally without an access violation, abort or leak report. Four further processes add simultaneous retained-element/range queries and AddRef/Release during subtree removal, plus second-client queries during first-client unsubscribe/release; these pass 139/140/139/140 assertions, again with clean debugger exits. Logs are Lifetime-1 through Lifetime-29; the strict first cohort is summarized in Lifetime-final-25-results.json.

ProgressAndAsync uses the same completed 275643 UTF-16-character document, SHA-256 `AAFDBA9646C32640EDD2A748AB23A60A1B2632D87B3E4A176ABA08BFCEB02C3C` for its UTF-8 bytes, in equivalent Debug x64 Direct2D builds. A temporary fixed-response override loads the original downloaded text; it has been restored byte-for-byte before committing. The original provider is built from Release commit `509ec4202975358a9a1dead0d4625047f987f57c` in a temporary worktree. The active native client traverses seven elements, retains a text pattern/range and subscribes to bounds/name/value; it explicitly unsubscribes/releases before the released phase. The native clients confirm matching document length and hash in both builds.

The final controlled comparison runs the application, window operations and native client on the same private desktop, which is never switched into view. Eight timed maximize/restore cycles per phase use ShowWindow followed by a completed HTTP layout read, at DPI 96, with matching document, skin and initial window size. Every native maximized/restored state is asserted. CDB is attached to both builds with all counting breakpoints disabled during timing; separate two-cycle passes collect work counts. No build or other UIA test runs concurrently. These measurements describe this isolated desktop and HTTP-completion workload, not a universal rendering-speed guarantee. The fresh no-client baseline uses a separate identical process from the original active/released baseline.

| Client phase | Before maximize mean, ms | After maximize mean, ms | Before restore mean, ms | After restore mean, ms |
| --- | ---: | ---: | ---: | ---: |
| No client | 644.85 | 296.36 | 639.27 | 288.25 |
| Active traversal/subscriptions | 3494.34 | 3619.50 | 2536.64 | 3873.81 |
| Client unsubscribed/released | 668.82 | 302.67 | 661.53 | 285.37 |

No-client maximize/restore latency falls by about 54%/55%; after explicit client release it falls by about 55%/57%. Active-client maximize/restore is about 4%/53% slower and remains follow-up work, as scoped in the request. No absolute timing threshold or universal speedup is claimed. The earlier ordinary-desktop measurements were 1033.02/3827.75 ms before and 913.69/996.68 ms after without the benchmark client, 4143.04/3577.67 versus 5976.44/4321.05 with it, and 1023.09/3838.41 versus 1026.57/1017.07 after release (maximize/restore). Later debugger observations found unsolicited desktop subscriptions, so those measurements cannot establish a strictly no-UIA-client comparison. They are retained as observations, not the controlled result.

The following counters are deltas over two actual maximize/restore cycles. Values count WindowsUIAutomationValue constructions, including transient values; UpdateProperties/Notify columns count entry attempts, which may return without work. The Queue column counts actual calls to the dispatcher queue.

| Build / phase | New providers | New ranges | Values | UpdateProperties | Notify | Queue |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Before / no client | 3 | 0 | 6427 | 132 | 202 | 61 |
| After / no client | 0 | 0 | 0 | 0 | 0 | 0 |
| Before / active | 0 | 0 | 4123 | 84 | 380 | 42 |
| After / active | 8 | 0 | 348 | 116 | 461 | 46 |
| Before / released | 0 | 0 | 6424 | 131 | 202 | 61 |
| After / released | 0 | 0 | 0 | 0 | 150 | 0 |

Before the controlled client connects, the old provider retains 15 semantic nodes, initially 9 COM providers (12 after resizing), and 735 cached property values. The new provider has zero nodes, zero node capacity, zero bounds hooks and zero subscriptions after document load and verified resizing. Its client connection creates 7 providers and one range; subscription observation discovers 15 nodes, retains 45 values for the three requested properties, and attaches 130 composition-bounds hooks. Resizing materializes 8 additional providers to deliver subscribed events from previously unvisited elements. After client unsubscribe/exit, the text range is destroyed, event/property subscription counts and cached values are zero, and all 130 bounds hooks are detached. Fifteen providers remain with outstanding COM references (fourteen have count 1, one has count 2), consistent with UIA infrastructure retention; they are not an owning node cache. Required control observation can remain while those references exist. The released resize pass creates no objects/snapshots and queues no work. Normal application shutdown completes with exit code 0 and no debugger crash/leak report; the separate 25-process Playground cohort also validates shutdown with retained references and Debug leak checks.

Authoritative isolated artifacts are Progress-private-before-idle-final.cdb.log, Progress-private-before.cdb.log (the lines marked active/released valid), Progress-private-after-final.cdb.log, and Progress-private-{before,after}-{idle,active,released}.json. Earlier default-desktop counters are contaminated and excluded. The first cross-desktop resize probe did not change native window state and is also excluded; the final benchmark runs on the target desktop and asserts each transition. Temporary benchmark source/settings are removed, and tutorial binaries are rebuilt from the restored source.

Only one 2560x1440 monitor at DPI 96 is available. Nondefault DPI and a second monitor are unverified. Windows currently returns a null foreground HWND, so native desktop focus-event delivery is also unverified; the new control's logical focus and both clients' immediate unvisited-descendant Name events are required. CDB separately observed the provider raise its focus event.

Final cleanup restores ProgressAndAsync/Main.cpp exactly to its original tracked bytes and removes the temporary project user settings and baseline worktree. Tutorial-clean-x64.log and Tutorial-clean-Win32.log both finish with zero warnings/errors. Published Windows files match the GacUI CodePack output byte-for-byte; Tools has no changes. Release was committed and pushed in the requested order: `e01a8af7` contains all 20 updated authored XML files; `ccadd878` contains the remaining 18 generated/published files.

# PROPOSALS

- No.1 Simplify tutorial declarations and separate layout/compiler rejection tests [CONFIRMED]
- No.2 Make UIA observation and COM objects follow demand and retire detached attachments [CONFIRMED]
- No.3 Translate UiaList item mouse coordinates into the popup owner's coordinates [CONFIRMED]

## No.1 Simplify tutorial declarations and separate layout/compiler rejection tests

Defaults and equivalent shared-track declarations will be omitted after recording before-state evidence. Keep descriptor structure, spans, payload properties and scripts. Resolve axes per descriptor group and reset inheritance at independent layout roots. Compare both architecture generation, builds and live tutorial states.

### CODE CHANGE

Move runtime rejection cases and the invalid sibling-mask matrix to TestCompositions_EazyLayoutFailures.cpp and register the file in UnitTest.vcxproj and its filters. Keep valid matrix geometry, tiny equal weights and rebuild/ownership tests in TestCompositions_EasyLayout.cpp. Share the small descriptor fixture through TestCompositions.h. Move the namespace round trip and all 30 compiler rejection inputs into TestResource.cpp inside its existing SetGuiMainProxy/SetupGacGenNativeController lifecycle; preserve diagnostic output and position assertions. No window or remote protocol is needed for these cases.

### CONFIRMED

The 256 omissions preserve effective descriptor settings across both docking axes, fills-only inherited/explicit directions, independent nested roots, both table orientations and shared tracks. Both architecture resource generation and tutorial builds pass; the live layout comparison and positive frame tests show no layout regression, with the five timing-dependent raw-capture differences explicitly recorded above. The requested negative-test relocation preserves rejection assertions and diagnostics without introducing remote frames. No EasyLayout runtime/compiler repair was needed. This proposal is complementary to the UIA and inspector work and is retained.

## No.2 Make UIA observation and COM objects follow demand and retire detached attachments

Audit and reproduce the current eager scan and provider retention before implementation. Required behavior and acceptance remain the complete UI Automation matrix copied above; no lifecycle fix is confirmed from source inspection alone.

### CODE CHANGE

Replace the owning provider cache with a lock-protected non-owning pointer, returning an acquired COM reference from each factory call. Serialize final Release against acquisition, then post cleanup through a shutdown-safe dispatcher; retained text ranges keep the necessary provider references. Separate semantic discovery from detachable control/model observation. Observe represented controls and their window removal boundary; scan descendant event sources only while counted IRawElementProviderAdviseEvents subscriptions require them. Snapshot only requested properties/focus/selection, read scalar values without materializing providers, and store provider-valued snapshot identity without retaining COM objects. Detach owned handler tokens and model callbacks when demand ends. Retire removed composition subtrees synchronously, before deferring disconnect, and rediscover reinserted controls as new attachments. Derive radio/combo/popup relationships from the live control tree instead of eagerly populated node collections. Preserve synchronous subscribed WindowClosed and HWND event-map cleanup.

The existing Review3 client completed 844 assertions, including 25 modal deletion/subscription cycles, against the original Playground under CDB. Its final Window.Close reached normal process termination without an access violation. The reported shutdown crash has not been reproduced by this scenario; the lifecycle changes still require the additional retained-reference and release checks listed above.

First implementation regression: CDB caught a freed node during the first WM_GETOBJECT response. `Ptr(this)` creates an independent reference counter for this non-reflectable Object; the factory must copy the existing context-owned Ptr, as Children already does. Correct the two factory references before continuing regression verification.

The next Review3 run passed R3-01 through R3-11, then encountered Windows managed HWND-proxy timeouts during the native desktop-wide WindowClosed subscription. The target UI thread was idle in GetMessage under CDB. Scope ordinary native subscriptions to the tested modal HWND, preserving native event delivery and explicit COM release while avoiding unrelated desktop providers. Hosted windows require the original desktop scope with exact RuntimeId filtering; using only the host HWND omitted their WindowClosed events. The final ordinary/hosted Review3 runs pass 847/839 assertions respectively, including each 25-cycle modal loop. OwnedWindows already filters HWNDs by target PID.

Further showcase verification exposed UIA call timeouts while repeatedly resolving radio-group parents. CDB samples repeatedly land in the new live-tree radio discovery: every child query scans every window control, then repeats that scan for every radio member. Restrict candidate discovery to the queried control's descendants, deduplicate controllers, and reject non-mutex buttons before walking composition ancestors. Group membership and its common parent still come from the current tree, independently of provider discovery. The showcase nested-menu event order now passes pointer, keyboard and ExpandCollapse entry. Its palette-replacement check must reacquire controls after changing the global template: retained old attachments becoming unavailable is now required behavior. Verify that retirement and selection preservation explicitly.

Final concurrency review also moves callback allocation/destruction outside the dispatcher posting lock. Destroying a failed post's captured COM reference may queue its own final-release cleanup, so it must not reenter the same lock. The provider acquisition lock uses the repository's `lockProvider` naming convention.

The final lifetime rerun exposed an event-only discovery race: insertion only queued a scan, so an application could immediately focus the new control before its focus hook existed. The previous native focus counter also accepted unrelated desktop focus events. Observe inserted subtrees synchronously when subscriptions are active, before the application can perform its next mutation; keep event delivery queued. Require the native focus callback to identify the actual unvisited descendant. Repeat the full lifetime sequence after this correction. CDB confirms Raise(UIA_AutomationFocusChangedEventId) for the unvisited descendant, but GetForegroundWindow returns null and SetForegroundWindow fails in this session. Require an immediate post-insertion Name property event from that descendant for both clients and verify its logical focus. Require its native focus callback when the target is foreground; otherwise explicitly report desktop focus delivery as unverified, matching the existing showcase focus prerequisites.

The first final stress series stopped on process 6 after all native assertions passed: the debugger showed HttpAutomationService::OnHttpRequestReceived terminating on an outstanding /Automation/Playground/Controls request while the main thread waited in StopWindowsHttpAutomationService. The temporary launcher had abandoned startup HTTP requests after two seconds under the debugger. Increase that startup request deadline to 60 seconds, preserve strict server failure behavior, and restart the 25-process series; also require a zero debugger exit code and reject abort/Debug Error output. This failure is separate from provider shutdown and is not counted as a successful run. A second debugger capture establishes a separate hosted HTTP startup deadlock: an already queued GET is serviced synchronously by HttpServerApi::Start on the UI thread, before GuiApplication::Run; ProcessHttpRequest then waits for that same thread through SharedAsyncService. The lifetime launcher now waits for the native main HWND to become visible and does not issue startup HTTP requests. This keeps the provider lifetime stress independent of premature HTTP requests; the captured HTTP startup issue is recorded separately.

The final native client check keeps the second client querying while the first unsubscribes/releases on another MTA thread, and queries retained element/range references concurrently with subtree removal. Successful reads before retirement and UIA_E_ELEMENTNOTAVAILABLE afterward are both valid; unexpected errors, unbounded completion or stale text fail the check. This supplements the existing four concurrent readers in Transitions and passes all four renderer/hosting combinations.

### CONFIRMED

The no-client private-desktop checks demonstrate zero COM materialization, semantic-tree observation and snapshot work, including the large-document resize case. Controlled subscription/release checks demonstrate selective materialization, requested-property snapshots, balanced native range release and complete removal of subscription-only bounds hooks/caches. Outstanding infrastructure COM references are accounted for rather than equated with a surviving benchmark client. Retained-reference, concurrent removal, event-only descendant, modal deletion, ordinary/hosted and both renderer checks pass, including 25 consecutive full debugger processes and four added concurrent runs. The full source unit suite and both architectures build successfully. The original reported shutdown crash remains unreproduced, and desktop-focus delivery/nondefault display configurations retain the limitations stated above. Active-client performance regresses in the measured workload and is explicitly left for the user's later optimization work; this proposal confirms removal of eager overhead, not an active-client speedup. All three proposals address separate requested changes, so the combined implementation is retained.

## No.3 Translate UiaList item mouse coordinates into the popup owner's coordinates

Trace item event forwarding and the popup positioning API, then correct the authored XML event handler. Preserve clicked-node state, stale-node checks and existing keyboard/double-click behavior.

### CODE CHANGE

Reproduced using the native-controls fixture: right-click at inspector client (565,165), window origin (730,316), opened Inspect at physical (1294,432), approximately 49 pixels above the click. OnItemMouseEvent preserves style-local x/y while changing compositionSource/eventSource to the list; popup positioning adds the owner control's global origin. Convert through the clicked node's visible style GlobalBounds and subtract nodeTree.BoundsComposition.GlobalBounds before ShowPopup. Keep the original menuNode capture, busy/synthetic checks, owner control and normal popup screen-edge adjustment. Baseline HTTP/native bounds are saved in the refactor artifact folder; this capture's main bitmap was unavailable.

### CONFIRMED

UiaList's authored XML now converts the clicked style-local point through GlobalBounds into the owning tree's coordinates. Both architecture resource outputs and Debug builds pass. The final HTTP/native-bounds smoke run reports exact click/menu origins (1289,433), (1295,457), (1295,481), and (475,449) after scrolling, moving and resizing. A visible screen-edge click (2555,1379) produces menu origin (2441,1361), matching work-area clamping for the 119x31 popup on the 2560x1392 work area. Navigation, Enter, double-click expansion, dismissal, stale-node refresh, Inspect of the clicked node despite another selection, and exactly one Invoke all pass; the inspector and native fixture close normally. Only DPI 96 and the single available monitor were verified. This proposal is retained alongside the other two.

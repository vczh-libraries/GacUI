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

# UiaList implementation plan

This document specifies UiaList's current behavior and acceptance criteria. The original implementation plan has been updated for the process dropdown, node context menu, property filtering and persistent provider groups. Recorded historical runs remain in Verification; criteria without explicit results are not claims of completed verification. Paths below are relative to the repository root unless explicitly described as relative to `Tools/UiaList`.

## DETAILS

### 1. Scope and resolved design decisions

Implement a native C++20 GacUI inspector with the three main tabs and two modal windows described in AGENTS.md. Use Windows APIs and COM for process discovery, capture, UIA inspection, and actions. Keep the application Windows-only. Runtime C++ reflection, a UiaList CLI, a UiaList unit-test executable, UIA support inside GacUI, continuous monitoring, recording, and arbitrary COM object exploration are outside this implementation.

The following decisions resolve ambiguities before coding:

| Requirement | Implementation contract |
| --- | --- |
| Inspect any Windows UI | Inspect accessible top-level windows on the current interactive desktop and everything their UIA providers expose. Elevation, secure desktops, protected capture, and unexposed virtualized content remain Windows/provider boundaries. Do not invent missing nodes, properties, or actions. |
| Node `TYPE` is a COM interface | An inspector receives `IUIAutomationElement`, not the target's `IRawElementProviderSimple` or a separate button COM interface. Probe the public client element versions and display the highest successfully queried `IUIAutomationElementN` name, falling back to `IUIAutomationElement`. Show the semantic role separately. |
| Provider names in a node label | Display names such as `IInvokeProvider` as the documented provider counterpart of an acquired client pattern. A tooltip explains this mapping. It is not evidence of querying that provider interface on the remote object. |
| All nodes | Traverse the selected root's **Raw View** tree, including non-control and non-content nodes. Do not enumerate the entire desktop subtree, filter by the root PID, or automatically realize virtualized items. |
| Render selected UI | Show an on-demand captured image of the selected window, including its non-client frame. Show a capture timestamp and an explicit selected-window Refresh command. The image is a snapshot, not an interactive remoting surface. |
| Hover/click the image | Hit-test the corresponding snapshot's UIA bounding rectangles. Click selects the inspector node and opens Nodes; it never sends mouse input or changes keyboard focus in the target. “Focused node” in this workflow means the inspector's selected node. |
| Refresh | Processes refreshes only on main-window opening and its Refresh button. Selected-window data refreshes on window activation in Processes, its own Refresh command, and after successful target mutations. No timer, heartbeat, or UIA subscription automatically refreshes either tree. |
| Writable properties | UIA element properties have no generic setter. Only explicitly mapped pattern setters produce editors. Supported read-only values remain inspectable; only ValueKind::Unsupported rows are omitted from the Properties grid. Action diagnostics and text-range attributes retain their unsupported states. |
| All providers | Support every standard pattern in the selected Windows SDK, including read-only patterns, versioned patterns, and LegacyIAccessible. GacUI's native Windows providers are a separate implementation from this client inspector. |
| OS language | Choose from the user's preferred Windows UI languages at startup, normalize supported Chinese/Japanese variants, and install the resulting GacUI locale before constructing windows. Default/fallback is `en-US`. |
| Error recovery | Follow the repository's Tools policy: normal capability absence is data; unexpected failures report their operation and error and terminate. Do not add retry loops, reconnect logic, blanket exception suppression, or background recovery. |

UIA control types are semantic IDs, while patterns are independently acquired behavior contracts. The client API and physical-coordinate distinction are documented in Microsoft's [UIA client interfaces](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-clientportal) and [screen scaling guidance](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-screenscaling).

### 2. Repository foundations and implementation boundaries

Read these before implementing the corresponding part:

- [Repository instructions](../../.github/copilot-instructions.md), [Project.md](../../Project.md), [coding](../../.github/Guidelines/Coding.md), [threading](../../.github/Guidelines/Coding_MultiThreading.md), and [source inventories](../../.github/Guidelines/SourceFileManagement.md).
- [Knowledge-base index](../../.github/KnowledgeBase/Index.md), [GacUI index](../../.github/KnowledgeBase/Index_GacUI.md), and the coding/testing learning documents they reference.
- [GacBuild/GacGen](../../.github/KnowledgeBase/KB_GacUI_Design_GacGenAndGacBuild.md), [C++ view models](../../.github/KnowledgeBase/manual/gacui/advanced/vm.md), [XML resources](../../.github/Guidelines/GacUIXmlResource.md), and [layout](../../.github/Guidelines/GacUILayout.md).
- [Bindable grids](../../.github/KnowledgeBase/manual/gacui/components/controls/list/bindabledatagrid.md), [grid visualizers](../../.github/KnowledgeBase/manual/gacui/components/controls/list/datagrid_visualizer.md), [localization](../../.github/KnowledgeBase/manual/gacui/advanced/localization.md), and [modal windows](../../.github/KnowledgeBase/KB_GacUI_Design_MainWindowModalWindow.md).
- [Build](../../.github/Guidelines/Building.md), [GUI execution and automation](../../.github/Guidelines/Running-GacUI.md), [debugging](../../.github/Guidelines/Debugging.md), and [native dialog handling](../../.github/Guidelines/Running-ComputerUse.md).

[ToDo/Task_UIA.md](../../ToDo/Task_UIA.md) supplies the provider/control-role research. Its GacUI provider implementation instructions are a separate task. Do not execute them or `job.completeUia.prompt.md` as part of implementing this inspector unless that task is explicitly requested. This plan supplements its provider view with client interfaces, returned text-range objects, and the SDK inventory below.

Keep business and inspection behavior in C++ view models and their model services. XML owns controls, layout, bindings, focus presentation, and modal presentation. View models must not hold controls, compositions, generated window instances, or callbacks that manipulate controls. The sole startup bridge creates a root view model and supplies it to the generated main-window constructor. No handwritten code belongs in generated files, including their nominal user-editable regions.

### 3. Solution, file ownership, and generation

Create the following under `Tools/UiaList` during implementation. These are proposed files, not files created by this planning task.

| Path | Responsibility |
| --- | --- |
| `UiaList.sln` | Three product projects; Debug/Release and Win32/x64 configurations. |
| `GacUI.xml` | Resource-discovery driver containing `<GacUI/>`; separate from Resource.xml. |
| `Gaclib/Gaclib.vcxproj` and `.filters` | Static library compiling the required existing dependency sources once. |
| `UiaList/UiaList.vcxproj` and `.filters` | Static library compiling generated UI and handwritten view models/model services. References Gaclib. |
| `UiaList/UI/Resource.xml` | Resource metadata, code-generation configuration, and imports of the authored resources below. |
| `UiaList/UI/ViewModel.xml` | Workflow declarations of interfaces, enums, and data records. No inspection implementations. |
| `UiaList/UI/MainWindow.xml` | Processes, UI, and Nodes tabs and selected-target commands. |
| `UiaList/UI/PropertyWindow.xml` | Properties and Actions tabs, row visualizers, result navigation, modal coordination. |
| `UiaList/UI/MultilineWindow.xml` | Draft text, validation, OK/Cancel, and read-only long-text display. |
| `UiaList/UI/ActionTemplates.xml` | Parameter controls and provider-specific layouts, including text ranges. |
| `UiaList/UI/Strings.xml` | Synchronized `en-US`, `zh-CN`, and `ja-JP` localized strings. |
| `UiaList/Source/` | GacBuild-generated C++ only, including the compressed-resource loader. |
| `UiaList/ViewModel/UiaListViewModel.h/.cpp` | Root/session/navigation and process/node view models. |
| `UiaList/ViewModel/PropertyViewModel.h/.cpp` | Typed values, grid drafts, edit sessions, and modal state. |
| `UiaList/ViewModel/ActionViewModel.h/.cpp` | Pattern sections, typed arguments, dispatch, and results. Split text-range implementation into `TextRangeViewModel.h/.cpp`. |
| `UiaList/ViewModel/UiaCatalog.Windows.h/.cpp` | Named SDK descriptors and typed pattern adapters; the single catalog shared by display and dispatch. |
| `UiaList/ViewModel/UiaSession.Windows.h/.cpp` | COM owner thread, element/pattern/range handles, snapshots, and HRESULT boundary. |
| `UiaList/ViewModel/ProcessDiscovery.Windows.h/.cpp` | Process forest and top-level-window enumeration. |
| `UiaList/ViewModel/WindowCapture.Windows.h/.cpp` | Capture lifecycle, pixels, physical extent, and DPI metadata. |
| `UiaListApp/UiaListApp.vcxproj`, `.filters`, `Main.cpp`, and `app.manifest` | Windows subsystem executable, theme, locale, root injection, DPI declaration, debug automation, and normal shutdown. |
| `Verification/` | Native fixture source/project, scenario manifests, and UI-driving/evidence scripts described in verification. This is development support, not another UiaList product frontend. |

Use the existing `Test/GacUISrc/CppTest/CppTest.vcxproj` and `Tools/GacGen` project files as setting references, not as blindly copied dependency graphs. `Import` currently contains Vlpp dependencies but no GacUI or DarkSkin pair. Therefore Gaclib consumes existing `Release/GacUI.h/.cpp`, `Release/GacUI.Windows.h/.cpp`, and `Release/DarkSkin.h/.cpp` read-only and adds `Release` to includes alongside required `Import`. Its explicit dependency source inventory is `Import/Vlpp.cpp`, `Vlpp.Windows.cpp`, `VlppOS.cpp`, `VlppOS.Windows.cpp`, `VlppRegex.cpp`, `VlppReflection.cpp`, `VlppGlrParser.cpp`, and `VlppWorkflowLibrary.cpp`, plus those three Release `.cpp` files. Add their headers to the project/filter inventory as well. Do not compile Linux pairs, Workflow compiler/runtime, GacUICompiler, reflection-registration pairs, or duplicate a release pair through a shared-source project. Give UiaListApp direct project references to both Gaclib and UiaList with library dependency inputs enabled.

All product configurations use toolset `v145`, latest Windows 10 SDK, C++20, Unicode, `VCZH_DEBUG_NO_REFLECTION`, and `UseLibraryDependencyInputs=true` on static-library project references. Only Debug defines `VCZH_CHECK_MEMORY_LEAKS`. Use consistent runtime-library settings across all linked projects. Keep Win32 native handle types intact; convert handle display through pointer-sized unsigned values, never through 32-bit `int` on x64. Declare Per-Monitor V2 DPI awareness before window creation. Additional native libraries for the chosen capture/client code include `UIAutomationCore.lib`, `Ole32.lib`, `OleAut32.lib`, `User32.lib`, `Gdi32.lib`, `Dwmapi.lib`, `D3D11.lib`, `Dxgi.lib`, `Windowscodecs.lib`, and `RuntimeObject.lib`, plus the renderer's existing dependencies.

The resource configuration is:

```xml
<Resource>
  <Folder name="GacGenConfig">
    <Xml name="Metadata">
      <ResourceMetadata Name="UiaList" Version="1.0"/>
    </Xml>
    <Folder name="Cpp">
      <Text name="Name">UiaList</Text>
      <Text name="SourceFolder">../Source</Text>
      <Text name="NormalInclude">GacUI.h</Text>
      <Text name="ReflectionInclude">GacUIReflection.h</Text>
      <Text name="CppCompressed">UiaListResource.cpp</Text>
    </Folder>
  </Folder>
  <!-- Authored view-model, window, template, and localization resources. -->
</Resource>
```

`CppCompressed` belongs inside `GacGenConfig/Cpp`. Do not substitute the different `Compressed` setting or configure an external runtime resource/assembly. Run the sibling Tools repository's `GacBuild.ps1 -FileName <absolute Tools/UiaList/GacUI.xml>`; it generates both architectures and merges them into Source. Add every generated source/header to the project and filters explicitly after generation. No wildcard project inventory and no second code generator. Use the existing generated compressed loader plugin, and verify the static-library link retains it. The app must launch using only its executable and ordinary system/runtime dependencies, with no XML or `.bin` files beside it. Details are in the [GacGen output contract](../../.github/KnowledgeBase/KB_GacUI_Design_GacGenAndGacBuild.md#production-outputs-and-their-consumers).

### 4. View-model contract

Declare the following in `UiaList/UI/ViewModel.xml` under `uialist::vm`; implement them in the handwritten ViewModel files. The table is the minimum public contract. Use typed Workflow lists/interfaces and generated C++ signatures; use Vlpp observable collections exposed through the generated collection contracts. Every mutable display property has a change event; mutations and notifications occur only on the GacUI thread. Command methods queue asynchronous work and return immediately unless they only change local presentation state.

| Interface/record | Data and operations |
| --- | --- |
| `IUiaListViewModel` | Synthetic `ProcessRoot` and `NodeRoot` objects, `SelectedWindow`, `Preview`, `SelectedNode`, `ActiveTab`, `IsBusy`, `Status`, `LocaleName`, nullable `PropertyDialog`; `Initialize()`, `RefreshProcesses()`, `SelectWindow(window)`, `RefreshWindow()`, `SelectNode(node)`, `InspectNode(node)`, `RequestClose()`. Initialization is idempotent for repeated window-open notifications. |
| `IProcessNodeViewModel` | `Kind` (SyntheticRoot/Process/Window), `DisplayText`, PID, parent PID, executable, optional creation time, window key/title/class/handle text, process-only `Children`, qualifying `Windows`, `IsExpanded`; a window row always has empty Children and Windows lists. SyntheticRoot has children but no process/window identity. |
| `INodeViewModel` | `IsSyntheticRoot`, session-local `NodeId`, `ParentId`, `RuntimeIdText`, `Children`, `DisplayText`, `RoleText`, `ClientInterfaceName`, `ProviderNames`, physical `Bounds`, `IsOffscreen`, `IsExpanded`, `IsSelected`. The synthetic root has no element key or actions. COM pointers are never part of this interface. |
| `IPreviewViewModel` | Image data/presentation image, pixel dimensions, physical capture rectangle, timestamp, `State`, `Status`, nullable `HoverNode`, and image-local highlight rectangle; `HoverAt(x,y)`, `Leave()`, `SelectAt(x,y)`. Coordinates here are snapshot pixels after the XML presentation transform. |
| `IPropertyDialogViewModel` | Its own node reference and generation, `Title`, `Rows`, `Sections`, selected property row, nullable `TextDialog`, `IsBusy`, `Status`; `Refresh()`, `SelectRow(row)`, `BeginEdit(row)`, `CommitEdit(row)`, `CancelEdit(row)`, `OpenText(row)`, `Close()`. |
| `IPropertyRowViewModel` | Stable row key `(source, ID/member)`, localized label plus canonical name/ID, `Value`, `DisplayValue`, `ValueState`, `EditorKind`, `CanEdit`, `IsMultiline`, `IsSelected`, `IsEditing`, typed draft/text, `ValidationMessage`, optional setter descriptor. |
| `IActionSectionViewModel` | Pattern ID, provider/client interface names, localized heading, typed readouts, `Parameters`, `Commands`, `Results`, capability flags, `IsBusy`; commands dispatch to the owning typed pattern adapter. Each pattern has its own section, with version families grouped visually. |
| `IActionParameterViewModel` | Stable key, label, kind, required/nullable policy, enum choices, typed draft, range/units, validation state. Element/range parameters use reference pickers rather than raw pointer/address input. |
| `IActionCommandViewModel` | Stable command key, label, `CanExecute`, `IsBusy`, `Execute()`. Commands contain no script method names interpreted at runtime; dispatch is compiled C++. |
| `IActionResultViewModel` | Operation, completion state, formatted scalar or list, element references, text-range references, count and optional paging state. Results remain associated with the dialog that issued them. |
| `ITextRangeViewModel` | Session-local range key, owning element key, title, text preview, selected attribute rows, child references, bounding rectangles; access to the range operations in section 10. |
| `ITextDialogViewModel` | `Title`, `OriginalText`, `DraftText`, `IsReadOnly`, `CanAccept`, `ValidationMessage`; `Accept()`, `Cancel()`. Closing via the title bar has Cancel semantics. |

Use an internal tagged value representation for null, unsupported, mixed, Boolean, integer, floating-point, string, enum, point, rectangle, element reference, text-range reference, array, and opaque COM result. These states are distinct; `0`, `false`, an empty string, an empty array, and null must not display as unsupported. Formatting is a view-model responsibility. The view only selects the matching template/editor and binds prepared values.

All XML events use `-eval`, including open/close, selection, double-click, keys, mouse, command buttons, and modal completion. Dialog view models are created by commands and made reachable from the root. XML reacts to changed dialog state, creates the generated modal with that view model, and uses GacUI's asynchronous modal APIs. A rendering-only XML helper prevents opening a second window for the same dialog instance. A modal's close callback clears its corresponding VM state and restores inspector focus. No generated C++ code-behind method implements these events.

### 5. Ownership, work scheduling, and session state

The root owns model services and all live view models. A dedicated `vl::Thread` with `vl::TaskQueue` owns the UIA COM apartment for the app lifetime. Initialize COM as MTA there, create `CUIAutomation8`, and keep all UIA element, pattern, range, and returned opaque object references on that thread. Its native interface names and SDK declarations come from `UIAutomationClient.h`. Do not use a generic thread pool for apartment-owned objects or pass them through XML.

Assign a monotonically increasing generation on each window selection and explicit window refresh. A target mutation reserves a successor generation, published through the handoff below. Request/result records carry generation, request serial, and dialog identity. The worker maintains dictionaries mapping opaque application keys to COM references. Snapshots crossing to the UI contain only copied value records, reference keys, and owned pixel buffers. GacUI controls, observable collections, and native image objects are created/updated only after dispatch through `GetApplication()->InvokeInMainThread(nullptr, completion)` (`Source/Application/Controls/GuiApplication.h`). This app has one UI thread; the null thread context avoids passing a control into the view model.

Queue policy:

1. Process refresh and root selection are serialized inspection jobs. `vl::TaskQueue` has no remove/replace API: queued read lambdas check their request serial/generation when reached and return before calling UIA if obsolete. Also check cancellation/generation between UIA calls and traversal batches. A single pending-read slot can coalesce repeated requests without changing TaskQueue.
2. An action captures its target key, generation, arguments, and dialog identity at submission. Before enqueueing, validate its current session and drafts; enqueueing is its acceptance boundary. Accepted commands retain a worker-side session/operand lease until execution completes, including if the inspector subsequently closes the dialog or selects another target. Recheck live capabilities before the actual call, but do not reject an accepted command merely because the UI generation changed. Never retarget it, repeat it, or silently discard it. Disable repeat submission until completion.
3. Ordinary read completions publish only when their generation/request still matches. A mutation completion carries `(baseGeneration, successorGeneration, actionSerial)` and its post-action snapshot. If the UI still owns that base/action, atomically install the successor and update the surviving dialog before notifications; do not increment the expected generation early and then discard the very completion advancing it. If a newer selection/refresh already won, discard only this command's display result. Its execution/lease cleanup still complete on the original session. Unexpected errors remain fatal even if their dialog is gone.
4. Capture has its own owner/context so waiting for a frame cannot block the UIA queue. Only one capture request per selected generation is active. Frame-arrival callbacks copy data and schedule completion; they never update GacUI objects.
5. Hover is local to the immutable published snapshot, so pointer motion issues no COM calls and cannot flood the queue.

States are `NoSelection -> Loading -> Ready`, with a separate Preview unavailable state when UIA succeeds but capture is unsupported. A failed operation follows the error contract below. Process refresh does not implicitly rebuild a surviving selected target. If the selected window is absent from the completed process snapshot, clear selection, dialogs, preview and nodes, return to NoSelection, and disable UI/Nodes. Selecting another window closes old inspector dialogs and starts a new generation.

Normal close stops accepting work, invalidates UI completions, revokes capture callbacks, drains accepted commands and started reads, releases all apartment-owned references on their owning thread, then joins workers before destroying the root. Queue the final release/`CoUninitialize` job before `TaskQueue::QueueExitTask`, and run the queue through `RunTaskQueue`. Do not block the UI thread while a worker is waiting for a UI callback; workers never use `InvokeInMainThreadAndWait`. Keep the controller/root alive until queued completion dispatches can safely reject work. A provider call already inside COM is not canceled by dropping a request; do not use `TerminateThread` or claim that cancellation interrupts it. Set `IUIAutomation2::put_ConnectionTimeout` and `put_TransactionTimeout` to 5000 milliseconds, retaining their operation identity in timeout diagnostics; these UIA settings are not a universal deadline for arbitrary in-process provider code. Diagnose a hung provider through the normal debugger workflow. Microsoft's [UIA threading guidance](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-threading) and [client timeout interface](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nn-uiautomationclient-iuiautomation2) define these contracts.

### 6. Processes tab

Use `CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)` and `Process32FirstW`/`Process32NextW` for the process list and parent PIDs. Use `EnumWindows` for top-level HWNDs and `GetWindowThreadProcessId` to associate windows with processes. The process parent relationship and window ownership relationship are different; an owned top-level dialog stays under the PID that owns its HWND. [PROCESSENTRY32W](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32w) and [EnumWindows](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumwindows) define these inputs.

For deciding whether a process branch has visible UI, require at least one enumerated window with `IsWindowVisible`, no `DWMWA_CLOAKED` flag, and nonempty window bounds. Minimized windows still qualify. Compute `HasVisibleUI(process) = OwnQualifyingWindow || AnyChildHasVisibleUI` bottom-up after fixing parentage. The combo beside Refresh contains exactly the retained process hierarchy, with no window rows or visible synthetic root. The details list contains only qualifying top-level windows owned directly by the selected PID. A retained headless ancestor has an empty list; its visible descendants remain selectable.

Construct the entire forest before binding it. Missing/exited parents become root processes; do not synthesize fake parents. Use creation time, when queryable, to reject PID-reuse parent relationships where the proposed parent is newer than the child. Break invalid cycles deterministically by making the offending process a root; never recurse forever. Preserve basic PID/name information if an optional process metadata query is denied. Sort roots and process children by executable name then PID, and each process's windows by title then pointer-sized HWND value.

Process labels contain executable and PID; the window details columns contain title or localized untitled text, class, and full HWND. Process selection stays on Processes and clears list selection without changing the inspected target. A single left click on the exact window row, including an already selected row, or Enter revalidates HWND/PID identity and switches to UI with a fresh capture/tree session. Background/header clicks never activate an old selection. On explicit refresh, preserve the selected process by PID/creation time when available and inspection by HWND/PID; clear missing identities and dependent state. A surviving inspected target does not require another capture. Preserve process request serials and inspection generations.

### 7. UIA tree acquisition, identity, and labels

Resolve the selected HWND with `IUIAutomation::ElementFromHandle`. Acquire its RawView children with `IUIAutomationTreeWalker` and a small `IUIAutomationCacheRequest` (`AutomationElementMode_Full`). Cache the display/hit-test fields: Name, ControlType, BoundingRectangle, IsOffscreen, ProcessId, AutomationId, NativeWindowHandle, IsControlElement, and IsContentElement. Discover patterns through the catalog, not through role assumptions. Read the complete property set only when opening a property dialog.

Traverse iteratively in provider order. Keep explicit parent/depth/preorder indexes and finish all exposed descendants without hidden depth or node-count limits. Batch progress back to the UI while keeping the old published snapshot visibly marked loading; only a complete new snapshot becomes Ready and usable for image hit testing. Check cancellation between batches. The tree can span several processes and native HWNDs inside the selected UIA root. Returned relationships, text-range children, and virtualization search results are references, not extra duplicate raw-tree child edges.

Use `GetRuntimeId`/`CompareElements` for identity within the live session. Compare every integer of a runtime-ID array; never use a display string hash alone, sibling index, name, AutomationId, PID, or HWND as unique element identity. If the runtime ID is unavailable, maintain session-local keys with `CompareElements`. Do not persist these keys across restarts. Keep a visited-identity set to diagnose a malformed provider cycle instead of looping. On refresh, remap selection/expansion by element identity where still valid; otherwise select the surviving nearest ancestor, or root. Close a property dialog whose element no longer exists in the refreshed tree unless it owns a still-valid detached result reference.

For each node, probe the public client element interfaces from `IUIAutomationElement9` down to the base using known SDK IIDs. `E_NOINTERFACE` means the version is absent. The name is an actual client interface successfully acquired; it is independent of the remote provider's internal implementation. The label is exactly `TEXT (TYPE, PROVIDER, ...)` or `(TYPE, PROVIDER, ...)` when Name is empty. Use UIA Name for TEXT; normalize line breaks/tabs to spaces for this one-line label only. Preserve complete original text in Properties. Sort provider counterparts by numeric pattern ID and remove duplicate displayed names. Put ControlType's canonical name and numeric value in the secondary line/tooltip rather than pretending `Button` is a COM interface.

Both trees bind `ItemSource` to a synthetic root object, not a list. ProcessRoot.Children contains the process forest; NodeRoot.Children contains the selected target root as its sole child. GacUI hides the synthetic root, so binding the target element directly would incorrectly hide the inspected window node. These UI-only container roots are not UIA elements or fake process rows.

Nodes uses a bindable tree. Click selects, double-click expands/collapses, and right-click opens a localized Inspect command for that exact node. Enter also inspects. Invalidate the menu on tree/generation changes and validate the actual node before opening a modal; a stale node must not fall through to the previous selection. Preview navigation expands every ancestor, selects the requested key, switches to Nodes, and scrolls the visible row into view after expansion/layout. VM state stores expanded/selected keys. In the XML-local presentation helper, start from `tree.NodeRootProvider.RootNode`, match bound NodeIds using `GetBindingValue(child)` while walking `INodeProvider.GetChild`, expand ancestors with `SetExpanding(true)`, compute the row with `tree.NodeItemView.CalculateNodeVisibilityIndex(node)`, then call `ClearSelection()`, `SetSelected(index, true)`, and `EnsureItemVisible(index)` after dispatch/layout. Never store `INodeProvider` or a control mapping in the C++ VM or bind the getter-only SelectedItem as a writable property. Use direct item-template bindings for mutable labels, or explicit node-data notifications where the chosen text accessor requires them. API definitions are in `Source/Controls/ListControlPackage/GuiListControls.h` and `DataSource_INodeProvider.h`. Do not assume expanding a GacUI bindable tree triggers lazy child loading: its provider can eagerly prepare children.

### 8. Captured preview and coordinate contract

Use Windows Graphics Capture for a one-frame snapshot of the selected HWND. The baseline for preview is Windows 10 version 1903 or later, matching [IGraphicsCaptureItemInterop::CreateForWindow](https://learn.microsoft.com/en-us/windows/win32/api/windows.graphics.capture.interop/nf-windows-graphics-capture-interop-igraphicscaptureiteminterop-createforwindow). Use the Windows SDK C++ COM/WinRT ABI interfaces; no managed runtime or extra UI framework is required. Check capture support before allocating the session. Use a D3D11 BGRA device and a free-threaded frame pool, whose [FrameArrived callback runs off the UI thread](https://learn.microsoft.com/en-us/uwp/api/windows.graphics.capture.direct3d11captureframepool.createfreethreaded?view=winrt-26100).

Initialize the capture context with the matching multithreaded WinRT apartment lifetime. Register both FrameArrived and capture-item Closed before StartCapture, and arm one 5000-millisecond request deadline. First frame, Closed, deadline, and user cancellation race through one completion flag: only the winner publishes/completes; all later callbacks release their resources without touching the view. A Closed event or no-frame deadline returns a localized Preview unavailable result. No retry is scheduled. This is a one-shot capture bound, not periodic polling or a heartbeat.

On a selected-window refresh:

1. Sample physical extended-frame bounds using `DwmGetWindowAttribute(DWMWA_EXTENDED_FRAME_BOUNDS)` and record the selected HWND/PID/generation. This is the intended image origin; `GetWindowRect` may include invisible resizing borders and is not interchangeable.
2. Start capture for this HWND and accept the first nonempty frame belonging to the request. Copy only its `ContentSize` area into owned CPU pixels via a staging texture, respecting row pitch. Release the frame before closing the pool/session. The [capture documentation](https://learn.microsoft.com/en-us/windows/apps/develop/media-authoring-processing/screen-capture) describes frame lifetime and content-size handling.
3. Sample extended-frame bounds again. Accept a coordinate mapping only if origin/size were unchanged and the frame dimensions match that physical extent. If they changed during sampling, leave the new preview unavailable with a localized instruction to refresh; do not publish a guessed transform or automatically loop for another capture.
4. Pair pixels, bounds, timestamp, and UIA rectangles into one published snapshot generation. Capture and UIA are separate observations, not an atomic target-app transaction. A changing target must be refreshed after it settles. Do not mix a new frame with old-generation node rectangles.
5. On the UI thread, load the pixels into a GacUI image using the native image service (an in-memory WIC-encoded image is a valid bridge), then bind an image composition inside a scroll container. Dispose the previous presentation image after bindings no longer reference it.

Use a fixed scale of one captured physical pixel per GacUI logical preview unit, with both scroll bars as needed. Do not auto-fit or silently distort the image. Let `q` be the pointer coordinate relative to the visible viewport in logical units, `s` the scroll offset, and `o` the image layout offset inside the viewport. Then snapshot pixel position is `p = q + s - o`; physical target position is `captureOrigin + p`. Image-local rectangle coordinates are `nodeBounds - captureOrigin`. If a composition mouse event already provides image-local coordinates, do not add the scroll offset again. Clip drawing and pointer acceptance to the image extent. GacUI's current DPI conversion governs presentation only; UIA physical coordinates are never rescaled a second time.

At hover, among nodes with finite, positive rectangles containing the point, excluding IsOffscreen nodes, choose maximum raw-tree depth and then lowest preorder index for a tie. Search all candidates even when a child's rectangle extends beyond its parent's. Draw a clipped high-contrast outline and expose its label; clear it on mouse leave or background. A full-size root can be selected when no deeper rectangle contains the point. A click recomputes at its own coordinates, so an old hover result cannot select the wrong node.

This is a documented bounding-rectangle hit test. UIA does not promise rectangle containment equals a provider's shaped or overlapping hit-test result. A global `ElementFromPoint` call is unsuitable for the preview because the inspector or an occluding window may be at those screen coordinates. Keep this limitation visible in preview help; do not label a rectangle estimate as an actual live target mouse hit.

Minimized/hidden/cloaked windows and unsupported/protected capture get an unavailable placeholder with no hover/click target. A minimized window remains selectable and its UIA nodes remain usable if inspection succeeds. A newly hidden/cloaked target can retain its snapshot until explicit process Refresh removes it and clears dependent state. Do not restore, move, activate, or unprotect the target merely to capture it. SDR capture is the initial display contract; record HDR color fidelity as a platform limitation rather than promising lossless HDR rendering. Revoke frame callbacks and release devices/surfaces on ordinary completion, selection change, and close.

### 9. Properties grid and editing

Use columns **Property**, **Value**, and a narrow **Details** column. The first includes localized category/label and canonical SDK property/member name plus numeric ID where applicable. Group element properties first, then acquired pattern properties, then client metadata. Preserve stable ID order within groups. Omit only native ValueKind::Unsupported rows on every publication. Preserve supported false, zero, empty, null, mixed and read-only values. Restore selection by property key among the remaining rows, or clear it when the row disappears. Unsupported action results, text-range attributes and diagnostics remain visible in their own pages.

Maintain one explicit descriptor catalog for all named SDK properties. Use `IUIAutomationElement::GetCurrentPropertyValueEx(id, TRUE, ...)`, compare with the reserved NotSupported value, and preserve exact VARIANT types. Add pattern getter rows that have no standalone property ID, such as returned headers, selection arrays, text ranges, and extended-property arrays. Do not claim to enumerate arbitrary vendor-registered property IDs: UIA has no general discovery API for all unknown custom registrations. Display custom information only when a known standard API returns it.

Value conversion must handle BSTR length rather than null-terminated assumptions, VARIANT_BOOL, signed/unsigned integers, floating point, enums and unknown enum values, SAFEARRAY dimensions/lower bounds, UIA element arrays, runtime IDs, bounding rectangles, and reserved mixed/unsupported values. Show opaque interface results with their known public interface and a session reference; never print a pointer as if it were meaningful object content. Long strings/arrays have a visible count and a full-detail viewer or explicit paging; no unmarked truncation. Password state never justifies synthesizing a value or recovering hidden text; show only what the provider legitimately returns.

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

Use a `BindableDataGrid` with custom `GridVisualizerTemplate` templates. Keep its normal editor factory unset. In the Value visualizer, bind `RowValue` to the generated property-row interface and show a label or embedded text/enum editor according to VM-owned `IsEditing`/`EditorKind`. This achieves row-specific editors without replacing the grid's column-scoped EditorFactory during cell selection. Follow the existing `Test/Resources/App/FullControlTest/DataGridComponents.xml` visualizer pattern; `Source/Controls/ListControlPackage/GuiDataGridExtensions.cpp` supplies the row's binding value to the template.

Clicking an editable value calls BeginEdit and copies current value into a draft. Enter commits an inline draft; Escape cancels. Selecting another row commits a valid inline draft first and keeps the old row selected if validation fails. Scrolling/recycling preserves the draft in the VM and never commits by itself. Do not rely on LostFocus as the sole commit trigger. Programmatic TextChanged notifications update drafts only when the matching edit session is active; they never call setters directly. While a setter runs, disable that dialog's mutations. Read back the value after success instead of assuming the provider accepted the submitted representation unchanged.

`SelectedCell` is getter-only and SelectedCellChanged runs after the change. The XML presentation keeps the previous GridPos and restores a rejected move with deferred, reentrancy-guarded `grid.SelectCell(oldPos, false)`. For an asynchronous commit, keep the requested destination row key as pending navigation, restore the old cell while busy, then resolve/select the destination after readback and generation remapping. After BeginEdit/layout, explicitly call the embedded editor's `SetFocused()`; built-in EditorFactory focus management is not active. Handle Enter/Escape on the actual editor and mark the event handled so the grid does not navigate simultaneously. These APIs are declared in `Source/Controls/ListControlPackage/GuiDataGridControls.h`.

The Details `...` button appears only on the selected row for multiline/long-detail values. `GridVisualizerTemplate.Selected` describes a selected cell, so derive row selection using the grid's selected-cell event and row binding value; do not hide the button merely because another cell in that same row is selected. Multiline values never instantiate the inline text editor. Open the text modal owned by the property window; its draft is isolated until OK. Cancel, Escape, and title-bar close make zero provider calls. OK validates then submits one setter, and closes only after success; read-only viewers have no writable acceptance path. Preserve Unicode, blank lines, and final line breaks.

### 10. Complete pattern and returned-object UI

`UiaCatalog.Windows.*` owns one typed adapter per standard pattern. Each descriptor includes the actual SDK pattern constant, client IID, provider display name, property getters, parameter schemas, commands, and result conversion. Acquire patterns with `IUIAutomationElement::GetCurrentPatternAs`; a successful availability property alone does not substitute for acquiring the interface. Optional version QI failure is normal absence. Recheck live pattern support and mutability immediately before executing a write. If a formerly writable row is now read-only, cancel that draft and refresh its displayed capability without calling the setter.

The Actions tab has a scroll container with `ExtendToFullWidth="true"` and a vertical `RepeatStack`. Every supported pattern appears, even if it only has getters. Every card stretches to the viewport width, wraps explanatory labels, and contains current readouts, typed parameters, commands, and results. Versioned cards identify both interfaces and share the base-operation presentation so a method is not duplicated. If only the versioned pattern was acquired, its inherited operations must still be available. Patterns that merely accompany one another, such as Grid and Table, remain separate capabilities.

The registry below is the required SDK baseline. The local Windows SDK `10.0.26100.0` declares 35 patterns (IDs 10000 through 10034). The [Microsoft pattern-ID reference](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpattern-ids) is useful for links, but the SDK plus individual interface documentation governs omissions such as Selection2. Except for the explicit version rows, a pattern named X maps to client `IUIAutomationXPattern` and provider label `IXProvider`.

| Pattern ID / name | Readouts and exact client operations | Parameter UI and acceptance behavior |
| --- | --- | --- |
| 10000 Invoke | `Invoke()` | One Invoke button. Fixture action counter increases once, including when the action opens a target modal. |
| 10001 Selection | CanSelectMultiple, IsSelectionRequired, `GetCurrentSelection()` | Refresh/read selection list with inspectable element links. Cover none/one/multiple. |
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
| 10022 ObjectModel | `GetUnderlyingObjectModel()` | Query button; null or opaque COM reference description. No arbitrary IDispatch crawling/editor or application-specific object-model operations. |
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

### 11. Localization, presentation, and automation

Use `LocalizedStrings DefaultLocale="en-US"`, complete `en-US`, `zh-CN`, and `ja-JP` entries, and `-str` bindings. All authored text includes tabs, columns, buttons, parameter labels, validation, busy/unavailable status, help, tooltips, result-state words, units, and modal titles. Canonical COM/SDK identifiers, runtime IDs, process names, target text, and returned provider strings remain verbatim data; place localized explanatory labels around them. Translation key sets and parameter signatures must match exactly.

At startup call Windows [GetUserPreferredUILanguages](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-getuserpreferreduilanguages) with `MUI_LANGUAGE_NAME` and its two-call buffer-size protocol, choose the first supported preferred language, map neutral `en`/`ja`/`zh` and their regional variants to `en-US`/`ja-JP`/`zh-CN`, then fall back to `en-US`. Document that Traditional Chinese receives the supplied Simplified Chinese translation until a separate translation is added. Call `GetApplication()->SetLocale(vl::Locale(localeName))` before constructing windows. GacUI's default `Locale::UserDefault()` uses Windows regional locale, which may differ from display language, so relying on it alone would not implement this chosen policy. The relevant local implementations are `Source/Application/Controls/GuiApplication.cpp` and `Import/VlppOS.Windows.cpp`.

Declare the localized resource class as `uialist::Strings`. Inject its generated localized-string interface into the C++ root via `uialist::Strings::Get(vl::Locale(localeName))` and pass the same interface to child VMs. This is the source for VM-formatted labels, validation and status; do not duplicate translations in C++. XML imports that resource through `ref.LocalizedStrings` for static `-str` bindings. The generated `DialogStrings::Get(Locale)` in `Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUI.h` is the local precedent. Runtime language switching is not a product feature in this version.

Use ordinary `SetupWindowsDirect2DRenderer()` and DarkSkin, with native separate modal windows. Main content fills the window; tree/grid and preview areas expand, and provider groups fill the scroll width without fixed-height clipping. Set an initial main client size around 1100 by 760 logical units and a usable minimum around 720 by 480; property modal around 900 by 650 and text modal around 700 by 450. These are layout starting values, subject to verification of Japanese/Chinese text and 200% DPI. Keyboard paths must cover tree navigation, Enter to activate selected windows/nodes, Tab/Shift+Tab, Enter/Escape in drafts, and Escape for modal cancellation. Defer tab switching, expansion/layout changes, and modal deletion until after input dispatch where required by GacUI's event rules. Composition events use current `ev.mouseMove-eval`/`ev.mouseDown-eval` and `GuiMouseEventArgs.button`; do not copy removed left-button event names.

In Debug only, construct `vl::presentation::windows::WindowsAutomationService`, substitute it, and call `StartWindowsHttpAutomationService` with the parsed `/AsPort` (default 8888) before the application loop. Use the functions already exposed by `Release/GacUI.Windows.h`; no dependency on test-only remoting helpers. On ordinary shutdown stop the HTTP endpoint, call the service's Stop, and unsubstitute in that order before releasing the controller. The exact reference composition is in `Test/GacUISrc/CppTest/Main.cpp` and the current [GUI execution guideline](../../.github/Guidelines/Running-GacUI.md).

The Debug contract is `http://localhost:<AsPort>/Automation/UiaListApp/Controls` and `.../IO`, with `IO/<WINDOW-ID>` for native modal windows. Name significant XML controls consistently (main tabs, process combo/tree, window list, node tree, preview, property grid, action cards, text editor, OK/Cancel) so Controls output is a stable way to operate the inspector. HTTP IO uses exactly `application/json; charset=utf8`; `Queued` is acceptance, never completion. Release compiles out service startup and exposes no listener. An occupied port is a startup diagnostic and exit, not a silent alternate port.

### 12. Failure and resource contracts

The model boundary distinguishes these outcomes explicitly:

| Outcome | Required behavior |
| --- | --- |
| Empty enumeration/search, null optional relationship, missing pattern, `E_NOINTERFACE` version probe, reserved NotSupported/Mixed, or a documented optional getter returning `UIA_E_NOTSUPPORTED` | Return a typed empty/unsupported/mixed state. No retry or fatal diagnostic. For example optional Styles getters may report unsupported via HRESULT; this is not a provider failure. |
| Invalid local draft or range/element operand from the wrong session | Localized validation, no provider call, keep draft for correction. |
| Capability became read-only/absent during a draft | End the edit as unavailable, refresh capability/readout, make no write. |
| Optional process metadata denied or process disappeared during enumeration | Retain available snapshot identity, or omit a vanished row before publishing. Do not request privileges or retry. |
| Selected window is absent at an explicit process refresh, or the user successfully executed its Close action | Apply the specified normal selection-clearing transition. |
| Capture reports unsupported, or target is in the defined non-capturable state, or sampled geometry is inconsistent | Preview unavailable with reason; no guessed image/hit test. Successful UIA inspection is still usable. |
| Failed UIA call outside the documented absence cases above, access denied on selected inspection, provider error, unavailable element during active read/action, unexpected capture/device failure, or automation startup failure | Report operation, pattern/property ID as applicable, HWND/PID/session identity, and HRESULT/Win32 code; terminate with failure. Do not convert failure to false, null, unsupported, or a successful action result. |
| Internal invariant or unexpected exception | Let the failure reach the process/debugger boundary. No broad recovery catch or continued execution. |

Use only documented capability-absence HRESULTs at the specific call site; do not classify every failed pattern retrieval as unsupported. Encode allowed absence HRESULTs in each descriptor/getter adapter; [Styles](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingstyles) is one explicit example. UIA errors arising inside a job cross back to the application's fatal boundary, preserving the diagnostic and exit status. Do not build a recovery notification dialog that keeps a broken session alive. Avoid logging passwords or full target document text in diagnostics.

Normal lifetime management releases COM references, BSTRs, VARIANTs, SAFEARRAYs, D3D resources and Win32 handles with their matching allocator/owner rules. Retain returned interfaces before their enclosing VARIANT/array is cleared. Do not place constructor/destructor-bearing global objects in these modules. Debug leak detection runs after ordinary main/root/service shutdown; process-terminating exception paths do not need speculative cleanup machinery.

### 13. Implementation sequence and exit gates

Complete these milestones in dependency order. Each exit gate requires the verification cases below; do not count a placeholder card, TODO handler, or disabled unimplemented command as support.

| Milestone | Deliverable | Exit gate |
| --- | --- | --- |
| M1 Shell and resource pipeline | Three product projects, compressed UI, generated interfaces, C++ root, main/modals, locales, Debug automation. | B01-B05, L01-L03 and architecture audit; all four build configurations work before adding inspection complexity. |
| M2 Native fixture and catalog | Controlled Windows fixture, named SDK catalogs, typed result conversion, MTA owner/queue. | C01-C04, F01-F04 and type cases P02. Fixture call/state oracle exists before pattern implementation. |
| M3 Process selection and raw tree | Correct process forest, manual refresh, session identity, labels, complete RawView snapshots. | D01-D06, N01-N05, lifecycle S01-S03. |
| M4 Preview | One-frame capture, image scrolling, transforms, geometric hover, reveal in Nodes. | U01-U06 on both architectures and mixed-DPI desktop configurations. |
| M5 Property inspection/editing | Full property catalog, typed values, row editors, text modal, readback. | P01-P08 and modal/input L02-L03. |
| M6 Patterns and text | All 35 pattern adapters/cards; element operations; returned-reference and text-range workspaces. | A00 for every catalog row, A01-A35 below, R01-R10 and C03-C04. |
| M7 Integration and delivery | Refresh consistency, error exits, normal teardown, all translations/layouts, packaging. | Entire verification matrix has recorded results; no unexplained skips, leaks, or stale generated output. |

Do not modify GacUI or imported/release sources to accommodate the app design. If implementation exposes a demonstrable library bug, isolate its root cause and follow the repository's source-of-truth and required generation/test workflow. Any C++ changes trigger the existing `Test/GacUISrc/UnitTest` verification required by Project.md; that does not create a separate UiaList unit-test product.

## VERIFICATION

### Verification status and evidence

All application checks below are **planned, not run**. This document-only request is verified by requirement coverage, API/source review, Markdown/link checks, and preservation of AGENTS.md. Implementation acceptance requires a result for every applicable row below and every operation in the pattern/range tables.

For each run record: case ID, commit, Windows/SDK versions, inspector and target configuration/architecture, locale/DPI/monitor arrangement, fixture scenario, initial state, exact input sequence, expected result, actual result, pass/fail, and evidence paths. Evidence consists of inspector Controls dumps, screenshots for layout/geometry, and the fixture's independent state and exact call records for mutations. A button rendering, a zero exit code without checking logs, or HTTP `Queued` alone never proves the feature.

### Controlled fixture and independent comparison

Build a native Windows fixture under `Tools/UiaList/Verification` with its own solution/project, supporting Win32/x64. It is a development target that exposes ordinary Win32 controls and synthetic UIA providers, not a test mode or CLI edition of UiaList. It must use independent expected data and call logging, not import UiaList's adapters/catalog code to generate its oracle. Launch UiaList and fixture under the documented repository execution/debugging procedures on an unlocked interactive desktop.

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
| U03 Hit rules | Exercise overlaps, parent/child depth, child outside parent, tied depth, zero/invalid/offscreen rectangles, root background, and mouse leave. Results match maximum depth then preorder; no stale hover on click. |
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

### Pattern/action and range acceptance

**A00 applies to every one of the 35 registry rows:** test pattern absent and present, exact provider/client naming, all readouts, every parameter editor, all commands, null/empty returned results where valid, dynamic capability changes, and error diagnostic paths. The fixture must assert actual target key, client operation's provider counterpart, typed arguments, call count, and resulting state. Read-only pattern cards pass by correct complete readings, not invented commands. Versioned patterns are tested alone and with their base pattern.

Cases **A01 through A35** correspond respectively to numeric pattern IDs **10000 through 10034** in section 10. Each row's acceptance behavior is mandatory. For methods with enumerated inputs (scroll amounts, dock/visual states, navigate direction, input type, zoom unit), exercise every documented value supported by its fixture scenario plus local rejection of invalid values. Use separate disposable fixtures for Window.Close and expected failures so one action cannot invalidate later evidence.

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

## REVIEW COMMENTS

No unresolved design question requires input before implementation. The factual client/provider, property-setter, capture, and locale distinctions from the original requirements are resolved in DETAILS. AGENTS.md remains the original requirement source and is not edited by this plan.

### Provider-group layout acceptance

Actions keeps Providers, Results, References and Text ranges. All provider and range sections use full-width titled group boxes without expansion state. Parameterless getters remain readouts, parameterized getters run only on valid commits, and explicit mutations/listening/workspace actions execute once. No display/initialization path starts one full inspection per group. Use independent fixture logs for call counts.

Measure approximately five pixels between visible controls and around groups, including skin insets. Empty validation/status/reference rows have zero minimum size and no padding; actual text wraps. Check controls with/without parameters, readouts, returned references and short/long text-range groups through Controls and screenshots at default/reduced sizes. Verify the last control remains reachable after scrolling. XML padding equality alone is not acceptance evidence.
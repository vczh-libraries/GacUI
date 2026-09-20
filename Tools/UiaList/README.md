# UiaList

UiaList is a Windows UI Automation inspector built with GacUI and C++20. It lists the process forest, captures a selected window, walks its complete UIA Raw View, and exposes properties, standard patterns, returned elements, and text ranges.

Select a process in the hierarchical combo beside **Refresh**, then single-click one of its visible windows in the details list. **UI** displays a captured snapshot; hovering outlines the deepest matching UIA rectangle, and clicking selects that node in **Nodes**. Right-click a node and choose **Inspect**, or press Enter, to open **Properties** and **Actions**. Double-click expands/collapses the node. Refresh is explicit; successful mutations refresh the selected target. Actions operate on the inspected application.

The dropdown retains headless ancestors of visible descendants, but each process lists only its own visible, uncloaked, nonempty top-level windows. Minimized windows remain selectable even when their preview is unavailable. Process refresh preserves surviving PID/creation-time and HWND/PID identities without recapturing the inspected window. Selecting another process only changes the window list.

The property grid omits unsupported properties and retains supported false, zero, empty, null, mixed and read-only values. It offers editors only for explicitly mapped setters. Text whose single-line behavior cannot be established uses a modal editor. The Actions page contains always-visible provider group boxes, typed arguments, results, reference navigation, and text-range workspaces. Canonical SDK names and target text remain untranslated. The surrounding UI selects English, Simplified Chinese, or Japanese from Windows' preferred UI languages.

## Build

Use Visual Studio 2026 with toolset v145 and the latest Windows 10 SDK. Projects provide Debug/Release for x64/Win32. Their settings follow the existing repository projects, with explicit source inventories and consistent runtime libraries.

From this solution directory:

```powershell
& C:\Code\VczhLibraries\Tools\Tools\GacBuild.ps1 -FileName C:\Code\VczhLibraries\GacUI\Tools\UiaList\GacUI.xml
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable UiaListApp -Configuration Debug -Platform x64
```

`CLI` is the repository launch wrapper's mode. UiaList itself has no command-line frontend. Its generated, compressed resource is linked into the executable; no XML or resource binary needs to accompany it.

## Source ownership

- `UiaList/UI` owns all views, bindings, events, view-model declarations, and translations.
- `UiaList/Source` is generated exclusively by GacBuild. Edit the authored XML and regenerate.
- `UiaList/ViewModel` implements the view models and native services. UIA objects belong to a dedicated MTA queue; capture uses a separate worker and transfers owned pixels to the UI thread.
- [UiaCatalog.Windows.cpp](UiaList/ViewModel/UiaCatalog.Windows.cpp) owns the named SDK property, pattern, control-type, text-attribute and metadata inventories, declared in [UiaCatalog.Windows.h](UiaList/ViewModel/UiaCatalog.Windows.h). [UiaCatalog.Actions.Windows.cpp](UiaList/ViewModel/UiaCatalog.Actions.Windows.cpp), [UiaCatalog.Dispatch.Windows.cpp](UiaList/ViewModel/UiaCatalog.Dispatch.Windows.cpp) and [UiaCatalog.TextRange.Windows.cpp](UiaList/ViewModel/UiaCatalog.TextRange.Windows.cpp) describe and dispatch supported operations. [CheckCatalog.ps1](Verification/CheckCatalog.ps1) compares the inventory with the selected SDK; historical counts do not replace that check.
- `Gaclib` compiles the existing Import/Release dependencies. `UiaListApp` supplies startup, locale, theme, lifetime, and the Debug automation endpoint.
- `Verification` contains an independent Win32/UIA fixture and developer scripts. The product does not depend on them.

Debug exposes `http://localhost:<AsPort>/Automation/UiaListApp/Controls` and `/IO`. `/AsPort:<decimal port>` accepts one value in 1..65535 and defaults to 8888. Malformed, empty, duplicate, and out-of-range values fail before startup. Release does not start that endpoint. Run CppTest, CppTest_Metaonly, and UiaListApp together on 8888, 8890, and 8891 respectively. This argument changes only automation; the remoting protocol remains fixed at 8888. See [verification instructions and recorded coverage](Verification/README.md) and the full [acceptance matrix](Verification/README.md#acceptance-matrix), including the pattern-operation inventory, R01-R10 text-range cases and provider-group layout criteria.

The Actions page displays pure parameterless getter results directly. Parameterized getters run after a valid edit is committed with Enter, focus loss, a choice change, or accepting the text editor. Mutations and range-workspace/listening operations remain explicit commands. LegacyIAccessible's default action is enabled only when the provider currently reports an available action; expected unsupported and unavailable results remain visible without terminating the inspector.

## Inspection boundaries

A node's `TYPE` label identifies the highest public client `IUIAutomationElementN` interface successfully acquired, with `IUIAutomationElement` as the base. Its semantic ControlType is separate. Provider labels such as `IInvokeProvider` are the documented counterparts of acquired client patterns; the inspector does not directly query those provider interfaces on the target. The Raw View traversal includes non-control/non-content and cross-process descendants of the selected root, without a desktop-wide traversal or automatic realization of virtualized items.

Returned elements and text ranges are references, not additional raw-tree children. An element inside the selected subtree can be revealed in Nodes; an external reference is inspectable without inventing ancestry or changing the selected root. Range comparison uses the owning document's identity, including `TextChild.TextContainer`, rather than the node that happened to return the range. Refresh or target mutation requires reacquiring ranges; local Clone/Find/endpoint operations retain their workspace operands.

The inspector covers named standard descriptors in the selected SDK. Unknown vendor registrations, provider implementation internals and arbitrary returned COM object methods are outside generic discovery. Opaque ObjectModel and IAccessible results remain opaque. Supported false, zero, empty, null and mixed values are distinct from Unsupported. UIA has no generic property setter: the [property-editor contract](Verification/README.md#property-editor-acceptance-contract) lists the explicit method mappings and the verified native single-line Edit predicate.

GacUI's own Windows provider implementation is documented separately in the [GacUI UI Automation knowledge-base page](../../.github/KnowledgeBase/KB_GacUI_Design_UIAutomation.md). Its [gap report](../../ToDo/UiaGap.md) tracks implementation gaps against Microsoft contracts. Neither document is a replacement for the inspector's broader SDK catalog.

## Preview semantics

The preview is a captured snapshot, not a live input surface. Hover and click use the published generation's physical UIA bounding rectangles; clicking selects an inspector node without sending target input. The hit test selects the deepest eligible rectangle, breaking ties by preorder, and can consider children outside their parent's bounds. This rectangle estimate is not a guarantee about a provider's shaped or overlapping hit test.

Preview acceptance requires matching capture extents and node rectangles from one published generation. With viewport pointer `q`, scroll offset `s` and image layout offset `o`, image position is `q + s - o`, target position adds the physical capture origin, and a node's image rectangle subtracts that origin. Image-local pointer events must not add scrolling again; UIA physical coordinates must not receive a second DPI conversion. The image keeps one captured pixel per logical preview unit, with scrolling instead of automatic fitting. See U01-U06 in the [acceptance matrix](Verification/README.md#processes-tree-and-preview-checks) for geometry and unavailable-capture checks.

Minimized, hidden, cloaked, protected or otherwise unsupported capture can leave inspection usable with an unavailable preview. Capture must not restore, activate, move or unprotect the target merely to obtain pixels. A protected black frame may be indistinguishable from valid black content; HDR fidelity and secure-desktop access remain platform limitations. The current verification record distinguishes measured Controls bounds from actual screenshot evidence.

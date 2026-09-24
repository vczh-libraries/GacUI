# UiaList: Windows UI Automation Inspector

UiaListApp is the Windows desktop inspector in `GacUI/Tools/UiaList`. Use it to discover application windows, inspect their UI Automation trees, read properties, and exercise supported actions. Run `UiaListApp.exe`; its resources are embedded, so no resource files need to accompany it.

## Inspect a Window

1. In **Processes**, select a process from the hierarchical dropdown beside **Refresh**. The list shows that process's visible top-level windows. An ancestor retained only because it has visible descendants can have an empty window list.
2. Single-click a window row, or select it and press Enter. **UI** opens a captured snapshot of that window.
3. Hover over the snapshot to outline the deepest matching UI Automation node. Click to reveal that node in **Nodes**. Scroll when the snapshot is larger than the preview area.
4. In **Nodes**, right-click a node and choose **Inspect**, or select it and press Enter. Double-click expands or collapses the tree.

The preview is a snapshot. Clicking it selects an inspector node; it does not click the inspected application. A minimized or otherwise unavailable preview does not necessarily prevent inspecting the window's nodes. **Refresh** explicitly refreshes process discovery and preserves surviving selections; selecting a different process only changes the window list.

## Properties and Actions

The inspection window has **Properties** and **Actions** tabs. Properties lists supported values, including false, zero, empty and read-only values. Editable properties offer an embedded editor or a **...** button opening a multiline editor. Unsupported properties are omitted; not every property has a setter.

Actions groups the target's supported operations by provider. Parameterless getters display their results directly. Parameterized getters run when valid edits are committed, such as with Enter or focus loss. Mutations and text-range workspace commands require explicit activation. These actions operate on the inspected application, so invoking them can change its state.

Returned element references can be inspected; references within the selected tree can also be revealed in **Nodes**. Text-range workspaces support inspecting and manipulating ranges. Refresh or target mutation can invalidate an earlier range and require acquiring it again. Availability depends on what the target application's UI Automation providers expose.

The surrounding UI selects English, Simplified Chinese or Japanese from Windows' preferred UI languages. Standard interface names and text from the target retain their original spelling.

## Launch and Automation

The executable is Windows-only. Build and launch instructions are in `GacUI/Tools/UiaList/README.md`; the release tool build instructions are in `Release/Tools/README.md`.

Debug builds expose `http://localhost:8888/Automation/UiaListApp/Controls` and `/IO`. Pass `/AsPort:8891`, for example, when another application uses the default port. One decimal port from 1 through 65535 is accepted. Release builds do not start an HTTP automation endpoint; Windows UI Automation remains available.

For GacUI applications' own UI Automation support, see [Windows UI Automation](./KB_GacUI_Design_UIAutomation.md).

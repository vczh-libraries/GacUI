# UiaList

This folder is for a `UiaList` tool, to inspect any UI application on Windows, revealing all information with UI Automation. No need to consider anything about cross-platform. `UiaList` is based on GacUI, a pure C++ application built on top of GacUI, Windows API, COMs.

Check out and follow [best practices](../BestPractices.md):
- Tool name is `UiaList`.
- The GUI version is named `UiaListApp`.
- The TUI version is currently out of scope, skip this part.

## UI Organization

The main part is a tab control:
- `Processes` tab:
  - A process-only hierarchical combo box fills the row beside `Refresh`. Retain processes whose own or descendant top-level windows are visible, uncloaked and have nonempty bounds; the synthetic root is hidden.
  - Discovery runs on initialization and explicit Refresh. Preserve process selection by PID/creation time and inspection by HWND/PID; discard obsolete worker results.
  - A details list contains only the selected process's qualifying windows, with localized title, window class and pointer-sized HWND columns. A retained headless ancestor has an empty list.
  - A single left click on a window row or Enter activates that exact window and switches to UI. Process selection, column headers and empty space never activate the previous window.
- `UI` tab:
  - Render the UI of a selected window, scroll bars are required when the selected window is too large.
  - Mouse hovering on the selected window shows a rectangle on the deepest UIA node under the cursor.
  - Clicking the selected window jumps to the `Nodes` tab, expanding to the focused UIA node.
- `Nodes` tab:
  - A tree view rendering all UIA nodes from the selected top level window.
  - If a UIA node has text, it renders `TEXT (TYPE)`, otherwise `(TYPE)`. `TYPE` is its actual COM interface name. We only considered public COM interfaces in UIA concept.
  - If a UIA node has action providers, each one is listed like `(TYPE, IInvokeProvider, ...)`
  - Right-click a node and choose the localized `Inspect`, or press Enter, to open its modal property window. Double-click retains tree expansion/collapse. Stale nodes/menu commands cannot inspect another selection.

Property Window:
- The main part is a tab control:
  - The `Properties` tab renders supported properties, filtering only native `ValueKind::Unsupported` on every publication. Supported false, zero, empty, null, mixed and read-only values remain.
    - If a property can be modified, clicking the cell should open an embedded editor.
    - For any property that is supposed to accept multiple lines of text, there will be no embedded editors, instead in the third column there will be a `...` button appears when the row is selected.
    - The `...` button for multiple lines of text opened another modal window, with a text box, OK button, Cancel button.
  - The `Actions` tab renders all supported `IInvokeProvider` and all other providers in a proper way.
    - Providers are always-visible titled group boxes stacked at full scroll width; text-range sections share the same rendering. Keep approximately five pixels between visible objects, compact empty status/validation rows, and verify actual bounds/screenshots.
    - Parameterless getters are readouts, parameterized getters run on valid commits, and mutation/listening/workspace commands require explicit activation. Rendering a group must not queue a complete inspection or mutation.

## Debugging

Debug exposes the automation service on `/AsPort:<1-65535>`, defaulting to 8888; Release starts no endpoint. Native Windows GacUI applications also support UIA. The HTTP Controls/IO endpoint provides complementary inspection, including actual popup/modal window IDs. Use the repository execution wrappers and exact `application/json; charset=utf8` content type.

## UI Automation Background

Use the current SDK inventories in `UiaList/ViewModel/UiaCatalog.Windows.h/.cpp` and the adapters in `UiaCatalog.Actions.Windows.cpp`, `UiaCatalog.Dispatch.Windows.cpp`, and `UiaCatalog.TextRange.Windows.cpp` to identify inspectable types and operations. Compare them with the selected Windows SDK using `Verification/CheckCatalog.ps1`. The full inspector acceptance matrix and pattern/range contracts are maintained in [Verification/README.md](Verification/README.md#acceptance-matrix).

GacUI's implemented provider behavior is documented in [the UI Automation knowledge-base page](../../.github/KnowledgeBase/KB_GacUI_Design_UIAutomation.md); [the UI Automation gap report](../../ToDo/UiaGap.md) records requirements to verify against Microsoft documentation. Keep the inspector's complete SDK coverage separate from the subset of providers implemented by GacUI.

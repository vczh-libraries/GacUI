# UiaList

This folder is for a `UiaList` tool, to inspect any UI application on Windows, revealing all information with UI Automation. No need to consider anything about cross-platform. `UiaList` is based on GacUI, a pure C++ application built on top of GacUI, Windows API, COMs.

Practice complete Model-View-ViewModel pattern:
- GacUI XML Resource defines view model interfaces, UI, and how UI react to view models by data binding.
- All non-rendering features should be represented by view model interfaces.
- Implementation of view model interfaces therefore no need to know anything about the actual UI, although it inevitably depends on GacUI source code.
- A root interface will be offered to the main window, all sub view models are accessible from it.
- The design of view model interfaces and data structure offered by the view model interfaces should already be in the shape of the UI, to reduce calculation needed in GacUI XML Resource.
- View model implementations are required to be in C++.
- All events are required to handle with the `-eval` binding in GacUI XML Resource.
- It is forbidden to modify any generated files.

You are not recommended to change GacUI itself unless you find any bug that the root cause is actually in GacUI.

## Important vcxproj Settings

- `Windows SDK Version`: `Windows 10.0 SDK (latest)`
- `Platform Toolset`: `v145 for Microsoft C++ Build Tools`
- `C++ Language Standard`: `ISO C++ 20 Standard`
- `Include Directories`: add the `REPO-ROOT/Import` folder
- `Use Library Dependency Inputs`: `Yes`
- `Preprocessor`:
  - `VCZH_DEBUG_NO_REFLECTION`: all reflection code will be eliminated during compiling
  - debug profile
    - `VCZH_CHECK_MEMORY_LEAKS`
    - memory leaks checking should only be applied on debug profile.

Settings are expected to be aligned with other

## Basic File Organization

The design reserves the ability to add unit test and CLI version of `UiaList` but they are non-goal right now.

- `UiaList.sln`.
- `Gaclib/Gaclib.vcxproj`: a library to add all necessary files from `REPO-ROOT/Import`.
- `UiaList/UiaList.vcxproj`: a library to compile GacUI XML Resource with view model implementation.
  - `UI`: a folder for GacUI XMl Resource.
    - All texts on UI are required to be localizable, default to `en-US` in the XML, but GacUI should pick up the language based on OS language automatically.
    - Offer English, Chinese, Japanese translations and keep them in sync.
  - `Source`: a folder for generated C++ code from `UI`:
    - Binary resource should be printed in C++ so that the app does not need to load an extra binary resource. This is done by adding `CppCompressed`  to `GacGenConfig`.
    - `REPO-ROOT/../Tools/Tools/GacBuild.ps1` should be used to generate this folder.
  - `ViewModel`: View model implementation.
- `UiaListApp/UiaListApp.vcxproj`: a GacUI based app running `UiaList`, as the actual interactable tool.

## UI Organization

`UiaListApp`

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

All inspectable types should already be included in a research document `REPO-ROOT/ToDo/Task_UIA.md`.

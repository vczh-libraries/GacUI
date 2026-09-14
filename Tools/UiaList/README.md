# UiaList

UiaList is a Windows UI Automation inspector built with GacUI and C++20. It lists the process forest, captures a selected window, walks its complete UIA Raw View, and exposes properties, standard patterns, returned elements, and text ranges.

Double-click a window in **Processes**. **UI** displays a captured snapshot; hovering outlines the deepest matching UIA rectangle, and clicking selects that node in **Nodes**. Double-click a node to open **Properties** and **Actions**. Refresh is explicit; successful mutations refresh the selected target. Actions operate on the inspected application.

The property grid offers editors only for explicitly mapped setters. Text whose single-line behavior cannot be established uses a modal editor. The Actions page contains the acquired patterns, typed arguments, results, reference navigation, and text-range workspaces. Canonical SDK names and target text remain untranslated. The surrounding UI selects English, Simplified Chinese, or Japanese from Windows' preferred UI languages.

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
- `Gaclib` compiles the existing Import/Release dependencies. `UiaListApp` supplies startup, locale, theme, lifetime, and the Debug automation endpoint.
- `Verification` contains an independent Win32/UIA fixture and developer scripts. The product does not depend on them.

Debug exposes `http://localhost:8888/Automation/UiaListApp/Controls` and `/IO`. Release does not start that endpoint. See [verification instructions and recorded coverage](Verification/README.md) and the full [acceptance plan](Planning.md).

# TODO

## Progressing

- `GuiHostedController`, `GuiHostedMonitor` and `GuiHostedWindow`.
  - User offers an ordinary `INativeController` and `INativeWindow` implementation.
  - `GuiHostedController` and `GuiHostedWindow` use the above implementation to implement the `hosted single window` mode.
    - The only `GuiHostedWindow` passed to `IWindowService::Run` is the main window.
      - It is special that it could partially control and react to the underlying native `INativeWindow`.
      - It always fill the full `GuiHostedMonitor`.
      - When the underlying native `INativeWindow` become inactived, main window and all `GuiHostedWindow` are inactivated.
      - When main window is disabled, it doesn't affect the underlying native `INativeWindow`.
      - ... (the window manager handles all the details)
    - `GuiHostedWindow` is free to move.
    - A window manager that treats main window as the desktop and other `GuiHostedWindow` as windows.
    - DPI still changes in runtime.
  - Implementing these interfaces enable GacUI to run in the `hosted single window` mode, all GacUI windows and menus are rendered in one native window.

- Stop using `GetCurrentController()` in `Source_GacUI_Windows`.

## OS Provider Features

- Drag and Drop framework.
- Windows
  - `INativeImage::SaveToStream` handle correctly for git format. It is possible that LoadFromStream need to process diff between git raw frames.

## Control Features

- `IColumnItemView`.
  - Change column size from `int` to `{minSize:int, columnOption:Absolute|Percentage, absolute:int, percentage:float}`
  - Column drag and drop.
    - ListView: only raises an event, developers need to update column headers and data by themselves.
    - DataGrid: swap column object, cells are changed due to binding.
- ListView.
  - `GroupedListView` and `BindableGroupedListView` from `GuiVirtualListView`: Group headers on all views.
  - `TreeListView` and `BindableTreeListView` from `GuiVirtualTreeView`.
- New `GuiListControl::IItemArranger` that can accept a `GuiRepeatCompositionBase`.
- Chart control.
- Code editor (need VlppParser2)
- Dock container.
  - Use drag and drop to perform docking
  - A small framework for implementing dock guiding UI
  - A predefined dockable tool window for users that are fine with the default behavior about how to transfer content to the dock container
  - A "binary tree layout/control" for implementing VS-like dock container
- Touch support.

## Architecture

- New skin.
  - New default control templates with animation, written in XML generated C++ code.
  - A window can be called to update all its controls' and components' template.
- UI Automation.
- Test Automation on virtual command line mode by comparing characters rendered on a very small viewport.
- Test Automation on Tutorials.
- Use the embedded data codegen / compress / decompress functions from `VlppParser2` to replace one in `GacUI`.

## GacUI Resource Compiler

- `<eval Eval="expression"/>` tags.
- In the final pass, only workflow scripts are printed.
  - Use WorkflowCompiler.exe to do codegen externally.

## Graphics

- 2D drawing API.
- restriction-based MetaImageElement.
  - Remove PolygonElement.
  - Default non-text element renderer using 2D drawing API.
- Meta3DElement and Meta3D data structure.
  - Default Meta3DElement renderer using MetaImageElement with a surface sorting based algorithm.
- GIF player.
- video player.

## GacStudio

## Porting to New Platforms

- SyncTree architecture that streams layout/element changes per `GuiControlHost`.
  - or called remoting whatever
- Port GacUI to other platforms:
  - Windows
    - Command-line/Powershell in Windows (hosted)
    - GDI (hosted mode)
    - Direct2d (hosted mode)
    - UWP (hosted mode + sync tree, optional)
  - Linux
    - Ncurses on Ubuntu (hosted)
    - gGac repo: complete development process for release
  - macOS
    - iGac repo: complete development process for release
  - Web Assembly (hosted mode + sync tree)
    - Canvas?
    - DOM?

## Binders for other Programming Languages

- Xml still generates C++ files with optional files.
  - User need to compile C++ code by themselves into a DLL.
  - User need to specify all involved (including the default) themes.
  - Generated DLL functions are for implementing view model or SyncTree.
  - ViewModel metadata in JSON are also provided.
  - Resources are required to be generated in C++ files since the DLL won't provide interfaces for loading external resources.
- Applications written in other language can:
  - Implement view model.
  - Render the UI via SyncTree.
- Languages:
  - JavaScript / TypeScript through Web assembly
  - .NET (core?)
  - Python
- Other options:
  - UI becomes an EXE, the view model implementation is communicated via pipe or socket.

## Optional

### GacUI Resource Compiler

- Remove all loader implementation, enabling custom control types from developers.
  - Try not to include `GacUI.cpp` if `VCZH_DEBUG_METAONLY_REFLECTION` is on.
  - `mynamespaces::VirtualClasses_X` for adding virtual classes deriving from `mynamespaces::X`.
    - Containing methods like: `GuiSelectableButton* CheckBox()`.
  - `mynamespaces::InstanceLoader_X` for implementing different kinds of properties / constructors that do not exist in `mynamespaces::X`.
    - following a naming convention, e.g. `GuiTableComposition`'s rows/columns properties.
    - searching for correct default control templates.
- Incremental build inside single resource.
  -  Calculate dependencies by only parsing.
  -  Cache workflow assembly per resource in file.
  -  Codegen c++ from multiple workflow assembly.

### MISC

- Use collection interfaces on function signatures.
  - Only if `Vlpp` decides to add collection interfaces.

# TODO

## Progressing

- Issues
  - Big cursor of document empty line
  - Focused tab header becomes black
- Theme
  - Stop using Webdings in default template.
  - Add HostedWindow theme.
- Add "Open New Window" button to Tutorials/ControlTemplates/WindowSkin.
- One GacGen.exe instead of GacGen32.exe and GacGen64.exe

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

## Porting to New Platforms

- SyncTree architecture that streams layout/element changes, requiring Hosted for the first version.
- ViewModel architecture that streams object changes.
  - Requires all pointers are shared.
- Port GacUI to other platforms:
  - Unit Test
    - Simplified CLI (Hosted)
  - Windows
    - Command-line/Powershell in Windows (Hosted)
    - GDI (Hosted or SyncTree)
    - Direct2d (Hosted or SyncTree)
    - UWP (Hosted and SyncTree)
  - Linux
    - Ncurses on Ubuntu (Hosted)
    - gGac repo: complete development process for release
  - macOS
    - iGac repo: complete development process for release
  - Web Assembly (Hosted + SyncTree)
    - Canvas?
    - DOM?

## Binders for other Programming Languages

- User need to specify which ViewModel interfaces are involved in streaming
  - Metadata are offered so that users could write their own codegen
  - Client side implementation for interfaces. These objects are given to the UI, they send out commands and waiting for receiving real view model data
  - ViewModel streaming implementation for server side. You give them all implementations of interfaces, it handles commands and send back real view model data
    - Users could use metadata to implement it in other languages other than C++
- Applications written in other language can:
  - Implement view model.
  - Render the UI via SyncTree.
- Languages:
  - JavaScript / TypeScript through Web assembly
  - .NET (core?)
  - Python

## Streaming Tutorials

- A GacUI D2D process connecting to a server process for streaming ViewModel
  - ViewModel implements in C++ and C#
- A GacUI SyncTree process connecting to a server process for streaming graphics
  - GDI+ implements in C#
  - D2D implements in C++

## GacStudio

## GacUI Resource Compiler

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

## MISC

- Use collection interfaces on function signatures.
  - Only if `Vlpp` decides to add collection interfaces.

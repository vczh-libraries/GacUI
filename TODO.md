# TODO

## Progressing

- Issue on `<LocalizedStrings/>`
  - Enable external injection in C++ code or `<LocalizedStringsInjection/>`.
- Issue on GacGen
  - `A10: Expression of type "system::Function<system::Void, system::Object>^" cannot implicitly convert to "system::Function<system::Void, system::String>^".`
    - When define `event RequestConfirm(string);` with `<ev.RequestConfirm-eval><![CDATA[{}]]></ev.RequestConfirm-eval>`
  - `event RequestConfirm(string{});` generates `::vl::Event<void()> RequestConfirm;`
- Issue on dialog service
  - message box default button.
  - message box is not shown at the center.
  - message box disable `X` button if `Cancel` is not in the button list or `OK` is the only button.
  - New tab on `FullControlTest` to show options of dialogs.
  - font dialog alt to list and press UP/DOWN jumps to incorrect item.
  - font dialog alt `K` should focus to dropdown instead of waiting for `R`, `G`, `B`.
  - file dialog can't expand/collapse tree view by key.
  - file dialog filter should filter files in list view.
  - file dialog append extension if file name doesn't have it.
- Issues on GDI
  - Big cursor of document empty line (GDI)
  - Non main window doesn't shrink when moving back to low DPI monitor.
- ~~<ComboButton/>~~
- Add document for `<ComboButton/>` and its `DropdownControl` property.
- Theme
  - Add HostedWindow theme.
  - `INativeWindow` add callback for state changing.
    - Including `MaximizedBox`, `MinimizedBox`, `Border`, `SizeBox`, `IconVisible`, `TitleBar`, `Icon`, `Title`, `SizeState`.
    - In `GuiControlHost` or `GuiWindow`, setting border or state doesn't update the control template, it is updated in that callback.
    - Delete `GuiControlHost` and `GuiWindow`'s `OnVisualStatusChanged`.
- ~~Add default dialog service, will be use in hosted mode.~~
  - Create `GacUI_Utilities`, depending on `GacUI_Controls`, to store all services.
    - Move `GuiSharedAsyncService` here.
    - A `GuiFakeDialogServiceBase` implements `INativeDialogService`, taking windows that receives view models.
    - A `GuiSharedCallbackService` (rename from `WindowsCallbackService`) implements `INativeCallbackService`, providing additional members to invoke callbacks.
      - Add `Invoker()` returning `INativeCallbackInvoker` to `INativeCallbackService`.
    - A `GuiFakeClipboardService` that transfer objects in the current process, not talking to the OS.
    - `GuiInitializeUtilities` and `GuiFinalizeUtilities` substitute fake services by default optionally.
  - Predefined reflectable view models for dialogs, with predefined implementations.
    - In `GacUI_Utilities` and `GacUI_Utilities_Reflection`.
  - Predefined windows implemented in XML.
    - In `GacUI_Utilities_Controls` and `GacUI_Utilities_Reflection`.
    - With `GuiFakeDialogService` that gives predefined windows to `GuiFakeDialogService`.
  - A way to subsitute services.
    - Rename `SetCurrentController` to `SetNativeController`.
    - `GetCurrentController` returns a `INativeController` implementation that allow subsituting services.
      - If a service is not provided, and there is also no substitution, it crashes.
      - If a service is used, substituting crashes.
    - `GetNativeServiceSubstitution` returns an object to
      - Config how services are substituted (force activated, or only activate when it is not provided).
    - Call `GuiInitializeUtilities` and `GuiFinalizeUtilities` around `GuiMain`.
  - Substitutable services:
    - Clipboard
    - Dialog
- Remove SysKey callbacks, merge into Key callbacks.
- Add "Open New Window" button to Tutorials/ControlTemplates/WindowSkin.
- Add new tutorial to use `FakeDialogService` based on `FullControlTest`'s new tab with localization for `zh-CN`.
- Rewrite calculator state machine demo, when "+" is pressed, jump into "WaitingAnotherOperandForPlus" state machine, instead of storing the operation in a loop. So there will be no loop except for waiting for numbers.
- Check makefile for ParserGen/GlrParserGen/CodePack/CppMerge/GacGen
  - Write maketools.sh
- Rewrite GacBuild.ps1 in C++ (optional)
- Add DarkSkin and Dialog in Reflection(32|64).bin (optional)

## OS Provider Features

- Drag and Drop framework.
  - Substitutable.
  - `GuiFakeDragAndDropService`.
  - Substitute DragAndDrop by default optionally.
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

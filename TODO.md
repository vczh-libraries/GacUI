# TODO

## Done but not Released

## Known Issues

- `GuiVirtualRepeatCompositionBase`.
  - Eliminate double `ForceCalculateSizeImmediately()` calls in `TestCompositions_VirtualRepeat.cpp` (TODO) and related files.
- FakeDialogService
  - `FakeDialogServiceBase::ShowModalDialogAndDelete` place the window in the center of `owner` instead of the screen.
  - Specify multiple extensions in one filter, exactly like Win32 API.
  - Extensions not applied before checking file existance.
- Expanding collapsing tree nodes cause the whole list to rebuild, which glitch during multiple passes of layout.
  - Only affected items need to rebuild.
- Bindable list control (optional)
  - When a property referenced by `<att.XXXProperty>` is updated, the list item is not refreshed.
    - Need to find a way to listen to the event.

## Progressing (before release)

- MISC
  - Check everywhere that uses `CachedBoundsChanged.Attach`.
  - Enlarging window slower than shrinking.
- DarkSkin
  - Need to hardcode a minimum size for scroll bar handler. When list has too many items, the handler will disappear.
  - Add minimum size control to `<PartialView>` (should have been done).
  - Use it in `<PartialView ref.Name="handle">`.
  - https://github.com/vczh-libraries/GacUI/issues/73
- Check makefiles in Release repo
- Document `GuiVirtualRepeatCompositionBase`'s `UseMinimumTotalSize` property.
- Document `GuiRibbonGroupTemplate::SubMenuTemplate` if exists.
- Test `GuiVirtualRepeatCompositionBase::GetAdoptedSize`.
- Test `GuiVirtualRepeatCompositionBase::GetTotalSize` with `UseMinimumTotalSize`.

## Unit Test Framework

- UnitTest.vcxproj
  - Complete `TestCompositions_Bounds.cpp`.
  - Test controls with a unit test only platform provider running in hosted mode
    - Each character takes exactly `FontSize x FontSize`
    - Deal with `\r` and `\n` when multiline is enabled
  - Test against more code as many as possible
- Add above functionality to `GacUI.UnitTest.cpp`, `GacUI.UnitTest.h`, `GacUI.UnitTest.Reflection ...`
- In release repo add more tools that just call `GacUI.UnitTest.cpp`:
  - Load x86 bin + workflow script and execute.
  - Load x64 bin + workflow script and execute.
  - Render unit test results, especially each frame of intermediate rendering result.
    - Can navigate to workflow script.

## Progressing

- Rewrite `GacBuild.ps1` in C++
- Get rid of `Deploy.bat` in `GacGen.ps1` and `GacGen.exe`
- DarkSkin Color Theme.
  - Create a `DarkSkinPalette` class with a static getter method to retrive default colors.
    - Update all `Style.xml` colors to use `DarkSkinPalette`.
  - Add a static setter to `DarkSkinPalette`.
    - A window can be called to update all its controls' and components' template.
    - The above function will be called inside the setter.
- Consider `-ani` binding, create an animation controller object that change the binded property, with predefined interpolation and other stuff.
  - All types that can do interpolation are value types, consider following formats:
    - "NAME:initial value"
    - "NAME(initial value in expression)"
    - Need to be consistent with animation object
  - Consider multiple `-ani` batch control, state configuration and transition, story board, connection to animation coroutine, etc.
- Facade
  - If `<XFacade>` or `<x:XFacade>` is an accessible and default constructible object, then `<X>` or `<x:X>` triggers a facade.
  - A facade is a class with following methods:
    - **AddChild**: Accept a child facade or a child object.
    - **ApplyTo**: Accept a parent object, which is not a facade.
    - **Initialize** (optional): Called on the instance object between construction and `<ref.Ctor>`.
  - A facade could have properties but only accept assignment or `-eval` binding.
  - A facade could have an optional **InstanceFacadeVerifier** executed on GacGen compile time.
  - Built-in Layout and Form facade.

## Optional

- `INativeWindow` add callback for state changing.
  - Including `MaximizedBox`, `MinimizedBox`, `Border`, `SizeBox`, `IconVisible`, `TitleBar`, `Icon`, `Title`, `SizeState`.
  - In `GuiControlHost` or `GuiWindow`, setting border or state doesn't update the control template, it is updated in that callback.
  - Delete `GuiControlHost` and `GuiWindow`'s `OnVisualStatusChanged`.
- FakeDialogService
  - message box disable `X` button if `Cancel` is not in the button list or `OK` is the only button.
- GDI
  - Big cursor of document empty line (GDI)
  - In hosted mode, non-main window doesn't shrink when moving back to low DPI monitor.
- Hosted
  - When dragging left/top border if the main window, the window move if the size is smaller than the minimum size.
- Rewrite calculator state machine demo, when "+" is pressed, jump into "WaitingAnotherOperandForPlus" state machine, instead of storing the operation in a loop. So there will be no loop except for waiting for numbers.
- Check makefile for ParserGen/GlrParserGen/CodePack/CppMerge/GacGen
  - Write maketools.sh
- Rewrite GacBuild.ps1 in C++
- Add `MoveToScreenCenterAfterLayout` as what is done in `FakeDialogServiceBase::ShowModalDialogAndDelete`.

## Document

## OS Provider Features

- Drag and Drop framework.
  - Substitutable.
  - `GuiFakeDragAndDropService`.
  - Substitute DragAndDrop by default optionally.
- Windows
  - `INativeImage::SaveToStream` handle correctly for git format. It is possible that LoadFromStream need to process diff between git raw frames.

## Control Features

- Check all control key operation, ensure accessibility.
- `IColumnItemView`.
  - Change column size from `int` to `{minSize:int, columnOption:Absolute|Percentage, absolute:int, percentage:float}`
  - Column drag and drop.
    - ListView: only raises an event, developers need to update column headers and data by themselves.
    - DataGrid: swap column object, cells are changed due to binding.
- ListView.
  - `GroupedListView` and `BindableGroupedListView` from `GuiVirtualListView`: Group headers on all views.
  - `TreeListView` and `BindableTreeListView` from `GuiVirtualTreeView`.
    - Or add such feature to `GuiBindableDataGrid` or a new class `GuiBindableTreeGrid`.
      - Try to reuse code from treeview.
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
  - Calculate dependencies by only parsing.
  - Cache workflow assembly per resource in file.
  - Codegen c++ from multiple workflow assembly.

## MISC

- Use collection interfaces on function signatures.
  - Only if `Vlpp` decides to add collection interfaces.

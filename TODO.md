# TODO

## Completed but not released

- `vl::Variant`
- `vl::Overloading`
- Removed `DEFINE_GUI_GRAPHICS_ELEMENT(TYPE, NAME)`
  - private: `friend class GuiElementBase<TYPE>;`
  - private: `static constexpr const wchar_t* ElementTypeName = NAME;`
- Removed `DEFINE_CACHED_RESOURCE_ALLOCATOR(TKEY, TVALUE)`
  - New base class: `GuiCachedResourceAllocatorBase<Self, TKEY, TVALUE>`
- Removed `DEFINE_GUI_GRAPHICS_RENDERER(TELEMENT, TRENDERER, TTARGET)`
  - New base class: `GuiElementRendererBase<TELEMENT, TRENDERER, TTARGET>`
  - private: `friend class GuiElementRendererBase<TELEMENT, TRENDERER, TTARGET>;`
- Renamed: `GuiGraphicsComposition::FindComposition` -> `FindVisibleComposition`

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
- For all list controls, adding item could cause flashing during rendering for about 1 flame.
  - If this issue is solved, remove document in `Breaking changes from 1.0` and `List Controls`

## Release Milestone (current: 1.2.9.0)

- A general remoting `INativeController` implementation.
  - Add hittest (only for main window)/cursor messages, or implement it with SyncDom.

## Release Milestone (1.2.10.0)

- Implement basic control (`Source_GacUI_CoreApplication` controls only) unit test based on streaming
  - `SolidLabelElement.
    - Each character takes exactly `FontSize x FontSize`
    - Deal with `\r` and `\n` when multiline is enabled
  - Implement `ImageFrameElement`.
  - Metadata from requests are needed from the beginning for codegen, metadata will be updated and included in release.
  - `INativeImageService`:
    - Save image metadata (width, height, type, etc) to binary resource
    - For `INativeImageService::CreateImage*` functions it sends binary data to the receiver and wait for respond of metadata.
    - Unit test only `<PsuedoImage/>` to specify only the size in resource.
- GacUI Binary Resource (can't move to next release)
  - Upgrade GacUI XML Resource to 1.3, force on all instead of only depended or depending resource.
  - Require binary pattern "[GMR-1.3]" at the beginning of the binary resource.

## Release Milestone (1.2.11.0)

- All control unit test (using DarkSkin)
  - Add window resizing constraint messages.
  - Implement `ColorizedTextElement` and `DocumentElement`.
    - Think about how to calculate size for document.
  - A viewer to view unit test results logged from SyncDom and other stuff after each time when layout stops.
  - In release repo add more tools that just call `GacUI.UnitTest.cpp`:
    - Load x86 bin + workflow script and execute.
    - Load x64 bin + workflow script and execute.
    - Render unit test results, especially each frame of intermediate rendering result.
      - Can navigate to workflow script.
- Move unit test utilities to `GacUI.UnitTest.cpp`, `GacUI.UnitTest.h`, `GacUI.UnitTest.Reflection ...`
- Document
  - Unit test framework.
  - Unit test framework in Vlpp.
  - SyncDom.
  - Remote protocol.

## Release Milestone (1.2.12.0)

- SyncObj architecture that streams ViewModel object changes.
  - See README.md in Workflow repo (**ViewModel Remoting C++ Codegen**).
  - GacGen offers Metadata of interfaces
  - Network protocols are not included as default implementation
- New tutorials
  - A GacUI D2D process connecting to a server process for streaming ViewModel
    - ViewModel implements in C++ and C#
- Document
  - SyncObj.
  - Fix `/doc/current/home.html`
    - Introduction to hosted / remote
  - Fix `/doc/current/gacui/home.html`
    - Introduction to hosted / remote

## Release Milestone (1.3.0.0)

- A remote protocol implementation on existing `INativeController` implementation.
  - Network protocols are not included as default implementation
- New tutorials
  - A GacUI SyncDom process connecting to a server process for streaming graphics
    - GDI+ implements in C#
    - GDI/D2D implements in C++
- Rewrite `GacBuild.ps1` and `GacClear.ps1` in C++, but still keep them just doing redirection for backward compatibility.
- Get rid of `Deploy.bat` in `GacGen.ps1` and `GacGen.exe`

## Release Milestone (future releases)

- `Variant` and `Union` with full support.
  - Document.
- Strict check in different for-each loops.
- More optimistic SyncDom strategy to reduce messages.
- Windows
  - Ensure `INativeWindow::(Before|After)Closing()` is not called on non-main-window between the main window is closed and the application exits.
- Enlarging window slower than shrinking.
- https://github.com/vczh-libraries/Vlpp/issues/9

## OS Provider Features

- Drag and Drop framework.
  - Substitutable.
  - `GuiFakeDragAndDropService`.
  - Substitute DragAndDrop by default optionally.
- Windows
  - `INativeImage::SaveToStream` handle correctly for git format. It is possible that LoadFromStream need to process diff between git raw frames.
- UI Automation.
- Test Automation.
  - Standard test for OS providers, may need a test purpose automation service to do https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput

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
- Upgraded Code editor (need VlppParser2)
- Dock container.
  - Use drag and drop to perform docking
  - A small framework for implementing dock guiding UI
  - A predefined dockable tool window for users that are fine with the default behavior about how to transfer content to the dock container
  - A "binary tree layout/control" for implementing VS-like dock container
- Touch support.

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

- Port GacUI to other platforms:
  - Unit Test (Hosted, SyncDom)
  - Windows
    - GDI (**Normal**, **Hosted**, SyncDom)
    - Direct2d (**Normal**, **Hosted**, SyncDom)
    - UWP (Hosted + SyncDom)
  - Linux
    - gGac repo: improve development process for release
  - macOS
    - iGac repo: improve development process for release
  - Web Assembly (Hosted + SyncDom)
    - Canvas?
    - DOM?
  - CLI (optional, needs dedicated skin)
    - Command-line/Powershell in Windows (Hosted, SyncDom)
    - Ncurses on Ubuntu (Hosted, SyncDom)
- Port GacUI to other languages:
  - Applications written in other language can:
    - Implement view model (SyncObj).
    - Render the UI (SyncDom).
  - Languages:
    - JavaScript / TypeScript through Web assembly
    - .NET
    - Python

## GacUI (unprioritized)

- DarkSkin Color Theme.
  - Create a `DarkSkinPalette` class with a static getter method to retrive default colors.
    - Update all `Style.xml` colors to use `DarkSkinPalette`.
  - Add a static setter to `DarkSkinPalette`.
    - A window can be called to update all its controls' and components' template.
    - The above function will be called inside the setter.
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
- Add `MoveToScreenCenterAfterLayout` as what is done in `FakeDialogServiceBase::ShowModalDialogAndDelete`.
- New default control templates with animation, written in XML generated C++ code.
- Use the embedded data codegen / compress / decompress functions from `VlppParser2` to replace one in `GacUI`.
- Use collection interfaces on function signatures.
  - Only if `Vlpp` decides to add collection interfaces.

## GacUI Resource Compiler (unplanned releases)

- Consider `-ani` binding, create an animation controller object that change the binded property, with predefined interpolation and other stuff.
  - All types that can do interpolation are value types, consider following formats:
    - "NAME:initial value"
    - "NAME(initial value in expression)"
    - Need to be consistent with animation object
  - Consider multiple `-ani` batch control, state configuration and transition, story board, connection to animation coroutine, etc.
- `<eval Eval="expression"/>` tags.
  - A facade is a class with following methods:
    - **AddChild**: Accept a child facade or a child object.
    - **ApplyTo**: Accept a parent object, which is not a facade.
    - **Initialize** (optional): Called on the instance object between construction and `<ref.Ctor>`.
  - A facade could have properties but only accept assignment or `-eval` binding.
  - A facade could have an optional **InstanceFacadeVerifier** executed on GacGen compile time.
  - Built-in Layout and Form facade.
- Facade
  - If `<XFacade>` or `<x:XFacade>` is an accessible and default constructible object, then `<X>` or `<x:X>` triggers a facade.

## GacUI Resource Compiler (unprioritized)

- In the final pass, only workflow scripts are printed.
  - Use WorkflowCompiler.exe to do codegen externally.
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

## New C++/Doc Compiler based on VlppParser2

## GacStudio

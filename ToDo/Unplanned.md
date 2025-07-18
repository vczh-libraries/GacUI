# TODO

## Ideas

- GacUI Binary Resource (when new features are added)
  - Upgrade GacUI XML Resource to 1.3, force on all resources instead of only depended or depending resource.
  - Require binary pattern "[GXR-1.3]" at the beginning of the binary resource.
  - Resource compiler and loader will check the version and only accept 1.3.
- Create another release folder, containing a new h/cpp grouping configuration.
  - Group files into how vcxitems described in GacUISrc.
  - Create each lib project for each grouped source files.
  - Create each exe project to reference just enough/designed subset of libs.
  - Verify if source dependencies are satisfied.
- Remote protocol player for Windows (and port to others)
  - GUI runs compiled XML resource in an separated process
  - Single GUI mode
  - Multiple GUI mode (with a simple window manager, and a test app for displaying instructions)
  - Implemented in both C++ and TypeScript

## Probably

- Strict check in different for-each loops.
- A new non-XML instance format
- Compact version of table's colummns and rows property so that they could also be written in attributes.
- `<eval Eval="expression"/>` tags.
- `<ez:Layout/>`
  - `xmlns:ez` by default:
    - `presentation::composition::easy_layout::GuiEasy*Composition`
    - `presentation::composition::easy_layout::GuiEasy*Layout`
  - A `vl::presentation::composition::easy_layout::GuiEasyLayoutComposition`
    - with properties:
      - `Top`, `Bottom`, `Left`, `Right`: boolean of border visibility
      - `Padding`: thickness of border and between all leaf containers
    - accepting following tags as child:
      - `<ez:Top/>`, `<ez:Bottom/>`, `<ez:Left/>`, `<ez:Right/>`, `<ez:Fill Percentage="1">`: Will be implemented by stack or table
      - `<ez:Row RowSpan="1" ColumnSpan="1"/>`, `<ez:Column RowSpan="1" ColumnSpan="1"/>`: will be implemented by table
      - Is it possible to make them attributes of children instead of tags?
    - properties of other `ez:` object
      - `Padding`, `-1` by default means inheriting the value from its parent, defining a new padding of its children
  - Any `ez:` layout could have multiple `ez:` layout or one control/Composition
    - Such `control` or `composition` will get `AlignmentToParent` changed in `BuildLayout` if all components are `-1` at the moment
    - A flag will store into such object, so the second call of `BuildLayout` will know the value is set by itself, and update it propertly
      - Such flag could be the `<ez:Layout/>` itself passed to calling `SetInternalProperty`
  - There is no constraint about the number and their order of `ez:` child object
    - All consecutive rows or columns will be grouped into a table
    - All direct child of rows of a rows, or columns of a column, are flattened
    - Use stack if possible
  - A `BuildLayout` method will be called after the layout tree is prepared. Changing the layout tree will not take effect without calling this method.

## OS Provider Features

- Windows
  - `INativeImage::SaveToStream` handle correctly for git format. It is possible that LoadFromStream need to process diff between git raw frames.
- UI Automation.
- Test Automation.
  - Standard test for OS providers, may need a test purpose automation service to do https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput

## Control Features

- `IColumnItemView`.
  - Change column size from `int` to `{minSize:int, columnOption:Absolute|Percentage, absolute:int, percentage:float}`
  - Column drag and drop.
    - ListView: only raises an event, developers need to update column headers and data by themselves.
    - DataGrid: swap column object, cells are changed due to binding.
- ListView.
  - Make a common template base class for `IItemProvider` implementation of bindable and non-bindable pair of the same virtual control:
    - The template argument would be the base class which implements the differences.
    - `list::TextItemProvider` vs `GuiBindableTextList::ItemSource`.
    - `list::ListViewItemProvider` vs `GuiBindableListView::ItemSource`.
      - Extends to `list::DataProvider`.
    - `tree::TreeViewItemRootProvider` vs `GuiBindableTreeView::ItemSource`.
  - `GuiBindableDataGrid`:
    - Add customizable row visualizer.
    - The default (or `nullptr`) row visualizer displays cell visualizer and editor.
  - `GuiBindableTreeDataGrid`:
    - Offer a default group header row visualizer when users only need one level of collapsable `GuiBindableDataGrid`.
    - Replace the new `list::DataProvider`'s base class with `tree::NodeItemProvider` offering `GuiBindableTreeView::ItemSource` to make a useful data source.
- Upgraded Code editor (need VlppParser2)
- Touch support.

## Drag and Drop

- Drag and Drop framework.
  - Substitutable.
  - `GuiFakeDragAndDropService`.
    - Activated by substitution when an OS dependent implementation is not available.
- Dock container.
  - Use drag and drop to perform docking.
  - A small framework for implementing dock guiding UI.
  - A predefined dockable tool window for users that are fine with the default behavior about how to transfer content to the dock container.
  - A "binary tree layout/control" for implementing VS-like dock container.

## Graphics

- 2D drawing API, optional at runtime.
  - Default non-text element renderer using 2D drawing API.
    - Activated only when 2D drawing API is available and renderer implementations are unavailable.
  - Ensure OS providers without 2D drawing API still work.
- restriction-based Meta2DElement.
  - If 2D drawing API is not available, display a text using `SolidLabel`.
- Meta3DElement and Meta3D data structure.
  - Default Meta3DElement renderer using Meta2DElement with a surface sorting based algorithm.
- 2D Chart control based on Meta2DElement.
- 3D Chart control based on Meta3DElement.
- GIF player.
- video player.

## Porting to New Platforms

- Port GacUI to other platforms:
  - Unit Test (**Remote**)
  - Windows
    - GDI (**Normal**, **Hosted**, **Remote**)
    - Direct2d (**Normal**, **Hosted**, **Remote**)
    - UWP (Remote)
  - Linux
    - gGac repo: improve development process for release
  - macOS
    - iGac repo: improve development process for release
  - Browser
    - HTTP for test purpose (Remote)
    - WebAssembly (Remote)
  - CLI (optional, needs dedicated skin)
    - Command-line/Powershell in Windows (Remote)
    - Ncurses on Ubuntu (Remote)
- Port GacUI to other languages:
  - Applications written in other language can:
    - Implement view model (SyncObj).
    - Render the UI (Remote).
  - Languages:
    - JavaScript / TypeScript
      - HTTP for test purpose
      - WebAssembly
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
- Facade
  - A facade is a class with following methods:
    - **AddChild**: Accept a child facade or a child object.
    - **ApplyTo**: Accept a parent object, which is not a facade.
    - **Initialize** (optional): Called on the instance object between construction and `<ref.Ctor>`.
  - A facade could have properties but only accept assignment or `-eval` binding.
  - A facade could have an optional **InstanceFacadeVerifier** executed on GacGen compile time.
  - Built-in Layout and Form facade.
  - If `<XFacade>` or `<x:XFacade>` is an accessible and default constructible object, then `<X>` or `<x:X>` triggers a facade.

## New C++/Doc Compiler based on VlppParser2

## GacStudio

- Run the editing GUI using remote protocol.
- Run the symbol server in a separate process.
  - e.g. for rendering if all properties in a binding expression is observable, providing fast editing tool.
- Dedicated solution-project(dependency,external dependency)-item file format.
  - Solution builds GacUI.xml
  - Project builds GacUI XML Resource index file.
  - Each file contains only one item.
- When create a UI object in a wizard, ask for:
  - Localization item.
    - Need to mark properties in reflection about if it needs localization.
  - ViewModel item.
  - ViewModel implementation item for testing.
- Dedicated composition/control property sheet configuration for each class.

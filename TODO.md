# TODO

## Completed but not released (1.2.11.0)

- Fixed:
  - `#include <arm_acle.h>` is needed for `__yield` in `VlppOS.cpp`
- Added:
  - `IGuiRemoteProtocolChannel<TPackage>` and `IGuiRemoteProtocolChannelReceiver<TPackage>`
  - `GuiRemoteProtocolFromJsonChannel` creates `IGuiRemoteProtocol` from `IGuiRemoteProtocolChannel<Ptr<JsonNode>>`
  - `GuiRemoteJsonChannelFromProtocol` creates `IGuiRemoteProtocolChannel<Ptr<JsonNode>>` from `IGuiRemoteProtocol`
  - `SetupRawWindowsDirect2DRenderer`
  - `SetupRawWindowsGDIRenderer`

## Known Issues

- Replace `LoadLibrary` with `GetModuleHandle` in `EnableCrossKernelCrashing`.
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
  - If this issue is solved, remove document in `Breaking changes from 1.0` and `List Controls`.
- `controller(Unr|R)elatedPlugins` in `IGuiPlugin(Manager)?` lower dependency safety.
  - Change `GUI_PLUGIN_NAME` to `GUI_PLUGIN_CONTROLLER_(UN)RELATED`.
  - Remove the two parameters from `IGuiPlugin`, the macro above already specified it clear enough.
  - Unrelated plugins are not allowed to depend on related plugins.
- TODO in `GuiRemoteProtocolAsyncChannelSerializer<TPackage>::ChannelThreadProc`.
- `ProceduredThread` and `LambdaThread` cause small memory leak.
  - `delete this;` eventually calls `SuspendThread` on the thread itself, making all following clean up code skipped.
  - Windows confirmed, Linux need to test.
- TODO in `GuiRemoteWindow::OnControllerConnect`.

## Known Issues (Unit Test)

- Tests that assuming DarkSkin:
  - `(H|V)(Tracker|Scroll)/Mouse`
  - `ToolstripSplitButton`
  - `GuiDatePicker/Mouse`
- Tests that are OS awared:
  - `GuiDatePicker`, `GuiDateComboBox`, inject unit test specific time and locale object. Otherwise Window and Linux will see different printed date in snapshots.
- Same issue
  - `(H|V)(Tracker|Scroll)/Mouse`
    - when `Drag to Center` the handler should be highlighted, because the mouse is right above the handler.
  - `GuiBindableDataGrid/ComboEditor`
    - When a data grid cell editor is created under the mouse, the editor does not receive `MouseEvent` event.
- `GuiScrollContainer`
  - Only calling `Set(Horizontal|Vertical)AlwaysVisible(false)` doesn't make scrolls disappear. `SetVisible(false)` on scrolls are verified called.
  - When the content is changed, configuration needs 2 idle frames to be correctly configured.
- `GuiListItemTemplate/ArrangerAndAxis(WithScrolls)`
  - items are not aligned to proper corner when scrolls are invisible.
- Stop hard-coding coordinates:
  - `GetListItemLocation`.
  - `GuiToolstripMenuBar/Cascade/ClickSubMenu`'s `Hover on File/New` try to avoid specifying relative coordinate just because half of the menu item is covered.

## Release (optional)

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

## Release Milestone (1.2.11.0)

- GacUI
  - Fix `Global Objects` in `GacUI.h`.
  - For diff-disabled (default) unit tests, it calls diff function explicitly to log both rendering/diff commands.
    - Call `UpdateDomInplace` to verify results.
  - Mouse wheel trigger functions.
  - Unit Test Snapshot Viewer
    - Show dom nodes in a tree view in the right side of the rendering tab optionally.
    - Select dom node and jump to other tabs with highlight.
- Remote protocol
  - Investigate about generating data structure in flat memory layout, so that no effort needed for binary serialization 
- Remote protocol channel
  - [x] `IGuiRemoteProtocolChannel<TPackage>` and `IGuiRemoteProtocolChannelReceiver<TPackage>`.
  - Json channel
    - [x] `GuiRemoteProtocolFromJsonChannel` creates `IGuiRemoteProtocol` from `IGuiRemoteProtocolChannel<Ptr<JsonNode>>`.
    - [x] `GuiRemoteJsonChannelFromProtocol` creates `IGuiRemoteProtocolChannel<Ptr<JsonNode>>` from `IGuiRemoteProtocol`.
    - [x] Unit test
  - [ ] Binary channel
  - [x] Json to `ObjectString<T>` channel conversion
  - [x] Sync to Async channel conversion
    - [ ] Test sending disconnect or other events when `Submit()` is pending on the event.
      - Support failure injection in unit test, making `UnitTestRemoteProtocol::Submit()` returns `true` to its `disconnected` parameter.
    - [ ] `Start` -> `StartWithUIThread` and `StartWithUIAndChannelThread`.
      - Move all UI and channel thread resources to two structs for easy refactoring.
      - The `StartWithUIThread` is useful when channel communication does not require to be done in a single thread, everything will be called from the UI thread and the receiver could be called from any non-UI thread.
- Remote protocol redirection back to native rendering:
  - `RemotingTest_Core`
  - `RemotingTest_Rendering_Win32`
  - [x] [NamedPipe](https://learn.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o)
    - Disallow disconnect and reconnect.
  - [ ] [HttpServer](https://learn.microsoft.com/en-us/windows/win32/http/using-http-server-api)
    - Allow only one client by verifying a random guid in the message which is given from the registration request.
    - Allow disconnect and reconnect.
    - When TypeScript rendering is completed, the demo in `Release` repo would become a website server.
  - [ ] [HttpClient](https://learn.microsoft.com/en-us/windows/win32/winhttp/winhttp-start-page)
  - [ ] Use string first, and then change to binary.
  - Implement the remote protocol on a native `INativeController` instance.
    - It could not be used on `GuiHostedController` or `GuiRemoteController`, which is not a native implementation.
    - [ ] Crash when close.
    - [ ] Moving across different dpi monitors doesn't change window size
      - [ ] Ensure DPI is properly handled in `Core`
    - [ ] IO not handled
    - [ ] QueryClose not handled
    - [ ] MinSize not handled when resizing the main window
      - Or implement it in `1.2.13.0`
    - [ ] Measuring information sent from Rendering to Core even when there is no requirement or size changing
  - The experiment will only run a very simple UI that covers all implemented remote protocol so far.
- `UpdateDomInplace` performed binary search, create a common implementation to share with `SortedList`.
- `GuiRemoteProtocolFromJsonChannel::OnReceive` should use a map instead of a series of if-statments.
- `GuiRemoteJsonChannelFromProtocol::Write` should use a map instead of a series of if-statments.
- `vl::presentation::remoteprotocol::channeling::ConvertUtfString` move into `Vlpp` repo.
- Document
  - `SetupRawWindowsDirect2DRenderer`
  - `SetupRawWindowsGDIRenderer`

## Release Milestone (1.2.12.0)

- More unit test
  - Ribbon Controls
  - `GuiControl` and servives
  - `[TAB]`
  - `ControlThemeName` property
  - Multiple active `GuiWindow`, modal, order, `Enabled`
  - `GuiTab` item manipulation and `[TAB]`
  - `MouseWheel`
    - Behavior on scrolls, trackers, scroll containers and list controls
    - `GuiListControl` item events
    - `GuiVirtualTreeListControl` node events
  - A button calling a modal window
- GacUI
  - `<RawRendering/>` element.
    - It will be mapped to `GDIElement` or `Direct2DElement` in different renderers.
    - In remote protocol, it is an element with no extra properties.
    - In HTML, it would open a `<div/>` and you can do whatever you want using JavaScript.
- JavaScript rendering:
  - Delete all `GacJS` code. This repo will be used to implement the HTML logic.
  - A codegen for remote protocol and print TypeScript code.
  - In the test project, C++ side will start a HTTP service on Windows (`RemotingTest_Core`).
  - JavaScript side will separate the rendering and the network protocol.
  - Try DOM.
  - Try Canvas.
    - https://github.com/WICG/canvas-formatted-text/blob/main/README.md
      - layout provider could not be done until this is implemented.
  - Try EsBuild to replace WebPack.
  - The experiment will only run a very simple UI that covers all implemented remote protocol so far.
- Copy control unit tests, snapshots and snapshot viewer to `Release` repo.
  - Build and run test.
    - Explain this in decicated `README.md` and mention it in the root one.
  - Verify vcxproj contains all files.
  - Ensure build woriflow tutorials.
  - Update Win11 menu tutorial in `Release` repo to use new virtual control

## Release Milestone (1.2.13.0)

- All control unit test (using DarkSkin)
  - Tooltip.
  - Dialogs.
  - A mechanism to replace `vl::filesystem` implementation.
  - Add window resizing constraint messages.
  - Implement `ColorizedTextElement`
    - Typing trigger functions in unit test framework.
    - `GuiSinglelineTextBox`
    - `GuiMultilineTextBox`
    - `GuiBindableDataGrid` with predefined text box editor.
    - Ribbon with predefined text box toolstrip component.
  - Implement `DocumentElement`.
    - Think about how to calculate size for document.
    - `GuiDocumentViewer`
    - `GuiDocumentLable`
      - `<DocumentTextBox/>`
  - Check all control key operation, ensure accessibility.
    - Cannot scroll `GuiScrollView`, key operations needed and also need to be disabled for `GuiListControl` because it has already defined item selection keys.
    - `Gui(Bindable)TextList` cannot check or uncheck items by key (enter and space).
    - `GuiVirtualListView` cannot operate column headers by key.
    - Active menus without `Alt` assighed (`TAB` and arrow keys?).
    - `GuiDateComboBox` does not receive focus property, so that when open year/month combo by `ALT` or mouse, list item cannot be selected only by key.
      - Add `Alt` and fix `Mouse` test cases after this is fixed.
- Sample unit test project included in release.
  - Reflection enabled
    - Application and `LoadMainWindow` script in separated XML resource files.
    - Application and `LoadMainWindow` script in separated compiled resource files.
  - Reflection disabled
    - Application in compiled C++ files.
    - There is no `LoadMainWindow` in this case.
- Document
  - Unit test framework.
  - Unit test framework in Vlpp.
  - Remote Protocol.
  - Starting GacUI Core with Remote Protocol.
  - Starting GacUI Rendering with `GuiRemoteRendererSingle`.

## Release Milestone (1.3.0.0)

- Implement `ColorizedTextElement` and `DocumentElement` in all already implemented remote renderers.

## Release Milestone (1.4.0.0)

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

## Release Milestone (1.4.1.0)

- A remote protocol implementation on existing `INativeController` implementation.
  - Network protocols are not included as default implementation.
  - Take care of `wchar_t` when server and client have different understanding to its size.
- New tutorials.
  - A GacUI SyncDom process connecting to a server process for streaming graphics.
    - GDI+ implements in C#.
    - GDI/D2D implements in C++.
- Rewrite `GacBuild.ps1` and `GacClear.ps1` in C++, but still keep them just doing redirection for backward compatibility.
- Get rid of `Deploy.bat` in `GacGen.ps1` and `GacGen.exe`.

## Release Milestone (1.5.0.0)
- `Variant` and `Union` with full support.
  - Document.
  - Document `vl::Overloading`.
- Extensible CLI argument parser acception different OS convention, serialization and module dependencies.
  - Structured error report.
  - Extensible error message localization relying on GacUI XML Resource localization feature.
  - Pre-made main function per OS, defining arguments for different renderers for the current OS, including remoting server with predefine protocols.
- More optimistic SyncDom strategy to reduce messages.
- Windows
  - Ensure `INativeWindow::(Before|After)Closing()` is not called on non-main-window between the main window is closed and the application exits.
- Enlarging window slower than shrinking.
- https://github.com/vczh-libraries/Vlpp/issues/9

## Release Milestone (future releases)

- Strict check in different for-each loops.
- A new non-XML instance format
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
    - GDI (**Normal**, **Hosted**, Remote)
    - Direct2d (**Normal**, **Hosted**, Remote)
    - UWP (Remote)
  - Linux
    - gGac repo: improve development process for release
  - macOS
    - iGac repo: improve development process for release
  - Browser (Remote)
    - HTTP for test purpose
    - WebAssembly
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

# Known Issues

## ToDo

- Replace `LoadLibrary` with `GetModuleHandle` in `EnableCrossKernelCrashing`.
- `GuiVirtualRepeatCompositionBase`.
  - Eliminate double `ForceCalculateSizeImmediately()` calls in `TestCompositions_VirtualRepeat.cpp` (TODO) and related files.
- TODO in `GuiRemoteProtocolAsyncChannelSerializer<TPackage>::ChannelThreadProc`.
- TODO in `GuiRemoteWindow::OnControllerConnect`.
- Bindable list control (optional)
  - When a property referenced by `<att.XXXProperty>` is updated, the list item is not refreshed.
    - Need to find a way to listen to the event.
- `controller(Unr|R)elatedPlugins` in `IGuiPlugin(Manager)?` lower dependency safety.
  - Change `GUI_PLUGIN_NAME` to `GUI_PLUGIN_CONTROLLER_(UN)RELATED`.
  - Remove the two parameters from `IGuiPlugin`, the macro above already specified it clear enough.
  - Unrelated plugins are not allowed to depend on related plugins.

## Known Issues

- FakeDialogService
  - `FakeDialogServiceBase::ShowModalDialogAndDelete` place the window in the center of `owner` instead of the screen.
  - Specify multiple extensions in one filter, exactly like Win32 API.
  - Extensions not applied before checking file existance.
- Expanding collapsing tree nodes cause the whole list to rebuild, which glitch during multiple passes of layout.
  - Only affected items need to rebuild.
- For all list controls, adding item could cause flashing during rendering for about 1 flame.
  - If this issue is solved, remove document in `Breaking changes from 1.0` and `List Controls`.
- `ProceduredThread` and `LambdaThread` cause small memory leak.
  - `delete this;` eventually calls `SuspendThread` on the thread itself, making all following clean up code skipped.
  - Windows confirmed, Linux need to test.
- `GuiDocumentViewer`
  - Loading super large text into one single paragraph (~0.2M lines) too slow
      - Workaround: Change paragraph mode to multiline.
    - Demo: `https://github.com/vczh-libraries/GacUI/blob/master/Test/Resources/UnitTestSnapshots/Controls/Ribbon/GuiRibbonButtons/Dropdowns.json`
    - **Paragraph mode** plus **doubleLineBreaksBetweenParagraph==true** plus **no empty line** causing the whole text loaded into one single `IGuiGraphicsParagraph`.
    - This is the default configuration for `GuiDocumentViewer` and `GuiDocumentLabel`, it could be changed with the `Behavior` property in XML.
    - Root cause in `IDWriteTextLayout::GetMetrics`, taking most of the time.
  - When loading super big content to the control while you don't need user to undo to the previous state:
    - To load such text into `GuiMultilineTextBox` uses `LoadTextAndClearUndoRedo`.
    - To load prepared large document into `GuiDocumentViewer` uses `LoadDocumentAndClearUndoRedo`.
    - Future editing still work with undo.

## Remote Protocol

- `GuiRemoteGraphicsRenderTarget::fontHeights` could be moved to `GuiRemoteGraphicsResourceManager` as the measuring should not be different.
- `RemotingTest_Rendering_Win32`
  - Clicking `Fatal Error` in `RemotingTest_Rendering_Win32 /Pipe` sometimes hang.

## Unit Test

- Tests that assuming DarkSkin:
  - `(H|V)(Tracker|Scroll)/Mouse`
  - `ToolstripSplitButton`
  - `GuiDatePicker/Mouse`
  - `GuiRibbonGallery` and `GuiBindableRibbonGalleryList` Up/Down/Dropdown button.
  - `Application/Windows` window frame operation.
- Stop hard-coding coordinates:
  - `GetListItemLocation`.
  - `GuiToolstripMenuBar/Cascade/ClickSubMenu`'s `Hover on File/New` try to avoid specifying relative coordinate just because half of the menu item is covered.
  - Tests that affected by text measuring:
    - `TestApplication_Window.cpp`: `Dragging`, `ShowModal` (locating window title)
    - `TestControls_Basic_GuiTab.cpp`: `Natigation` (locating tab page header)
    - `TestControls_CoreApplication_GuiLabel.cpp`: `GuiLabel`.
      - Asserted a label size.
    - `TestControls_List_GuiVirtualListView.cpp`
      - Shrinks of text size causing 100 items to be displayed without a scroll bar in List mode. Making it more causes a stack overflow while disposing Json objects. Fixed to have more text in each item.
    - `TestControls_Ribbon_GuiRibbonGroup.cpp`
      - Need size to make ribbon groups collapsed.
- Compositions created or moved under the cursor, proper mouse events are not triggered.
  - `(H|V)(Tracker|Scroll)/Mouse`
    - when `Drag to Center` the handler should be highlighted, because the mouse is right above the handler.
  - `GuiBindableDataGrid/ComboEditor`
    - When a data grid cell editor is created under the mouse, the editor does not receive `MouseEvent` event.
- `Application/FocusedAndHide`
  - The hidden button does not lost focus.
- `GuiScrollContainer`
  - Only calling `Set(Horizontal|Vertical)AlwaysVisible(false)` doesn't make scrolls disappear. `SetVisible(false)` on scrolls are verified called.
  - When the content is changed, configuration needs 2 idle frames to be correctly configured.
- `GuiListItemTemplate/ArrangerAndAxis(WithScrolls)`
  - items are not aligned to proper corner when scrolls are invisible.
- Features with no plan in unit test:
  - `GuiControl`
    - `QueryService` and `AddService`
- Accessibility (has workaround)
  - Cannot scroll `GuiScrollView`, key operations needed and also need to be disabled for `GuiListControl` because it has already defined item selection keys.
  - Active top level menus (in a menu bar) without `Alt` property assigned.
    - Investigate about menu bar accepting tab actions. It appears on Windows tab menu buttons could have focus.
    - When a menu bar is activated by `[TAB]`, keep pressing `[TAB]` will go through all menu buttons and pass it to the next control after ending.
    - When a menu is activated by `[TAB]`, keep pressing `[TAB]` will go through all menu buttons and repeat, until `[ESC]`.
    - `[SPACE]` or `[ENTER]` selects a highlighted menu button when tabbing.
  - `GuiVirtualListView` cannot operate column headers by key.
- `GuiRemoteController` Reconnecting
  - Test sending disconnect or other events when `Submit()` is pending on the event.
    - Especially during the first connection, it is observed in GacJS that when it is the first renderer and failed during connection ballbacks, Core can't respond to other renderers again.
  - Support failure injection in unit test, making `UnitTestRemoteProtocol::Submit()` returns `true` to its `disconnected` parameter.

## Depriorized

- FakeDialogService
  - message box disable `X` button if `Cancel` is not in the button list or `OK` is the only button.
- GDI
  - In hosted mode, non-main window doesn't shrink when moving back to low DPI monitor.
- IME re-composing:
  - When Japanese IME is active, pressing SPACE on a select text gives me another chance to re-compose it. Figure out how it work and implement it.

# Known Issues

## Core

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
- Remote Protocol
  - `GuiRemoteGraphicsRenderTarget::fontHeights` could be moved to `GuiRemoteGraphicsResourceManager` as the measuring should not be different.

## Unit Test

- Tests that assuming DarkSkin:
  - `(H|V)(Tracker|Scroll)/Mouse`
  - `ToolstripSplitButton`
  - `GuiDatePicker/Mouse`
  - `GuiRibbonGallery` and `GuiBindableRibbonGalleryList` Up/Down/Dropdown button. 
- Stop hard-coding coordinates:
  - `GetListItemLocation`.
  - `GuiToolstripMenuBar/Cascade/ClickSubMenu`'s `Hover on File/New` try to avoid specifying relative coordinate just because half of the menu item is covered.
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

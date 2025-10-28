# !!!KNOWLEDGE BASE!!!

# DOCUMENT REQUEST

## TOPIC

GuiListControl has a complex design to enable customized item rendering, layouting and management. You need to research on these 3 topics.

In GuiListControl.h's leading comment block there is explanatations about how each sub controls manage its own sub items. It would help you understanding the code structure better.

### Insight

#### Architecture Overview

`GuiListControl` implements a sophisticated three-layer architecture enabling customized item rendering, layouting, and management:

**Layer 1: Data Layer (IItemProvider)**
- Provides data to the list control
- Located in `Source/Controls/ListControlPackage/DataSource_IItemProvider.h`
- Core interface methods:
  - `Count()`: Returns total number of items
  - `GetTextValue(itemIndex)`: Returns text representation of an item
  - `GetBindingValue(itemIndex)`: Returns boxed `description::Value` of an item
  - `RequestView(identifier)`: Returns additional view interfaces for specialized data access
  - `AttachCallback(IItemProviderCallback*)` and `DetachCallback(IItemProviderCallback*)`: Register/unregister for notifications
- `IItemProviderCallback` receives notifications via `OnItemModified(start, count, newCount, itemReferenceUpdated)`
- Base implementation `ItemProviderBase` manages callbacks and editing counter

**Layer 2: Layout Layer (IItemArranger)**
- Decides how to arrange item controls spatially
- Located in `Source/Controls/ListControlPackage/GuiListControls.h` as nested interface
- Core interface methods:
  - `AttachListControl(GuiListControl*)` and `DetachListControl()`: Lifecycle management
  - `SetCallback(IItemArrangerCallback*)`: Receives callback for creating/destroying item controls
  - `GetTotalSize()`: Returns total size of all items
  - `GetVisibleStyle(itemIndex)`: Returns item template for visible items, null if not visible
  - `GetVisibleIndex(ItemStyle*)`: Returns item index from template, -1 if not visible
  - `ReloadVisibleStyles()`: Recreates all visible item controls
  - `OnViewChanged(Rect bounds)`: Called when viewport changes
  - `FindItemByVirtualKeyDirection(itemIndex, key)`: Navigation support
  - `EnsureItemVisible(itemIndex)`: Scrolling support
  - `GetAdoptedSize(expectedSize)`: Size calculation for fitting items
- Implements `IItemProviderCallback` to respond to data changes
- Base implementation `RangedItemArrangerBase` delegates to `GuiVirtualRepeatCompositionBase`

**Layer 3: Rendering Layer (ItemStyle/ItemTemplate)**
- Item templates define visual appearance of each item
- Base template is `GuiListItemTemplate` in `Source/Controls/Templates/GuiControlTemplates.h`
- Properties include `Selected`, `Index`, `AssociatedListControl`
- Derived templates add specialized properties (e.g., `GuiTextListItemTemplate` adds `Checked`, `TextColor`)
- Item template is created via `TemplateProperty<GuiListItemTemplate>` function pointer
- Each item can be wrapped in background button if `displayItemBackground` is true

#### Item Rendering Management

**Item Creation and Lifecycle**

`GuiListControl::ItemCallback` implements both `IItemProviderCallback` and `IItemArrangerCallback`:

**Creation Flow (InstallStyle)**:
1. Arranger calls `callback->CreateItem(itemIndex)`
2. `ItemCallback::CreateItem` validates index and calls `itemStyleProperty` function to create template
3. Template returned from the function becomes the `ItemStyle`
4. If `displayItemBackground` is true:
   - Creates `GuiSelectableButton` with `ThemeName::ListItemBackground`
   - Wraps item template inside background button's container
   - Synchronizes `Selected` state between template and button
   - Returns wrapper as `ItemStyleBounds` instead of template itself
5. Otherwise template itself is `ItemStyleBounds`
6. Calls `OnStyleInstalled(itemIndex, style, false)` to initialize properties:
   - `SetFont(GetDisplayFont())`
   - `SetContext(GetContext())`
   - `SetText(itemProvider->GetTextValue(itemIndex))`
   - `SetVisuallyEnabled(GetVisuallyEnabled())`
   - `SetSelected(false)`
   - `SetIndex(itemIndex)`
   - `SetAssociatedListControl(this)`
7. Calls `AttachItemEvents(style)` to hook up mouse events
8. Stores mapping from `ItemStyle*` to `ItemStyleBounds*` in `installedStyles` dictionary

**Destruction Flow (UninstallStyle)**:
1. Arranger calls `callback->ReleaseItem(style)`
2. `ItemCallback::ReleaseItem` looks up style in `installedStyles`
3. Calls `OnStyleUninstalled(style)` which calls `DetachItemEvents(style)`
4. Removes from `installedStyles` dictionary
5. Calls `SafeDeleteComposition(bounds)` to destroy compositions

**Event Handling**:
- Each visible style has attached mouse event handlers stored in `VisibleStyleHelper`
- Events captured: left/middle/right button down/up/doubleclick, mouse move/enter/leave
- Handler translates composition-level events to item-level events:
  - Gets `itemIndex` from arranger via `GetVisibleIndex(style)`
  - Creates `GuiItemMouseEventArgs` or `GuiItemEventArgs` with `itemIndex`
  - Fires corresponding item event on `GuiListControl` (e.g., `ItemLeftButtonDown`)

**Property Updates**:
- When item provider notifies modification via `OnItemModified(start, count, newCount, itemReferenceUpdated)`
- If `itemReferenceUpdated == false` and `count == newCount` (content update only):
  - For each visible item in range, calls `OnStyleInstalled(itemIndex, style, true)` to refresh properties
- Otherwise arranger handles recreation

#### Item Layout Management

**Virtual Repeat Composition System**

All predefined arrangers use `RangedItemArrangerBase` which delegates to `GuiVirtualRepeatCompositionBase`:

**Arranger Types** (in `Source/Controls/ListControlPackage/GuiListControlItemArrangers.h`):
1. `FreeHeightItemArranger`: Uses `GuiRepeatFreeHeightItemComposition`
   - Caches heights of all items individually
   - Suitable for items with varying heights
2. `FixedHeightItemArranger`: Uses `GuiRepeatFixedHeightItemComposition`
   - All items have same height (maximum of minimum heights)
   - Most efficient for uniform lists
3. `FixedSizeMultiColumnItemArranger`: Uses `GuiRepeatFixedSizeMultiColumnItemComposition`
   - Multiple columns with fixed item size (max width and height)
   - Grid layout
4. `FixedHeightMultiColumnItemArranger`: Uses `GuiRepeatFixedHeightMultiColumnItemComposition`
   - Multiple columns with fixed height but variable width per item
   - Suitable for tag clouds or varying-width items

**Virtual Repeat Template Pattern**:

`VirtualRepeatRangedItemArrangerBase<TVirtualRepeatComposition>` template creates `ArrangerRepeatComposition`:
- Overrides `CreateStyleInternal(index)`:
  - Calls `arranger->callback->CreateItem(index)` to get `ItemStyle`
  - Calls `arranger->callback->GetItemBounds(itemStyle)` to get wrapping composition
  - Returns bounds composition to repeat composition
- Overrides `DeleteStyleInternal(style)`:
  - Calls `arranger->callback->GetItem(style)` to unwrap to `ItemStyle`
  - Calls `arranger->callback->ReleaseItem(itemStyle)` to destroy
- Overrides `Layout_UpdateIndex(style, index)`:
  - Updates `itemStyle->SetIndex(index)` when items scroll

**Coordination with Repeat Composition**:

`RangedItemArrangerBase` bridges list control and repeat composition:
- `OnAttached(IItemProvider*)`: Creates `ArrangerItemSource` wrapper and calls `repeat->SetItemSource(itemSource)`
- `OnItemModified(start, count, newCount, itemReferenceUpdated)`: If `itemReferenceUpdated` is true, calls `itemSource->ItemChanged(start, count, newCount)` to notify repeat composition
- `AttachListControl(GuiListControl*)`: Calls `repeat->SetAxis(listControl->GetAxis())` to share coordinate transformer
- `SetCallback(IItemArrangerCallback*)`: Adds repeat composition to container and sets dummy item template to enable virtual mode
- `OnViewChanged(Rect bounds)`: Calls `repeat->SetViewLocation(bounds.LeftTop())` and `repeat->SetExpectedBounds(Rect({0,0}, bounds.GetSize()))`
- Event forwarding: `ViewLocationChanged`, `TotalSizeChanged`, `AdoptedSizeInvalidated` from repeat to callback

**IGuiAxis Integration**:
- `IGuiAxis` transforms between real coordinates and virtual coordinates
- Shared by list control and repeat composition via `repeat->SetAxis(listControl->GetAxis())`
- Used for key navigation direction mapping
- Default is `GuiDefaultAxis` (identity transformation)

#### Item Data Management

**IItemProvider View System**

Providers expose multiple view interfaces via `RequestView(identifier)`:

**Common Views**:

1. **ITextItemView** (for `GuiVirtualTextList`):
   - `GetChecked(itemIndex)` and `SetChecked(itemIndex, value)`: Checkbox state
   - Used by `TextItemProvider` for `TextItem` data
   
2. **IListViewItemView** (for `GuiVirtualListView`):
   - `GetSmallImage(itemIndex)`, `GetLargeImage(itemIndex)`: Icons
   - `GetText(itemIndex)`: Main text
   - `GetSubItem(itemIndex, subItemIndex)`: Sub-item text
   - `GetDataColumnCount()`, `GetDataColumn(index)`: Data projection
   - `GetColumnCount()`, `GetColumnText(index)`: Column headers
   - Used by `ListViewItemProvider` for `ListViewItem` data

3. **IColumnItemView** (for `GuiVirtualListView` columns):
   - `GetColumnSize(index)`, `SetColumnSize(index, value)`: Column widths
   - `GetDropdownPopup(index)`: Context menu for column
   - `GetSortingState(index)`: Visual sort indicator
   
4. **INodeItemView** (for `GuiVirtualTreeListControl`):
   - `RequestNode(index)`: Convert item index to `INodeProvider*`
   - `CalculateNodeVisibilityIndex(node)`: Convert node to index (-1 if collapsed)
   - Used by `NodeItemProvider` to flatten tree into list
   
5. **ITreeViewItemView** (for tree nodes):
   - `GetNodeImage(node)`: Node icon
   - Used by `TreeViewItemRootProvider` for `TreeViewItem` data

6. **IDataGridView** (for `GuiVirtualDataGrid`):
   - `GetBindingCellValue(row, column)`, `SetBindingCellValue(row, column, value)`: Cell data
   - `GetCellSpan(row, column)`: Merging (not yet implemented)

**Provider Hierarchy**:

**Concrete Providers** (hold actual data):
- `TextItemProvider`: List of `Ptr<TextItem>`
- `ListViewItemProvider`: List of `Ptr<ListViewItem>` with columns
- `TreeViewItemRootProvider`: Tree of `Ptr<MemoryNodeProvider>` with `TreeViewItem` data
- `DataProvider` (for data grid): Holds filtered/sorted view

**Bindable Providers** (wrap observable data sources):
- `TextItemBindableProvider`: Wraps `IValueObservableList` as text items
- `ListViewItemBindableProvider`: Wraps `IValueObservableList` as list view items
- `TreeViewItemBindableRootProvider`: Wraps object graph as tree
- Uses reflection (`description::Value`) to access properties

**Converter Providers**:
- `NodeItemProvider`: Converts `INodeRootProvider` (tree) to `IItemProvider` (flat list)
- Implements `INodeItemView` for node access
- Tracks expanded/collapsed state
- Updates indices when tree structure changes

**Base Class Hierarchy**:
- `ItemProviderBase`: Manages callbacks and editing counter
- `ListProvider<T>`: Extends `ItemProviderBase` and `ObservableListBase<T>` for observable list functionality
- `NodeRootProviderBase`: Base for tree providers

**Callback Flow**:

When data changes:
1. Provider calls `InvokeOnItemModified(start, count, newCount, itemReferenceUpdated)`
2. `ItemProviderBase::InvokeOnItemModified` iterates through all `callbacks` (usually contains arranger and list control's `ItemCallback`)
3. Each callback's `OnItemModified` is invoked
4. Arranger's `OnItemModified`: Notifies repeat composition if `itemReferenceUpdated`
5. List control's `ItemCallback::OnItemModified`: Forwards to `GuiListControl::OnItemModified`
6. `GuiListControl::OnItemModified`: Refreshes visible item properties if content changed

#### Selection Management

**GuiSelectableListControl** extends `GuiListControl`:

**Selection State**:
- `selectedItems`: `SortedList<vint>` of selected indices
- `multiSelect`: Boolean enabling multi-selection
- `selectedItemIndexStart` and `selectedItemIndexEnd`: Range for shift-selection

**Selection Methods**:
- `SetSelected(itemIndex, value)`: Set single item selection
  - Updates `selectedItems`
  - Calls `OnItemSelectionChanged(itemIndex, value)` which updates visible style's `Selected` property
  - Fires `SelectionChanged` event
- `SelectItemsByClick(itemIndex, ctrl, shift, leftButton)`: Multi-selection logic
  - Right-click without selection: skip (keep current selection)
  - No modifiers: Clear all, select clicked item
  - Ctrl: Toggle clicked item
  - Shift: Select range from `selectedItemIndexStart` to clicked item
  - Ctrl+Shift: Add range to selection
- `SelectItemsByKey(code, ctrl, shift)`: Keyboard navigation
  - Arrow keys: Navigate and select
  - Home/End: First/last item
  - PageUp/PageDown: Page navigation
  - Uses `FindItemByVirtualKeyDirection` to find target item
  - Applies same ctrl/shift logic as mouse
  - Calls `EnsureItemVisible` after selection
- `ClearSelection()`: Removes all selections, fires event

**Synchronization with Item Styles**:
- `OnStyleInstalled`: Sets `style->SetSelected(selectedItems.Contains(itemIndex))`
- `OnItemModified`: Clears selection if item count changes or selected items modified

**Special Behavior**:
- Left-click always processes selection
- Right-click on already-selected item keeps selection (for context menu)
- Shift selection uses `SetMultipleItemsSelectedSilently` to batch update without firing multiple events

#### Specialized List Controls

**Control Hierarchy**:

1. **GuiTextList**:
   - Uses `TextItemProvider`
   - Item template: `GuiTextListItemTemplate`
   - Additional properties: `Checked`, `TextColor`
   - `GetItems()`: Direct access to writable `Ptr<TextItem>` list

2. **GuiListView**:
   - Uses `ListViewItemProvider`
   - Item template: Varies by view (List/SmallIcon/LargeIcon/Tile/Detail)
   - Column management via `GetColumns()` and `GetDataColumns()`
   - Column arranger: `ListViewColumnItemArranger` for detail view

3. **GuiTreeView**:
   - Uses `TreeViewItemRootProvider` + `NodeItemProvider`
   - Item template: `GuiTreeItemTemplate`
   - Properties: `Expanding`, `Expandable`, `Level`, `Image`
   - Node manipulation via `Nodes()->GetRootNode()` and `MemoryNodeProvider::Children()`

4. **GuiBindableTextList**:
   - Uses `TextItemBindableProvider`
   - Data source: `IValueObservableList` via `GetItemSource()`/`SetItemSource()`
   - Automatically updates when observable list changes
   - `NotifyItemDataModified(start, count)`: Manual refresh for property changes

5. **GuiBindableListView**:
   - Uses `ListViewItemBindableProvider`
   - Similar to GuiBindableTextList but with columns

6. **GuiBindableTreeView**:
   - Uses `TreeViewItemBindableRootProvider`
   - Constructor parameter `reverseMappingProperty`: Property name for accessing `INodeProvider*` from data
   - `NotifyNodeDataModified(value)`: Refresh when node properties change

7. **GuiVirtualDataGrid**:
   - Uses `DataProvider` (filtered/sorted view)
   - Cell editors: `SetCellValueSaved` when editor closes
   - `SelectCell(value, openEditor)`: Navigate to cell and optionally edit

**Common Pattern**:
- Concrete controls (`GuiTextList`) manage their own data structures
- Bindable controls (`GuiBindableTextList`) wrap user-provided data sources via reflection
- All inherit from `GuiListControl` or `GuiSelectableListControl`
- Data manipulation via provider's view interfaces
- Visual customization via item templates

#### Integration with GuiScrollView

`GuiListControl` extends `GuiScrollView`:

**Size Calculation**:
- `QueryFullSize()`: Returns `itemArranger->GetTotalSize()` as `fullSize`
- Called by scroll view to determine scroll range

**View Updates**:
- `UpdateView(Rect viewBounds)`: Called by scroll view when viewport changes
- Forwards to `itemArranger->OnViewChanged(viewBounds)`
- Arranger updates visible items based on new viewport

**Adopted Size**:
- `GetAdoptedSize(Size expectedSize)`: Calculate minimal size to fit items
- Tracks scroll bar visibility in `adoptedSizeDiffWithScroll` and `adoptedSizeDiffWithoutScroll`
- Delegates to `itemArranger->GetAdoptedSize(expectedViewSize)`
- Used by responsive layouts to size list controls appropriately

**Rendering Lifecycle**:
- `OnRenderTargetChanged`: Recreates style and arranger (calls `SetStyleAndArranger`)
- `OnBeforeReleaseGraphicsHost`: Clears style and arranger to release resources
- `AfterControlTemplateInstalled_`: Reloads visible styles and recalculates view

**Focus Handling**:
- Constructor with `acceptFocus=true`: Attaches mouse down handlers to `boundsComposition`
- Mouse down calls `SetFocused()` if visually enabled
- Sets `focusableComposition` to `boundsComposition`

#### Template and Arranger Coordination

**SetStyleAndArranger Process**:

Called when item template or arranger changes:

1. **Detach Old Arranger**:
   - `itemProvider->DetachCallback(itemArranger)` (stops data notifications)
   - `itemArranger->DetachListControl()` (releases list control reference)
   - `itemArranger->SetCallback(nullptr)` (removes item creation callback)

2. **Clear Cached Items**:
   - `callback->ClearCache()` iterates `installedStyles`
   - Calls `UninstallStyle` for each, then `SafeDeleteComposition`

3. **Update Properties**:
   - `itemStyleProperty = styleProperty`
   - `itemArranger = arranger`

4. **Attach New Arranger**:
   - `itemArranger->SetCallback(callback.Obj())` (arranger adds repeat composition to container)
   - `itemArranger->AttachListControl(this)` (arranger gets axis from list control)
   - `itemProvider->AttachCallback(itemArranger.Obj())` (arranger receives data notifications)

5. **Reset Scrolls**:
   - Set both horizontal and vertical scroll positions to 0

6. **Recalculate View**:
   - `CalculateView()` triggers size query and viewport update

**PredefinedListItemTemplate Pattern**:

Helper template for item templates in `Source/Controls/ListControlPackage/GuiListControls.h`:

```cpp
template<typename TBase>
class PredefinedListItemTemplate : public TBase
```

- Hooks `AssociatedListControlChanged` event
- When list control is set:
  - Stores `listControl` pointer
  - Calls `OnInitialize()` (derived class initialization)
  - Calls `OnRefresh()` (derived class property update)
- `RefreshItem()`: Public method to manually call `OnRefresh()`
- Ensures template initializes only once even if event fires multiple times

**Display Item Background**:

`displayItemBackground` property controls wrapping:
- `true` (default): Each item wrapped in `GuiSelectableButton` with `ThemeName::ListItemBackground`
  - Background button handles selection visual state
  - Template obtains background template from list control template via `GetBackgroundTemplate()`
- `false`: Template used directly as bounds
- Changing this property triggers `SetStyleAndArranger` to recreate all items
- `GuiControl::GetFocused` checks if the control is currently focused
- `GuiControl::SetFocused` is the public API to focus a control:
  - Returns early if no focusable composition
  - Returns early if control is not visually enabled
  - Returns early if focusable composition is not eventually visible
  - Gets the graphics host from the focusable composition
  - Calls `host->SetFocus(focusableComposition)`
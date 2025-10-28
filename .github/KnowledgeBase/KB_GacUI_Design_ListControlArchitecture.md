# List Control Architecture

## Overview

`GuiListControl` implements a three-layer architecture that separates data management, layout arrangement, and visual rendering. This architecture enables flexible customization while maintaining efficient virtualization for large datasets.

## Three-Layer Architecture

### Layer 1: Data Layer (IItemProvider)

**Purpose**: Provides data to the list control

**Location**: `Source/Controls/ListControlPackage/DataSource_IItemProvider.h`

**Core Interface Methods**:
- `Count()`: Returns total number of items
- `GetTextValue(itemIndex)`: Returns text representation of an item
- `GetBindingValue(itemIndex)`: Returns boxed `description::Value` of an item
- `RequestView(identifier)`: Returns additional view interfaces for specialized data access
- `AttachCallback(IItemProviderCallback*)` and `DetachCallback(IItemProviderCallback*)`: Register/unregister for notifications

**Callback Mechanism**:
- `IItemProviderCallback` receives notifications via `OnItemModified(start, count, newCount, itemReferenceUpdated)`
- `itemReferenceUpdated` flag indicates whether item identity changed (requires recreation) or just content changed (can refresh in place)
- Base implementation `ItemProviderBase` manages callbacks and editing counter

**View System**: Providers expose multiple view interfaces via `RequestView(identifier)` for specialized data access:
- `ITextItemView`: Checkbox state for `GuiVirtualTextList`
- `IListViewItemView`: Icons, text, and sub-items for `GuiVirtualListView`
- `IColumnItemView`: Column widths, sort states, context menus
- `INodeItemView`: Tree node access for `GuiVirtualTreeListControl`
- `ITreeViewItemView`: Node icons for tree nodes
- `IDataGridView`: Cell data, editors, visualizers, sorting, filtering

### Layer 2: Layout Layer (IItemArranger)

**Purpose**: Decides how to arrange item controls spatially

**Location**: `Source/Controls/ListControlPackage/GuiListControls.h` as nested interface

**Core Interface Methods**:
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

**Responsibilities**:
- Implements `IItemProviderCallback` to respond to data changes
- Creates/destroys visible item controls via callback
- Manages spatial layout and virtualization

**Base Implementation**: `RangedItemArrangerBase` delegates to `GuiVirtualRepeatCompositionBase`

### Layer 3: Rendering Layer (ItemStyle/ItemTemplate)

**Purpose**: Defines visual appearance of each item

**Base Template**: `GuiListItemTemplate` in `Source/Controls/Templates/GuiControlTemplates.h`

**Properties**:
- `Selected`: Selection state
- `Index`: Item index in the list
- `AssociatedListControl`: Reference to parent list control

**Derived Templates**: Add specialized properties (e.g., `GuiTextListItemTemplate` adds `Checked`, `TextColor`)

**Creation**: Item template is created via `TemplateProperty<GuiListItemTemplate>` function pointer

**Background Wrapping**: Each item can be wrapped in background button if `displayItemBackground` is true

## Item Rendering Management

### Item Lifecycle

**Creation Flow (InstallStyle)**:

`GuiListControl::ItemCallback` implements both `IItemProviderCallback` and `IItemArrangerCallback`. When arranger needs an item:

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

### Event Handling

Each visible style has attached mouse event handlers stored in `VisibleStyleHelper`:

**Events Captured**:
- Left/middle/right button down/up/doubleclick
- Mouse move/enter/leave

**Translation Process**:
1. Handler gets `itemIndex` from arranger via `GetVisibleIndex(style)`
2. Creates `GuiItemMouseEventArgs` or `GuiItemEventArgs` with `itemIndex`
3. Fires corresponding item event on `GuiListControl` (e.g., `ItemLeftButtonDown`)

### Property Updates

When item provider notifies modification via `OnItemModified(start, count, newCount, itemReferenceUpdated)`:

- If `itemReferenceUpdated == false` and `count == newCount` (content update only):
  - For each visible item in range, calls `OnStyleInstalled(itemIndex, style, true)` to refresh properties
- Otherwise arranger handles recreation

## Item Layout Management

### Virtual Repeat Composition System

All predefined arrangers use `RangedItemArrangerBase` which delegates to `GuiVirtualRepeatCompositionBase`.

### Arranger Types

**Location**: `Source/Controls/ListControlPackage/GuiListControlItemArrangers.h`

1. **FreeHeightItemArranger**: Uses `GuiRepeatFreeHeightItemComposition`
   - Caches heights of all items individually
   - Suitable for items with varying heights

2. **FixedHeightItemArranger**: Uses `GuiRepeatFixedHeightItemComposition`
   - All items have same height (maximum of minimum heights)
   - Most efficient for uniform lists

3. **FixedSizeMultiColumnItemArranger**: Uses `GuiRepeatFixedSizeMultiColumnItemComposition`
   - Multiple columns with fixed item size (max width and height)
   - Grid layout

4. **FixedHeightMultiColumnItemArranger**: Uses `GuiRepeatFixedHeightMultiColumnItemComposition`
   - Multiple columns with fixed height but variable width per item
   - Suitable for tag clouds or varying-width items

### Virtual Repeat Template Pattern

`VirtualRepeatRangedItemArrangerBase<TVirtualRepeatComposition>` template creates `ArrangerRepeatComposition`:

**Template Overrides**:
- `CreateStyleInternal(index)`:
  - Calls `arranger->callback->CreateItem(index)` to get `ItemStyle`
  - Calls `arranger->callback->GetItemBounds(itemStyle)` to get wrapping composition
  - Returns bounds composition to repeat composition
- `DeleteStyleInternal(style)`:
  - Calls `arranger->callback->GetItem(style)` to unwrap to `ItemStyle`
  - Calls `arranger->callback->ReleaseItem(itemStyle)` to destroy
- `Layout_UpdateIndex(style, index)`:
  - Updates `itemStyle->SetIndex(index)` when items scroll

### Coordination with Repeat Composition

`RangedItemArrangerBase` bridges list control and repeat composition:

**Data Integration**:
- `OnAttached(IItemProvider*)`: Creates `ArrangerItemSource` wrapper and calls `repeat->SetItemSource(itemSource)`
- `OnItemModified(start, count, newCount, itemReferenceUpdated)`: If `itemReferenceUpdated` is true, calls `itemSource->ItemChanged(start, count, newCount)` to notify repeat composition

**List Control Integration**:
- `AttachListControl(GuiListControl*)`: Calls `repeat->SetAxis(listControl->GetAxis())` to share coordinate transformer
- `SetCallback(IItemArrangerCallback*)`: Adds repeat composition to container and sets dummy item template to enable virtual mode

**View Updates**:
- `OnViewChanged(Rect bounds)`: Calls `repeat->SetViewLocation(bounds.LeftTop())` and `repeat->SetExpectedBounds(Rect({0,0}, bounds.GetSize()))`

**Event Forwarding**: `ViewLocationChanged`, `TotalSizeChanged`, `AdoptedSizeInvalidated` from repeat to callback

### IGuiAxis Integration

`IGuiAxis` transforms between real coordinates and virtual coordinates:

- Shared by list control and repeat composition via `repeat->SetAxis(listControl->GetAxis())`
- Used for key navigation direction mapping
- Default is `GuiDefaultAxis` (identity transformation)

## Item Data Management

### Provider Hierarchy

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

### Callback Flow

When data changes:

1. Provider calls `InvokeOnItemModified(start, count, newCount, itemReferenceUpdated)`
2. `ItemProviderBase::InvokeOnItemModified` iterates through all `callbacks` (usually contains arranger and list control's `ItemCallback`)
3. Each callback's `OnItemModified` is invoked
4. Arranger's `OnItemModified`: Notifies repeat composition if `itemReferenceUpdated`
5. List control's `ItemCallback::OnItemModified`: Forwards to `GuiListControl::OnItemModified`
6. `GuiListControl::OnItemModified`: Refreshes visible item properties if content changed

## Selection Management

`GuiSelectableListControl` extends `GuiListControl` to add selection support.

### Selection State

- `selectedItems`: `SortedList<vint>` of selected indices
- `multiSelect`: Boolean enabling multi-selection
- `selectedItemIndexStart` and `selectedItemIndexEnd`: Range for shift-selection

### Selection Methods

**SetSelected**:
- `SetSelected(itemIndex, value)`: Set single item selection
- Updates `selectedItems`
- Calls `OnItemSelectionChanged(itemIndex, value)` which updates visible style's `Selected` property
- Fires `SelectionChanged` event

**SelectItemsByClick**:
- `SelectItemsByClick(itemIndex, ctrl, shift, leftButton)`: Multi-selection logic
- Right-click without selection: skip (keep current selection for context menu)
- No modifiers: Clear all, select clicked item
- Ctrl: Toggle clicked item
- Shift: Select range from `selectedItemIndexStart` to clicked item
- Ctrl+Shift: Add range to selection

**SelectItemsByKey**:
- `SelectItemsByKey(code, ctrl, shift)`: Keyboard navigation
- Arrow keys: Navigate and select
- Home/End: First/last item
- PageUp/PageDown: Page navigation
- Uses `FindItemByVirtualKeyDirection` to find target item
- Applies same ctrl/shift logic as mouse
- Calls `EnsureItemVisible` after selection

**ClearSelection**:
- Removes all selections
- Fires event

### Synchronization with Item Styles

- `OnStyleInstalled`: Sets `style->SetSelected(selectedItems.Contains(itemIndex))`
- `OnItemModified`: Clears selection if item count changes or selected items modified

### Special Behavior

- Left-click always processes selection
- Right-click on already-selected item keeps selection (for context menu)
- Shift selection uses `SetMultipleItemsSelectedSilently` to batch update without firing multiple events

## Specialized List Controls

### Control Hierarchy and Templates

#### GuiVirtualTextList

**Item Provider**: `TextItemProvider` (concrete) or `TextItemBindableProvider` (bindable)

**View-Template Mapping** (set in `SetView(TextListView)` method):

1. **TextListView::Text**: 
   - Template: `list::DefaultTextListItemTemplate`
   - Arranger: `list::FixedHeightItemArranger`
   
2. **TextListView::Check**:
   - Template: `list::DefaultCheckTextListItemTemplate` (derived from `DefaultTextListItemTemplate`)
   - Arranger: `list::FixedHeightItemArranger`
   - Overrides `CreateBulletStyle()` to create checkbox
   
3. **TextListView::Radio**:
   - Template: `list::DefaultRadioTextListItemTemplate` (derived from `DefaultTextListItemTemplate`)
   - Arranger: `list::FixedHeightItemArranger`
   - Overrides `CreateBulletStyle()` to create radio button

**Template Structure** (`DefaultTextListItemTemplate`):
- Location: `Source/Controls/ListControlPackage/ItemTemplate_ITextItemView.h`
- Contains `bulletButton` (optional `GuiSelectableButton`)
- Contains `textElement` (`GuiSolidLabelElement` for text display)
- Hooks property changes: `Checked`, `TextColor`, `Font`, `Text`
- `CreateBulletStyle()`: Returns `nullptr` for Text view, checkbox/radio for Check/Radio views

**Default View**: Constructor calls `SetView(TextListView::Text)`

**Data Access**: `GetItems()` returns direct access to writable `Ptr<TextItem>` list (concrete only)

#### GuiVirtualListView

**Item Provider**: `ListViewItemProvider` (concrete) or `ListViewItemBindableProvider` (bindable)

**View-Template Mapping** (set in `SetView(ListViewView)` method):

1. **ListViewView::BigIcon**:
   - Template: `list::BigIconListViewItemTemplate`
   - Arranger: `list::FixedSizeMultiColumnItemArranger`
   - Contains large image and centered text
   
2. **ListViewView::SmallIcon**:
   - Template: `list::SmallIconListViewItemTemplate`
   - Arranger: `list::FixedSizeMultiColumnItemArranger`
   - Contains small image and text
   
3. **ListViewView::List**:
   - Template: `list::ListListViewItemTemplate`
   - Arranger: `list::FixedHeightMultiColumnItemArranger`
   - Contains small image and text in list format
   
4. **ListViewView::Tile**:
   - Template: `list::TileListViewItemTemplate`
   - Arranger: `list::FixedSizeMultiColumnItemArranger`
   - Contains large image, main text, and data column texts
   - Uses `GuiTableComposition` for text layout
   
5. **ListViewView::Information**:
   - Template: `list::InformationListViewItemTemplate`
   - Arranger: `list::FixedHeightItemArranger`
   - Contains large image, main text, column headers, and data texts
   - Uses `GuiTableComposition` for text layout with column labels
   
6. **ListViewView::Detail**:
   - Template: `list::DetailListViewItemTemplate`
   - Arranger: `list::ListViewColumnItemArranger`
   - Contains image, main text, and sub-item texts in columns
   - Uses `GuiTableComposition` with dynamic columns matching column count

**Template Location**: All templates in `Source/Controls/ListControlPackage/ItemTemplate_IListViewItemView.h`

**Common Base**: All templates derive from `list::DefaultListViewItemTemplate` which derives from `PredefinedListItemTemplate<GuiListItemTemplate>`

**Default View**: Constructor calls `SetView(ListViewView::Detail)`

**Column Management**: `GetColumns()` and `GetDataColumns()` for column configuration

#### GuiVirtualTreeView

**Item Provider**: `TreeViewItemRootProvider` + `NodeItemProvider` (concrete) or `TreeViewItemBindableRootProvider` (bindable)

**Template**: `tree::DefaultTreeItemTemplate`
- Arranger: `list::FixedHeightItemArranger`
- Location: `Source/Controls/ListControlPackage/ItemTemplate_ITreeViewItemView.h`

**Template Structure**:
- Contains `expandingButton` (`GuiSelectableButton` for expand/collapse)
- Contains `table` (`GuiTableComposition` for layout)
- Contains `imageElement` (`GuiImageFrameElement` for node icon)
- Contains `textElement` (`GuiSolidLabelElement` for node text)
- Hooks property changes: `Expanding`, `Expandable`, `Level`, `Image`, `Text`, `TextColor`, `Font`
- `OnLevelChanged`: Adjusts indentation based on tree level
- Expanding button handles double-click to expand/collapse node

**Node Manipulation**: `Nodes()->GetRootNode()` and `MemoryNodeProvider::Children()` for tree structure

#### GuiVirtualDataGrid

**Item Provider**: `DataProvider` (filtered/sorted view)

**Template**: `list::DefaultDataGridItemTemplate`
- Arranger: `list::ListViewColumnItemArranger`
- Location: `Source/Controls/ListControlPackage/GuiDataGridControls.h`

**Template Structure**:
- Derives from `DefaultListViewItemTemplate`
- Contains `textTable` (`GuiTableComposition` for cell layout)
- Contains `dataVisualizerFactories` array (one per column)
- Contains `dataVisualizers` array (created from factories)
- Contains `dataCells` array (`GuiCellComposition` for each column)
- Contains `currentEditor` (`IDataEditor*` for in-place editing)

**Responsibilities**:
- Handles cell selection and editor opening via mouse events
- `UpdateSubItemSize()`: Synchronizes cell widths with column widths
- `NotifyOpenEditor(column, editor)`: Opens editor in specific cell
- `NotifyCloseEditor()`: Closes current editor
- `NotifySelectCell(column)`: Highlights selected cell with focus rectangle

**See Data Grid Advanced Features** section for visualizers, editors, sorters, and filters.

#### GuiComboBoxListControl

**Item Template**: None initially
- Property `itemStyleProperty` starts as empty `TemplateProperty<GuiListItemTemplate>`
- User must set via `SetItemTemplate()` to customize dropdown item appearance
- When `itemStyleProperty` is null:
  - Control template's `TextVisible` is set to `true` (shows text directly)
  - Selected item text displayed in control template
- When `itemStyleProperty` is set:
  - Control template's `TextVisible` is set to `false`
  - Custom item template renders the selected item

**Template Lifecycle**:
- `InstallStyleController(itemIndex)`: Creates item template instance and adds to button
- `RemoveStyleController()`: Destroys current item template instance
- Template synchronized with list control properties: `Text`, `Context`, `VisuallyEnabled`

**Usage Pattern**: Combo box delegates to contained `GuiSelectableListControl` for dropdown list rendering, but uses separate template for displaying selected item in collapsed state

#### GuiBindableRibbonGalleryList

**Item Template**: None initially
- Property `itemStyle` starts as empty `TemplateProperty<GuiListItemTemplate>`
- User must set via `SetItemTemplate()` to render gallery items
- Template is forwarded to internal `itemList` (`GuiBindableTextList`)
- Same template used for both inline gallery and dropdown menu

**Menu Rendering**:
- Dropdown menu uses `GuiRepeatStackComposition` with `GuiRepeatFlowComposition` per group
- Each group header created via control template's `CreateGalleryItemGroup()`
- Each item background created via control template's `CreateGalleryItemBackground()`
- Item template instance placed inside background button

**Usage Pattern**: Ribbon gallery requires user-provided template since gallery items are highly customized (e.g., font names, colors, styles)

### Common Pattern

- Concrete controls (`GuiTextList`) manage their own data structures
- Bindable controls (`GuiBindableTextList`) wrap user-provided data sources via reflection
- All inherit from `GuiListControl` or `GuiSelectableListControl`
- Data manipulation via provider's view interfaces
- Visual customization via item templates

## Integration with GuiScrollView

`GuiListControl` extends `GuiScrollView`.

### Size Calculation

- `QueryFullSize()`: Returns `itemArranger->GetTotalSize()` as `fullSize`
- Called by scroll view to determine scroll range

### View Updates

- `UpdateView(Rect viewBounds)`: Called by scroll view when viewport changes
- Forwards to `itemArranger->OnViewChanged(viewBounds)`
- Arranger updates visible items based on new viewport

### Adopted Size

- `GetAdoptedSize(Size expectedSize)`: Calculate minimal size to fit items
- Tracks scroll bar visibility in `adoptedSizeDiffWithScroll` and `adoptedSizeDiffWithoutScroll`
- Delegates to `itemArranger->GetAdoptedSize(expectedViewSize)`
- Used by responsive layouts to size list controls appropriately

### Rendering Lifecycle

- `OnRenderTargetChanged`: Recreates style and arranger (calls `SetStyleAndArranger`)
- `OnBeforeReleaseGraphicsHost`: Clears style and arranger to release resources
- `AfterControlTemplateInstalled_`: Reloads visible styles and recalculates view

### Focus Handling

- Constructor with `acceptFocus=true`: Attaches mouse down handlers to `boundsComposition`
- Mouse down calls `SetFocused()` if visually enabled
- Sets `focusableComposition` to `boundsComposition`

## Template and Arranger Coordination

### SetStyleAndArranger Process

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

### PredefinedListItemTemplate Pattern

Helper template for item templates in `Source/Controls/ListControlPackage/GuiListControls.h`:

```cpp
template<typename TBase>
class PredefinedListItemTemplate : public TBase
```

**Lifecycle**:
- Hooks `AssociatedListControlChanged` event
- When list control is set:
  - Stores `listControl` pointer
  - Calls `OnInitialize()` (derived class initialization)
  - Calls `OnRefresh()` (derived class property update)
- `RefreshItem()`: Public method to manually call `OnRefresh()`
- Ensures template initializes only once even if event fires multiple times

### Display Item Background

`displayItemBackground` property controls wrapping:

- `true` (default): Each item wrapped in `GuiSelectableButton` with `ThemeName::ListItemBackground`
  - Background button handles selection visual state
  - Template obtains background template from list control template via `GetBackgroundTemplate()`
- `false`: Template used directly as bounds
- Changing this property triggers `SetStyleAndArranger` to recreate all items

## Data Grid Advanced Features

`GuiVirtualDataGrid` and `GuiBindableDataGrid` provide advanced cell rendering, editing, sorting, and filtering capabilities.

### Data Visualizer System

**Purpose**: Customizes how each cell is displayed in the data grid

**Core Interfaces** (in `Source/Controls/ListControlPackage/GuiDataGridInterfaces.h`):

1. **IDataVisualizerFactory**:
   - `CreateVisualizer(dataGridContext)`: Creates visualizer instances
   - Used to generate visualizers for each visible cell

2. **IDataVisualizer**:
   - `GetTemplate()`: Returns `GuiGridVisualizerTemplate*` for rendering
   - `BeforeVisualizeCell(itemProvider, row, column)`: Updates visualizer before display
   - `SetSelected(value)`: Highlights selected cell
   - `NotifyDeletedTemplate()`: Cleanup when template is destroyed

**Base Implementations** (in `Source/Controls/ListControlPackage/GuiDataGridExtensions.h`):

1. **DataVisualizerBase**:
   - Standard implementation of `IDataVisualizer`
   - Stores `factory`, `dataGridContext`, `visualizerTemplate`
   - `BeforeVisualizeCell`: Updates template properties from data
     - Sets colors from control template: `PrimaryTextColor`, `SecondaryTextColor`, `ItemSeparatorColor`
     - Sets images from `IListViewItemView`: `LargeImage`, `SmallImage`
     - Sets text from column data: main text or sub-item text
     - Sets binding values from `IDataGridView`: `RowValue`, `CellValue`

2. **DataVisualizerFactory**:
   - Stores `templateFactory` (function creating template) and optional `decoratedFactory`
   - Decorator pattern: Creates template, then creates child template from decorated factory
   - Forwards all property changes from parent to child template using macro-generated event handlers
   - Enables composing visualizers (e.g., border + focus rectangle + content)

**Predefined Visualizer Templates** (in `Source/Controls/ListControlPackage/GuiDataGridExtensions.cpp`):

1. **MainColumnVisualizerTemplate**:
   - First column with image and text
   - Uses `GuiTableComposition` with 3 rows × 2 columns
   - Image in cell (1,0), text in cell (0-2,1)
   - Hooks: `TextChanged`, `FontChanged`, `PrimaryTextColorChanged`, `SmallImageChanged`

2. **SubColumnVisualizerTemplate**:
   - Other columns with text only
   - Simple `GuiBoundsComposition` with `GuiSolidLabelElement`
   - Uses `SecondaryTextColor` instead of primary

3. **HyperlinkVisualizerTemplate**:
   - Extends `SubColumnVisualizerTemplate`
   - Blue text color (hardcoded)
   - Underlines on mouse hover
   - Hand cursor

4. **FocusRectangleVisualizerTemplate**:
   - Decorator adding focus rectangle around selected cell
   - Creates `GuiFocusRectangleElement` with 1px margin
   - Shows/hides based on `Selected` property
   - Uses `ContainerComposition` pattern for decoration

5. **CellBorderVisualizerTemplate**:
   - Decorator adding cell borders
   - Two `GuiSolidBorderElement` (left and top borders)
   - Color from `ItemSeparatorColor` property
   - Uses `ContainerComposition` pattern for decoration

**Integration with DefaultDataGridItemTemplate** (in `Source/Controls/ListControlPackage/GuiDataGridControls.cpp`):

- Each row template has `dataVisualizerFactories` array (one per column)
- `GetDataVisualizerFactory(row, column)`:
  - First tries custom factory from `IDataGridView::GetCellDataVisualizerFactory`
  - Falls back to `defaultMainColumnVisualizerFactory` for column 0
  - Falls back to `defaultSubColumnVisualizerFactory` for other columns
- Default factories are composed via decorator pattern:
  - `CellBorderVisualizerTemplate` wraps `FocusRectangleVisualizerTemplate` wraps `MainColumnVisualizerTemplate`
  - `CellBorderVisualizerTemplate` wraps `FocusRectangleVisualizerTemplate` wraps `SubColumnVisualizerTemplate`
- `OnRefresh()`: Creates visualizers if not exist, calls `BeforeVisualizeCell` to update data
- `DeleteVisualizer(column)`: Calls `NotifyDeletedTemplate()`, removes from parent, destroys composition

**Lifecycle**:

1. Grid creates `DefaultDataGridItemTemplate` for each visible row
2. Template calls `GetDataVisualizerFactory` for each column
3. Factory creates visualizer via `CreateVisualizer(dataGridContext)`
4. Visualizer creates template composition hierarchy
5. Template calls `BeforeVisualizeCell` when data changes
6. Visualizer updates template properties from data
7. When cell scrolls out, visualizer is deleted via `DeleteVisualizer`

### Data Editor System

**Purpose**: Allows in-place editing of cells

**Core Interfaces** (in `Source/Controls/ListControlPackage/GuiDataGridInterfaces.h`):

1. **IDataEditorFactory**:
   - `CreateEditor(dataGridContext)`: Creates editor instances
   - Provided by column to enable editing

2. **IDataEditor**:
   - `GetTemplate()`: Returns `GuiGridEditorTemplate*` for editing UI
   - `BeforeEditCell(itemProvider, row, column)`: Initializes editor before opening
   - `GetCellValueSaved()`: Returns true if user confirmed edit
   - `NotifyDeletedTemplate()`: Cleanup when template is destroyed

**Base Implementation** (in `Source/Controls/ListControlPackage/GuiDataGridExtensions.cpp`):

1. **DataEditorBase**:
   - Standard implementation of `IDataEditor`
   - Stores `factory`, `dataGridContext`, `editorTemplate`
   - `BeforeEditCell`: Updates template properties and attaches `CellValueChanged` event
   - `OnCellValueChanged`: Calls `dataGridContext->RequestSaveData()` when value changes
   - `GetCellValueSaved`: Delegates to `editorTemplate->GetCellValueSaved()`

2. **DataEditorFactory**:
   - Stores `templateFactory` (function creating template)
   - `CreateEditor`: Creates `DataEditorBase`, creates template from factory, assigns to editor

**Editor State** (in `Source/Controls/ListControlPackage/GuiDataGridControls.h`):
- `currentEditor`: Currently opened editor (`Ptr<IDataEditor>`)
- `currentEditorPos`: Position of edited cell (`GridPos`)
- `currentEditorOpeningEditor`: Flag preventing recursive calls

**Opening Editor** (`StartEdit` in `GuiVirtualDataGrid`):

1. Calls `StopEdit()` to close any existing editor
2. Calls `NotifySelectCell(row, column)` to update selection
3. Gets visible style for the row via arranger
4. Calls `IDataGridView::GetCellDataEditorFactory(row, column)` to get factory
5. Sets `currentEditorOpeningEditor = true` (prevents recursion)
6. Creates editor via `factory->CreateEditor(this)`
7. Sets editor's alt key to "E" for accessibility
8. Calls `editor->BeforeEditCell(itemProvider, row, column)` to initialize
9. Calls `itemStyle->NotifyOpenEditor(column, editor)` to show editor in cell
10. Sets `currentEditorOpeningEditor = false`

**Closing Editor** (`StopEdit`):

1. If item provider is editing, calls `NotifyCloseEditor()` on template
2. Otherwise directly closes editor
3. Clears alt composition and control
4. Resets `currentEditor` and `currentEditorPos`

**Saving Data** (`RequestSaveData`):

1. Called when user confirms edit (e.g., presses Enter)
2. Calls `itemProvider->PushEditing()` to enter editing mode
3. Gets value from `currentEditor->GetTemplate()->GetCellValue()`
4. Calls `dataGridView->SetBindingCellValue(row, column, value)` to update data
5. Calls `itemProvider->PopEditing()` to exit editing mode
6. Calls `itemStyle->NotifyCellEdited()` to refresh visualizer

**User Interactions**:

Mouse Events (in `DefaultDataGridItemTemplate`):
- `OnCellLeftButtonUp`: If not in editor, calls `SelectCell(pos, true)` to open editor
- `OnCellRightButtonUp`: If not in editor, calls `SelectCell(pos, false)` to select without editing
- `OnCellButtonDown`: Marks event as handled if click is inside editor (prevents closing)
- `IsInEditor`: Checks if event source is within editor composition hierarchy

Keyboard Events (in `GuiVirtualDataGrid::OnKeyDown`):
- **Enter**: Saves data, toggles editor open/close, refocuses control if editor closed
- **Escape**: If editor open, closes editor without saving, refocuses control
- **Left/Right**: Navigates between cells, closes editor if open
- **Other keys**: Handled by editor control itself

**Editor Template Integration** (in `DefaultDataGridItemTemplate`):

`NotifyOpenEditor`:
1. Stores `currentEditor` pointer
2. Gets cell composition for the column
3. Gets editor bounds composition
4. Sets font and context from row template
5. Removes from old parent if needed
6. Adds editor composition to cell with full alignment
7. Focuses the editor's focus control
8. Hides the visualizer for that column

`NotifyCloseEditor`:
1. Shows all visualizers again
2. Removes editor composition from parent
3. Clears `currentEditor` pointer

**Alt Action Integration**:
- Editor can provide `IGuiAltAction` service for keyboard shortcuts
- `GetActivatingAltHost` checks if editor has alt action support
- If supported, sets editor as alt composition and control

### Data Sorter System

**Purpose**: Enables sorting rows by column in `GuiBindableDataGrid`

**Core Interfaces** (in `Source/Controls/ListControlPackage/GuiBindableDataGrid.h`):

1. **IDataProcessorCallback**:
   - `GetItemProvider()`: Access to item provider
   - `OnProcessorChanged()`: Called when sorter/filter structure changes

2. **IDataSorter**:
   - `SetCallback(callback)`: Receives notification callback
   - `Compare(row1, row2)`: Returns <0, 0, >0 for ordering

**Base Implementation** (in `Source/Controls/ListControlPackage/GuiBindableDataGrid.cpp`):

**DataSorterBase**:
- Stores `callback` pointer
- `InvokeOnProcessorChanged()`: Notifies callback when sorter changes
- `SetCallback(value)`: Sets callback pointer

**Predefined Sorter Implementations**:

1. **DataMultipleSorter** (multi-level sorting):
   - Stores `leftSorter` and `rightSorter`
   - `Compare`: First tries `leftSorter`, if result is 0, tries `rightSorter`
   - `SetLeftSorter`, `SetRightSorter`: Updates sub-sorters and their callbacks
   - Enables sorting by primary column, then secondary column, etc.

2. **DataReverseSorter** (reverse order):
   - Stores single `sorter`
   - `Compare`: Returns negated result from `sorter->Compare`
   - Used for descending sort

**Integration with DataColumn**:

Each `DataColumn` has `associatedSorter` property:
- `SetSorter(value)`:
  - Detaches old sorter callback
  - Attaches new sorter callback to `dataProvider`
  - If this column is currently sorted, re-sorts using new sorter
  - Otherwise just notifies change

**Integration with DataProvider** (in `Source/Controls/ListControlPackage/GuiBindableDataGrid.cpp`):

**State**:
- `currentSorter`: Active sorter (`Ptr<IDataSorter>`)
- `virtualRowToSourceRow`: Maps visible row index to source row index

**Sorting Process** (`SortByColumn`):

1. Gets sorter from column via `columns[column]->GetSorter()`
2. If column is -1 or no sorter, clears `currentSorter`
3. If ascending, uses column's sorter directly
4. If descending, wraps in `DataReverseSorter`
5. Updates all columns' `sortingState`:
   - Sorted column: `Ascending` or `Descending`
   - Other columns: `NotSorted`
6. Calls `NotifyColumnChanged()` to update column headers
7. Calls `ReorderRows(true)` to rebuild row order

**Row Reordering** (`ReorderRows`):

1. Gets old row count
2. Clears `virtualRowToSourceRow`
3. If filter exists, builds `virtualRowToSourceRow` with filtered indices
4. If no filter but sorter exists, builds `virtualRowToSourceRow` with all indices
5. If sorter exists and rows present:
   - Calls `SortLambda` on `virtualRowToSourceRow` array
   - Lambda compares using `sorter->Compare(itemSource->Get(a), itemSource->Get(b))`
   - Uses spaceship operator (`<=>`) for stable sort (preserves original order for equal items)
6. If `invokeCallback` is true, fires `OnItemModified` event with new row count

**User Interaction** (`GuiVirtualDataGrid::OnColumnClicked`):

1. Checks if column is sortable via `dataGridView->IsColumnSortable(column)`
2. Gets current sorting state from `columnItemView->GetSortingState(column)`
3. Cycles through states:
   - `NotSorted` → `Ascending`: Calls `SortByColumn(column, true)`
   - `Ascending` → `Descending`: Calls `SortByColumn(column, false)`
   - `Descending` → `NotSorted`: Calls `SortByColumn(-1, false)` to unsort

**Data Access** (`GetBindingValue`):
- If `virtualRowToSourceRow` exists: `itemSource->Get(virtualRowToSourceRow->Get(itemIndex))`
- Otherwise: `itemSource->Get(itemIndex)` (direct access)

**Update Handling** (`OnItemSourceModified`):
- If no sorter, no filter, and count unchanged: Direct notification
- Otherwise: Calls `ReorderRows(true)` to rebuild sorted view

### Data Filter System

**Purpose**: Enables filtering rows based on criteria in `GuiBindableDataGrid`

**Core Interface** (in `Source/Controls/ListControlPackage/GuiBindableDataGrid.h`):

**IDataFilter**:
- `SetCallback(callback)`: Receives notification callback
- `Filter(row)`: Returns true if row should be visible

**Base Implementation**:

**DataFilterBase**:
- Stores `callback` pointer
- `InvokeOnProcessorChanged()`: Notifies callback when filter changes
- `SetCallback(value)`: Sets callback pointer

**Predefined Filter Implementations** (in `Source/Controls/ListControlPackage/GuiBindableDataGrid.cpp`):

1. **DataMultipleFilter** (base for AND/OR):
   - Stores `filters` list (`List<Ptr<IDataFilter>>`)
   - `AddSubFilter(value)`: Adds filter, sets its callback, notifies change
   - `RemoveSubFilter(value)`: Removes filter, clears its callback, notifies change
   - `SetCallback(value)`: Sets callback on self and all sub-filters

2. **DataAndFilter** (all must match):
   - `Filter(row)`: Returns `true` if all sub-filters return `true`
   - Uses `From(filters).All(lambda)` LINQ-style query

3. **DataOrFilter** (any must match):
   - `Filter(row)`: Returns `true` if any sub-filter returns `true`
   - Uses `From(filters).Any(lambda)` LINQ-style query

4. **DataNotFilter** (negation):
   - Stores single `filter`
   - `Filter(row)`: Returns `!filter->Filter(row)`
   - `SetSubFilter(value)`: Updates filter and its callback

**Integration with DataColumn**:

Each `DataColumn` has `associatedFilter` property:
- `SetFilter(value)`:
  - Detaches old filter callback
  - Attaches new filter callback to `dataProvider`
  - Calls `dataProvider->OnProcessorChanged()` to rebuild filter

**Integration with DataProvider**:

**State**:
- `additionalFilter`: User-provided filter (`Ptr<IDataFilter>`)
- `currentFilter`: Combined filter from all columns + additional filter
- `virtualRowToSourceRow`: Maps visible row index to source row index (created when filtering)

**Filter Building** (`RebuildFilter`):

1. Clears old `currentFilter` and its callback
2. Collects all non-null filters from columns
3. Adds `additionalFilter` if exists
4. If any filters exist:
   - Creates `DataAndFilter`
   - Adds all collected filters as sub-filters
   - Sets `currentFilter` to the AND filter
5. Sets callback on `currentFilter`

**Row Filtering** (`ReorderRows`):

1. If `currentFilter` exists:
   - Creates `virtualRowToSourceRow` list
   - Iterates through all source rows
   - For each row, calls `currentFilter->Filter(itemSource->Get(i))`
   - Adds row index to `virtualRowToSourceRow` if filter returns `true`
2. If no filter but sorter exists:
   - Creates `virtualRowToSourceRow` with all indices (for sorting)
3. If neither filter nor sorter:
   - `virtualRowToSourceRow` is null (direct access to source)

**Update Handling** (`OnProcessorChanged`):

- Called when any filter or sorter changes structure
- Calls `RebuildFilter()` to rebuild combined filter
- Calls `ReorderRows(true)` to rebuild filtered/sorted view

**Data Access** (`GetBindingValue`):
- Uses same virtualization as sorter
- If `virtualRowToSourceRow` exists: Maps virtual index to source index
- Otherwise: Direct access to source

**User Interaction**:
- Filters are typically set programmatically via `DataColumn::SetFilter`
- No built-in UI for filter configuration (user must provide custom UI)
- `GuiBindableDataGrid::SetAdditionalFilter` allows programmatic filtering

### Integration Between Systems

**DataProvider as Central Coordinator**:

In `Source/Controls/ListControlPackage/GuiBindableDataGrid.cpp`:

`DataProvider` implements multiple interfaces and coordinates all systems:

1. **IDataProcessorCallback**:
   - `OnProcessorChanged()`: Called when filter/sorter structure changes
   - Rebuilds filter via `RebuildFilter()`
   - Reorders rows via `ReorderRows(true)`

2. **IDataGridView**:
   - `GetCellDataVisualizerFactory(row, column)`: Returns column's visualizer factory
   - `GetCellDataEditorFactory(row, column)`: Returns column's editor factory
   - `IsColumnSortable(column)`: Checks if column has sorter
   - `SortByColumn(column, ascending)`: Triggers sorting
   - `GetBindingCellValue(row, column)`: Gets cell value via column's value property
   - `SetBindingCellValue(row, column, value)`: Sets cell value via column's writable property

3. **IListViewItemView**:
   - Provides text and images for cells
   - Uses column's text property to extract text from row data

4. **IColumnItemView**:
   - Provides column headers, sizes, popups, sorting states
   - Manages column-level UI

5. **IItemProvider**:
   - Provides row count (after filtering)
   - Provides row data (after sorting/filtering via `virtualRowToSourceRow`)

**Callback Flow**:

1. User modifies filter/sorter on column
2. Column calls `dataProvider->OnProcessorChanged()`
3. DataProvider rebuilds filter and reorders rows
4. DataProvider fires `OnItemModified` event
5. List control's arranger receives event
6. Arranger recreates visible item templates
7. Templates create visualizers for each cell
8. Visualizers display filtered/sorted data

**Data Flow**:

```
Source Data (IValueEnumerable)
    ↓
Filter (IDataFilter::Filter)
    ↓
Sort (IDataSorter::Compare)
    ↓
virtualRowToSourceRow mapping
    ↓
DataProvider::GetBindingValue (virtual → source)
    ↓
DataColumn::GetCellValue (property extraction)
    ↓
IDataVisualizer::BeforeVisualizeCell (display)
```

**Edit Flow**:

```
User clicks cell
    ↓
GuiVirtualDataGrid::SelectCell(pos, true)
    ↓
GuiVirtualDataGrid::StartEdit
    ↓
IDataEditorFactory::CreateEditor
    ↓
IDataEditor::BeforeEditCell (load current value)
    ↓
DefaultDataGridItemTemplate::NotifyOpenEditor (show editor)
    ↓
User edits value
    ↓
Editor fires CellValueChanged event
    ↓
GuiVirtualDataGrid::RequestSaveData
    ↓
DataProvider::SetBindingCellValue
    ↓
DataColumn::SetCellValue (property write)
    ↓
DataProvider::InvokeOnItemModified
    ↓
DefaultDataGridItemTemplate::NotifyCellEdited (refresh visualizer)
```

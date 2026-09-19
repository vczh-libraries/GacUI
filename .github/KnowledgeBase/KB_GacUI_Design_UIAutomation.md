# Windows UI Automation

This page describes the current source implementation of Windows UI Automation (UIA). It is an inventory of exposed controls, properties, providers and tree construction, not a statement that every Microsoft control contract is satisfied. The separate [UIA review task](../../../GacUI/TODO_Task_UiaReview.md) records requirements and gaps to verify. Operational guidance is in [Running-GacUI.md](../Guidelines/Running-GacUI.md#windows-specific).

## Implementation map

All provider classes below are in `vl::presentation::windows`. Control classes are in `vl::presentation::controls`, unless another namespace is given.

| Source | Responsibility |
| --- | --- |
| [WindowsUIAutomation.Windows.h/.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.cpp) | `WindowsUIAutomationListener`, native-window discovery, `WM_GETOBJECT`, and the UI-thread dispatcher. |
| [WindowsUIAutomationProvider.Windows.h](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.h) | Shared node, context, lifetime, COM provider and pattern declarations. |
| [WindowsUIAutomationSemantic.Windows.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp) | Actual-class role dispatch, pattern availability, logical items, parent/child relationships, names and bounds. |
| [WindowsUIAutomationProvider.Windows.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp) | COM interfaces, element properties, fragment navigation, identity, hit testing and focus. |
| [WindowsUIAutomationActions.Windows.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp) | Invoke, Toggle, SelectionItem, ExpandCollapse, Value, RangeValue, Scroll, ScrollItem and VirtualizedItem operations. |
| [WindowsUIAutomationPatterns.Windows.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp) | Selection, Grid, Table, ItemContainer, MultipleView, Window and Transform operations. |
| [WindowsUIAutomationText.Windows.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp) | `ITextProvider`, `WindowsUIAutomationTextRange` and document-offset conversion. |
| [WindowsUIAutomationState.Windows.cpp](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp) | Control hooks, model callbacks, property snapshots, deferred events and provider retirement. |

The [Direct2D](../../../GacUI/Source/PlatformProviders/Windows/Direct2D/WinDirect2DApplication.cpp) and [GDI](../../../GacUI/Source/PlatformProviders/Windows/GDI/WinGDIApplication.cpp) initialization paths start the listener before their renderer main function and stop it afterward. The listener is compiled under `VCZH_MSVC`. It handles `WM_GETOBJECT` with `UiaRootObjectId` through `INativeMessageHandler`, returning `UiaReturnRawElementProvider`; it does not implement an MSAA bridge.

The implementation accesses live GacUI controls and document models in the same process. There is no UIA semantic transport in `Source/PlatformProviders/Remote`, or corresponding TUI provider. A remote renderer's drawing tree and the HTTP automation service do not supply the missing control-model bridge. Raw renderer setup starts the listener through the shared initialization path, but `BindWindows()` needs `GuiApplication` and its main window to bind semantic roots.

## Common properties and COM providers

`WindowsUIAutomationProvider` implements `IRawElementProviderSimple`, `IRawElementProviderFragment`, `IRawElementProviderFragmentRoot` and the 19 pattern interfaces listed below. Its `QueryInterface` interface set is fixed. Actual pattern availability comes from `WindowsUIAutomationNode::Supports`, `GetPatternProvider` and the matching `Is...PatternAvailable` properties; a successful COM cast alone does not establish availability.

`Read()` marshals control access to the UI thread and checks node lifetime and any requested pattern. Operations that request an enabled target also check the control and its window. `Invoke()` and `Window.Close()` are posted: success means that the work was queued, so clients must observe the resulting state separately.

| Property or fragment member | Current value/source |
| --- | --- |
| `Name` | `GuiControl::GetText()` by default; empty for every `GuiDocumentCommonInterface` and `GuiComboBoxBase`. Logical item names are listed below. |
| `ControlType` | Actual-class dispatch plus a small number of theme/state distinctions, listed below. |
| `FrameworkId`, `ClassName` | `GacUI`; `GacUI.` followed by the numeric UIA role ID. The class name is not a reflected C++ class name. |
| `AutomationId` | `gacui-` followed by a context-assigned node ID. It is not the resource instance name and is not a persistent cross-launch identifier. |
| `AccessKey`, `AcceleratorKey` | The owning control's `GetAlt()`; for `GuiToolstripButton`, the command shortcut's name when present. |
| `ProcessId`, `NativeWindowHandle` | Current process ID; HWND only when `IsRoot()` is true. Hosted logical child windows therefore do not report their own HWND. |
| `IsControlElement` | Always true for every exposed node. |
| `IsContentElement` | False for logical headers/header items/calendar headers, MenuBar, ScrollBar and Separator; true otherwise. |
| `IsEnabled` | Control `GetVisuallyEnabled()` and native window `IsEnabled()`. |
| `HasKeyboardFocus` | Owning control is focused, and the node is a control or a selected logical item. |
| `IsKeyboardFocusable` | Owning control has a focusable composition, or the node supports SelectionItem. |
| `IsPassword` | True only for `GuiSinglelineTextBox` with a nonzero password character. |
| `IsOffscreen` | True when the calculated bounding rectangle has zero width or height. |
| Fragment `BoundingRectangle` | Composition global bounds, clipped through composition ancestors, converted to physical screen coordinates. Closed windows, invisible compositions and unrealized items produce an empty rectangle. |
| Fragment runtime ID | Nonroots return `UiaAppendRuntimeId` plus their node ID. Roots return no appended ID and use their HWND host provider. |
| Pattern availability | Boolean properties for all 19 patterns below, recomputed from `Supports()`. |

The element-property switch explicitly handles the properties above and the pattern state in the following tables. Other property IDs return `VT_EMPTY`; for example, it does not supply `LabeledBy`, `HelpText` or `ControllerFor`. Several pattern members are implemented through their provider interface without a separate entry in the element-property switch. In particular, fragment bounds exist even though the switch has no `UIA_BoundingRectanglePropertyId` case.

## Control inventory

Pattern names in the tables map to `I<Name>Provider`, for example Text means `ITextProvider`. Every row also receives the common properties above. Base-class capabilities are cumulative unless a condition is stated. A Scroll capability below requires a `GuiScrollView` with both horizontal and vertical scroll objects; it does not require a currently scrollable range.

| Actual control family | UIA role | Available patterns and specific state |
| --- | --- | --- |
| `GuiWindow`, including a plain `GuiPopup` | Window | Window, Transform. Window visual/interaction state, modal/topmost state, maximize/minimize capabilities and move/resize capabilities. |
| `GuiMenu`, `GuiToolstripMenu`, `GuiRibbonToolstripMenu` | Menu; Group for `ThemeName::RibbonGroupMenu` | Window and Transform inherited from `GuiWindow`, despite the different role. Popup children are described below. |
| `GuiTooltip` | ToolTip | Window and Transform inherited from `GuiWindow`. Tooltip open/close event IDs are used. |
| `GuiButton` without selectable-button behavior | Button | Invoke. Template child controls are not enumerated as children. |
| `GuiSelectableButton` outside the menu special case | RadioButton with a group controller; otherwise CheckBox | SelectionItem for grouped buttons; otherwise Toggle. Selected state comes from `GetSelected()`. The role follows group membership, not the CheckBox/RadioButton theme name. |
| `GuiMenuButton`, including `GuiToolstripButton` | With a submenu: SplitButton when `GetSubMenuHost() != this`, otherwise MenuItem. Without a submenu: Button in a toolbar, MenuItem under menu/menu bar, otherwise Button. | ExpandCollapse when a submenu exists. Invoke only when auto-selection and group membership are absent and there is no submenu or it has a separate host. Toggle for ungrouped auto-selection buttons; SelectionItem for grouped buttons. Toolstrip command shortcuts also supply `AcceleratorKey`. |
| `GuiComboBoxBase`, `GuiComboButton`, `GuiDateComboBox` | ComboBox | ExpandCollapse from their submenu; no Invoke or Value. Their normal menu-button initialization disables auto-selection. The base mapping has no Selection; changing inherited group/auto-selection settings still affects the generic SelectionItem/Toggle predicates. |
| `GuiComboBoxListControl` | ComboBox | Base combo patterns plus Selection, aggregated from its contained list. `CanSelectMultiple=false`, `IsSelectionRequired=false`; current selected text is not the Name or Value. |
| `GuiSinglelineTextBox` | Edit | Value and Text; password mode removes Text, denies Value reads and leaves Value write support subject to edit mode/enabled state. `ValueIsReadOnly` follows `GuiDocumentEditMode`. |
| `GuiMultilineTextBox` | Document | Value, Text and conditional Scroll. Value read-only state follows edit mode. |
| `GuiDocumentViewer`, `GuiDocumentLabel`, and XML virtual type `GuiDocumentTextBox` | Document | Text; viewer also has conditional Scroll. These classes do not receive Value merely because edit mode is Editable. |
| `GuiScrollView`, `GuiScrollContainer` | Pane unless a more-derived class supplies a role | Conditional Scroll, with percentages, view sizes and scrollable-axis flags. |
| `GuiScroll` | ProgressBar for ProgressBar theme; Slider for HTracker/VTracker; otherwise ScrollBar | RangeValue. Position, minimum 0, maximum `GetMaxPosition()`, small/big move; progress is read-only. Orientation is vertical for VScroll/VTracker and horizontal otherwise. |
| `GuiTab`, `GuiRibbonTab` | Tab | Selection; single selection required. Children are page controls, not template header buttons. |
| `GuiTabPage`, `GuiRibbonTabPage` | TabItem | SelectionItem; selected state and SelectionContainer reference the owner tab. Bounds use the header button whose Context references the page when found. |
| `GuiListControl` | List | ItemContainer and conditional Scroll. No Selection unless it is also a `GuiSelectableListControl`. |
| `GuiSelectableListControl`, `GuiVirtualTextList`, `GuiTextList`, `GuiBindableTextList` | List | Selection, ItemContainer and conditional Scroll. Multi-selection follows `GetMultiSelect()`; selection is not required. Text-list checked state is on logical items. |
| `GuiListViewBase`, `GuiVirtualListView`, `GuiListView`, `GuiBindableListView` | List; DataGrid for `GuiVirtualListView` in Detail view | Base list patterns; `GuiVirtualListView` adds MultipleView, and Detail adds Grid/Table. Six views: BigIcon, SmallIcon, List, Tile, Information, Detail. |
| `GuiVirtualDataGrid`, `GuiBindableDataGrid` | DataGrid | Selection, ItemContainer, conditional Scroll, and Grid/Table when `IDataGridView` is available or the inherited list view is in Detail. MultipleView is suppressed when `IDataGridView` is available; a custom model without that view uses the ordinary `GuiVirtualListView` availability rules. Cell editing and selection are described below. |
| `GuiVirtualTreeListControl`, `GuiVirtualTreeView`, `GuiTreeView`, `GuiBindableTreeView` | Tree | Selection, ItemContainer and conditional Scroll. Tree items come from the hierarchical node model. |
| `GuiDatePicker` | Calendar | Selection, Grid, Table and ItemContainer when a `templates::GuiCommonDatePickerLook` is found in its composition subtree. Calendar grid dimensions are six weeks by seven days. |
| `GuiMenuBar`, `GuiToolstripMenuBar` | MenuBar | No specific pattern. Not a content element. |
| `GuiToolstripToolBar`, `GuiRibbonToolstrips` | ToolBar | No specific pattern. |
| `GuiRibbonGroup`, `GuiRibbonButtons`, `GuiRibbonGallery` | Group | No specific pattern. `GuiBindableRibbonGalleryList` adds ExpandCollapse using its popup and `RequestedDropdown` event; it has no aggregate Selection pattern. |
| `GuiLabel` | Text | Name from text, no Text pattern. |
| Other `GuiControl`/`GuiCustomControl` descendants, including `GuiRibbonIconLabel` | Pane by fallback | No specific pattern. GroupBox theme maps to Group; MenuSplitter/ToolstripSplitter/ToolstripSplitterInMenu/RibbonSplitter map to Separator; RibbonToolstripHeader maps to Text. Visible associated child controls are collected through compositions. |

Class definitions are in [GuiBasicControls.h](../../../GacUI/Source/Application/Controls/GuiBasicControls.h), [GuiWindowControls.h](../../../GacUI/Source/Application/Controls/GuiWindowControls.h), [GuiButtonControls.h](../../../GacUI/Source/Controls/GuiButtonControls.h), [GuiContainerControls.h](../../../GacUI/Source/Controls/GuiContainerControls.h), [GuiScrollControls.h](../../../GacUI/Source/Controls/GuiScrollControls.h), [GuiDateTimeControls.h](../../../GacUI/Source/Controls/GuiDateTimeControls.h), the [list control package](../../../GacUI/Source/Controls/ListControlPackage/GuiListControls.h), the [toolstrip package](../../../GacUI/Source/Controls/ToolstripPackage/GuiMenuControls.h) and [GuiDocumentViewer.h](../../../GacUI/Source/Controls/TextEditorPackage/GuiDocumentViewer.h). Bindable subclasses use the same runtime class dispatch; reflection is not required by this implementation.

## Logical item inventory

`WindowsUIAutomationNodeKind` supplies semantic items independently of whether a list has realized their templates.

| Node kind | Role, name and parent | Patterns and properties |
| --- | --- | --- |
| Item in an ordinary list | ListItem; `IItemProvider::GetTextValue(row)`; parent is list | ScrollItem and VirtualizedItem; SelectionItem for a selectable list. Text lists also expose Toggle when view is not `TextListView::Text`. Both check and radio visual styles use the model's independent checked value. |
| Item in a Grid-capable list | DataItem row; item text; parent is grid | Same list-item patterns, without GridItem/TableItem. Children are logical Cells. |
| TreeNode | TreeItem; root provider `GetTextValue(node)`; actual tree parent or tree control | SelectionItem, ScrollItem, VirtualizedItem, ExpandCollapse. Leaf nodes report LeafNode and reject expand/collapse actions. |
| Cell | DataItem; first-column text or corresponding `IListViewItemView::GetSubItem`; parent is row Item | GridItem, TableItem, SelectionItem, ScrollItem, VirtualizedItem; Invoke when `IDataGridView::GetCellDataEditorFactory` returns a factory. Row/column, spans of 1, containing grid and column header association. |
| Header | Header; `Column headers`; parent is list/grid | No specific pattern; not content. Children are HeaderItems. |
| HeaderItem | HeaderItem; `GetColumnText(column)`; parent is Header | Invoke for every logical header; Transform when `IColumnItemView` exists. With that view, `ItemStatus` is an empty string, Ascending or Descending; otherwise it is `VT_EMPTY`. Resize sets column width; move/rotate are unavailable. |
| CalendarDay | ListItem; `year-month-day`; parent is calendar | SelectionItem, GridItem, TableItem. Position is week/day; selected state compares year/month/day with picker date. No ScrollItem or VirtualizedItem. |
| CalendarHeader | HeaderItem; locale short weekday name; parent is calendar | No specific pattern; not content; no composition/bounding rectangle. |

Items are cached by owner, kind, row, column and tree-node identity. List model changes shift the row indices of unaffected nodes; replaced/removed ranges retire their old nodes. Tree providers retain the model-node identity. Column rebuilds retire Cells and HeaderItems. Switching a list view away from Detail makes its Cells/Headers unavailable through `IsLive()`.

All list rows are enumerated, including offscreen rows. Tree enumeration follows expanded branches; ItemContainer search traverses the complete tree, including collapsed descendants. `FindItemByProperty` supports Name, AutomationId, SelectionItemIsSelected, ControlType, or property ID 0 for enumeration. `Realize()` expands tree ancestors and calls `EnsureItemVisible`; `ScrollIntoView()` delegates to the same operation. Thus UIA virtualization availability is not limited to items that currently lack a visual template.

## Pattern state and actions

The controls and items above receive the following implemented members through their available providers. This table also distinguishes explicit element-property support from provider-only state.

| Provider | Implemented state/actions |
| --- | --- |
| `IInvokeProvider` | Queued button BeforeClicked/Clicked/AfterClicked, column-click event, or cell `SelectCell(..., true)`. Raises Invoked before dispatching the action. |
| `IToggleProvider` | Toggle and ToggleState; explicit `ToggleToggleState` property; only On/Off. |
| `ISelectionProvider` | GetSelection, CanSelectMultiple, IsSelectionRequired; these state members have no explicit element-property cases. Text and Selection share the C++ GetSelection signature, dispatched by pattern support. |
| `ISelectionItemProvider` | Select/AddToSelection/RemoveFromSelection, IsSelected, SelectionContainer; explicit `SelectionItemIsSelected`. |
| `IExpandCollapseProvider` | Expand/Collapse, ExpandCollapseState; explicit `ExpandCollapseExpandCollapseState`. |
| `IValueProvider` | Whole-text SetValue, Value, IsReadOnly; explicit `ValueValue` and `ValueIsReadOnly`. |
| `IRangeValueProvider` | Numeric SetValue, Value, Minimum/Maximum, SmallChange/LargeChange, IsReadOnly; corresponding element-property cases. Numeric writes are converted to `vint` positions. |
| `IScrollProvider` | Scroll/SetScrollPercent, horizontal/vertical percentages and view sizes, scrollable flags; corresponding element-property cases. |
| `IScrollItemProvider`, `IVirtualizedItemProvider` | ScrollIntoView and Realize use the same list realization/visibility path. |
| `IGridProvider`, `IGridItemProvider` | GetItem, RowCount/ColumnCount; Row/Column, RowSpan/ColumnSpan, ContainingGrid. These members are provider-only in the property switch. |
| `ITableProvider`, `ITableItemProvider` | Column headers and the matching cell column header; empty row-header arrays; RowOrColumnMajor is RowMajor. These members are provider-only. |
| `IItemContainerProvider` | FindItemByProperty over logical model items. |
| `IMultipleViewProvider` | GetViewName, SetCurrentView, CurrentView and GetSupportedViews; explicit `MultipleViewCurrentView`. |
| `IWindowProvider` | SetVisualState, Close, WaitForInputIdle and window capability/state getters. WaitForInputIdle validates a nonnegative timeout and immediately reports true. These state getters have no explicit element-property cases. |
| `ITransformProvider` | Move/Resize/Rotate and CanMove/CanResize/CanRotate. Rotate always fails; window move/resize use physical coordinates, header resize converts physical width to GacUI units. State getters are provider-only. |
| `ITextProvider` | DocumentRange, GetSelection, SupportedTextSelection, GetVisibleRanges, RangeFromPoint and RangeFromChild. Ranges implement `ITextRangeProvider`, described below. |

No Text2, TextEdit, TextChild, Selection2, Transform2, LegacyIAccessible, spreadsheet or custom-navigation interfaces are declared by this provider.

## Text boxes and documents

The text provider adapts `GuiDocumentCommonInterface`, declared in [GuiDocumentCommonInterface.h](../../../GacUI/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h). [GuiDocumentViewer.h/.cpp](../../../GacUI/Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp) implements both textbox families on that same document infrastructure: `GuiSinglelineTextBox` derives from `GuiDocumentLabel`; `GuiMultilineTextBox` derives from `GuiDocumentViewer`. The XML `GuiDocumentTextBox` is a virtual type for `GuiDocumentLabel`, registered in [GuiInstanceLoader_Document.cpp](../../../GacUI/Source/Compiler/InstanceLoaders/GuiInstanceLoader_Document.cpp).

`GuiDocumentEditMode` controls the exposed operations. ViewOnly returns no selection and SupportedTextSelection=None. Selectable and Editable return one range, including a degenerate caret range, and SupportedTextSelection=Single. Text remains available in read-only modes. Value setters require Editable. Text range selection calls `SetCaret`; it does not perform an edit. Whole-text Value replacement calls the control's `SetText`. There is no UIA range deletion, insertion or clipboard operation in this implementation.

The text adapter has these concrete behaviors:

- `WindowsUIAutomationTextSnapshot` flattens document runs into a string and parallel `TextPos` offsets. It inserts one CRLF between model paragraphs and U+FFFC for each nontext content run. Named embedded runs also map their names to offsets.
- Offsets count Windows UTF-16 code units. CRLF and low-surrogate offsets map to the preceding caret position, including surrogate pairs across run boundaries.
- Text ranges retain their originating `DocumentModel`. Replacing that model invalidates a retained range. Edits within the same model adjust endpoints using the common text prefix/suffix and clamp them to the new text length; this is not an edit-history tracker.
- Character boundaries follow distinct caret positions. Word boundaries use `iswalnum`/`iswspace` transitions. Line boundaries use paragraph or rendered caret-y changes; Paragraph uses model rows. Format compares eight style attributes. Page and Document have only whole-document boundaries.
- Supported attributes are IsReadOnly, HorizontalTextAlignment, FontName, FontSize, FontWeight, IsItalic, UnderlineStyle, StrikethroughStyle, ForegroundColor and BackgroundColor. They come from style/alignment summaries, return the mixed sentinel when the summary lacks one value, and use the not-supported sentinel for other attributes. Font size is converted with `size * 72 / 96`.
- `GetBoundingRectangles` builds and merges visible character rectangles using caret bounds, excludes CR/LF, and clips them to the node rectangle. Degenerate ranges produce no rectangles. `RangeFromPoint` converts physical-screen coordinates back to document coordinates and returns a caret range.
- `GetVisibleRanges` scans visible caret rectangles and groups contiguous offsets. `ScrollIntoView` adjusts only the vertical position of a `GuiScrollView`; it does not scroll a single-line label-based editor horizontally.
- Named embedded controls become document children through `GetDocumentItems()` in text order. Range `GetChildren` filters them to its offset interval; `RangeFromChild` returns the one-character placeholder range. Plain image runs and hyperlink runs do not get dedicated semantic nodes from this adapter. `GetEnclosingElement` always returns the document provider.

Password mode is special to `GuiSinglelineTextBox`: Name remains empty, Text is unavailable, the Value element property stays empty, and `IValueProvider::get_Value` returns `E_ACCESSDENIED`. The Value setter still follows the ordinary Editable/enabled checks.

## Combo boxes and menu popups

[GuiComboControls.cpp](../../../GacUI/Source/Controls/ListControlPackage/GuiComboControls.cpp) implements `GuiComboBoxBase` as a `GuiMenuButton` that creates a `GuiMenu` submenu. `GuiComboButton` inserts arbitrary dropdown content; `GuiComboBoxListControl` inserts its selectable list into the submenu container. [GuiDateTimeControls.cpp](../../../GacUI/Source/Controls/GuiDateTimeControls.cpp) inserts a date picker for `GuiDateComboBox`.

The UIA mapping currently recognizes a combo's role, ExpandCollapse and list selection, but **does not attach its dropdown subtree to the combo node**. `Children()` suppresses ordinary `GuiButton` descendants and has no combo-specific dropdown branch. `Parent()` does not redirect a combo submenu or contained list to its combo. Standard Windows mode exposes the submenu through its own HWND root, as it does other menus. Hosted mode adds every opening logical child window, including that menu, directly below the main-window root.

The current standard-window shape is therefore a ComboBox in its owner's fragment and a separate Menu fragment containing the List/Calendar/other dropdown control. Hosted mode instead has the ComboBox and opening Menu in different branches below the main root. Ordinary menus also use separate HWND roots in standard mode and main-root children in hosted mode; submenu host buttons do not enumerate their submenu as a child.

List-combo selection has a separate explicit bridge: `UiaCombo()` finds a registered combo whose contained list owns the item. Item selected state comes from `GetSelectedIndex()`, SelectionContainer becomes the combo, and selecting an item updates `SetSelectedIndex()` and closes the popup. The combo's `GetSelection()` searches the contained list even though the combo does not enumerate it as a child. This selection association is not a parent/child association.

The intended combo-owned dropdown tree requested for accessibility review is consequently an outstanding gap, not an implemented feature. The [review task](../../../GacUI/TODO_Task_UiaReview.md) evaluates that requirement and menu topology separately. Name is currently empty for list, date and arbitrary-content combos, and no Value is supplied for their selected display text.

## Data grid cells, visualizers and editors

[GuiDataGridControls.cpp](../../../GacUI/Source/Controls/ListControlPackage/GuiDataGridControls.cpp) and [GuiDataGridInterfaces.h](../../../GacUI/Source/Controls/ListControlPackage/GuiDataGridInterfaces.h) define `GuiVirtualDataGrid`, `list::DefaultDataGridItemTemplate`, `list::IDataGridView`, `list::IDataVisualizer` and `list::IDataEditor`. The semantic hierarchy for the default grid is:

```text
DataGrid
  Header
    HeaderItem (column)
  DataItem (row)
    DataItem (cell: row, column)
      controls from the active editor, or controls from the visualizer
```

The row Item creates one Cell per model column. Cell identity and GridItem/TableItem relationships remain on the logical Cell, independently of its visible contents. `GetItem(row, column)` returns that logical node without requiring a realized template. Cell bounds use the actual `GuiCellComposition` when the realized row is a `DefaultDataGridItemTemplate`; other list-view detail cells derive bounds from the row and column widths.

When the selected cell has an opened editor, `Cell.Children()` collects associated controls from the editor template. Otherwise it collects them from the cell composition. The default template physically puts the editor inside that cell and hides the visualizer while editing; closing the editor removes it and restores the visualizer. `Parent()` recognizes a control's enclosing `GuiCellComposition` under a default grid item and returns the matching logical Cell. Thus the active visualizer/editor controls are inside the cell in the UIA tree, rather than siblings of the row or grid. Plain graphics/compositions without an associated control do not become extra semantic children.

Selecting a Cell calls `SelectCell(position, false)`; invoking an editable Cell queues `SelectCell(position, true)`. Editing needs a realized `DefaultDataGridItemTemplate` and an editor factory. The queued UIA Invoke path ignores the boolean result, so returning success does not prove an editor opened; offscreen callers can first use ScrollItem/VirtualizedItem and then inspect the Cell's children. Custom template layouts are not covered by a general composition-override API.

Current selection has two levels: row Item selection follows the list, while a data-grid Cell compares `GetSelectedCell()`. The container's `GetSelection()` collects direct selected rows and recurses only through tree nodes, so it does not return the selected Cell. This distinction, editor replacement eventing and retained editor-provider behavior are review targets. The cell Name continues to come from the item view while its children switch between visualizer and editor.

## Tree lifetime, events and limits

Ordinary containers collect visible associated controls through intervening compositions. Buttons and scroll controls suppress their template children; lists synthesize logical items instead of exposing background buttons; documents expose named embedded controls; tabs expose their pages. There is no generic composition internal-property override in the current UIA code, so a layout table or a composition-only splitter does not acquire UIA semantics by itself.

`Control()` stores one `WindowsUIAutomationLifetime` under the control internal-property key `GacUI.Windows.UIAutomation`. Binding a window scans its composition tree before clients enumerate children. Window composition updates trigger another deferred scan, including controls added later. The retained lifetime object avoids duplicate control event hookups, attaches list/tree/column callbacks and retires nodes on destruction. `IsLive()` rejects disposed controls, stopped contexts, retired nodes, unavailable detail nodes and controls without a registered related `GuiWindow`. Provider retirement queues `UiaDisconnectProvider`; immutable runtime IDs remain readable during disconnection.

Notifications are posted through the UI dispatcher and coalesced per node. `Notify()` compares a watched subset of element properties, raises PropertyChanged for differences, optionally raises ChildrenInvalidated, and then raises queued automation events. This is the actual event inventory:

| Trigger | Notification path |
| --- | --- |
| Control text, visibility, enabled state, template or focus changes | Property comparison; structure invalidation for visibility/template; TextChanged for document text and AutomationFocusChanged when focus becomes true. |
| Window open/close and child composition updates | Window/Menu/ToolTip event IDs according to actual class, structure invalidation, plus main-root invalidation for hosted child windows. |
| Selectable-button state/group/auto-selection; menu opening/shortcut text | Property comparison; submenu opening also requests structure invalidation. |
| Scroll position/total/page size | Property comparison for scroll control and nearest ancestor scroll view. |
| List item/template/arranger/column changes; tree expansion/model changes | Logical-node maintenance and structure invalidation. |
| List selection | Selection_Invalidated on the list. Explicit UIA item selection also queues ElementSelected or ElementRemovedFromSelection on the item. |
| Grid selected-cell changes; date navigation | Structure invalidation. Date changes also compare properties. |
| Document caret/selection changes | Text_TextSelectionChanged. |
| UIA Invoke | Invoke_Invoked before the queued action. |

The watched property list includes Name, AcceleratorKey, ItemStatus, Orientation, Enabled/Offscreen/Focus/Content flags, Value, Toggle, selected/expanded state, range and scroll state, current view, ControlType and selected pattern-availability properties. It does not cover every readable property or provider member: bounds, password state, window state, grid counts and selection policy have no watched entries. Edit-mode and password changes also have no dedicated hook in this module. These are facts about present notification coverage; the [review task](../../../GacUI/TODO_Task_UiaReview.md) specifies the contract checks still needed.

For the underlying layout, model and platform mechanisms, see [List Control Architecture](./KB_GacUI_Design_ListControlArchitecture.md), [Hosted Mode Window Management](./KB_GacUI_Design_HostedModeWindowManagement.md), [Control Focus Switching](./KB_GacUI_Design_ControlFocusSwitchingAndTabAltHandling.md) and [Platform Initialization](./KB_GacUI_Design_PlatformInitialization.md).

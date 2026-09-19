#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using namespace compositions;
	using Kind = WindowsUIAutomationNodeKind;

	list::IListViewItemView* UiaListView(GuiControl* control)
	{
		auto list = dynamic_cast<GuiListViewBase*>(control);
		return list ? dynamic_cast<list::IListViewItemView*>(list->GetItemProvider()->RequestView(WString::Unmanaged(list::IListViewItemView::Identifier))) : nullptr;
	}
	list::IDataGridView* UiaDataGrid(GuiControl* control)
	{
		auto grid = dynamic_cast<GuiVirtualDataGrid*>(control);
		return grid ? dynamic_cast<list::IDataGridView*>(grid->GetItemProvider()->RequestView(WString::Unmanaged(list::IDataGridView::Identifier))) : nullptr;
	}
	list::IColumnItemView* UiaColumns(GuiControl* control)
	{
		auto list = dynamic_cast<GuiListViewBase*>(control);
		return list ? dynamic_cast<list::IColumnItemView*>(list->GetItemProvider()->RequestView(WString::Unmanaged(list::IColumnItemView::Identifier))) : nullptr;
	}
	GuiDocumentCommonInterface* UiaDocument(GuiControl* control) { return dynamic_cast<GuiDocumentCommonInterface*>(control); }
	GuiSelectableButton::MutexGroupController* UiaRadioController(GuiControl* control)
	{
		auto button = dynamic_cast<GuiSelectableButton*>(control);
		if (!button) return nullptr;
		for (auto composition = control->GetBoundsComposition()->GetParent(); composition; composition = composition->GetParent())
		{
			// Item and cell templates contain application controls. The surrounding
			// list does not turn their radio groups into implementation details.
			if (dynamic_cast<templates::GuiGridCellTemplate*>(composition) || dynamic_cast<templates::GuiListItemTemplate*>(composition)) break;
			auto parent = composition->GetAssociatedControl();
			if (dynamic_cast<GuiTabPage*>(parent)) break;
			if (dynamic_cast<GuiTab*>(parent) || dynamic_cast<GuiListControl*>(parent) || dynamic_cast<GuiDatePicker*>(parent) || dynamic_cast<GuiButton*>(parent)) return nullptr;
		}
		return dynamic_cast<GuiSelectableButton::MutexGroupController*>(button->GetGroupController());
	}
	Ptr<WindowsUIAutomationNode> UiaRadioGroup(WindowsUIAutomationNode* node)
	{
		auto group = UiaRadioController(node->control);
		if (!group) return nullptr;
		Ptr<WindowsUIAutomationNode> anchor;
		for (auto candidate : node->context->nodes)
			if (candidate->kind == Kind::Control && candidate->IsLive() && UiaRadioController(candidate->control) == group) { anchor = candidate; break; }
		return anchor ? node->context->Item(anchor, Kind::RadioGroup) : nullptr;
	}
	GuiComboBoxListControl* UiaCombo(WindowsUIAutomationNode* node)
	{
		if (!dynamic_cast<GuiSelectableListControl*>(node->control)) return nullptr;
		for (auto candidate : node->context->combos)
		{
			if (candidate->kind != Kind::Control || !candidate->IsLive()) continue;
			if (auto combo = dynamic_cast<GuiComboBoxListControl*>(candidate->control); combo && combo->GetContainedListControl() == node->control) return combo;
		}
		return nullptr;
	}
	GuiControl* UiaPopupOwner(WindowsUIAutomationNode* node)
	{
		if (dynamic_cast<GuiTooltip*>(node->control)) return GetApplication()->GetTooltipOwner();
		if (!dynamic_cast<GuiMenu*>(node->control)) return nullptr;
		for (auto candidate : node->context->nodes)
		{
			if (candidate->kind != Kind::Control || !candidate->IsLive()) continue;
			if (auto menu = dynamic_cast<GuiMenuButton*>(candidate->control); menu && menu->GetSubMenu() == node->control) return menu;
			if (auto gallery = dynamic_cast<GuiBindableRibbonGalleryList*>(candidate->control); gallery && gallery->GetSubMenu() == node->control) return gallery;
		}
		return nullptr;
	}

	template<typename T>
	T* UiaFindComposition(GuiGraphicsComposition* composition)
	{
		if (auto result = dynamic_cast<T*>(composition)) return result;
		for (auto child : composition->Children()) if (auto result = UiaFindComposition<T>(child)) return result;
		return nullptr;
	}
	templates::GuiCommonDatePickerLook* UiaCalendar(GuiControl* control)
	{
		return dynamic_cast<GuiDatePicker*>(control) ? UiaFindComposition<templates::GuiCommonDatePickerLook>(control->GetBoundsComposition()) : nullptr;
	}

	bool UiaSpatialColumnMajor(GuiControl* control)
	{
		auto list = dynamic_cast<GuiListControl*>(control);
		return list && dynamic_cast<list::FixedHeightMultiColumnItemArranger*>(list->GetArranger());
	}

	Size UiaSpatialGrid(GuiControl* control)
	{
		auto list = dynamic_cast<GuiListControl*>(control);
		if (!list) return Size(-1, -1);
		auto count = list->GetItemProvider()->Count();
		if (dynamic_cast<list::FixedSizeMultiColumnItemArranger*>(list->GetArranger()))
		{
			auto repeat = UiaFindComposition<GuiRepeatFixedSizeMultiColumnItemComposition>(control->GetBoundsComposition());
			if (!repeat) return Size(-1, -1);
			auto columns = repeat->GetColumnCount();
			return Size(columns, (count + columns - 1) / columns);
		}
		if (UiaSpatialColumnMajor(control))
		{
			auto repeat = UiaFindComposition<GuiRepeatFixedHeightMultiColumnItemComposition>(control->GetBoundsComposition());
			if (!repeat) return Size(-1, -1);
			auto rows = repeat->GetRowCount();
			return Size((count + rows - 1) / rows, rows);
		}
		return Size(-1, -1);
	}

	GridPos UiaGridPosition(WindowsUIAutomationNode* node)
	{
		if (node->kind != Kind::Item) return GridPos(node->row, node->column);
		auto size = UiaSpatialGrid(node->control);
		return UiaSpatialColumnMajor(node->control)
			? GridPos(node->row % size.y, node->row / size.y)
			: GridPos(node->row / size.x, node->row % size.x);
	}

	bool WindowsUIAutomationNode::IsLive()
	{
		if (retired || !context || context->stopped || disposed->IsDisposed()) return false;
		for (auto parent = control->GetParent(); parent; parent = parent->GetParent())
			if (auto text = dynamic_cast<GuiSinglelineTextBox*>(parent); text && text->GetPasswordChar()) return false;
		if (owner && !owner->IsLive()) return false;
		if (kind == Kind::DocumentObject)
		{
			vint begin, end;
			if (!UiaDocumentObjectRange(this, begin, end)) return false;
		}
		auto window = Window();
		if (!window || !GetApplication() || !GetApplication()->GetWindows().Contains(window)) return false;
		if (kind == Kind::Cell || kind == Kind::Header || kind == Kind::HeaderItem)
		{
			if (!UiaListView(control)) return false;
			auto list = dynamic_cast<GuiVirtualListView*>(control);
			if (list && list->GetView() != ListViewView::Detail) return false;
			if (column >= UiaListView(control)->GetColumnCount()) return false;
		}
		return true;
	}
	GuiControl* WindowsUIAutomationNode::Control() { return control; }
	GuiWindow* WindowsUIAutomationNode::Window()
	{
		return dynamic_cast<GuiWindow*>(control->GetRelatedControlHost());
	}
	HWND WindowsUIAutomationNode::Handle()
	{
		auto native = context->hosted ? GetHostedApplication()->GetNativeWindowHost() : Window()->GetNativeWindow();
		auto form = dynamic_cast<IWindowsForm*>(native);
		return form ? form->GetWindowHandle() : nullptr;
	}
	bool WindowsUIAutomationNode::IsRoot()
	{
		return kind == Kind::Control && dynamic_cast<GuiWindow*>(control) && (!context->hosted || control == GetApplication()->GetMainWindow());
	}
	NativePoint WindowsUIAutomationNode::ScreenOrigin()
	{
		auto origin = Window()->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
		if (context->hosted)
		{
			auto hostOrigin = GetHostedApplication()->GetNativeWindowHost()->GetClientBoundsInScreen().LeftTop();
			origin.x += hostOrigin.x;
			origin.y += hostOrigin.y;
		}
		return origin;
	}

	vint WindowsUIAutomationNode::ItemIndex()
	{
		if (treeNode) return dynamic_cast<GuiVirtualTreeListControl*>(control)->GetNodeItemView()->CalculateNodeVisibilityIndex(treeNode.Obj());
		return row;
	}

	GuiGraphicsComposition* WindowsUIAutomationNode::Composition()
	{
		if (kind == Kind::TabContent) return control->GetBoundsComposition();
		if (kind == Kind::Control)
		{
			if (auto page = dynamic_cast<GuiTabPage*>(control))
			{
				// Header controls already carry the page as Context in the template.
				Func<GuiGraphicsComposition*(GuiGraphicsComposition*)> findHeader;
				findHeader = [&](GuiGraphicsComposition* composition) -> GuiGraphicsComposition*
				{
					if (auto button = dynamic_cast<GuiButton*>(composition->GetAssociatedControl()); button && button->GetContext().GetRawPtr() == page) return composition;
					if (dynamic_cast<GuiTabPage*>(composition->GetAssociatedControl())) return nullptr;
					for (auto child : composition->Children()) if (auto header = findHeader(child)) return header;
					return nullptr;
				};
				if (auto header = findHeader(page->GetOwnerTab()->GetBoundsComposition())) return header;
			}
			return control->GetBoundsComposition();
		}
		if (kind == Kind::CalendarDay)
		{
			auto calendar = UiaCalendar(control);
			return calendar ? calendar->GetDayButton(row, column)->GetBoundsComposition() : nullptr;
		}
		if (kind == Kind::CalendarHeader)
		{
			// The common look places weekday labels directly above its six week rows.
			auto calendar = UiaCalendar(control);
			if (!calendar) return nullptr;
			auto cell = dynamic_cast<GuiCellComposition*>(calendar->GetDayButton(0, column)->GetBoundsComposition()->GetParent());
			auto table = cell ? dynamic_cast<GuiTableComposition*>(cell->GetParent()) : nullptr;
			return table ? table->GetSitedCell(cell->GetRow() - 1, column) : nullptr;
		}
		auto listControl = dynamic_cast<GuiListControl*>(control);
		if (!listControl) return nullptr;
		if (kind == Kind::Header || kind == Kind::HeaderItem)
		{
			auto arranger = dynamic_cast<list::ListViewColumnItemArranger*>(listControl->GetArranger());
			if (!arranger || arranger->GetColumnButtons().Count() == 0) return nullptr;
			auto button = arranger->GetColumnButtons()[kind == Kind::Header ? 0 : column];
			return kind == Kind::Header ? button->GetBoundsComposition()->GetParent() : button->GetBoundsComposition();
		}
		auto index = ItemIndex();
		auto style = index < 0 ? nullptr : listControl->GetArranger()->GetVisibleStyle(index);
		if (kind == Kind::Cell && dynamic_cast<list::DefaultDataGridItemTemplate*>(style))
		{
			for (auto child : style->Children())
				if (auto table = dynamic_cast<GuiTableComposition*>(child); table && column < table->GetColumns()) return table->GetSitedCell(0, column);
		}
		return style;
	}

	UiaRect WindowsUIAutomationNode::Bounds()
	{
		if (kind == Kind::RadioGroup)
		{
			UiaRect result = {};
			for (auto child : Children())
			{
				auto bounds = child->Bounds();
				if (bounds.width <= 0 || bounds.height <= 0) continue;
				if (result.width <= 0) result = bounds;
				else
				{
					auto right = max(result.left + result.width, bounds.left + bounds.width);
					auto bottom = max(result.top + result.height, bounds.top + bounds.height);
					result.left = min(result.left, bounds.left);
					result.top = min(result.top, bounds.top);
					result.width = right - result.left;
					result.height = bottom - result.top;
				}
			}
			return result;
		}
		if (kind == Kind::DocumentObject) return UiaDocumentObjectBounds(this);
		if (!Window()->GetOpening()) return {};
		auto composition = Composition();
		if (!composition || !composition->GetEventuallyVisible()) return {};
		auto bounds = composition->GetGlobalBounds();
		if (kind == Kind::Cell && !dynamic_cast<GuiCellComposition*>(composition))
		{
			auto view = dynamic_cast<list::IColumnItemView*>(dynamic_cast<GuiListControl*>(control)->GetItemProvider()->RequestView(WString::Unmanaged(list::IColumnItemView::Identifier)));
			if (view)
			{
				vint x = 0;
				for (vint c = 0; c < column; c++) x += view->GetColumnSize(c);
				bounds.x1 += x;
				bounds.x2 = min(bounds.x2, bounds.x1 + view->GetColumnSize(column));
			}
		}
		for (auto parent = composition->GetParent(); parent; parent = parent->GetParent())
		{
			auto clip = parent->GetGlobalBounds();
			bounds.x1 = max(bounds.x1, clip.x1);
			bounds.y1 = max(bounds.y1, clip.y1);
			bounds.x2 = min(bounds.x2, clip.x2);
			bounds.y2 = min(bounds.y2, clip.y2);
		}
		if (bounds.Width() <= 0 || bounds.Height() <= 0) return {};
		auto native = Window()->GetNativeWindow();
		auto origin = ScreenOrigin();
		auto position = native->Convert(bounds.LeftTop());
		auto size = native->Convert(bounds.GetSize());
		return { (double)(origin.x.value + position.x.value), (double)(origin.y.value + position.y.value), (double)size.x.value, (double)size.y.value };
	}

	WString WindowsUIAutomationNode::Name()
	{
		if (kind == Kind::Control)
		{
			if (auto metadata = UiaMetadata(control))
			{
				if (metadata->name) return metadata->name.Value();
				if (metadata->label && !metadata->labelDisposed->IsDisposed()) return metadata->label->GetText();
			}
			if (auto tooltip = dynamic_cast<GuiTooltip*>(control)) return UiaTooltipText(tooltip->GetTemporaryContentControl());
		}
		switch (kind)
		{
		case Kind::RadioGroup: return L"";
		case Kind::DocumentObject: return UiaDocumentObjectName(this);
		case Kind::TreeNode: return dynamic_cast<GuiVirtualTreeListControl*>(control)->GetNodeRootProvider()->GetTextValue(treeNode.Obj());
		case Kind::Item: return dynamic_cast<GuiListControl*>(control)->GetItemProvider()->GetTextValue(row);
		case Kind::Cell: return column == 0 ? UiaListView(control)->GetText(row) : UiaListView(control)->GetSubItem(row, column - 1);
		case Kind::Header: return UiaLocalizedText(control, L"ColumnHeaders", L"Column headers");
		case Kind::HeaderItem: return UiaListView(control)->GetColumnText(column);
		case Kind::CalendarHeader: return dynamic_cast<GuiDatePicker*>(control)->GetDateLocale().GetShortDayOfWeekName(column);
		case Kind::TabContent: return L"";
		case Kind::CalendarDay:
			{
				auto date = UiaCalendar(control)->GetDateOfDayButton(row, column);
				return itow(date.year) + L"-" + itow(date.month) + L"-" + itow(date.day);
			}
		default:
			// Input/document contents are exposed by Value/Text/Selection, not a label.
			if (UiaDocument(control) || dynamic_cast<GuiComboBoxBase*>(control)) return L"";
			return control->GetText();
		}
	}

	CONTROLTYPEID WindowsUIAutomationNode::Role()
	{
		if (kind == Kind::RadioGroup) return UIA_GroupControlTypeId;
		if (kind == Kind::DocumentObject) return documentRun.Cast<DocumentHyperlinkRun>() ? UIA_HyperlinkControlTypeId : UIA_ImageControlTypeId;
		switch (kind)
		{
		case Kind::Item: return owner->Supports(UIA_TablePatternId) ? UIA_DataItemControlTypeId : UIA_ListItemControlTypeId;
		case Kind::TreeNode: return UIA_TreeItemControlTypeId;
		case Kind::Cell: return UIA_DataItemControlTypeId;
		case Kind::Header: return UIA_HeaderControlTypeId;
		case Kind::HeaderItem: case Kind::CalendarHeader: return UIA_HeaderItemControlTypeId;
		case Kind::CalendarDay: return UIA_ListItemControlTypeId;
		case Kind::TabContent: return UIA_PaneControlTypeId;
		default: break;
		}
		if (dynamic_cast<GuiTooltip*>(control)) return UIA_ToolTipControlTypeId;
		if (dynamic_cast<GuiMenu*>(control)) return control->GetControlThemeName() == theme::ThemeName::RibbonGroupMenu ? UIA_GroupControlTypeId : UIA_MenuControlTypeId;
		if (dynamic_cast<GuiPopup*>(control)) return UIA_PaneControlTypeId;
		if (dynamic_cast<GuiWindow*>(control)) return UIA_WindowControlTypeId;
		if (dynamic_cast<GuiVirtualDataGrid*>(control)) return UIA_DataGridControlTypeId;
		if (auto listView = dynamic_cast<GuiVirtualListView*>(control); listView && listView->GetView() == ListViewView::Detail) return UIA_DataGridControlTypeId;
		if (dynamic_cast<GuiVirtualTreeListControl*>(control)) return UIA_TreeControlTypeId;
		if (dynamic_cast<GuiListControl*>(control)) return UIA_ListControlTypeId;
		if (dynamic_cast<GuiSinglelineTextBox*>(control)) return UIA_EditControlTypeId;
		if (UiaDocument(control)) return UIA_DocumentControlTypeId;
		if (dynamic_cast<GuiComboBoxBase*>(control)) return UIA_ComboBoxControlTypeId;
		if (dynamic_cast<GuiDatePicker*>(control)) return UIA_CalendarControlTypeId;
		if (auto menu = dynamic_cast<GuiMenuButton*>(control))
		{
			if (menu->GetSubMenu()) return menu->GetSubMenuHost() != menu ? UIA_SplitButtonControlTypeId : UIA_MenuItemControlTypeId;
			for (auto parent = menu->GetParent(); parent; parent = parent->GetParent())
			{
				if (dynamic_cast<GuiToolstripToolBar*>(parent)) return UIA_ButtonControlTypeId;
				if (dynamic_cast<GuiMenu*>(parent) || dynamic_cast<GuiMenuBar*>(parent)) return UIA_MenuItemControlTypeId;
			}
			return UIA_ButtonControlTypeId;
		}
		if (dynamic_cast<GuiMenuBar*>(control)) return UIA_MenuBarControlTypeId;
		if (dynamic_cast<GuiToolstripToolBar*>(control) || dynamic_cast<GuiRibbonToolstrips*>(control)) return UIA_ToolBarControlTypeId;
		if (dynamic_cast<GuiRibbonGroup*>(control) || dynamic_cast<GuiRibbonButtons*>(control) || dynamic_cast<GuiRibbonGallery*>(control)) return UIA_GroupControlTypeId;
		if (auto button = dynamic_cast<GuiSelectableButton*>(control)) return dynamic_cast<GuiSelectableButton::MutexGroupController*>(button->GetGroupController()) ? UIA_RadioButtonControlTypeId : UIA_CheckBoxControlTypeId;
		if (dynamic_cast<GuiButton*>(control)) return UIA_ButtonControlTypeId;
		if (dynamic_cast<GuiTabPage*>(control)) return UIA_TabItemControlTypeId;
		if (dynamic_cast<GuiTab*>(control)) return UIA_TabControlTypeId;
		if (dynamic_cast<GuiScroll*>(control))
		{
			switch (control->GetControlThemeName())
			{
			case theme::ThemeName::ProgressBar: return UIA_ProgressBarControlTypeId;
			case theme::ThemeName::HTracker: case theme::ThemeName::VTracker: return UIA_SliderControlTypeId;
			default: return UIA_ScrollBarControlTypeId;
			}
		}
		if (dynamic_cast<GuiLabel*>(control)) return UIA_TextControlTypeId;
		switch (control->GetControlThemeName())
		{
		case theme::ThemeName::GroupBox: return UIA_GroupControlTypeId;
		case theme::ThemeName::MenuSplitter: case theme::ThemeName::ToolstripSplitter: case theme::ThemeName::ToolstripSplitterInMenu: case theme::ThemeName::RibbonSplitter: return UIA_SeparatorControlTypeId;
		case theme::ThemeName::RibbonToolstripHeader: return UIA_TextControlTypeId;
		default: break;
		}
		return UIA_PaneControlTypeId;
	}

	bool WindowsUIAutomationNode::Supports(PATTERNID pattern)
	{
		if (kind == Kind::RadioGroup) return pattern == UIA_SelectionPatternId;
		if (kind == Kind::DocumentObject) return pattern == UIA_InvokePatternId && documentRun.Cast<DocumentHyperlinkRun>();
		auto button = dynamic_cast<GuiSelectableButton*>(control);
		auto mutex = button ? dynamic_cast<GuiSelectableButton::MutexGroupController*>(button->GetGroupController()) : nullptr;
		auto list = dynamic_cast<GuiListControl*>(control);
		auto listView = dynamic_cast<GuiVirtualListView*>(control);
		auto text = dynamic_cast<GuiVirtualTextList*>(control);
		auto tabPage = dynamic_cast<GuiTabPage*>(control);
		auto menu = dynamic_cast<GuiMenuButton*>(control);
		auto document = UiaDocument(control);
		auto calendar = UiaCalendar(control);
		if (kind != Kind::Control)
		{
			switch (pattern)
			{
			case UIA_SelectionItemPatternId: return kind == Kind::CalendarDay || dynamic_cast<GuiSelectableListControl*>(control) && (kind == Kind::TreeNode || (UiaDataGrid(control) ? kind == Kind::Cell : kind == Kind::Item));
			case UIA_ScrollItemPatternId: case UIA_VirtualizedItemPatternId: return list && row >= 0 || kind == Kind::TreeNode;
			case UIA_ExpandCollapsePatternId: return kind == Kind::TreeNode;
			case UIA_TogglePatternId: return kind == Kind::Item && text && text->GetView() != TextListView::Text;
			case UIA_GridItemPatternId: return kind == Kind::Cell || kind == Kind::CalendarDay || kind == Kind::Item && UiaSpatialGrid(control).x >= 0;
			case UIA_TableItemPatternId: return kind == Kind::Cell || kind == Kind::CalendarDay;
			case UIA_TransformPatternId: return kind == Kind::HeaderItem && UiaColumns(control);
			case UIA_ValuePatternId:
				if (kind == Kind::Cell) if (auto grid = UiaDataGrid(control))
					return grid->GetBindingCellValue(row, column).GetBoxedValue().Cast<reflection::description::IValueType::TypedBox<WString>>() != nullptr;
				return false;
			case UIA_InvokePatternId: return kind == Kind::Cell && UiaDataGrid(control) && UiaDataGrid(control)->GetCellDataEditorFactory(row, column) || kind == Kind::HeaderItem;
			default: return false;
			}
		}
		switch (pattern)
		{
		case UIA_InvokePatternId:
			if (dynamic_cast<GuiComboBoxBase*>(control)) return false;
			if (menu) return Role() == UIA_SplitButtonControlTypeId || !menu->GetAutoSelection() && !menu->GetGroupController() && !menu->GetSubMenu();
			return dynamic_cast<GuiButton*>(control) && !button;
		case UIA_TogglePatternId: return button && !mutex && (!menu || menu->GetAutoSelection());
		case UIA_SelectionItemPatternId: return tabPage || mutex;
		case UIA_SelectionPatternId: return dynamic_cast<GuiSelectableListControl*>(control) || dynamic_cast<GuiTab*>(control) || calendar || dynamic_cast<GuiComboBoxListControl*>(control);
		case UIA_ExpandCollapsePatternId: return menu && menu->GetSubMenu() || dynamic_cast<GuiBindableRibbonGalleryList*>(control);
		case UIA_ValuePatternId: return document != nullptr;
		case UIA_TextPatternId:
			if (auto single = dynamic_cast<GuiSinglelineTextBox*>(control); single && single->GetPasswordChar()) return false;
			return document != nullptr;
		case UIA_ScrollItemPatternId:
			for (auto parent = control->GetParent(); parent; parent = parent->GetParent())
				if (dynamic_cast<GuiScrollView*>(parent)) return true;
			return false;
		case UIA_RangeValuePatternId: return dynamic_cast<GuiScroll*>(control);
		case UIA_ScrollPatternId:
			if (auto view = dynamic_cast<GuiScrollView*>(control)) return view->GetHorizontalScroll() && view->GetVerticalScroll();
			return false;
		case UIA_GridPatternId: return UiaSpatialGrid(control).x >= 0 || Supports(UIA_TablePatternId);
		case UIA_TablePatternId: return UiaDataGrid(control) || listView && listView->GetView() == ListViewView::Detail || calendar;
		case UIA_ItemContainerPatternId: return list || calendar;
		case UIA_MultipleViewPatternId: return listView && !UiaDataGrid(control);
		case UIA_WindowPatternId: case UIA_TransformPatternId: return dynamic_cast<GuiWindow*>(control) && !dynamic_cast<GuiPopup*>(control);
		default: return false;
		}
	}

	bool WindowsUIAutomationNode::IsSelected()
	{
		if (kind == Kind::CalendarDay)
		{
			auto a = dynamic_cast<GuiDatePicker*>(control)->GetDate();
			auto b = UiaCalendar(control)->GetDateOfDayButton(row, column);
			return a.year == b.year && a.month == b.month && a.day == b.day;
		}
		if (kind == Kind::Cell)
		{
			if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(control)) return grid->GetSelectedCell() == GridPos(row, column);
		}
		if (kind == Kind::Item || kind == Kind::Cell || kind == Kind::TreeNode)
		{
			if (auto combo = UiaCombo(this)) return combo->GetSelectedIndex() == ItemIndex();
			auto list = dynamic_cast<GuiSelectableListControl*>(control);
			return list && ItemIndex() >= 0 && list->GetSelected(ItemIndex());
		}
		if (auto page = dynamic_cast<GuiTabPage*>(control)) return page->GetOwnerTab()->GetSelectedPage() == page;
		if (auto button = dynamic_cast<GuiSelectableButton*>(control)) return button->GetSelected();
		return false;
	}

	bool WindowsUIAutomationNode::IsFocusable()
	{
		if (kind == Kind::CalendarDay) return UiaCalendar(control)->GetDayButton(row, column)->GetFocusableComposition() != nullptr;
		return control->GetFocusableComposition() && (kind == Kind::Control || kind == Kind::Cell && UiaDataGrid(control) && IsSelected());
	}

	bool WindowsUIAutomationNode::IsFocused()
	{
		if (kind == Kind::CalendarDay) return UiaCalendar(control)->GetDayButton(row, column)->GetFocused();
		if (!control->GetFocused()) return false;
		if (kind == Kind::Control)
			for (auto parent = control->GetParent(); parent; parent = parent->GetParent())
				if (auto calendar = UiaCalendar(parent))
				{
					for (vint r = 0; r < 6; r++) for (vint c = 0; c < 7; c++)
						if (calendar->GetDayButton(r, c) == control) return false;
					break;
				}
		if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(control))
		{
			auto selected = grid->GetSelectedCell();
			return selected.row >= 0 && selected.column >= 0 ? kind == Kind::Cell && IsSelected() : kind == Kind::Control;
		}
		return kind == Kind::Control;
	}

	void UiaCollectChildren(WindowsUIAutomationContext* context, GuiGraphicsComposition* composition, List<Ptr<WindowsUIAutomationNode>>& children)
	{
		for (auto child : composition->Children())
		{
			if (!child->GetVisible()) continue;
			if (auto control = child->GetAssociatedControl())
			{
				if (!UiaRadioController(control)) children.Add(context->Control(control));
			}
			else UiaCollectChildren(context, child, children);
		}
	}

	WString UiaTooltipText(GuiControl* control)
	{
		if (!control) return L"";
		if (control->GetText().Length()) return control->GetText();
		WString text;
		for (vint i = 0; i < control->GetChildrenCount(); i++)
		{
			auto child = UiaTooltipText(control->GetChild(i));
			if (child.Length()) text += (text.Length() ? L" " : L"") + child;
		}
		return text;
	}

	bool UiaTooltipInteractive(GuiControl* control)
	{
		if (!control) return false;
		if (control->GetFocusableComposition()) return true;
		for (vint i = 0; i < control->GetChildrenCount(); i++) if (UiaTooltipInteractive(control->GetChild(i))) return true;
		return false;
	}

	List<Ptr<WindowsUIAutomationNode>> WindowsUIAutomationNode::Children()
	{
		List<Ptr<WindowsUIAutomationNode>> result;
		auto self = context->nodes[context->nodes.IndexOf(this)];
		if (kind == Kind::RadioGroup)
		{
			auto group = UiaRadioController(control);
			for (auto candidate : context->nodes)
				if (candidate->kind == Kind::Control && candidate->IsLive() && UiaRadioController(candidate->control) == group) result.Add(candidate);
		}
		else if (kind == Kind::DocumentObject)
		{
			result = UiaTextChildren(this, 0, -1);
		}
		else if (kind == Kind::TabContent)
		{
			if (owner->IsSelected()) UiaCollectChildren(context, control->GetBoundsComposition(), result);
		}
		else if (kind == Kind::Header)
		{
			for (vint c = 0; c < UiaListView(control)->GetColumnCount(); c++) result.Add(context->Item(owner, Kind::HeaderItem, -1, c));
		}
		else if (kind == Kind::Item && owner->Supports(UIA_TablePatternId))
		{
			for (vint c = 0; c < UiaListView(control)->GetColumnCount(); c++) result.Add(context->Item(owner, Kind::Cell, row, c));
		}
		else if (kind == Kind::Cell)
		{
			if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(control); grid && grid->GetSelectedCell() == GridPos(row, column) && grid->GetOpenedEditor())
			{
				UiaCollectChildren(context, grid->GetOpenedEditor()->GetTemplate(), result);
			}
			else if (auto cell = dynamic_cast<GuiCellComposition*>(Composition()))
			{
				UiaCollectChildren(context, cell, result);
			}
		}
		else if (kind == Kind::Item && !UiaListView(control) && !dynamic_cast<GuiVirtualTextList*>(control))
		{
			if (auto style = Composition()) UiaCollectChildren(context, style, result);
		}
		else if (kind == Kind::TreeNode || kind == Kind::Control && dynamic_cast<GuiVirtualTreeListControl*>(control))
		{
			auto tree = dynamic_cast<GuiVirtualTreeListControl*>(control);
			auto parent = treeNode ? treeNode : tree->GetNodeRootProvider()->GetRootNode();
			if (!treeNode || treeNode->GetExpanding())
				for (vint i = 0; i < parent->GetChildCount(); i++) result.Add(context->Item(treeNode ? owner : self, Kind::TreeNode, -1, -1, parent->GetChild(i)));
		}
		else if (kind == Kind::Control)
		{
			if (UiaDocument(control))
			{
				result = UiaTextChildren(this, 0, -1);
			}
			else if (dynamic_cast<GuiTabPage*>(control))
			{
				if (IsSelected()) result.Add(context->Item(self, Kind::TabContent));
			}
			else if (auto tab = dynamic_cast<GuiTab*>(control))
			{
				if (auto ribbon = dynamic_cast<GuiRibbonTab*>(tab)) UiaCollectChildren(context, ribbon->GetBeforeHeaders(), result);
				for (auto page : tab->GetPages()) result.Add(context->Control(page));
				if (auto ribbon = dynamic_cast<GuiRibbonTab*>(tab)) UiaCollectChildren(context, ribbon->GetAfterHeaders(), result);
			}
			else if (auto list = dynamic_cast<GuiListControl*>(control))
			{
				if (Supports(UIA_TablePatternId)) result.Add(context->Item(self, Kind::Header));
				for (vint i = 0; i < list->GetItemProvider()->Count(); i++) result.Add(context->Item(self, Kind::Item, i));
			}
			else if (auto calendar = UiaCalendar(control))
			{
				result.Add(context->Control(calendar->GetYearCombo()));
				result.Add(context->Control(calendar->GetMonthCombo()));
				for (vint c = 0; c < 7; c++) result.Add(context->Item(self, Kind::CalendarHeader, -1, c));
				for (vint r = 0; r < 6 /* GuiCommonDatePickerLook stores six weeks; its dimension getters are transposed. */; r++) for (vint c = 0; c < 7; c++) result.Add(context->Item(self, Kind::CalendarDay, r, c));
			}
			else if (!dynamic_cast<GuiButton*>(control) && !dynamic_cast<GuiScroll*>(control))
			{
				UiaCollectChildren(context, control->GetBoundsComposition(), result);
			}
			if (IsRoot() && context->hosted)
			{
				for (auto window : GetApplication()->GetWindows()) if (window != control && window->GetOpening() && !UiaPopupOwner(context->Control(window).Obj())) result.Add(context->Control(window));
			}
			if (auto menu = dynamic_cast<GuiMenuButton*>(control); menu && menu->GetSubMenuOpening()) result.Add(context->Control(menu->GetSubMenu()));
			if (auto gallery = dynamic_cast<GuiBindableRibbonGalleryList*>(control); gallery && gallery->GetSubMenu()->GetOpening()) result.Add(context->Control(gallery->GetSubMenu()));
			if (GetApplication()->GetTooltipOwner() == control)
				for (auto window : GetApplication()->GetWindows()) if (dynamic_cast<GuiTooltip*>(window) && window->GetOpening()) result.Add(context->Control(window));
		}
		if (kind == Kind::Control && !dynamic_cast<GuiTabPage*>(control) || kind == Kind::TabContent && owner->IsSelected())
		{
			List<Ptr<WindowsUIAutomationNode>> radios;
			for (auto candidate : context->nodes)
				if (candidate->kind == Kind::Control && candidate->IsLive() && candidate->Window() == Window() && UiaRadioController(candidate->control)) radios.Add(candidate);
			for (auto radio : radios)
			{
				auto group = UiaRadioGroup(radio.Obj());
				if (group->Parent() == self && !result.Contains(group.Obj())) result.Add(group);
			}
		}
		return result;
	}

	Ptr<WindowsUIAutomationNode> UiaFindTextParent(Ptr<WindowsUIAutomationNode> container, WindowsUIAutomationNode* target)
	{
		for (auto child : UiaTextChildren(container.Obj(), 0, -1))
		{
			if (child.Obj() == target) return container;
			if (child->kind == Kind::DocumentObject) if (auto result = UiaFindTextParent(child, target)) return result;
		}
		return nullptr;
	}

	Ptr<WindowsUIAutomationNode> WindowsUIAutomationNode::Parent()
	{
		if (kind == Kind::RadioGroup)
		{
			auto parent = control->GetParent();
			for (auto child : Children())
			{
				while (parent)
				{
					bool contains = false;
					for (auto current = child->control->GetParent(); current; current = current->GetParent())
						if (current == parent) { contains = true; break; }
					if (contains) break;
					parent = parent->GetParent();
				}
			}
			auto result = context->Control(parent ? parent : Window());
			return dynamic_cast<GuiTabPage*>(result->control) ? context->Item(result, Kind::TabContent) : result;
		}
		if (kind == Kind::Control) if (auto group = UiaRadioGroup(this)) return group;
		if (kind == Kind::DocumentObject) return UiaDocumentObjectParent(this);
		if (kind == Kind::TreeNode)
		{
			auto parent = treeNode->GetParent();
			if (parent == dynamic_cast<GuiVirtualTreeListControl*>(control)->GetNodeRootProvider()->GetRootNode()) return owner;
			return context->Item(owner, Kind::TreeNode, -1, -1, parent);
		}
		if (kind == Kind::Cell) return context->Item(owner, Kind::Item, row);
		if (kind == Kind::HeaderItem) return context->Item(owner, Kind::Header);
		if (owner) return owner;
		if (auto popupOwner = UiaPopupOwner(this)) return context->Control(popupOwner);
		if (IsRoot()) return nullptr;
		if (dynamic_cast<GuiWindow*>(control) && context->hosted) return context->Control(GetApplication()->GetMainWindow());
		if (auto page = dynamic_cast<GuiTabPage*>(control)) return context->Control(page->GetOwnerTab());
		for (auto composition = control->GetBoundsComposition()->GetParent(); composition; composition = composition->GetParent())
		{
			if (auto cell = dynamic_cast<GuiCellComposition*>(composition))
			{
				auto table = cell->GetParent();
				auto item = table ? dynamic_cast<list::DefaultDataGridItemTemplate*>(table->GetParent()) : nullptr;
				if (item && item->GetAssociatedListControl()) return context->Item(context->Control(item->GetAssociatedListControl()), Kind::Cell, item->GetIndex(), cell->GetColumn());
			}
			if (auto item = dynamic_cast<templates::GuiListItemTemplate*>(composition))
			{
				// Combo display templates are not owned by a list and have ordinary composition parents.
				if (auto list = item->GetAssociatedListControl()) return context->Item(context->Control(list), Kind::Item, item->GetIndex());
			}
			if (auto parent = composition->GetAssociatedControl())
			{
				if (dynamic_cast<GuiTabPage*>(parent)) return context->Item(context->Control(parent), Kind::TabContent);
				if (UiaDocument(parent))
				{
					if (auto result = UiaFindTextParent(context->Control(parent), this)) return result;
				}
				return context->Control(parent);
			}
		}
		return nullptr;
	}

	IRawElementProviderSimple* WindowsUIAutomationNode::Provider()
	{
		if (!provider) provider = new WindowsUIAutomationProvider(context->nodes[context->nodes.IndexOf(this)]);
		return provider.Obj();
	}

	HRESULT UiaString(const WString& value, BSTR* result)
	{
		if (!result) return E_POINTER;
		*result = SysAllocStringLen(value.Buffer(), (UINT)value.Length());
		return *result ? S_OK : E_OUTOFMEMORY;
	}

	HRESULT UiaNodeArray(const List<Ptr<WindowsUIAutomationNode>>& nodes, SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = SafeArrayCreateVector(VT_UNKNOWN, 0, (ULONG)nodes.Count());
		if (!*result) return E_OUTOFMEMORY;
		for (LONG i = 0; i < nodes.Count(); i++) SafeArrayPutElement(*result, &i, nodes[i]->Provider());
		return S_OK;
	}
}
#endif

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

	bool WindowsUIAutomationNode::IsLive()
	{
		if (retired || !context || context->stopped || disposed->IsDisposed()) return false;
		if (owner && !owner->IsLive()) return false;
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
		if (kind == Kind::CalendarHeader) return nullptr;
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
		switch (kind)
		{
		case Kind::TreeNode: return dynamic_cast<GuiVirtualTreeListControl*>(control)->GetNodeRootProvider()->GetTextValue(treeNode.Obj());
		case Kind::Item: return dynamic_cast<GuiListControl*>(control)->GetItemProvider()->GetTextValue(row);
		case Kind::Cell: return column == 0 ? UiaListView(control)->GetText(row) : UiaListView(control)->GetSubItem(row, column - 1);
		case Kind::Header: return L"Column headers";
		case Kind::HeaderItem: return UiaListView(control)->GetColumnText(column);
		case Kind::CalendarHeader: return dynamic_cast<GuiDatePicker*>(control)->GetDateLocale().GetShortDayOfWeekName(column);
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
		switch (kind)
		{
		case Kind::Item: return owner->Supports(UIA_GridPatternId) ? UIA_DataItemControlTypeId : UIA_ListItemControlTypeId;
		case Kind::TreeNode: return UIA_TreeItemControlTypeId;
		case Kind::Cell: return UIA_DataItemControlTypeId;
		case Kind::Header: return UIA_HeaderControlTypeId;
		case Kind::HeaderItem: case Kind::CalendarHeader: return UIA_HeaderItemControlTypeId;
		case Kind::CalendarDay: return UIA_ListItemControlTypeId;
		default: break;
		}
		if (dynamic_cast<GuiTooltip*>(control)) return UIA_ToolTipControlTypeId;
		if (dynamic_cast<GuiMenu*>(control)) return control->GetControlThemeName() == theme::ThemeName::RibbonGroupMenu ? UIA_GroupControlTypeId : UIA_MenuControlTypeId;
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
		if (auto button = dynamic_cast<GuiSelectableButton*>(control)) return button->GetGroupController() ? UIA_RadioButtonControlTypeId : UIA_CheckBoxControlTypeId;
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
		auto button = dynamic_cast<GuiSelectableButton*>(control);
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
			case UIA_SelectionItemPatternId: return kind == Kind::CalendarDay || dynamic_cast<GuiSelectableListControl*>(control) && (kind == Kind::Item || kind == Kind::TreeNode || kind == Kind::Cell);
			case UIA_ScrollItemPatternId: case UIA_VirtualizedItemPatternId: return list && row >= 0 || kind == Kind::TreeNode;
			case UIA_ExpandCollapsePatternId: return kind == Kind::TreeNode;
			case UIA_TogglePatternId: return kind == Kind::Item && text && text->GetView() != TextListView::Text;
			case UIA_GridItemPatternId: case UIA_TableItemPatternId: return kind == Kind::Cell || kind == Kind::CalendarDay;
			case UIA_TransformPatternId: return kind == Kind::HeaderItem && UiaColumns(control);
			case UIA_InvokePatternId: return kind == Kind::Cell && UiaDataGrid(control) && UiaDataGrid(control)->GetCellDataEditorFactory(row, column) || kind == Kind::HeaderItem;
			default: return false;
			}
		}
		switch (pattern)
		{
		case UIA_InvokePatternId:
			if (dynamic_cast<GuiComboBoxBase*>(control)) return false;
			if (menu) return !menu->GetAutoSelection() && !menu->GetGroupController() && (!menu->GetSubMenu() || menu->GetSubMenuHost() != menu);
			return dynamic_cast<GuiButton*>(control) && !button;
		case UIA_TogglePatternId: return button && !button->GetGroupController() && (!menu || menu->GetAutoSelection());
		case UIA_SelectionItemPatternId: return tabPage || button && button->GetGroupController();
		case UIA_SelectionPatternId: return dynamic_cast<GuiSelectableListControl*>(control) || dynamic_cast<GuiTab*>(control) || calendar || dynamic_cast<GuiComboBoxListControl*>(control);
		case UIA_ExpandCollapsePatternId: return menu && menu->GetSubMenu() || dynamic_cast<GuiBindableRibbonGalleryList*>(control);
		case UIA_ValuePatternId: return document && (dynamic_cast<GuiSinglelineTextBox*>(control) || dynamic_cast<GuiMultilineTextBox*>(control));
		case UIA_TextPatternId:
			if (auto single = dynamic_cast<GuiSinglelineTextBox*>(control); single && single->GetPasswordChar()) return false;
			return document != nullptr;
		case UIA_RangeValuePatternId: return dynamic_cast<GuiScroll*>(control);
		case UIA_ScrollPatternId:
			if (auto view = dynamic_cast<GuiScrollView*>(control)) return view->GetHorizontalScroll() && view->GetVerticalScroll();
			return false;
		case UIA_GridPatternId: case UIA_TablePatternId: return UiaDataGrid(control) || listView && listView->GetView() == ListViewView::Detail || calendar;
		case UIA_ItemContainerPatternId: return list || calendar;
		case UIA_MultipleViewPatternId: return listView && !UiaDataGrid(control);
		case UIA_WindowPatternId: case UIA_TransformPatternId: return dynamic_cast<GuiWindow*>(control);
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

	void UiaCollectChildren(WindowsUIAutomationContext* context, GuiGraphicsComposition* composition, List<Ptr<WindowsUIAutomationNode>>& children)
	{
		for (auto child : composition->Children())
		{
			if (!child->GetVisible()) continue;
			if (auto control = child->GetAssociatedControl()) children.Add(context->Control(control));
			else UiaCollectChildren(context, child, children);
		}
	}

	List<Ptr<WindowsUIAutomationNode>> WindowsUIAutomationNode::Children()
	{
		List<Ptr<WindowsUIAutomationNode>> result;
		auto self = context->nodes[context->nodes.IndexOf(this)];
		if (kind == Kind::Header)
		{
			for (vint c = 0; c < UiaListView(control)->GetColumnCount(); c++) result.Add(context->Item(owner, Kind::HeaderItem, -1, c));
		}
		else if (kind == Kind::Item && owner->Supports(UIA_GridPatternId))
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
				return UiaTextChildren(this, 0, -1);
			}
			else if (auto tab = dynamic_cast<GuiTab*>(control))
			{
				for (auto page : tab->GetPages()) result.Add(context->Control(page));
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
				for (auto window : GetApplication()->GetWindows()) if (window != control && window->GetOpening()) result.Add(context->Control(window));
			}
		}
		return result;
	}

	Ptr<WindowsUIAutomationNode> WindowsUIAutomationNode::Parent()
	{
		if (kind == Kind::TreeNode)
		{
			auto parent = treeNode->GetParent();
			if (parent == dynamic_cast<GuiVirtualTreeListControl*>(control)->GetNodeRootProvider()->GetRootNode()) return owner;
			return context->Item(owner, Kind::TreeNode, -1, -1, parent);
		}
		if (kind == Kind::Cell) return context->Item(owner, Kind::Item, row);
		if (kind == Kind::HeaderItem) return context->Item(owner, Kind::Header);
		if (owner) return owner;
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

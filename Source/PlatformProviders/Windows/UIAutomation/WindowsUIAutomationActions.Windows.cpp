#include "WindowsUIAutomationProvider.Windows.h"
#include <cmath>
#include <limits>

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using namespace compositions;
	using Kind = WindowsUIAutomationNodeKind;

	HRESULT WindowsUIAutomationProvider::Invoke()
	{
		return Queue([target = node]()
		{
			if (target->kind == Kind::DocumentObject)
			{
				UiaInvokeDocumentObject(target.Obj());
			}
			else if (target->kind == Kind::Cell)
			{
				UiaRaiseAutomationEvent(target->Provider(), UIA_Invoke_InvokedEventId);
				dynamic_cast<GuiVirtualDataGrid*>(target->control)->EnsureItemVisible(target->row);
				dynamic_cast<GuiVirtualDataGrid*>(target->control)->SelectCell(GridPos(target->row, target->column), true);
			}
			else if (target->kind == Kind::HeaderItem)
			{
				UiaRaiseAutomationEvent(target->Provider(), UIA_Invoke_InvokedEventId);
				auto list = dynamic_cast<GuiListViewBase*>(target->control);
				GuiItemEventArgs arguments(list->GetBoundsComposition());
				arguments.itemIndex = target->column;
				list->ColumnClicked.Execute(arguments);
			}
			else
			{
				auto button = dynamic_cast<GuiButton*>(target->control);
				if (button->GetAutoFocus()) button->SetFocused();
				button->BeforeClicked.Execute(button->GetNotifyEventArguments());
				if (!target->IsLive()) return;
				button->Clicked.Execute(button->GetNotifyEventArguments());
				if (!target->IsLive()) return;
				button->AfterClicked.Execute(button->GetNotifyEventArguments());
			}
		}, UIA_InvokePatternId);
	}
	HRESULT WindowsUIAutomationProvider::Toggle()
	{
		return Read([&]() -> HRESULT
		{
			if (node->kind == Kind::Item)
			{
				auto list = dynamic_cast<GuiVirtualTextList*>(node->control);
				auto items = dynamic_cast<list::ITextItemView*>(list->GetItemProvider()->RequestView(WString::Unmanaged(list::ITextItemView::Identifier)));
				items->SetChecked(node->row, !items->GetChecked(node->row));
			}
			else
			{
				auto button = dynamic_cast<GuiSelectableButton*>(node->control);
				button->SetSelected(!button->GetSelected());
			}
			node->context->Notify(node);
			return S_OK;
		}, UIA_TogglePatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::get_ToggleState(ToggleState* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { VARIANT value; node->Property(UIA_ToggleToggleStatePropertyId, &value); *result = (ToggleState)value.lVal; return S_OK; }, UIA_TogglePatternId);
	}

	HRESULT UiaSelect(Ptr<WindowsUIAutomationNode> node, bool selected, bool clear)
	{
		if (node->kind == Kind::CalendarDay)
		{
			if (!selected) return UIA_E_INVALIDOPERATION;
			if (!clear && !node->IsSelected()) return UIA_E_INVALIDOPERATION;
			auto picker = dynamic_cast<GuiDatePicker*>(node->control);
			picker->SetDate(UiaCalendar(node->control)->GetDateOfDayButton(node->row, node->column));
			if (node->IsLive()) picker->DateSelected.Execute(picker->GetNotifyEventArguments());
		}
		else if (node->kind == Kind::Cell && UiaDataGrid(node->control))
		{
			if (!selected) return UIA_E_INVALIDOPERATION;
			auto grid = dynamic_cast<GuiVirtualDataGrid*>(node->control);
			auto current = grid->GetSelectedCell();
			if (!clear && current.row >= 0 && current != GridPos(node->row, node->column)) return UIA_E_INVALIDOPERATION;
			grid->SelectCell(GridPos(node->row, node->column), false);
		}
		else if (node->kind == Kind::Item || node->kind == Kind::TreeNode || node->kind == Kind::Cell)
		{
			auto list = dynamic_cast<GuiSelectableListControl*>(node->control);
			auto index = node->ItemIndex();
			if (index < 0) return UIA_E_INVALIDOPERATION;
			auto combo = UiaCombo(node.Obj());
			if (combo && !selected) return UIA_E_INVALIDOPERATION;
			if (!clear && selected && !list->GetMultiSelect() && list->GetSelectedItems().Count() && !list->GetSelected(index)) return UIA_E_INVALIDOPERATION;
			if (clear) list->ClearSelection();
			list->SetSelected(index, selected);
			if (combo)
			{
				combo->SetSelectedIndex(index);
				combo->SetSubMenuOpening(false);
			}
		}
		else if (auto page = dynamic_cast<GuiTabPage*>(node->control))
		{
			if (!selected) return UIA_E_INVALIDOPERATION;
			if (!clear && page->GetOwnerTab()->GetSelectedPage() && !node->IsSelected()) return UIA_E_INVALIDOPERATION;
			page->GetOwnerTab()->SetSelectedPage(page);
		}
		else if (auto button = dynamic_cast<GuiSelectableButton*>(node->control))
		{
			if (!selected) return UIA_E_INVALIDOPERATION;
			if (!clear) if (auto group = UiaRadioGroup(node.Obj()))
			{
				auto children = group->Children();
				for (auto child : children) if (child != node && child->IsSelected()) return UIA_E_INVALIDOPERATION;
			}
			button->SetSelected(true);
		}
		if (node->IsLive()) node->context->Notify(node);
		return S_OK;
	}
	HRESULT WindowsUIAutomationProvider::Select() { return Read([&]() { return UiaSelect(node, true, true); }, UIA_SelectionItemPatternId, true); }
	HRESULT WindowsUIAutomationProvider::AddToSelection() { return Read([&]() { return UiaSelect(node, true, false); }, UIA_SelectionItemPatternId, true); }
	HRESULT WindowsUIAutomationProvider::RemoveFromSelection() { return Read([&]() { return UiaSelect(node, false, false); }, UIA_SelectionItemPatternId, true); }
	HRESULT WindowsUIAutomationProvider::get_IsSelected(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = node->IsSelected(); return S_OK; }, UIA_SelectionItemPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_SelectionContainer(IRawElementProviderSimple** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto parent = node->owner ? node->owner : node->Parent();
			if (node->owner) if (auto combo = UiaCombo(node.Obj())) parent = node->context->Control(combo);
			if (parent) { *result = parent->Provider(); (*result)->AddRef(); }
			return S_OK;
		}, UIA_SelectionItemPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_CanSelectMultiple(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { auto list = dynamic_cast<GuiSelectableListControl*>(node->control); *result = list && !UiaDataGrid(list) && list->GetMultiSelect(); return S_OK; }, UIA_SelectionPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_IsSelectionRequired(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT
		{
			if (auto tab = dynamic_cast<GuiTab*>(node->control)) *result = tab->GetPages().Count() > 0;
			else if (node->kind == Kind::RadioGroup) *result = From(node->Children()).Any([](auto child) { return child->IsSelected(); });
			else if (auto combo = dynamic_cast<GuiComboBoxListControl*>(node->control)) *result = combo->GetSelectedIndex() >= 0;
			else if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(node->control)) *result = grid->GetSelectedCell().row >= 0;
			else *result = dynamic_cast<GuiDatePicker*>(node->control) != nullptr;
			return S_OK;
		}, UIA_SelectionPatternId);
	}
	HRESULT UiaExpand(Ptr<WindowsUIAutomationNode> node, bool expanding)
	{
		if (node->treeNode)
		{
			if (!node->treeNode->GetChildCount()) return UIA_E_INVALIDOPERATION;
			node->treeNode->SetExpanding(expanding);
		}
		else if (auto gallery = dynamic_cast<GuiBindableRibbonGalleryList*>(node->control))
		{
			if (expanding) gallery->RequestedDropdown.Execute(gallery->GetNotifyEventArguments());
			else gallery->GetSubMenu()->Hide();
			node->context->Notify(node, true);
		}
		else dynamic_cast<GuiMenuButton*>(node->control)->SetSubMenuOpening(expanding);
		return S_OK;
	}
	HRESULT WindowsUIAutomationProvider::Expand() { return Read([&]() { return UiaExpand(node, true); }, UIA_ExpandCollapsePatternId, true); }
	HRESULT WindowsUIAutomationProvider::Collapse() { return Read([&]() { return UiaExpand(node, false); }, UIA_ExpandCollapsePatternId, true); }
	HRESULT WindowsUIAutomationProvider::get_ExpandCollapseState(ExpandCollapseState* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { VARIANT value; node->Property(UIA_ExpandCollapseExpandCollapseStatePropertyId, &value); *result = (ExpandCollapseState)value.lVal; return S_OK; }, UIA_ExpandCollapsePatternId);
	}
	HRESULT WindowsUIAutomationProvider::SetValue(LPCWSTR value)
	{
		if (!value) return E_INVALIDARG;
		return Read([&]() -> HRESULT
		{
			if (node->kind == Kind::Cell)
			{
				if (!UiaDataGrid(node->control)->GetCellDataEditorFactory(node->row, node->column)) return UIA_E_INVALIDOPERATION;
				UiaDataGrid(node->control)->SetBindingCellValue(node->row, node->column, reflection::description::BoxValue(WString(value)));
				if (node->IsLive()) node->context->Notify(node);
				return S_OK;
			}
			if (UiaDocument(node->control)->GetEditMode() != GuiDocumentEditMode::Editable) return UIA_E_INVALIDOPERATION;
			node->control->SetText(WString(value));
			return S_OK;
		}, UIA_ValuePatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::get_Value(BSTR* result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto text = dynamic_cast<GuiSinglelineTextBox*>(node->control);
			if (text && text->GetPasswordChar()) return E_ACCESSDENIED;
			if (node->kind == Kind::Cell) return UiaString(reflection::description::UnboxValue<WString>(UiaDataGrid(node->control)->GetBindingCellValue(node->row, node->column)), result);
			return UiaString(node->control->GetText(), result);
		}, UIA_ValuePatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_IsReadOnly(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT
		{
			if (node->kind == Kind::Cell) *result = !UiaDataGrid(node->control)->GetCellDataEditorFactory(node->row, node->column);
			else if (auto document = UiaDocument(node->control)) *result = document->GetEditMode() != GuiDocumentEditMode::Editable;
			else *result = node->Role() == UIA_ProgressBarControlTypeId;
			return S_OK;
		});
	}
	HRESULT WindowsUIAutomationProvider::SetValue(double value)
	{
		return Read([&]() -> HRESULT
		{
			auto scroll = dynamic_cast<GuiScroll*>(node->control);
			if (node->Role() == UIA_ProgressBarControlTypeId) return UIA_E_INVALIDOPERATION;
			if (!std::isfinite(value) || value < 0 || value > scroll->GetMaxPosition()) return E_INVALIDARG;
			scroll->SetPosition((vint)value);
			return S_OK;
		}, UIA_RangeValuePatternId, true);
	}
#define UIA_RANGE_GETTER(NAME, VALUE) \
	HRESULT WindowsUIAutomationProvider::get_##NAME(double* result) \
	{ \
		if (!result) return E_POINTER; \
		return Read([&]() -> HRESULT { auto scroll = dynamic_cast<GuiScroll*>(node->control); *result = (double)(VALUE); return S_OK; }, UIA_RangeValuePatternId); \
	}
	UIA_RANGE_GETTER(Value, scroll->GetPosition())
	UIA_RANGE_GETTER(Maximum, scroll->GetMaxPosition())
	UIA_RANGE_GETTER(Minimum, 0)
	UIA_RANGE_GETTER(LargeChange, node->Role() == UIA_ProgressBarControlTypeId ? std::numeric_limits<double>::quiet_NaN() : scroll->GetBigMove())
	UIA_RANGE_GETTER(SmallChange, node->Role() == UIA_ProgressBarControlTypeId ? std::numeric_limits<double>::quiet_NaN() : scroll->GetSmallMove())
#undef UIA_RANGE_GETTER

	HRESULT UiaScroll(GuiScroll* scroll, ScrollAmount amount)
	{
		vint delta;
		switch (amount)
		{
		case ScrollAmount_NoAmount: return S_OK;
		case ScrollAmount_LargeDecrement: delta = -scroll->GetBigMove(); break;
		case ScrollAmount_SmallDecrement: delta = -scroll->GetSmallMove(); break;
		case ScrollAmount_LargeIncrement: delta = scroll->GetBigMove(); break;
		case ScrollAmount_SmallIncrement: delta = scroll->GetSmallMove(); break;
		default: return E_INVALIDARG;
		}
		if (!scroll->GetMaxPosition()) return UIA_E_INVALIDOPERATION;
		scroll->SetPosition(scroll->GetPosition() + delta);
		return S_OK;
	}
	HRESULT WindowsUIAutomationProvider::Scroll(ScrollAmount horizontal, ScrollAmount vertical)
	{
		return Read([&]() -> HRESULT
		{
			auto view = dynamic_cast<GuiScrollView*>(node->control);
			auto result = UiaScroll(view->GetHorizontalScroll(), horizontal);
			return FAILED(result) ? result : UiaScroll(view->GetVerticalScroll(), vertical);
		}, UIA_ScrollPatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::SetScrollPercent(double horizontal, double vertical)
	{
		return Read([&]() -> HRESULT
		{
			auto valid = [](double value) { return std::isfinite(value) && (value == UIA_ScrollPatternNoScroll || value >= 0 && value <= 100); };
			if (!valid(horizontal) || !valid(vertical)) return E_INVALIDARG;
			auto view = dynamic_cast<GuiScrollView*>(node->control);
			auto h = view->GetHorizontalScroll(), v = view->GetVerticalScroll();
			if (horizontal != UIA_ScrollPatternNoScroll && !h->GetMaxPosition() || vertical != UIA_ScrollPatternNoScroll && !v->GetMaxPosition()) return UIA_E_INVALIDOPERATION;
			if (horizontal != UIA_ScrollPatternNoScroll) h->SetPosition((vint)(horizontal * h->GetMaxPosition() / 100));
			if (vertical != UIA_ScrollPatternNoScroll) v->SetPosition((vint)(vertical * v->GetMaxPosition() / 100));
			return S_OK;
		}, UIA_ScrollPatternId, true);
	}
#define UIA_SCROLL_GETTER(NAME, TYPE, AXIS, VALUE) \
	HRESULT WindowsUIAutomationProvider::get_##NAME(TYPE* result) \
	{ \
		if (!result) return E_POINTER; \
		return Read([&]() -> HRESULT { auto scroll = dynamic_cast<GuiScrollView*>(node->control)->Get##AXIS##Scroll(); *result = (VALUE); return S_OK; }, UIA_ScrollPatternId); \
	}
	UIA_SCROLL_GETTER(HorizontalScrollPercent, double, Horizontal, scroll->GetMaxPosition() ? 100.0 * scroll->GetPosition() / scroll->GetMaxPosition() : UIA_ScrollPatternNoScroll)
	UIA_SCROLL_GETTER(VerticalScrollPercent, double, Vertical, scroll->GetMaxPosition() ? 100.0 * scroll->GetPosition() / scroll->GetMaxPosition() : UIA_ScrollPatternNoScroll)
	UIA_SCROLL_GETTER(HorizontalViewSize, double, Horizontal, scroll->GetTotalSize() ? min(100.0, 100.0 * scroll->GetPageSize() / scroll->GetTotalSize()) : 100.0)
	UIA_SCROLL_GETTER(VerticalViewSize, double, Vertical, scroll->GetTotalSize() ? min(100.0, 100.0 * scroll->GetPageSize() / scroll->GetTotalSize()) : 100.0)
	UIA_SCROLL_GETTER(HorizontallyScrollable, BOOL, Horizontal, scroll->GetMaxPosition() > 0)
	UIA_SCROLL_GETTER(VerticallyScrollable, BOOL, Vertical, scroll->GetMaxPosition() > 0)
#undef UIA_SCROLL_GETTER
	HRESULT WindowsUIAutomationProvider::Realize()
	{
		return Read([&]() -> HRESULT
		{
			if (node->treeNode)
			{
				for (auto parent = node->treeNode->GetParent(); parent; parent = parent->GetParent()) parent->SetExpanding(true);
			}
			return dynamic_cast<GuiListControl*>(node->control)->EnsureItemVisible(node->ItemIndex()) ? S_OK : UIA_E_INVALIDOPERATION;
		}, UIA_VirtualizedItemPatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::ScrollIntoView()
	{
		return Read([&]() -> HRESULT
		{
			if (node->Supports(UIA_VirtualizedItemPatternId))
			{
				auto result = Realize();
				if (FAILED(result)) return result;
			}
			node->control->GetBoundsComposition()->ForceCalculateSizeImmediately();
			auto composition = node->Composition();
			if (!composition) return UIA_E_INVALIDOPERATION;
			for (auto parent = node->kind == Kind::Control ? node->control->GetParent() : node->control; parent; parent = parent->GetParent())
			{
				auto view = dynamic_cast<GuiScrollView*>(parent);
				if (!view) continue;
				auto bounds = composition->GetGlobalBounds();
				auto viewport = view->GetControlTemplateObject()->GetContainerComposition()->GetGlobalBounds();
				if (node->kind == Kind::Cell && !dynamic_cast<GuiCellComposition*>(composition))
				{
					auto columns = UiaColumns(node->control);
					vint left = bounds.x1;
					for (vint c = 0; c < node->column; c++) left += columns->GetColumnSize(c);
					bounds.x1 = left;
					bounds.x2 = left + columns->GetColumnSize(node->column);
				}
				auto position = view->GetViewPosition();
				if (bounds.x1 < viewport.x1) position.x += bounds.x1 - viewport.x1;
				else if (bounds.x2 > viewport.x2) position.x += min(bounds.x1 - viewport.x1, bounds.x2 - viewport.x2);
				if (bounds.y1 < viewport.y1) position.y += bounds.y1 - viewport.y1;
				else if (bounds.y2 > viewport.y2) position.y += min(bounds.y1 - viewport.y1, bounds.y2 - viewport.y2);
				view->SetViewPosition(position);
				view->GetBoundsComposition()->ForceCalculateSizeImmediately();
			}
			return S_OK;
		}, UIA_ScrollItemPatternId, true);
	}
}
#endif

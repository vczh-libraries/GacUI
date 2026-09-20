#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"
#include <cmath>

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using Kind = WindowsUIAutomationNodeKind;
	extern HRESULT UiaTextSelection(Ptr<WindowsUIAutomationNode> node, SAFEARRAY** result);

	void UiaSelectedChildren(Ptr<WindowsUIAutomationNode> node, List<Ptr<WindowsUIAutomationNode>>& selected)
	{
		if (node->kind == Kind::Control)
		{
			if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(node->control))
			{
				auto cell = grid->GetSelectedCell();
				if (cell.row >= 0 && cell.column >= 0) selected.Add(node->context->Item(node, Kind::Cell, cell.row, cell.column));
				return;
			}
			if (auto list = dynamic_cast<GuiSelectableListControl*>(node->control))
			{
				if (auto combo = UiaCombo(node.Obj()))
				{
					if (combo->GetSelectedIndex() >= 0) selected.Add(node->context->Item(node, Kind::Item, combo->GetSelectedIndex()));
				}
				else for (auto index : list->GetSelectedItems())
				{
					if (auto tree = dynamic_cast<GuiVirtualTreeListControl*>(list)) selected.Add(node->context->Item(node, Kind::TreeNode, -1, -1, tree->GetNodeItemView()->RequestNode(index)));
					else selected.Add(node->context->Item(node, Kind::Item, index));
				}
				return;
			}
		}
		for (auto child : node->Children())
		{
			if (child->Supports(UIA_SelectionItemPatternId) && child->IsSelected()) selected.Add(child);
			if (child->kind == Kind::TreeNode || child->kind == Kind::Item && UiaDataGrid(child->control)) UiaSelectedChildren(child, selected);
		}
	}
	HRESULT WindowsUIAutomationProvider::GetSelection(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			if (node->Supports(UIA_TextPatternId)) return UiaTextSelection(node, result);
			if (!node->Supports(UIA_SelectionPatternId)) return UIA_E_NOTSUPPORTED;
			List<Ptr<WindowsUIAutomationNode>> selected;
			if (auto combo = dynamic_cast<GuiComboBoxListControl*>(node->control)) UiaSelectedChildren(node->context->Control(combo->GetContainedListControl()), selected);
			else UiaSelectedChildren(node, selected);
			return UiaNodeArray(selected, result);
		});
	}
	HRESULT WindowsUIAutomationProvider::GetItem(int row, int column, IRawElementProviderSimple** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			int rows, columns;
			get_RowCount(&rows); get_ColumnCount(&columns);
			if (row < 0 || column < 0 || row >= rows || column >= columns) return E_INVALIDARG;
			Ptr<WindowsUIAutomationNode> item;
			if (auto size = UiaSpatialGrid(node->control); size.x >= 0)
			{
				auto index = UiaSpatialColumnMajor(node->control) ? column * size.y + row : row * size.x + column;
				if (index >= dynamic_cast<GuiListControl*>(node->control)->GetItemProvider()->Count()) return E_INVALIDARG;
				item = node->context->Item(node, Kind::Item, index);
			}
			else item = node->context->Item(node, UiaCalendar(node->control) ? Kind::CalendarDay : Kind::Cell, row, column);
			item->Provider()->QueryInterface(IID_PPV_ARGS(result));
			return S_OK;
		}, UIA_GridPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_RowCount(int* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { auto size = UiaSpatialGrid(node->control); *result = (int)(size.x >= 0 ? size.y : UiaCalendar(node->control) ? 6 : dynamic_cast<GuiListControl*>(node->control)->GetItemProvider()->Count()); return S_OK; }, UIA_GridPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_ColumnCount(int* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { auto size = UiaSpatialGrid(node->control); *result = (int)(size.x >= 0 ? size.x : UiaCalendar(node->control) ? 7 : UiaListView(node->control)->GetColumnCount()); return S_OK; }, UIA_GridPatternId);
	}
#define UIA_CELL_GETTER(NAME, VALUE) \
	HRESULT WindowsUIAutomationProvider::get_##NAME(int* result) \
	{ \
		if (!result) return E_POINTER; \
		return Read([&]() -> HRESULT { *result = (int)(VALUE); return S_OK; }, UIA_GridItemPatternId); \
	}
	UIA_CELL_GETTER(Row, UiaGridPosition(node.Obj()).row)
	UIA_CELL_GETTER(Column, UiaGridPosition(node.Obj()).column)
	UIA_CELL_GETTER(RowSpan, 1)
	UIA_CELL_GETTER(ColumnSpan, 1)
#undef UIA_CELL_GETTER
	HRESULT WindowsUIAutomationProvider::get_ContainingGrid(IRawElementProviderSimple** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT { node->owner->Provider()->QueryInterface(IID_PPV_ARGS(result)); return S_OK; }, UIA_GridItemPatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetRowHeaders(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() { List<Ptr<WindowsUIAutomationNode>> headers; return UiaNodeArray(headers, result); }, UIA_TablePatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetColumnHeaders(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			List<Ptr<WindowsUIAutomationNode>> headers;
			int count; get_ColumnCount(&count);
			for (vint c = 0; c < count; c++) headers.Add(node->context->Item(node, UiaCalendar(node->control) ? Kind::CalendarHeader : Kind::HeaderItem, -1, c));
			return UiaNodeArray(headers, result);
		}, UIA_TablePatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_RowOrColumnMajor(RowOrColumnMajor* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = RowOrColumnMajor_RowMajor; return S_OK; }, UIA_TablePatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetRowHeaderItems(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() { List<Ptr<WindowsUIAutomationNode>> headers; return UiaNodeArray(headers, result); }, UIA_TableItemPatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetColumnHeaderItems(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			List<Ptr<WindowsUIAutomationNode>> headers;
			headers.Add(node->context->Item(node->owner, UiaCalendar(node->control) ? Kind::CalendarHeader : Kind::HeaderItem, -1, node->column));
			return UiaNodeArray(headers, result);
		}, UIA_TableItemPatternId);
	}
	HRESULT WindowsUIAutomationProvider::FindItemByProperty(IRawElementProviderSimple* startAfter, PROPERTYID property, VARIANT value, IRawElementProviderSimple** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			if (property && property != UIA_NamePropertyId && property != UIA_AutomationIdPropertyId && property != UIA_SelectionItemIsSelectedPropertyId && property != UIA_ControlTypePropertyId) return E_INVALIDARG;
			auto previous = dynamic_cast<WindowsUIAutomationProvider*>(startAfter);
			if (startAfter && (!previous || previous->node->owner != node || !previous->node->IsLive())) return E_INVALIDARG;
			bool after = !startAfter;
			List<Ptr<WindowsUIAutomationNode>> items;
			if (auto tree = dynamic_cast<GuiVirtualTreeListControl*>(node->control))
			{
				Func<void(Ptr<tree::INodeProvider>)> collect;
				collect = [&](Ptr<tree::INodeProvider> parent)
				{
					for (vint i = 0; i < parent->GetChildCount(); i++)
					{
						auto child = parent->GetChild(i);
						items.Add(node->context->Item(node, Kind::TreeNode, -1, -1, child));
						collect(child);
					}
				};
				collect(tree->GetNodeRootProvider()->GetRootNode());
			}
			else CopyFrom(items, node->Children());
			for (auto child : items)
			{
				if (child->kind == Kind::Header || child->kind == Kind::CalendarHeader || child->kind == Kind::Control) continue;
				if (!after) { after = child == previous->node; continue; }
				WindowsUIAutomationValue candidate;
				if (property) child->Property(property, &candidate.value);
				if (!property || VarCmp(&candidate.value, &value, LOCALE_INVARIANT, 0) == VARCMP_EQ)
				{
					child->Provider()->QueryInterface(IID_PPV_ARGS(result)); return S_OK;
				}
			}
			return S_OK;
		}, UIA_ItemContainerPatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetViewName(int view, BSTR* result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			const wchar_t* keys[] = { L"BigIcon", L"SmallIcon", L"List", L"Tile", L"Information", L"Detail" };
			const wchar_t* names[] = { L"Large icons", L"Small icons", L"List", L"Tiles", L"Information", L"Details" };
			return view < 0 || view >= 6 ? E_INVALIDARG : UiaString(UiaLocalizedText(node->control, WString::Unmanaged(keys[view]), WString::Unmanaged(names[view])), result);
		}, UIA_MultipleViewPatternId);
	}
	HRESULT WindowsUIAutomationProvider::SetCurrentView(int view)
	{
		return Read([&]() -> HRESULT
		{
			if (view < 0 || view >= 6) return E_INVALIDARG;
			dynamic_cast<GuiVirtualListView*>(node->control)->SetView((ListViewView)view);
			node->context->Notify(node, true);
			return S_OK;
		}, UIA_MultipleViewPatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::get_CurrentView(int* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = (int)dynamic_cast<GuiVirtualListView*>(node->control)->GetView(); return S_OK; }, UIA_MultipleViewPatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetSupportedViews(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			*result = SafeArrayCreateVector(VT_I4, 0, 6);
			for (LONG i = 0; i < 6; i++) SafeArrayPutElement(*result, &i, &i);
			return S_OK;
		}, UIA_MultipleViewPatternId);
	}
	HRESULT WindowsUIAutomationProvider::SetVisualState(WindowVisualState state)
	{
		return Read([&]() -> HRESULT
		{
			auto window = dynamic_cast<GuiWindow*>(node->control);
			switch (state)
			{
			case WindowVisualState_Normal: window->ShowRestored(); break;
			case WindowVisualState_Maximized: if (!window->GetMaximizedBox()) return UIA_E_INVALIDOPERATION; window->ShowMaximized(); break;
			case WindowVisualState_Minimized: if (!window->GetMinimizedBox()) return UIA_E_INVALIDOPERATION; window->ShowMinimized(); break;
			default: return E_INVALIDARG;
			}
			return S_OK;
		}, UIA_WindowPatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::Close()
	{
		return Queue([target = node]() { dynamic_cast<GuiWindow*>(target->control)->Close(); }, UIA_WindowPatternId);
	}
	HRESULT WindowsUIAutomationProvider::WaitForInputIdle(int milliseconds, BOOL* result)
	{
		if (!result) return E_POINTER;
		if (milliseconds < 0) return E_INVALIDARG;
		return node->dispatcher->WaitForIdle(milliseconds, result, [target = node]() -> HRESULT
		{
			if (!target->IsLive()) return UIA_E_ELEMENTNOTAVAILABLE;
			return target->Supports(UIA_WindowPatternId) ? S_OK : UIA_E_NOTSUPPORTED;
		});
	}
#define UIA_WINDOW_GETTER(NAME, TYPE, VALUE) \
	HRESULT WindowsUIAutomationProvider::get_##NAME(TYPE* result) \
	{ \
		if (!result) return E_POINTER; \
		return Read([&]() -> HRESULT { auto window = dynamic_cast<GuiWindow*>(node->control); *result = (VALUE); return S_OK; }, UIA_WindowPatternId); \
	}
	UIA_WINDOW_GETTER(CanMaximize, BOOL, window->GetMaximizedBox())
	UIA_WINDOW_GETTER(CanMinimize, BOOL, window->GetMinimizedBox())
	UIA_WINDOW_GETTER(IsTopmost, BOOL, window->GetTopMost())
	UIA_WINDOW_GETTER(IsModal, BOOL, window->GetModal())
	UIA_WINDOW_GETTER(WindowVisualState, WindowVisualState, window->GetNativeWindow()->GetSizeState() == INativeWindow::Maximized ? WindowVisualState_Maximized : window->GetNativeWindow()->GetSizeState() == INativeWindow::Minimized ? WindowVisualState_Minimized : WindowVisualState_Normal)
	UIA_WINDOW_GETTER(WindowInteractionState, WindowInteractionState, window->GetBlockedByModalWindow() ? WindowInteractionState_BlockedByModalWindow : window->GetNativeWindow()->IsEnabled() ? WindowInteractionState_ReadyForUserInteraction : WindowInteractionState_Running)
#undef UIA_WINDOW_GETTER
	NativeRect UiaConstrainWindowBounds(WindowsUIAutomationNode* node, NativeRect bounds)
	{
		RECT container;
		if (node->context->hosted && !node->IsRoot())
		{
			auto size = GetHostedApplication()->GetNativeWindowHost()->GetClientSize();
			container = { 0, 0, (LONG)size.x.value, (LONG)size.y.value };
		}
		else
		{
			auto coordinate = [](NativeCoordinate value) { return (LONG)max((vint)LONG_MIN, min(value.value, (vint)LONG_MAX)); };
			RECT requested = { coordinate(bounds.x1), coordinate(bounds.y1), coordinate(bounds.x2), coordinate(bounds.y2) };
			MONITORINFO monitor = { sizeof(MONITORINFO) };
			GetMonitorInfo(MonitorFromRect(&requested, MONITOR_DEFAULTTOPRIMARY), &monitor);
			container = monitor.rcWork;
		}
		auto size = bounds.GetSize();
		auto x = max((vint)container.left, min(bounds.x1.value, max((vint)container.left, container.right - size.x.value)));
		auto y = max((vint)container.top, min(bounds.y1.value, max((vint)container.top, container.bottom - size.y.value)));
		return NativeRect(NativePoint(x, y), size);
	}

	HRESULT WindowsUIAutomationProvider::Move(double x, double y)
	{
		return Read([&]() -> HRESULT
		{
			if (!std::isfinite(x) || !std::isfinite(y)) return E_INVALIDARG;
			BOOL canMove = FALSE;
			get_CanMove(&canMove);
			if (!canMove) return UIA_E_INVALIDOPERATION;
			auto native = node->Window()->GetNativeWindow();
			if (node->context->hosted)
			{
				auto host = GetHostedApplication()->GetNativeWindowHost();
				if (node->IsRoot()) native = host;
				else
				{
					auto origin = host->GetClientBoundsInScreen().LeftTop();
					x -= origin.x.value;
					y -= origin.y.value;
				}
			}
			auto size = native->GetBounds().GetSize();
			// Keep the requested rectangle representable before monitor containment on Win32.
			x = max((double)LONG_MIN, min(x, (double)LONG_MAX - size.x.value));
			y = max((double)LONG_MIN, min(y, (double)LONG_MAX - size.y.value));
			native->SetBounds(UiaConstrainWindowBounds(node.Obj(), NativeRect(NativePoint((vint)x, (vint)y), size)));
			return S_OK;
		}, UIA_TransformPatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::Resize(double width, double height)
	{
		return Read([&]() -> HRESULT
		{
			if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0 || width > LONG_MAX || height > LONG_MAX) return E_INVALIDARG;
			BOOL canResize = FALSE;
			get_CanResize(&canResize);
			if (!canResize) return UIA_E_INVALIDOPERATION;
			auto native = node->Window()->GetNativeWindow();
			if (node->kind == Kind::HeaderItem)
			{
				UiaColumns(node->control)->SetColumnSize(node->column, native->Convert(NativeSize((vint)width, (vint)height)).x);
				return S_OK;
			}
			if (node->context->hosted && node->IsRoot()) native = GetHostedApplication()->GetNativeWindowHost();
			native->SetBounds(UiaConstrainWindowBounds(node.Obj(), NativeRect(native->GetBounds().LeftTop(), NativeSize((vint)width, (vint)height))));
			return S_OK;
		}, UIA_TransformPatternId, true);
	}
	HRESULT WindowsUIAutomationProvider::Rotate(double) { return Read([]() { return UIA_E_INVALIDOPERATION; }, UIA_TransformPatternId, true); }
	HRESULT WindowsUIAutomationProvider::get_CanMove(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = node->kind == Kind::Control && node->Window()->GetNativeWindow()->GetSizeState() == INativeWindow::Restored; return S_OK; }, UIA_TransformPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_CanResize(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = node->kind == Kind::HeaderItem || node->Window()->GetSizeBox() && node->Window()->GetNativeWindow()->GetSizeState() == INativeWindow::Restored; return S_OK; }, UIA_TransformPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_CanRotate(BOOL* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = FALSE; return S_OK; }, UIA_TransformPatternId);
	}
}
#endif

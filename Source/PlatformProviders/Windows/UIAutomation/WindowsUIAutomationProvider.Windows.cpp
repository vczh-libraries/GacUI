#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using Kind = WindowsUIAutomationNodeKind;

#define UIA_PATTERNS(F) \
	F(Invoke) F(Toggle) F(Selection) F(SelectionItem) F(ExpandCollapse) F(Value) F(RangeValue) \
	F(Scroll) F(ScrollItem) F(Grid) F(GridItem) F(Table) F(TableItem) F(ItemContainer) \
	F(VirtualizedItem) F(MultipleView) F(Window) F(Transform) F(Text)

	WindowsUIAutomationProvider::WindowsUIAutomationProvider(Ptr<WindowsUIAutomationNode> value)
		: rootProvider(value->IsRoot()), node(value) {}
	ULONG WindowsUIAutomationProvider::AddRef() { return ++references; }
	ULONG WindowsUIAutomationProvider::Release()
	{
		auto count = --references;
		if (!count) delete this;
		return count;
	}
	HRESULT WindowsUIAutomationProvider::QueryInterface(REFIID iid, void** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IRawElementProviderSimple)) *result = static_cast<IRawElementProviderSimple*>(this);
		else if (iid == __uuidof(IRawElementProviderFragment)) *result = static_cast<IRawElementProviderFragment*>(this);
		else if (iid == __uuidof(IRawElementProviderFragmentRoot)) *result = static_cast<IRawElementProviderFragmentRoot*>(this);
#define UIA_QUERY(NAME) else if (iid == __uuidof(I##NAME##Provider)) *result = static_cast<I##NAME##Provider*>(this);
		UIA_PATTERNS(UIA_QUERY)
#undef UIA_QUERY
		if (!*result) return E_NOINTERFACE;
		AddRef();
		return S_OK;
	}
	HRESULT WindowsUIAutomationProvider::Read(const Func<HRESULT()>& action, PATTERNID pattern, bool enabled)
	{
		return node->dispatcher->Run([&]() -> HRESULT
		{
			if (!node->IsLive()) return UIA_E_ELEMENTNOTAVAILABLE;
			if (pattern && !node->Supports(pattern)) return UIA_E_NOTSUPPORTED;
			if (enabled && (!node->control->GetVisuallyEnabled() || !node->Window()->GetNativeWindow()->IsEnabled())) return UIA_E_ELEMENTNOTENABLED;
			return action();
		});
	}
	HRESULT WindowsUIAutomationProvider::Queue(const Func<void()>& action, PATTERNID pattern)
	{
		return Read([&]() -> HRESULT
		{
			node->dispatcher->Queue([target = node, action, pattern]()
			{
				if (target->IsLive() && target->Supports(pattern) && target->control->GetVisuallyEnabled() && target->Window()->GetNativeWindow()->IsEnabled()) action();
			});
			return S_OK;
		}, pattern, true);
	}
	HRESULT WindowsUIAutomationProvider::get_ProviderOptions(ProviderOptions* result)
	{
		if (!result) return E_POINTER;
		*result = (ProviderOptions)(ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading);
		return S_OK;
	}
	HRESULT WindowsUIAutomationProvider::GetPatternProvider(PATTERNID pattern, IUnknown** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			if (!node->Supports(pattern)) return S_OK;
			switch (pattern)
			{
#define UIA_PATTERN(NAME) case UIA_##NAME##PatternId: return QueryInterface(__uuidof(I##NAME##Provider), (void**)result);
			UIA_PATTERNS(UIA_PATTERN)
#undef UIA_PATTERN
			default: return S_OK;
			}
		});
	}
	HRESULT WindowsUIAutomationProvider::GetPropertyValue(PROPERTYID property, VARIANT* result)
	{
		if (!result) return E_POINTER;
		VariantInit(result);
		return Read([&]() { return node->Property(property, result); });
	}
	HRESULT WindowsUIAutomationNode::Property(PROPERTYID property, VARIANT* result)
	{
		VariantInit(result);
		auto boolean = [&](bool value) { result->vt = VT_BOOL; result->boolVal = value ? VARIANT_TRUE : VARIANT_FALSE; };
		auto number = [&](LONG value) { result->vt = VT_I4; result->lVal = value; };
		auto string = [&](const WString& value) { result->vt = VT_BSTR; UiaString(value, &result->bstrVal); };
		switch (property)
		{
		case UIA_NamePropertyId: string(Name()); break;
		case UIA_LabeledByPropertyId:
			if (kind == Kind::Control) if (auto metadata = UiaMetadata(control); metadata && metadata->label && !metadata->labelDisposed->IsDisposed())
			{
				auto label = context->Control(metadata->label);
				if (label->IsLive()) { result->vt = VT_UNKNOWN; result->punkVal = label->Provider(); result->punkVal->AddRef(); }
			}
			break;
		case UIA_HelpTextPropertyId: if (kind == Kind::Control) string(UiaLocalizedText(control, L"HelpText", UiaTooltipText(control->GetTooltipControl()))); break;
		case UIA_ControlTypePropertyId: number(Role()); break;
		case UIA_FrameworkIdPropertyId: string(L"GacUI"); break;
		case UIA_ClassNamePropertyId: string(L"GacUI." + itow(Role())); break;
		case UIA_AutomationIdPropertyId: string(L"gacui-" + itow(id)); break;
		case UIA_AccessKeyPropertyId: string(control->GetAlt()); break;
		case UIA_ItemStatusPropertyId:
			if (kind == Kind::HeaderItem) if (auto columns = UiaColumns(control))
			{
				switch (columns->GetSortingState(column))
				{
				case ColumnSortingState::Ascending: string(UiaLocalizedText(control, L"Ascending", L"Ascending")); break;
				case ColumnSortingState::Descending: string(UiaLocalizedText(control, L"Descending", L"Descending")); break;
				default: string(L""); break;
				}
			}
			break;
		case UIA_AcceleratorKeyPropertyId:
			if (auto button = dynamic_cast<GuiToolstripButton*>(control); button && button->GetCommand() && button->GetCommand()->GetShortcut()) string(button->GetCommand()->GetShortcut()->GetName());
			break;
		case UIA_OrientationPropertyId:
			if (kind == Kind::Header || kind == Kind::Control && (dynamic_cast<GuiTab*>(control) || dynamic_cast<GuiMenuBar*>(control))) number(OrientationType_Horizontal);
			else if (dynamic_cast<GuiScroll*>(control))
			{
				auto theme = control->GetControlThemeName();
				number(theme == theme::ThemeName::VScroll || theme == theme::ThemeName::VTracker ? OrientationType_Vertical : OrientationType_Horizontal);
			}
			break;
		case UIA_ProcessIdPropertyId: number(GetCurrentProcessId()); break;
		case UIA_NativeWindowHandlePropertyId: if (IsRoot()) number((LONG)(LONG_PTR)Handle()); break;
		case UIA_IsControlElementPropertyId: boolean(true); break;
		case UIA_IsContentElementPropertyId:
			if (auto tooltip = dynamic_cast<GuiTooltip*>(control)) boolean(UiaTooltipInteractive(tooltip->GetTemporaryContentControl()));
			else boolean(kind != Kind::Header && kind != Kind::HeaderItem && kind != Kind::CalendarHeader && Role() != UIA_MenuBarControlTypeId && Role() != UIA_ScrollBarControlTypeId && Role() != UIA_SeparatorControlTypeId);
			break;
		case UIA_IsEnabledPropertyId: boolean(control->GetVisuallyEnabled() && Window()->GetNativeWindow()->IsEnabled()); break;
		case UIA_HasKeyboardFocusPropertyId: boolean(IsFocused()); break;
		case UIA_IsKeyboardFocusablePropertyId: boolean(IsFocusable()); break;
		case UIA_IsOffscreenPropertyId: { auto bounds = Bounds(); boolean(bounds.width <= 0 || bounds.height <= 0); break; }
		case UIA_IsPasswordPropertyId: { auto text = dynamic_cast<GuiSinglelineTextBox*>(control); boolean(text && text->GetPasswordChar()); break; }
		case UIA_BoundingRectanglePropertyId:
			{
				auto bounds = Bounds();
				result->vt = VT_ARRAY | VT_R8;
				result->parray = SafeArrayCreateVector(VT_R8, 0, 4);
				if (!result->parray) return E_OUTOFMEMORY;
				LONG index = 0;
				for (double value : { bounds.left, bounds.top, bounds.width, bounds.height }) { SafeArrayPutElement(result->parray, &index, &value); index++; }
				break;
			}
		case UIA_ValueValuePropertyId:
			if (Supports(UIA_ValuePatternId))
			{
				auto text = dynamic_cast<GuiSinglelineTextBox*>(control);
				if (!text || !text->GetPasswordChar())
				{
					result->vt = VT_BSTR;
					return static_cast<WindowsUIAutomationProvider*>(Provider())->get_Value(&result->bstrVal);
				}
			}
			break;
		case UIA_ValueIsReadOnlyPropertyId: if (Supports(UIA_ValuePatternId)) { BOOL readOnly; static_cast<WindowsUIAutomationProvider*>(Provider())->get_IsReadOnly(&readOnly); boolean(readOnly != FALSE); } break;
		case UIA_SelectionItemIsSelectedPropertyId: if (Supports(UIA_SelectionItemPatternId)) boolean(IsSelected()); break;
		case UIA_ToggleToggleStatePropertyId:
			if (Supports(UIA_TogglePatternId))
			{
				bool selected;
				if (kind == Kind::Item) selected = dynamic_cast<list::ITextItemView*>(dynamic_cast<GuiListControl*>(control)->GetItemProvider()->RequestView(WString::Unmanaged(list::ITextItemView::Identifier)))->GetChecked(row);
				else selected = dynamic_cast<GuiSelectableButton*>(control)->GetSelected();
				number(selected ? ToggleState_On : ToggleState_Off);
			}
			break;
		case UIA_ExpandCollapseExpandCollapseStatePropertyId:
			if (Supports(UIA_ExpandCollapsePatternId))
			{
				if (treeNode) number(treeNode->GetChildCount() == 0 ? ExpandCollapseState_LeafNode : treeNode->GetExpanding() ? ExpandCollapseState_Expanded : ExpandCollapseState_Collapsed);
				else if (auto gallery = dynamic_cast<GuiBindableRibbonGalleryList*>(control)) number(gallery->GetSubMenu()->GetOpening() ? ExpandCollapseState_Expanded : ExpandCollapseState_Collapsed);
				else number(dynamic_cast<GuiMenuButton*>(control)->GetSubMenuOpening() ? ExpandCollapseState_Expanded : ExpandCollapseState_Collapsed);
			}
			break;
		case UIA_RangeValueValuePropertyId:
			if (Supports(UIA_RangeValuePatternId)) { result->vt = VT_R8; result->dblVal = (double)dynamic_cast<GuiScroll*>(control)->GetPosition(); }
			break;
#define UIA_DOUBLE_PROPERTY(NAME, PATTERN, METHOD) \
		case UIA_##NAME##PropertyId: \
			if (Supports(UIA_##PATTERN##PatternId)) { result->vt = VT_R8; return static_cast<WindowsUIAutomationProvider*>(Provider())->METHOD(&result->dblVal); } \
			break;
		UIA_DOUBLE_PROPERTY(RangeValueMinimum, RangeValue, get_Minimum)
		UIA_DOUBLE_PROPERTY(RangeValueMaximum, RangeValue, get_Maximum)
		UIA_DOUBLE_PROPERTY(RangeValueSmallChange, RangeValue, get_SmallChange)
		UIA_DOUBLE_PROPERTY(RangeValueLargeChange, RangeValue, get_LargeChange)
		UIA_DOUBLE_PROPERTY(ScrollHorizontalScrollPercent, Scroll, get_HorizontalScrollPercent)
		UIA_DOUBLE_PROPERTY(ScrollVerticalScrollPercent, Scroll, get_VerticalScrollPercent)
		UIA_DOUBLE_PROPERTY(ScrollHorizontalViewSize, Scroll, get_HorizontalViewSize)
		UIA_DOUBLE_PROPERTY(ScrollVerticalViewSize, Scroll, get_VerticalViewSize)
#undef UIA_DOUBLE_PROPERTY
		case UIA_RangeValueIsReadOnlyPropertyId: if (Supports(UIA_RangeValuePatternId)) boolean(Role() == UIA_ProgressBarControlTypeId); break;
		case UIA_ScrollHorizontallyScrollablePropertyId: if (Supports(UIA_ScrollPatternId)) boolean(dynamic_cast<GuiScrollView*>(control)->GetHorizontalScroll()->GetMaxPosition() > 0); break;
		case UIA_ScrollVerticallyScrollablePropertyId: if (Supports(UIA_ScrollPatternId)) boolean(dynamic_cast<GuiScrollView*>(control)->GetVerticalScroll()->GetMaxPosition() > 0); break;
		case UIA_MultipleViewCurrentViewPropertyId: if (Supports(UIA_MultipleViewPatternId)) number((LONG)dynamic_cast<GuiVirtualListView*>(control)->GetView()); break;
#define UIA_BOOL_PROPERTY(NAME, PATTERN, METHOD) \
		case UIA_##NAME##PropertyId: \
			if (Supports(UIA_##PATTERN##PatternId)) { BOOL value = FALSE; auto hr = static_cast<WindowsUIAutomationProvider*>(Provider())->METHOD(&value); if (FAILED(hr)) return hr; boolean(value != FALSE); } \
			break;
		UIA_BOOL_PROPERTY(SelectionCanSelectMultiple, Selection, get_CanSelectMultiple)
		UIA_BOOL_PROPERTY(SelectionIsSelectionRequired, Selection, get_IsSelectionRequired)
		UIA_BOOL_PROPERTY(WindowCanMaximize, Window, get_CanMaximize)
		UIA_BOOL_PROPERTY(WindowCanMinimize, Window, get_CanMinimize)
		UIA_BOOL_PROPERTY(WindowIsModal, Window, get_IsModal)
		UIA_BOOL_PROPERTY(WindowIsTopmost, Window, get_IsTopmost)
#undef UIA_BOOL_PROPERTY
		case UIA_GridRowCountPropertyId: case UIA_GridColumnCountPropertyId:
			if (Supports(UIA_GridPatternId))
			{
				auto provider = static_cast<WindowsUIAutomationProvider*>(Provider());
				int value = 0;
				auto hr = property == UIA_GridRowCountPropertyId ? provider->get_RowCount(&value) : provider->get_ColumnCount(&value);
				if (FAILED(hr)) return hr;
				number(value);
			}
			break;
		case UIA_WindowWindowVisualStatePropertyId:
			if (Supports(UIA_WindowPatternId)) { WindowVisualState value; auto hr = static_cast<WindowsUIAutomationProvider*>(Provider())->get_WindowVisualState(&value); if (FAILED(hr)) return hr; number(value); }
			break;
		case UIA_WindowWindowInteractionStatePropertyId:
			if (Supports(UIA_WindowPatternId)) { WindowInteractionState value; auto hr = static_cast<WindowsUIAutomationProvider*>(Provider())->get_WindowInteractionState(&value); if (FAILED(hr)) return hr; number(value); }
			break;
#define UIA_AVAILABLE(NAME) case UIA_Is##NAME##PatternAvailablePropertyId: boolean(Supports(UIA_##NAME##PatternId)); break;
		UIA_PATTERNS(UIA_AVAILABLE)
#undef UIA_AVAILABLE
		default: break;
		}
		return S_OK;
	}
#undef UIA_PATTERNS

	HRESULT WindowsUIAutomationProvider::get_HostRawElementProvider(IRawElementProviderSimple** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() { return node->IsRoot() ? UiaHostProviderFromHwnd(node->Handle(), result) : S_OK; });
	}
	HRESULT WindowsUIAutomationProvider::Navigate(NavigateDirection direction, IRawElementProviderFragment** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			Ptr<WindowsUIAutomationNode> target;
			if (direction == NavigateDirection_Parent) target = node->Parent();
			else if (direction == NavigateDirection_FirstChild || direction == NavigateDirection_LastChild)
			{
				auto children = node->Children();
				if (children.Count()) target = children[direction == NavigateDirection_FirstChild ? 0 : children.Count() - 1];
			}
			else if (direction == NavigateDirection_NextSibling || direction == NavigateDirection_PreviousSibling)
			{
				if (auto parent = node->Parent())
				{
					auto siblings = parent->Children();
					auto index = siblings.IndexOf(node.Obj());
					if (index != -1)
					{
						index += direction == NavigateDirection_NextSibling ? 1 : -1;
						if (index >= 0 && index < siblings.Count()) target = siblings[index];
					}
				}
			}
			else return E_INVALIDARG;
			return target && target->IsLive() ? target->Provider()->QueryInterface(IID_PPV_ARGS(result)) : S_OK;
		});
	}
	HRESULT WindowsUIAutomationProvider::GetRuntimeId(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		// UIA asks for the immutable identity during UiaDisconnectProvider, after
		// the semantic target has already been retired. No control access is needed.
		if (rootProvider) return S_OK;
		*result = SafeArrayCreateVector(VT_I4, 0, 2);
		if (!*result) return E_OUTOFMEMORY;
		LONG index = 0, value = UiaAppendRuntimeId;
		SafeArrayPutElement(*result, &index, &value);
		index = 1; value = (LONG)node->id;
		return SafeArrayPutElement(*result, &index, &value);
	}
	HRESULT WindowsUIAutomationProvider::get_BoundingRectangle(UiaRect* result)
	{
		if (!result) return E_POINTER;
		*result = {};
		return Read([&]() -> HRESULT { *result = node->Bounds(); return S_OK; });
	}
	HRESULT WindowsUIAutomationProvider::GetEmbeddedFragmentRoots(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([]() { return S_OK; });
	}
	HRESULT WindowsUIAutomationProvider::SetFocus()
	{
		return Read([&]() -> HRESULT
		{
			// UIA may focus before a pattern action: focus must not change selection.
			if (!node->IsFocusable()) return UIA_E_INVALIDOPERATION;
			if (node->kind == Kind::CalendarDay) UiaCalendar(node->control)->GetDayButton(node->row, node->column)->SetFocused();
			else node->control->SetFocused();
			return S_OK;
		}, 0, true);
	}
	HRESULT WindowsUIAutomationProvider::get_FragmentRoot(IRawElementProviderFragmentRoot** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto root = node->context->Control(node->context->hosted ? GetApplication()->GetMainWindow() : node->Window());
			return root->Provider()->QueryInterface(IID_PPV_ARGS(result));
		});
	}
	Ptr<WindowsUIAutomationNode> UiaHitTest(Ptr<WindowsUIAutomationNode> node, double x, double y)
	{
		auto bounds = node->Bounds();
		bool contains = x >= bounds.left && y >= bounds.top && x < bounds.left + bounds.width && y < bounds.top + bounds.height;
		// A tab's logical page owns its body despite the header's disjoint bounds.
		if (!contains && !(node->kind == Kind::Control && dynamic_cast<GuiTabPage*>(node->control) && node->IsSelected())) return nullptr;
		auto children = node->Children();
		for (vint i = children.Count() - 1; i >= 0; i--)
		{
			auto child = children[i];
			// Native/hosted window hit testing has already chosen the topmost window.
			if (child->Window() != node->Window()) continue;
			if (auto hit = UiaHitTest(child, x, y)) return hit;
		}
		return contains && node->kind != Kind::RadioGroup ? node : nullptr;
	}
	HRESULT WindowsUIAutomationProvider::ElementProviderFromPoint(double x, double y, IRawElementProviderFragment** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto root = node;
			if (node->context->hosted)
			{
				auto origin = GetHostedApplication()->GetNativeWindowHost()->GetClientBoundsInScreen().LeftTop();
				auto native = GetCurrentController()->WindowService()->GetWindow(NativePoint((vint)x - origin.x.value, (vint)y - origin.y.value));
				if (!native) return S_OK;
				for (auto window : GetApplication()->GetWindows()) if (window->GetNativeWindow() == native) { root = node->context->Control(window); break; }
			}
			auto hit = UiaHitTest(root, x, y);
			return hit ? hit->Provider()->QueryInterface(IID_PPV_ARGS(result)) : S_OK;
		});
	}
	Ptr<WindowsUIAutomationNode> UiaFindFocus(Ptr<WindowsUIAutomationNode> node)
	{
		for (auto child : node->Children()) if (auto focused = UiaFindFocus(child)) return focused;
		if (node->IsFocused()) return node;
		return nullptr;
	}
	HRESULT WindowsUIAutomationProvider::GetFocus(IRawElementProviderFragment** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto focused = UiaFindFocus(node);
			return focused ? focused->Provider()->QueryInterface(IID_PPV_ARGS(result)) : S_OK;
		});
	}
}
#endif

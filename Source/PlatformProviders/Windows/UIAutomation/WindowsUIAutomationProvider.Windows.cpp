#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"
#include <limits>

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
		ULONG count;
		SPIN_LOCK(node->lockProvider)
		{
			count = --references;
			if (!count && node->provider == this) node->provider = nullptr;
		}
		if (!count)
		{
			node->dispatcher->Queue([target = node, removed = subscriptions]()
			{
				if (auto context = target->context; context && !context->stopped)
				{
					context->RemoveSubscriptions(removed);
					context->Collect();
				}
			});
			delete this;
		}
		return count;
	}

	void UiaChangeSubscription(Dictionary<vint, vint>& counts, vint id, vint change)
	{
		auto index = counts.Keys().IndexOf(id);
		auto count = (index == -1 ? 0 : counts.Values()[index]) + change;
		CHECK_ERROR(count >= 0, L"Unbalanced UI Automation subscription.");
		if (count) counts.Set(id, count); else counts.Remove(id);
	}

	HRESULT WindowsUIAutomationProvider::AdviseEvent(EVENTID eventId, SAFEARRAY* propertyIds, bool adding)
	{
		List<vint> properties;
		if (propertyIds)
		{
			VARTYPE type;
			if (SafeArrayGetDim(propertyIds) != 1 || FAILED(SafeArrayGetVartype(propertyIds, &type)) || type != VT_I4) return E_INVALIDARG;
			LONG first, last;
			if (FAILED(SafeArrayGetLBound(propertyIds, 1, &first)) || FAILED(SafeArrayGetUBound(propertyIds, 1, &last))) return E_INVALIDARG;
			for (LONG i = first; i <= last; i++)
			{
				LONG property;
				if (FAILED(SafeArrayGetElement(propertyIds, &i, &property))) return E_INVALIDARG;
				if (!properties.Contains(property)) properties.Add(property);
			}
		}
		if (eventId == UIA_AutomationPropertyChangedEventId && properties.Count() == 0) properties.Add(0);
		return Read([&]() -> HRESULT
		{
			if (!subscriptions) subscriptions = Ptr(new WindowsUIAutomationSubscriptions);
			if (!adding)
			{
				if (!subscriptions->events.Keys().Contains(eventId)) return E_INVALIDARG;
				for (auto property : properties) if (!subscriptions->properties.Keys().Contains(property)) return E_INVALIDARG;
			}
			auto change = adding ? 1 : -1;
			UiaChangeSubscription(subscriptions->events, eventId, change);
			UiaChangeSubscription(node->context->subscriptions.events, eventId, change);
			for (auto property : properties)
			{
				UiaChangeSubscription(subscriptions->properties, property, change);
				UiaChangeSubscription(node->context->subscriptions.properties, property, change);
			}
			node->context->RefreshObservation();
			return S_OK;
		});
	}
	HRESULT WindowsUIAutomationProvider::AdviseEventAdded(EVENTID eventId, SAFEARRAY* propertyIds) { return AdviseEvent(eventId, propertyIds, true); }
	HRESULT WindowsUIAutomationProvider::AdviseEventRemoved(EVENTID eventId, SAFEARRAY* propertyIds) { return AdviseEvent(eventId, propertyIds, false); }

	HRESULT WindowsUIAutomationProvider::QueryInterface(REFIID iid, void** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IRawElementProviderSimple)) *result = static_cast<IRawElementProviderSimple*>(this);
		else if (iid == __uuidof(IRawElementProviderFragment)) *result = static_cast<IRawElementProviderFragment*>(this);
		else if (iid == __uuidof(IRawElementProviderAdviseEvents) && rootProvider) *result = static_cast<IRawElementProviderAdviseEvents*>(this);
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
				if (label->IsLive()) { result->vt = VT_UNKNOWN; label->Provider()->QueryInterface(IID_PPV_ARGS(&result->punkVal)); }
			}
			break;
		case UIA_HelpTextPropertyId: if (kind == Kind::Control) string(UiaLocalizedText(control, L"HelpText", UiaTooltipText(control->GetTooltipControl()))); break;
		case UIA_ControlTypePropertyId: number(Role()); break;
		case UIA_FrameworkIdPropertyId: string(L"GacUI"); break;
		case UIA_ClassNamePropertyId: string(L"GacUI." + itow(Role())); break;
		case UIA_AutomationIdPropertyId:
			if (auto metadata = UiaMetadata(control); kind == Kind::Control && metadata) string(metadata->id);
			else string(L"");
			break;
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
			if (auto tab = dynamic_cast<GuiTab*>(control); kind == Kind::Control && tab)
			{
				auto order = tab->TypedControlTemplateObject(true)->GetTabOrder();
				number(order == TabPageOrder::TopToBottom || order == TabPageOrder::BottomToTop ? OrientationType_Vertical : OrientationType_Horizontal);
			}
			else if (kind == Kind::Header || kind == Kind::Control && dynamic_cast<GuiMenuBar*>(control)) number(OrientationType_Horizontal);
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
					return UiaString(kind == Kind::Cell ? reflection::description::UnboxValue<WString>(UiaDataGrid(control)->GetBindingCellValue(row, column)) : control->GetText(), &result->bstrVal);
				}
			}
			break;
		case UIA_ValueIsReadOnlyPropertyId: if (Supports(UIA_ValuePatternId)) boolean(kind == Kind::Cell ? !UiaDataGrid(control)->GetCellDataEditorFactory(row, column) : UiaDocument(control)->GetEditMode() != GuiDocumentEditMode::Editable); break;
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
#define UIA_RANGE_PROPERTY(NAME, VALUE) \
		case UIA_RangeValue##NAME##PropertyId: \
			if (Supports(UIA_RangeValuePatternId)) { auto scroll = dynamic_cast<GuiScroll*>(control); result->vt = VT_R8; result->dblVal = (double)(VALUE); } \
			break;
		UIA_RANGE_PROPERTY(Minimum, 0)
		UIA_RANGE_PROPERTY(Maximum, scroll->GetMaxPosition())
		UIA_RANGE_PROPERTY(SmallChange, Role() == UIA_ProgressBarControlTypeId ? std::numeric_limits<double>::quiet_NaN() : scroll->GetSmallMove())
		UIA_RANGE_PROPERTY(LargeChange, Role() == UIA_ProgressBarControlTypeId ? std::numeric_limits<double>::quiet_NaN() : scroll->GetBigMove())
#undef UIA_RANGE_PROPERTY
#define UIA_SCROLL_PROPERTY(NAME, AXIS, VALUE) \
		case UIA_Scroll##NAME##PropertyId: \
			if (Supports(UIA_ScrollPatternId)) { auto scroll = dynamic_cast<GuiScrollView*>(control)->Get##AXIS##Scroll(); result->vt = VT_R8; result->dblVal = (VALUE); } \
			break;
		UIA_SCROLL_PROPERTY(HorizontalScrollPercent, Horizontal, scroll->GetMaxPosition() ? 100.0 * scroll->GetPosition() / scroll->GetMaxPosition() : UIA_ScrollPatternNoScroll)
		UIA_SCROLL_PROPERTY(VerticalScrollPercent, Vertical, scroll->GetMaxPosition() ? 100.0 * scroll->GetPosition() / scroll->GetMaxPosition() : UIA_ScrollPatternNoScroll)
		UIA_SCROLL_PROPERTY(HorizontalViewSize, Horizontal, scroll->GetTotalSize() ? min(100.0, 100.0 * scroll->GetPageSize() / scroll->GetTotalSize()) : 100.0)
		UIA_SCROLL_PROPERTY(VerticalViewSize, Vertical, scroll->GetTotalSize() ? min(100.0, 100.0 * scroll->GetPageSize() / scroll->GetTotalSize()) : 100.0)
#undef UIA_SCROLL_PROPERTY
		case UIA_RangeValueIsReadOnlyPropertyId: if (Supports(UIA_RangeValuePatternId)) boolean(Role() == UIA_ProgressBarControlTypeId); break;
		case UIA_ScrollHorizontallyScrollablePropertyId: if (Supports(UIA_ScrollPatternId)) boolean(dynamic_cast<GuiScrollView*>(control)->GetHorizontalScroll()->GetMaxPosition() > 0); break;
		case UIA_ScrollVerticallyScrollablePropertyId: if (Supports(UIA_ScrollPatternId)) boolean(dynamic_cast<GuiScrollView*>(control)->GetVerticalScroll()->GetMaxPosition() > 0); break;
		case UIA_MultipleViewCurrentViewPropertyId: if (Supports(UIA_MultipleViewPatternId)) number((LONG)dynamic_cast<GuiVirtualListView*>(control)->GetView()); break;
		case UIA_SelectionCanSelectMultiplePropertyId:
			if (Supports(UIA_SelectionPatternId)) { auto list = dynamic_cast<GuiSelectableListControl*>(control); boolean(list && !UiaDataGrid(list) && list->GetMultiSelect()); }
			break;
		case UIA_SelectionIsSelectionRequiredPropertyId:
			if (Supports(UIA_SelectionPatternId))
			{
				if (auto tab = dynamic_cast<GuiTab*>(control)) boolean(tab->GetPages().Count() > 0);
				else if (kind == Kind::RadioGroup) boolean(From(Children()).Any([](auto child) { return child->IsSelected(); }));
				else if (auto combo = dynamic_cast<GuiComboBoxListControl*>(control)) boolean(combo->GetSelectedIndex() >= 0);
				else if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(control)) boolean(grid->GetSelectedCell().row >= 0);
				else boolean(dynamic_cast<GuiDatePicker*>(control) != nullptr);
			}
			break;
#define UIA_WINDOW_PROPERTY(NAME, VALUE) \
		case UIA_Window##NAME##PropertyId: \
			if (Supports(UIA_WindowPatternId)) { auto window = dynamic_cast<GuiWindow*>(control); boolean(VALUE); } \
			break;
		UIA_WINDOW_PROPERTY(CanMaximize, window->GetMaximizedBox())
		UIA_WINDOW_PROPERTY(CanMinimize, window->GetMinimizedBox())
		UIA_WINDOW_PROPERTY(IsModal, window->GetModal())
		UIA_WINDOW_PROPERTY(IsTopmost, window->GetTopMost())
#undef UIA_WINDOW_PROPERTY
		case UIA_GridRowCountPropertyId: case UIA_GridColumnCountPropertyId:
			if (Supports(UIA_GridPatternId))
			{
				auto size = UiaSpatialGrid(control);
				if (property == UIA_GridRowCountPropertyId) number((LONG)(size.x >= 0 ? size.y : UiaCalendar(control) ? 6 : dynamic_cast<GuiListControl*>(control)->GetItemProvider()->Count()));
				else number((LONG)(size.x >= 0 ? size.x : UiaCalendar(control) ? 7 : UiaListView(control)->GetColumnCount()));
			}
			break;
		case UIA_WindowWindowVisualStatePropertyId:
			if (Supports(UIA_WindowPatternId))
			{
				auto state = Window()->GetNativeWindow()->GetSizeState();
				number(state == INativeWindow::Maximized ? WindowVisualState_Maximized : state == INativeWindow::Minimized ? WindowVisualState_Minimized : WindowVisualState_Normal);
			}
			break;
		case UIA_WindowWindowInteractionStatePropertyId:
			if (Supports(UIA_WindowPatternId)) number(Window()->GetBlockedByModalWindow() ? WindowInteractionState_BlockedByModalWindow : Window()->GetNativeWindow()->IsEnabled() ? WindowInteractionState_ReadyForUserInteraction : WindowInteractionState_Running);
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

#include "UiaCatalog.Actions.Windows.h"

using namespace vl;
using namespace vl::collections;

namespace uialist::native
{
	ActionOutcome UiaSession::Execute(vint nodeKey, const ActionSpec& action, const List<ActionArgument>& arguments)
	{
		if (action.rangeKey) return ExecuteRange(action, arguments);
		ActionOutcome result;
		result.value = Ptr(new ValueData);
		auto current = DescribeActions(nodeKey, action.pattern);
		Ptr<ActionSpec> capability;
		if (current) for (auto&& item : current->commands) if (item->code == action.code) capability = item;
		if (!capability || !capability->enabled || arguments.Count() != capability->parameters.Count()) { result.available = false; return result; }
		for (vint i = 0; i < arguments.Count(); i++)
		{
			ActionArgument verified;
			verified.value = arguments[i].value;
			if (!ParseArgument(*capability->parameters[i].Obj(), arguments[i].text, arguments[i].reference, verified)) { result.available = false; return result; }
			if (verified.kind == ArgumentKind::Element && verified.reference && !elements.Keys().Contains(verified.reference)) { result.available = false; return result; }
		}
		auto element = elements[nodeKey];
		auto id = action.pattern;
		auto pattern = id ? AcquireCatalogPattern(element.Obj(), id) : ComPtr<IUnknown>();
		if (id && !pattern) { result.available = false; return result; }
		auto integer = [&](vint i) { return arguments[i].integer; };
		auto number = [&](vint i) { return arguments[i].number; };
		auto text = [&](vint i) { return arguments[i].text.Buffer(); };
		auto reference = [&](vint i) { return arguments[i].reference ? elements[arguments[i].reference].Obj() : nullptr; };
		auto check = [&](HRESULT hr) { CheckUia(hr, action.name + L"; pattern=" + itow(id) + L"; element=" + itow(nodeKey)); };
		auto objectResult = [&](IUnknown* raw) { ComPtr<IUnknown> keep(raw); result.value = ConvertObject(raw); };
		auto opaqueResult = [&](IUnknown* raw, const wchar_t* name)
		{
			result.value->type = VT_UNKNOWN;
			if (!raw) return;
			result.value->kind = ValueKind::Opaque; result.value->key = nextKey++; result.value->text = name;
			opaqueObjects.Add(result.value->key, ComPtr<IUnknown>(raw));
		};
		auto rangeResult = [&](IUIAutomationTextRange* raw, vint document) { ComPtr<IUIAutomationTextRange> keep(raw); result.value = ConvertRange(raw, document, action.name); };
		auto rangesResult = [&](IUIAutomationTextRangeArray* raw) { ComPtr<IUIAutomationTextRangeArray> keep(raw); result.value = ConvertRanges(raw, nodeKey, action.name); };
		auto arrayResult = [&](SAFEARRAY* raw, VARTYPE type) { result.value = ConvertArray(raw, type); SafeArrayDestroy(raw); };
		auto set = [&](PROPERTYID property) { result.available = SetProperty(nodeKey, property, arguments[0].text); };
		result.mutation = capability->mutation;
		switch (action.code)
		{
		case ActionCode::SetFocus: check(element->SetFocus()); break;
		case ActionCode::ClickablePoint:
			{
				POINT point = {}; BOOL available = FALSE; check(element->GetClickablePoint(&point, &available));
				result.value->kind = ValueKind::Array; result.value->text = L"available, screenX, screenY";
				result.value->items.Add(BooleanValue(available)); if (available) { result.value->items.Add(IntegerValue(point.x)); result.value->items.Add(IntegerValue(point.y)); }
			}
			break;
		case ActionCode::ElementMenu: check(QueryInterface<IUIAutomationElement3>(element.Obj())->ShowContextMenu()); break;
		case ActionCode::Metadata:
			{
				VARIANT value; VariantInit(&value); check(QueryInterface<IUIAutomationElement7>(element.Obj())->GetCurrentMetadataValue(integer(0), integer(1), &value)); result.value = Convert(value); VariantClear(&value);
			}
			break;
		case ActionCode::Invoke: check(PatternInterface<IUIAutomationInvokePattern>(pattern.Obj())->Invoke()); break;
		case ActionCode::SetValue: set(UIA_ValueValuePropertyId); break;
		case ActionCode::SetRangeValue: set(UIA_RangeValueValuePropertyId); break;
		case ActionCode::SetDock: set(UIA_DockDockPositionPropertyId); break;
		case ActionCode::SetView: set(UIA_MultipleViewCurrentViewPropertyId); break;
		case ActionCode::WindowState: set(UIA_WindowWindowVisualStatePropertyId); break;
		case ActionCode::Zoom: set(UIA_Transform2ZoomLevelPropertyId); break;
		case ActionCode::Scroll: case ActionCode::SetScrollPercent:
			{
				auto p = PatternInterface<IUIAutomationScrollPattern>(pattern.Obj()); BOOL horizontal = FALSE, vertical = FALSE;
				check(p->get_CurrentHorizontallyScrollable(&horizontal)); check(p->get_CurrentVerticallyScrollable(&vertical));
				if (action.code == ActionCode::Scroll) check(p->Scroll(horizontal ? static_cast<ScrollAmount>(integer(0)) : ScrollAmount_NoAmount, vertical ? static_cast<ScrollAmount>(integer(1)) : ScrollAmount_NoAmount));
				else check(p->SetScrollPercent(horizontal && !integer(0) ? number(1) : UIA_ScrollPatternNoScroll, vertical && !integer(2) ? number(3) : UIA_ScrollPatternNoScroll));
			}
			break;
		case ActionCode::Expand: check(PatternInterface<IUIAutomationExpandCollapsePattern>(pattern.Obj())->Expand()); break;
		case ActionCode::Collapse: check(PatternInterface<IUIAutomationExpandCollapsePattern>(pattern.Obj())->Collapse()); break;
		case ActionCode::GridItem:
			{
				IUIAutomationElement* value = nullptr; check(PatternInterface<IUIAutomationGridPattern>(pattern.Obj())->GetItem(integer(0), integer(1), &value)); objectResult(value);
			}
			break;
		case ActionCode::ViewName:
			{
				BSTR value = nullptr; check(PatternInterface<IUIAutomationMultipleViewPattern>(pattern.Obj())->GetViewName(integer(0), &value)); result.value = StringValue(value ? WString::CopyFrom(value, SysStringLen(value)) : WString()); SysFreeString(value);
			}
			break;
		case ActionCode::WaitForIdle:
			{
				BOOL idle = FALSE; check(PatternInterface<IUIAutomationWindowPattern>(pattern.Obj())->WaitForInputIdle(integer(0), &idle)); result.value = BooleanValue(idle);
			}
			break;
		case ActionCode::CloseWindow: check(PatternInterface<IUIAutomationWindowPattern>(pattern.Obj())->Close()); result.closedWindow = true; break;
		case ActionCode::SelectItem: check(PatternInterface<IUIAutomationSelectionItemPattern>(pattern.Obj())->Select()); break;
		case ActionCode::AddItem: check(PatternInterface<IUIAutomationSelectionItemPattern>(pattern.Obj())->AddToSelection()); break;
		case ActionCode::RemoveItem: check(PatternInterface<IUIAutomationSelectionItemPattern>(pattern.Obj())->RemoveFromSelection()); break;
		case ActionCode::Selection:
			{
				IUIAutomationElementArray* value = nullptr; check(PatternInterface<IUIAutomationSelectionPattern>(pattern.Obj())->GetCurrentSelection(&value)); objectResult(value);
			}
			break;
		case ActionCode::RowHeaders: case ActionCode::ColumnHeaders:
			{
				auto p = PatternInterface<IUIAutomationTablePattern>(pattern.Obj()); IUIAutomationElementArray* value = nullptr;
				check(action.code == ActionCode::RowHeaders ? p->GetCurrentRowHeaders(&value) : p->GetCurrentColumnHeaders(&value)); objectResult(value);
			}
			break;
		case ActionCode::RowHeaderItems: case ActionCode::ColumnHeaderItems:
			{
				auto p = PatternInterface<IUIAutomationTableItemPattern>(pattern.Obj()); IUIAutomationElementArray* value = nullptr;
				check(action.code == ActionCode::RowHeaderItems ? p->GetCurrentRowHeaderItems(&value) : p->GetCurrentColumnHeaderItems(&value)); objectResult(value);
			}
			break;
		case ActionCode::DocumentRange: case ActionCode::RangeFromPoint: case ActionCode::RangeFromChild:
			{
				auto p = PatternInterface<IUIAutomationTextPattern>(pattern.Obj()); IUIAutomationTextRange* value = nullptr;
				if (action.code == ActionCode::DocumentRange) check(p->get_DocumentRange(&value));
				else if (action.code == ActionCode::RangeFromChild) check(p->RangeFromChild(reference(0), &value));
				else check(p->RangeFromPoint({ integer(0), integer(1) }, &value));
				rangeResult(value, nodeKey);
			}
			break;
		case ActionCode::TextSelection: case ActionCode::VisibleRanges:
			{
				auto p = PatternInterface<IUIAutomationTextPattern>(pattern.Obj()); IUIAutomationTextRangeArray* value = nullptr;
				check(action.code == ActionCode::TextSelection ? p->GetSelection(&value) : p->GetVisibleRanges(&value)); rangesResult(value);
			}
			break;
		case ActionCode::Toggle: check(PatternInterface<IUIAutomationTogglePattern>(pattern.Obj())->Toggle()); break;
		case ActionCode::Move: check(PatternInterface<IUIAutomationTransformPattern>(pattern.Obj())->Move(number(0), number(1))); break;
		case ActionCode::Resize: check(PatternInterface<IUIAutomationTransformPattern>(pattern.Obj())->Resize(number(0), number(1))); break;
		case ActionCode::Rotate: check(PatternInterface<IUIAutomationTransformPattern>(pattern.Obj())->Rotate(number(0))); break;
		case ActionCode::ScrollItem: check(PatternInterface<IUIAutomationScrollItemPattern>(pattern.Obj())->ScrollIntoView()); break;
		case ActionCode::LegacySelection:
			{
				IUIAutomationElementArray* value = nullptr; check(PatternInterface<IUIAutomationLegacyIAccessiblePattern>(pattern.Obj())->GetCurrentSelection(&value)); objectResult(value);
			}
			break;
		case ActionCode::LegacySelect: check(PatternInterface<IUIAutomationLegacyIAccessiblePattern>(pattern.Obj())->Select(integer(0))); break;
		case ActionCode::LegacyDefault: check(PatternInterface<IUIAutomationLegacyIAccessiblePattern>(pattern.Obj())->DoDefaultAction()); break;
		case ActionCode::LegacyValue: check(PatternInterface<IUIAutomationLegacyIAccessiblePattern>(pattern.Obj())->SetValue(text(0))); break;
		case ActionCode::LegacyObject:
			{
				IAccessible* value = nullptr; check(PatternInterface<IUIAutomationLegacyIAccessiblePattern>(pattern.Obj())->GetIAccessible(&value)); opaqueResult(value, L"IAccessible");
			}
			break;
		case ActionCode::FindItem:
			{
				VARIANT argument; VariantInit(&argument); if (integer(1)) ToVariant(*this, *arguments[2].value.Obj(), argument);
				IUIAutomationElement* value = nullptr; auto hr = PatternInterface<IUIAutomationItemContainerPattern>(pattern.Obj())->FindItemByProperty(reference(0), integer(1), argument, &value); VariantClear(&argument); check(hr); objectResult(value);
			}
			break;
		case ActionCode::Realize: check(PatternInterface<IUIAutomationVirtualizedItemPattern>(pattern.Obj())->Realize()); break;
		case ActionCode::StartListening:
			{
				auto p = PatternInterface<IUIAutomationSynchronizedInputPattern>(pattern.Obj()); check(p->StartListening(static_cast<SynchronizedInputType>(integer(0)))); listening.Add(nodeKey, p);
			}
			break;
		case ActionCode::CancelListening: check(listening[nodeKey]->Cancel()); listening.Remove(nodeKey); break;
		case ActionCode::ObjectModel:
			{
				IUnknown* value = nullptr; check(PatternInterface<IUIAutomationObjectModelPattern>(pattern.Obj())->GetUnderlyingObjectModel(&value)); opaqueResult(value, L"IUnknown");
			}
			break;
		case ActionCode::RangeFromAnnotation:
			{
				IUIAutomationTextRange* value = nullptr; check(PatternInterface<IUIAutomationTextPattern2>(pattern.Obj())->RangeFromAnnotation(reference(0), &value)); rangeResult(value, nodeKey);
			}
			break;
		case ActionCode::CaretRange:
			{
				IUIAutomationTextRange* value = nullptr; BOOL active = FALSE; check(PatternInterface<IUIAutomationTextPattern2>(pattern.Obj())->GetCaretRange(&active, &value));
				ComPtr<IUIAutomationTextRange> keep(value); result.value->kind = ValueKind::Array; result.value->text = L"isActive, range";
				result.value->items.Add(BooleanValue(active)); result.value->items.Add(ConvertRange(value, nodeKey, action.name));
			}
			break;
		case ActionCode::ExtendedProperties:
			{
				ExtendedProperty* values = nullptr; int count = 0; auto hr = PatternInterface<IUIAutomationStylesPattern>(pattern.Obj())->GetCurrentExtendedPropertiesAsArray(&values, &count);
				if (hr == UIA_E_NOTSUPPORTED) result.value->kind = ValueKind::Unsupported;
				else
				{
					check(hr); result.value->kind = ValueKind::Array;
					for (int i = 0; i < count; i++)
					{
						auto pair = Ptr(new ValueData); pair->kind = ValueKind::Array;
						pair->items.Add(StringValue(values[i].PropertyName ? WString::CopyFrom(values[i].PropertyName, SysStringLen(values[i].PropertyName)) : WString()));
						pair->items.Add(StringValue(values[i].PropertyValue ? WString::CopyFrom(values[i].PropertyValue, SysStringLen(values[i].PropertyValue)) : WString()));
						result.value->items.Add(pair); SysFreeString(values[i].PropertyName); SysFreeString(values[i].PropertyValue);
					}
				}
				CoTaskMemFree(values);
			}
			break;
		case ActionCode::SpreadsheetItem:
			{
				IUIAutomationElement* value = nullptr; BSTR name = SysAllocStringLen(arguments[0].text.Buffer(), static_cast<UINT>(arguments[0].text.Length())); if (!name) throw Exception(L"SysAllocStringLen failed");
				auto hr = PatternInterface<IUIAutomationSpreadsheetPattern>(pattern.Obj())->GetItemByName(name, &value); SysFreeString(name); check(hr); objectResult(value);
			}
			break;
		case ActionCode::AnnotationObjects:
			{
				IUIAutomationElementArray* value = nullptr; check(PatternInterface<IUIAutomationSpreadsheetItemPattern>(pattern.Obj())->GetCurrentAnnotationObjects(&value)); objectResult(value);
			}
			break;
		case ActionCode::AnnotationTypes:
			{
				SAFEARRAY* value = nullptr; check(PatternInterface<IUIAutomationSpreadsheetItemPattern>(pattern.Obj())->GetCurrentAnnotationTypes(&value)); arrayResult(value, VT_I4);
			}
			break;
		case ActionCode::ZoomByUnit: check(PatternInterface<IUIAutomationTransformPattern2>(pattern.Obj())->ZoomByUnit(static_cast<ZoomUnit>(integer(0)))); break;
		case ActionCode::ChildRange:
			{
				auto p = PatternInterface<IUIAutomationTextChildPattern>(pattern.Obj()); IUIAutomationElement* container = nullptr; check(p->get_TextContainer(&container));
				ComPtr<IUIAutomationElement> keep(container); auto document = RetainElement(keep);
				IUIAutomationTextRange* value = nullptr; check(p->get_TextRange(&value)); rangeResult(value, document);
			}
			break;
		case ActionCode::GrabbedItems:
			{
				IUIAutomationElementArray* value = nullptr; check(PatternInterface<IUIAutomationDragPattern>(pattern.Obj())->GetCurrentGrabbedItems(&value)); objectResult(value);
			}
			break;
		case ActionCode::CompositionRange: case ActionCode::ConversionRange:
			{
				auto p = PatternInterface<IUIAutomationTextEditPattern>(pattern.Obj()); IUIAutomationTextRange* value = nullptr;
				check(action.code == ActionCode::CompositionRange ? p->GetActiveComposition(&value) : p->GetConversionTarget(&value)); rangeResult(value, nodeKey);
			}
			break;
		case ActionCode::Navigate:
			{
				IUIAutomationElement* value = nullptr; check(PatternInterface<IUIAutomationCustomNavigationPattern>(pattern.Obj())->Navigate(static_cast<NavigateDirection>(integer(0)), &value)); objectResult(value);
			}
			break;
		default: CHECK_FAIL(L"Action has no compiled dispatcher");
		}
		if (!result.available) result.mutation = false;
		if (!result.closedWindow) result.references = GetReferences();
		return result;
	}
}

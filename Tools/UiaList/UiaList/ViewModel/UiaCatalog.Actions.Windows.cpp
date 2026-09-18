#include "UiaCatalog.Actions.Windows.h"
#include <cmath>
#include <limits>

using namespace vl;
using namespace vl::collections;

namespace uialist::native
{
	Ptr<ValueData> IntegerValue(vint64_t value) { auto v = Ptr(new ValueData); v->kind = ValueKind::Signed; v->type = VT_I4; v->signedValue = value; return v; }
	Ptr<ValueData> NumberValue(double value) { auto v = Ptr(new ValueData); v->kind = ValueKind::Real; v->type = VT_R8; v->realValue = value; return v; }
	Ptr<ValueData> BooleanValue(bool value) { auto v = IntegerValue(value ? 1 : 0); v->kind = ValueKind::Boolean; v->type = VT_BOOL; return v; }
	Ptr<ValueData> StringValue(const WString& value) { auto v = Ptr(new ValueData); v->kind = ValueKind::String; v->type = VT_BSTR; v->text = value; return v; }

	Ptr<ArgumentSpec> TextArgument(const wchar_t* name, const WString& initial, bool multiline)
	{
		auto p = Ptr(new ArgumentSpec); p->name = name; p->initial = initial; p->multiline = multiline; return p;
	}
	Ptr<ArgumentSpec> NumberArgument(const wchar_t* name, double minimum, double maximum, double initial, bool integer)
	{
		auto p = TextArgument(name, integer ? i64tow(static_cast<vint64_t>(initial)) : ftow(initial));
		p->kind = integer ? ArgumentKind::Integer : ArgumentKind::Number; p->minimum = minimum; p->maximum = maximum; return p;
	}
	Ptr<ArgumentSpec> ChoiceArgument(const wchar_t* name, std::initializer_list<SetterChoice> choices)
	{
		auto p = TextArgument(name); p->kind = ArgumentKind::Choice;
		for (auto&& c : choices) p->choices.Add(c);
		if (p->choices.Count()) p->initial = itow(p->choices[0].id);
		return p;
	}
	Ptr<ArgumentSpec> ReferenceArgument(const wchar_t* name, bool range, bool nullable)
	{
		auto p = TextArgument(name); p->kind = range ? ArgumentKind::Range : ArgumentKind::Element; p->nullable = nullable; return p;
	}
	Ptr<ArgumentSpec> CatalogArgument(const wchar_t* name, const IdDescriptor* catalog, vint count, bool allowZero)
	{
		auto p = ChoiceArgument(name, {});
		if (allowZero) p->choices.Add({ 0, L"0" });
		for (vint i = 0; i < count; i++) p->choices.Add({ catalog[i].id, WString(catalog[i].name) + L" (" + itow(catalog[i].id) + L")" });
		if (p->choices.Count()) p->initial = itow(p->choices[0].id);
		return p;
	}
	Ptr<ActionSpec> AddAction(ActionSectionData& section, ActionCode code, const wchar_t* name, bool mutation, std::initializer_list<Ptr<ArgumentSpec>> arguments, bool enabled)
	{
		auto command = Ptr(new ActionSpec); command->code = code; command->pattern = section.pattern; command->rangeKey = section.rangeKey;
		command->name = name; command->mutation = mutation; command->enabled = enabled;
		switch (code)
		{
		case ActionCode::ClickablePoint: case ActionCode::Metadata: case ActionCode::GridItem:
		case ActionCode::ViewName: case ActionCode::Selection: case ActionCode::RowHeaders:
		case ActionCode::ColumnHeaders: case ActionCode::RowHeaderItems: case ActionCode::ColumnHeaderItems:
		case ActionCode::DocumentRange: case ActionCode::TextSelection: case ActionCode::VisibleRanges:
		case ActionCode::RangeFromPoint: case ActionCode::RangeFromChild: case ActionCode::LegacySelection:
		case ActionCode::LegacyObject: case ActionCode::FindItem: case ActionCode::ObjectModel:
		case ActionCode::RangeFromAnnotation: case ActionCode::CaretRange: case ActionCode::ExtendedProperties:
		case ActionCode::SpreadsheetItem: case ActionCode::AnnotationObjects: case ActionCode::AnnotationTypes:
		case ActionCode::ChildRange: case ActionCode::GrabbedItems: case ActionCode::CompositionRange:
		case ActionCode::ConversionRange: case ActionCode::Navigate:
			command->pureGetter = true;
			break;
		}
		for (auto&& p : arguments) command->parameters.Add(p);
		section.commands.Add(command); return command;
	}
	void AddReadout(ActionSectionData& section, const wchar_t* name, Ptr<ValueData> value)
	{
		PropertyData p; p.name = name; p.value = value; p.sourcePattern = section.pattern; section.readouts.Add(p);
	}

	bool ParseArgument(const ArgumentSpec& spec, const WString& text, vint reference, ActionArgument& value)
	{
		value.kind = spec.kind; value.text = text; value.reference = reference;
		bool valid = true;
		switch (spec.kind)
		{
		case ArgumentKind::Text:
			for (vint i = 0; i < text.Length(); i++) if (!text[i]) return false;
			return true;
		case ArgumentKind::Integer: case ArgumentKind::Choice:
			{
				auto parsed = wtoi64_test(text, valid);
				if (!valid || parsed < INT_MIN || parsed > INT_MAX) return false;
				value.integer = static_cast<int>(parsed);
				if (spec.kind == ArgumentKind::Integer) return parsed >= spec.minimum && parsed <= spec.maximum;
				for (auto&& choice : spec.choices) if (choice.id == value.integer) return true;
				return false;
			}
		case ArgumentKind::Number:
			value.number = wtof_test(text, valid);
			return valid && std::isfinite(value.number) && value.number >= spec.minimum && value.number <= spec.maximum;
		case ArgumentKind::Element: case ArgumentKind::Range: return reference != 0 || spec.nullable;
		case ArgumentKind::Variant: return value.value != nullptr;
		case ArgumentKind::AttributeList:
			{
				vint begin = 0;
				for (vint i = 0; i <= text.Length(); i++) if (i == text.Length() || text[i] == L',')
				{
					auto id = wtoi_test(text.Sub(begin, i - begin), valid); if (!valid) return false;
					bool found = false; for (vint j = 0; j < AttributeCatalogCount; j++) if (AttributeCatalog[j].id == id) found = true;
					if (!found) return false; value.ids.Add(static_cast<int>(id)); begin = i + 1;
				}
				return value.ids.Count() > 0;
			}
		}
		return false;
	}

	class ElementArrayArgument : public IUIAutomationElementArray
	{
	public:
		std::atomic<ULONG> references = 1;
		List<ComPtr<IUIAutomationElement>> elements;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** output) override
		{
			*output = nullptr;
			if (iid != IID_IUnknown && iid != __uuidof(IUIAutomationElementArray)) return E_NOINTERFACE;
			*output = static_cast<IUIAutomationElementArray*>(this); AddRef(); return S_OK;
		}
		ULONG STDMETHODCALLTYPE AddRef() override { return ++references; }
		ULONG STDMETHODCALLTYPE Release() override { auto count = --references; if (!count) delete this; return count; }
		HRESULT STDMETHODCALLTYPE get_Length(int* output) override { *output = static_cast<int>(elements.Count()); return S_OK; }
		HRESULT STDMETHODCALLTYPE GetElement(int index, IUIAutomationElement** output) override
		{
			*output = nullptr; if (index < 0 || index >= elements.Count()) return E_INVALIDARG;
			*output = elements[index].Obj(); if (*output) (*output)->AddRef(); return S_OK;
		}
	};

	void ToVariant(UiaSession& session, const ValueData& value, VARIANT& result)
	{
		VariantInit(&result);
		result.vt = value.type;
		switch (value.kind)
		{
		case ValueKind::Null: result.vt = VT_EMPTY; break;
		case ValueKind::Boolean: result.vt = VT_BOOL; result.boolVal = value.signedValue ? VARIANT_TRUE : VARIANT_FALSE; break;
		case ValueKind::Signed: result.lVal = static_cast<LONG>(value.signedValue); break;
		case ValueKind::Unsigned: result.ulVal = static_cast<ULONG>(value.unsignedValue); break;
		case ValueKind::Real: result.vt = VT_R8; result.dblVal = value.realValue; break;
		case ValueKind::String: result.vt = VT_BSTR; result.bstrVal = SysAllocStringLen(value.text.Buffer(), static_cast<UINT>(value.text.Length())); if (!result.bstrVal) throw Exception(L"SysAllocStringLen failed"); break;
		case ValueKind::Element: result.vt = VT_UNKNOWN; result.punkVal = value.key ? session.elements[value.key].Obj() : nullptr; if (result.punkVal) result.punkVal->AddRef(); break;
		case ValueKind::Range: result.vt = VT_UNKNOWN; result.punkVal = value.key ? session.ranges[value.key].range.Obj() : nullptr; if (result.punkVal) result.punkVal->AddRef(); break;
		case ValueKind::Array:
			{
				if (value.type == VT_UNKNOWN)
				{
					auto array = new ElementArrayArgument;
					for (auto&& item : value.items) array->elements.Add(session.elements[item->key]);
					result.punkVal = array;
					break;
				}
				auto type = static_cast<VARTYPE>(value.type & VT_TYPEMASK);
				result.parray = SafeArrayCreateVector(type, 0, static_cast<ULONG>(value.items.Count()));
				if (!result.parray) throw Exception(L"SafeArrayCreateVector failed");
				for (LONG index = 0; index < value.items.Count(); index++)
				{
					VARIANT item; ToVariant(session, *value.items[index].Obj(), item);
					void* address = type == VT_BSTR ? static_cast<void*>(item.bstrVal) : type == VT_UNKNOWN ? static_cast<void*>(item.punkVal) : static_cast<void*>(&item.llVal);
					CheckUia(SafeArrayPutElement(result.parray, &index, address), L"SafeArrayPutElement"); VariantClear(&item);
				}
			}
			break;
		default: throw Exception(L"Invalid typed UIA argument");
		}
	}

	template<typename T> Ptr<ValueData> ReadScalar(UiaSession& session, T& pattern, HRESULT (STDMETHODCALLTYPE T::*getter)(int*), const wchar_t* name)
	{
		int v = 0; CheckUia((pattern.*getter)(&v), name); return IntegerValue(v);
	}

	Ptr<ActionSectionData> UiaSession::DescribeActions(vint nodeKey, PATTERNID id)
	{
		auto element = elements[nodeKey];
		auto section = Ptr(new ActionSectionData); section->pattern = id;
		ComPtr<IUnknown> pattern;
		if (id)
		{
			pattern = AcquireCatalogPattern(element.Obj(), id);
			if (!pattern) return nullptr;
			for (vint i = 0; i < PatternCatalogCount; i++) if (PatternCatalog[i].id == id)
				section->name = WString(PatternCatalog[i].provider) + L" / " + PatternCatalog[i].client + L" (" + itow(id) + L")";
		}
		else section->name = L"IUIAutomationElement";
		auto add = [&](ActionCode code, const wchar_t* name, bool mutation, std::initializer_list<Ptr<ArgumentSpec>> args = {}, bool enabled = true) { return AddAction(*section.Obj(), code, name, mutation, args, enabled); };
		auto read = [&](const wchar_t* name, Ptr<ValueData> value) { AddReadout(*section.Obj(), name, value); };
		auto setter = [&](PROPERTYID property, ActionCode code, const wchar_t* name)
		{
			auto spec = DescribeSetter(nodeKey, property);
			auto parameter = TextArgument(L"value");
			if (spec->kind == SetterKind::Number) parameter = NumberArgument(L"value", spec->minimum, spec->maximum, spec->minimum);
			else if (spec->kind == SetterKind::Choice) { parameter->kind = ArgumentKind::Choice; CopyFrom(parameter->choices, spec->choices); if (parameter->choices.Count()) parameter->initial = itow(parameter->choices[0].id); }
			else parameter->multiline = spec->multiline;
			VARIANT current; VariantInit(&current);
			CheckUia(element->GetCurrentPropertyValueEx(property, TRUE, &current), L"Setter current value");
			auto value = Convert(current); VariantClear(&current);
			if (value->kind == ValueKind::String) parameter->initial = value->text;
			else if (value->kind == ValueKind::Real) parameter->initial = ftow(value->realValue);
			else if (value->kind == ValueKind::Signed) parameter->initial = i64tow(value->signedValue);
			add(code, name, true, { parameter }, spec->kind != SetterKind::None);
		};
		const double limit = (std::numeric_limits<double>::max)();
		auto boolean = [](const wchar_t* name) { return ChoiceArgument(name, { { 0, L"false" }, { 1, L"true" } }); };
		// These macros keep each current getter and its typed result adjacent. No method name is interpreted at runtime.
#define UIA_WIDE_INNER(T) L##T
#define UIA_WIDE(T) UIA_WIDE_INNER(T)
#define READ_INT(P, M) { int v = 0; CheckUia(P->get_Current##M(&v), L"get_Current" UIA_WIDE(#M)); read(UIA_WIDE(#M), IntegerValue(v)); }
#define READ_BOOL(P, M) { BOOL v = FALSE; CheckUia(P->get_Current##M(&v), L"get_Current" UIA_WIDE(#M)); read(UIA_WIDE(#M), BooleanValue(v != FALSE)); }
#define READ_DOUBLE(P, M) { double v = 0; CheckUia(P->get_Current##M(&v), L"get_Current" UIA_WIDE(#M)); read(UIA_WIDE(#M), NumberValue(v)); }
#define READ_ENUM(P, M, T) { T v = {}; CheckUia(P->get_Current##M(&v), L"get_Current" UIA_WIDE(#M)); read(UIA_WIDE(#M), IntegerValue(v)); }
#define READ_STRING(P, M) { BSTR v = nullptr; CheckUia(P->get_Current##M(&v), L"get_Current" UIA_WIDE(#M)); read(UIA_WIDE(#M), StringValue(v ? WString::CopyFrom(v, SysStringLen(v)) : WString())); SysFreeString(v); }
#define READ_ELEMENT(P, M) { IUIAutomationElement* v = nullptr; CheckUia(P->get_Current##M(&v), L"get_Current" UIA_WIDE(#M)); ComPtr<IUIAutomationElement> keep(v); read(UIA_WIDE(#M), ConvertObject(v)); }
#define READ_ELEMENTS(P, M) { IUIAutomationElementArray* v = nullptr; CheckUia(P->M(&v), UIA_WIDE(#M)); ComPtr<IUIAutomationElementArray> keep(v); read(UIA_WIDE(#M), ConvertObject(v)); }
#define READ_RANGE(P, M, D) { IUIAutomationTextRange* v = nullptr; CheckUia(P->M(&v), UIA_WIDE(#M)); ComPtr<IUIAutomationTextRange> keep(v); read(UIA_WIDE(#M), ConvertRange(v, D, UIA_WIDE(#M))); }
#define READ_RANGES(P, M) { IUIAutomationTextRangeArray* v = nullptr; CheckUia(P->M(&v), UIA_WIDE(#M)); ComPtr<IUIAutomationTextRangeArray> keep(v); read(UIA_WIDE(#M), ConvertRanges(v, nodeKey, UIA_WIDE(#M))); }
		switch (id)
		{
		case 0:
			add(ActionCode::SetFocus, L"SetFocus", true);
			add(ActionCode::ClickablePoint, L"GetClickablePoint", false);
			if (QueryInterface<IUIAutomationElement3>(element.Obj())) add(ActionCode::ElementMenu, L"ShowContextMenu", true);
			if (QueryInterface<IUIAutomationElement7>(element.Obj())) add(ActionCode::Metadata, L"GetCurrentMetadataValue", false, { CatalogArgument(L"propertyId", PropertyCatalog, PropertyCatalogCount), CatalogArgument(L"metadataId", MetadataCatalog, MetadataCatalogCount) });
			break;
		case UIA_InvokePatternId: add(ActionCode::Invoke, L"Invoke", true); break;
		case UIA_SelectionPatternId: case UIA_SelectionPattern2Id:
			{
				auto p = PatternInterface<IUIAutomationSelectionPattern>(pattern.Obj());
				READ_BOOL(p, CanSelectMultiple); READ_BOOL(p, IsSelectionRequired);
				READ_ELEMENTS(p, GetCurrentSelection);
				add(ActionCode::Selection, L"GetCurrentSelection", false);
				if (id == UIA_SelectionPattern2Id)
				{
					auto p2 = PatternInterface<IUIAutomationSelectionPattern2>(pattern.Obj());
					READ_ELEMENT(p2, FirstSelectedItem); READ_ELEMENT(p2, LastSelectedItem); READ_ELEMENT(p2, CurrentSelectedItem); READ_INT(p2, ItemCount);
				}
			}
			break;
		case UIA_ValuePatternId:
			{
				auto p = PatternInterface<IUIAutomationValuePattern>(pattern.Obj()); READ_STRING(p, Value); READ_BOOL(p, IsReadOnly);
				setter(UIA_ValueValuePropertyId, ActionCode::SetValue, L"SetValue");
			}
			break;
		case UIA_RangeValuePatternId:
			{
				auto p = PatternInterface<IUIAutomationRangeValuePattern>(pattern.Obj());
				READ_DOUBLE(p, Value); READ_BOOL(p, IsReadOnly); READ_DOUBLE(p, Minimum); READ_DOUBLE(p, Maximum); READ_DOUBLE(p, SmallChange); READ_DOUBLE(p, LargeChange);
				setter(UIA_RangeValueValuePropertyId, ActionCode::SetRangeValue, L"SetValue");
			}
			break;
		case UIA_ScrollPatternId:
			{
				auto p = PatternInterface<IUIAutomationScrollPattern>(pattern.Obj());
				READ_DOUBLE(p, HorizontalScrollPercent); READ_DOUBLE(p, VerticalScrollPercent); READ_DOUBLE(p, HorizontalViewSize); READ_DOUBLE(p, VerticalViewSize);
				READ_BOOL(p, HorizontallyScrollable); READ_BOOL(p, VerticallyScrollable);
				auto amount = [](const wchar_t* name) { return ChoiceArgument(name, { { ScrollAmount_NoAmount, L"NoAmount" }, { ScrollAmount_LargeDecrement, L"LargeDecrement" }, { ScrollAmount_SmallDecrement, L"SmallDecrement" }, { ScrollAmount_LargeIncrement, L"LargeIncrement" }, { ScrollAmount_SmallIncrement, L"SmallIncrement" } }); };
				add(ActionCode::Scroll, L"Scroll", true, { amount(L"horizontalAmount"), amount(L"verticalAmount") });
				add(ActionCode::SetScrollPercent, L"SetScrollPercent", true, { boolean(L"horizontalNoScroll"), NumberArgument(L"horizontalPercent", 0, 100), boolean(L"verticalNoScroll"), NumberArgument(L"verticalPercent", 0, 100) });
			}
			break;
		case UIA_ExpandCollapsePatternId:
			{
				auto p = PatternInterface<IUIAutomationExpandCollapsePattern>(pattern.Obj()); ExpandCollapseState state;
				CheckUia(p->get_CurrentExpandCollapseState(&state), L"ExpandCollapseState"); read(L"ExpandCollapseState", IntegerValue(state));
				add(ActionCode::Expand, L"Expand", true, {}, state != ExpandCollapseState_LeafNode && state != ExpandCollapseState_Expanded);
				add(ActionCode::Collapse, L"Collapse", true, {}, state != ExpandCollapseState_LeafNode && state != ExpandCollapseState_Collapsed);
			}
			break;
		case UIA_GridPatternId:
			{
				auto p = PatternInterface<IUIAutomationGridPattern>(pattern.Obj()); int rows = 0, columns = 0;
				CheckUia(p->get_CurrentRowCount(&rows), L"RowCount"); CheckUia(p->get_CurrentColumnCount(&columns), L"ColumnCount");
				read(L"RowCount", IntegerValue(rows)); read(L"ColumnCount", IntegerValue(columns));
				add(ActionCode::GridItem, L"GetItem", false, { NumberArgument(L"row", 0, rows - 1, 0, true), NumberArgument(L"column", 0, columns - 1, 0, true) }, rows > 0 && columns > 0);
			}
			break;
		case UIA_GridItemPatternId:
			{
				auto p = PatternInterface<IUIAutomationGridItemPattern>(pattern.Obj());
				READ_INT(p, Row); READ_INT(p, Column); READ_INT(p, RowSpan); READ_INT(p, ColumnSpan); READ_ELEMENT(p, ContainingGrid);
			}
			break;
		case UIA_MultipleViewPatternId:
			{
				auto p = PatternInterface<IUIAutomationMultipleViewPattern>(pattern.Obj()); READ_INT(p, CurrentView);
				SAFEARRAY* values = nullptr; CheckUia(p->GetCurrentSupportedViews(&values), L"GetCurrentSupportedViews"); read(L"GetCurrentSupportedViews", ConvertArray(values, VT_I4)); SafeArrayDestroy(values);
				auto spec = DescribeSetter(nodeKey, UIA_MultipleViewCurrentViewPropertyId);
				auto view = ChoiceArgument(L"viewId", {}); CopyFrom(view->choices, spec->choices); if (view->choices.Count()) view->initial = itow(view->choices[0].id);
				add(ActionCode::ViewName, L"GetViewName", false, { view }, view->choices.Count() > 0);
				setter(UIA_MultipleViewCurrentViewPropertyId, ActionCode::SetView, L"SetCurrentView");
			}
			break;
		case UIA_WindowPatternId:
			{
				auto p = PatternInterface<IUIAutomationWindowPattern>(pattern.Obj());
				READ_BOOL(p, CanMaximize); READ_BOOL(p, CanMinimize); READ_BOOL(p, IsModal); READ_BOOL(p, IsTopmost);
				READ_ENUM(p, WindowVisualState, WindowVisualState); READ_ENUM(p, WindowInteractionState, WindowInteractionState);
				setter(UIA_WindowWindowVisualStatePropertyId, ActionCode::WindowState, L"SetWindowVisualState");
				add(ActionCode::WaitForIdle, L"WaitForInputIdle", false, { NumberArgument(L"milliseconds", 0, 5000, 1000, true) });
				add(ActionCode::CloseWindow, L"Close", true);
			}
			break;
		case UIA_SelectionItemPatternId:
			{
				auto p = PatternInterface<IUIAutomationSelectionItemPattern>(pattern.Obj());
				READ_BOOL(p, IsSelected); READ_ELEMENT(p, SelectionContainer);
				BOOL selected = FALSE, multiple = FALSE, required = FALSE; CheckUia(p->get_CurrentIsSelected(&selected), L"IsSelected");
				IUIAutomationElement* raw = nullptr; CheckUia(p->get_CurrentSelectionContainer(&raw), L"SelectionContainer"); ComPtr<IUIAutomationElement> container(raw);
				int count = 0;
				if (container) if (auto selection = AcquirePattern<IUIAutomationSelectionPattern>(container.Obj(), UIA_SelectionPatternId))
				{
					CheckUia(selection->get_CurrentCanSelectMultiple(&multiple), L"CanSelectMultiple"); CheckUia(selection->get_CurrentIsSelectionRequired(&required), L"IsSelectionRequired");
					IUIAutomationElementArray* items = nullptr; CheckUia(selection->GetCurrentSelection(&items), L"GetCurrentSelection"); ComPtr<IUIAutomationElementArray> keep(items); if (items) CheckUia(items->get_Length(&count), L"Selection.Length");
				}
				add(ActionCode::SelectItem, L"Select", true);
				add(ActionCode::AddItem, L"AddToSelection", true, {}, !selected && (multiple || count == 0));
				add(ActionCode::RemoveItem, L"RemoveFromSelection", true, {}, selected && (!required || count > 1));
			}
			break;
		case UIA_DockPatternId:
			{
				auto p = PatternInterface<IUIAutomationDockPattern>(pattern.Obj()); READ_ENUM(p, DockPosition, DockPosition);
				setter(UIA_DockDockPositionPropertyId, ActionCode::SetDock, L"SetDockPosition");
			}
			break;
		case UIA_TablePatternId:
			{
				auto p = PatternInterface<IUIAutomationTablePattern>(pattern.Obj()); READ_ENUM(p, RowOrColumnMajor, RowOrColumnMajor);
				READ_ELEMENTS(p, GetCurrentRowHeaders); READ_ELEMENTS(p, GetCurrentColumnHeaders);
				add(ActionCode::RowHeaders, L"GetCurrentRowHeaders", false); add(ActionCode::ColumnHeaders, L"GetCurrentColumnHeaders", false);
			}
			break;
		case UIA_TableItemPatternId:
			{
				auto p = PatternInterface<IUIAutomationTableItemPattern>(pattern.Obj());
				READ_ELEMENTS(p, GetCurrentRowHeaderItems); READ_ELEMENTS(p, GetCurrentColumnHeaderItems);
				add(ActionCode::RowHeaderItems, L"GetCurrentRowHeaderItems", false); add(ActionCode::ColumnHeaderItems, L"GetCurrentColumnHeaderItems", false);
			}
			break;
		case UIA_TextPatternId: case UIA_TextPattern2Id: case UIA_TextEditPatternId:
			{
				auto p = PatternInterface<IUIAutomationTextPattern>(pattern.Obj());
				SupportedTextSelection selection; CheckUia(p->get_SupportedTextSelection(&selection), L"SupportedTextSelection"); read(L"SupportedTextSelection", IntegerValue(selection));
				READ_RANGE(p, get_DocumentRange, nodeKey); READ_RANGES(p, GetSelection); READ_RANGES(p, GetVisibleRanges);
				add(ActionCode::DocumentRange, L"DocumentRange", false); add(ActionCode::TextSelection, L"GetSelection", false); add(ActionCode::VisibleRanges, L"GetVisibleRanges", false);
				add(ActionCode::RangeFromPoint, L"RangeFromPoint", false, { NumberArgument(L"screenX", INT_MIN, INT_MAX, 0, true), NumberArgument(L"screenY", INT_MIN, INT_MAX, 0, true) });
				add(ActionCode::RangeFromChild, L"RangeFromChild", false, { ReferenceArgument(L"child") });
				if (id == UIA_TextPattern2Id) { add(ActionCode::RangeFromAnnotation, L"RangeFromAnnotation", false, { ReferenceArgument(L"annotation") }); add(ActionCode::CaretRange, L"GetCaretRange", false); }
				if (id == UIA_TextEditPatternId) { add(ActionCode::CompositionRange, L"GetActiveComposition", false); add(ActionCode::ConversionRange, L"GetConversionTarget", false); }
				if (id == UIA_TextPattern2Id)
				{
					auto p2 = PatternInterface<IUIAutomationTextPattern2>(pattern.Obj()); BOOL active; IUIAutomationTextRange* v = nullptr;
					CheckUia(p2->GetCaretRange(&active, &v), L"GetCaretRange"); ComPtr<IUIAutomationTextRange> keep(v);
					read(L"GetCaretRange.isActive", BooleanValue(active)); read(L"GetCaretRange", ConvertRange(v, nodeKey, L"GetCaretRange"));
				}
				if (id == UIA_TextEditPatternId)
				{
					auto edit = PatternInterface<IUIAutomationTextEditPattern>(pattern.Obj());
					READ_RANGE(edit, GetActiveComposition, nodeKey); READ_RANGE(edit, GetConversionTarget, nodeKey);
				}
			}
			break;
		case UIA_TogglePatternId:
			{
				auto p = PatternInterface<IUIAutomationTogglePattern>(pattern.Obj()); READ_ENUM(p, ToggleState, ToggleState); add(ActionCode::Toggle, L"Toggle", true);
			}
			break;
		case UIA_TransformPatternId: case UIA_TransformPattern2Id:
			{
				auto p = PatternInterface<IUIAutomationTransformPattern>(pattern.Obj()); BOOL move = FALSE, resize = FALSE, rotate = FALSE;
				CheckUia(p->get_CurrentCanMove(&move), L"CanMove"); CheckUia(p->get_CurrentCanResize(&resize), L"CanResize"); CheckUia(p->get_CurrentCanRotate(&rotate), L"CanRotate");
				read(L"CanMove", BooleanValue(move)); read(L"CanResize", BooleanValue(resize)); read(L"CanRotate", BooleanValue(rotate));
				add(ActionCode::Move, L"Move", true, { NumberArgument(L"screenX", -limit, limit), NumberArgument(L"screenY", -limit, limit) }, move);
				add(ActionCode::Resize, L"Resize", true, { NumberArgument(L"width", (std::numeric_limits<double>::min)(), limit, 100), NumberArgument(L"height", (std::numeric_limits<double>::min)(), limit, 100) }, resize);
				add(ActionCode::Rotate, L"Rotate", true, { NumberArgument(L"degrees", -limit, limit) }, rotate);
				if (id == UIA_TransformPattern2Id)
				{
					auto p2 = PatternInterface<IUIAutomationTransformPattern2>(pattern.Obj()); READ_BOOL(p2, CanZoom); READ_DOUBLE(p2, ZoomLevel); READ_DOUBLE(p2, ZoomMinimum); READ_DOUBLE(p2, ZoomMaximum);
					setter(UIA_Transform2ZoomLevelPropertyId, ActionCode::Zoom, L"Zoom");
					BOOL zoom = FALSE; CheckUia(p2->get_CurrentCanZoom(&zoom), L"CanZoom");
					add(ActionCode::ZoomByUnit, L"ZoomByUnit", true, { ChoiceArgument(L"zoomUnit", { { ZoomUnit_NoAmount, L"NoAmount" }, { ZoomUnit_LargeDecrement, L"LargeDecrement" }, { ZoomUnit_SmallDecrement, L"SmallDecrement" }, { ZoomUnit_LargeIncrement, L"LargeIncrement" }, { ZoomUnit_SmallIncrement, L"SmallIncrement" } }) }, zoom);
				}
			}
			break;
		case UIA_ScrollItemPatternId: add(ActionCode::ScrollItem, L"ScrollIntoView", true); break;
		case UIA_LegacyIAccessiblePatternId:
			{
				auto p = PatternInterface<IUIAutomationLegacyIAccessiblePattern>(pattern.Obj());
				READ_ELEMENTS(p, GetCurrentSelection);
				READ_INT(p, ChildId); READ_STRING(p, Name); READ_STRING(p, Value); READ_STRING(p, Description); READ_ENUM(p, Role, DWORD); READ_ENUM(p, State, DWORD); READ_STRING(p, Help); READ_STRING(p, KeyboardShortcut); READ_STRING(p, DefaultAction);
				BSTR defaultAction = nullptr;
				DWORD state = 0;
				CheckUia(p->get_CurrentDefaultAction(&defaultAction), L"LegacyIAccessible.DefaultAction");
				CheckUia(p->get_CurrentState(&state), L"LegacyIAccessible.State");
				auto actionable = defaultAction && SysStringLen(defaultAction) > 0 && !(state & (STATE_SYSTEM_UNAVAILABLE | STATE_SYSTEM_INVISIBLE));
				SysFreeString(defaultAction);
				add(ActionCode::LegacySelection, L"GetCurrentSelection", false);
				add(ActionCode::LegacyDefault, L"DoDefaultAction", true, {}, actionable);
				add(ActionCode::LegacyObject, L"GetIAccessible", false);
				add(ActionCode::LegacySelect, L"Select", true, { ChoiceArgument(L"flagsSelect", { { SELFLAG_TAKEFOCUS, L"SELFLAG_TAKEFOCUS (1)" }, { SELFLAG_TAKESELECTION, L"SELFLAG_TAKESELECTION (2)" }, { SELFLAG_EXTENDSELECTION, L"SELFLAG_EXTENDSELECTION (4)" }, { SELFLAG_ADDSELECTION, L"SELFLAG_ADDSELECTION (8)" }, { SELFLAG_REMOVESELECTION, L"SELFLAG_REMOVESELECTION (16)" }, { 3, L"TAKEFOCUS | TAKESELECTION (3)" }, { 5, L"TAKEFOCUS | EXTENDSELECTION (5)" }, { 9, L"TAKEFOCUS | ADDSELECTION (9)" }, { 17, L"TAKEFOCUS | REMOVESELECTION (17)" }, { 12, L"EXTENDSELECTION | ADDSELECTION (12)" }, { 20, L"EXTENDSELECTION | REMOVESELECTION (20)" }, { 13, L"TAKEFOCUS | EXTENDSELECTION | ADDSELECTION (13)" }, { 21, L"TAKEFOCUS | EXTENDSELECTION | REMOVESELECTION (21)" } }) });
				add(ActionCode::LegacyValue, L"SetValue", true, { TextArgument(L"szValue", {}, true) });
			}
			break;
		case UIA_ItemContainerPatternId:
			{
				auto value = TextArgument(L"value"); value->kind = ArgumentKind::Variant;
				add(ActionCode::FindItem, L"FindItemByProperty", false, { ReferenceArgument(L"startAfter", false, true), CatalogArgument(L"propertyId", PropertyCatalog, PropertyCatalogCount, true), value });
			}
			break;
		case UIA_VirtualizedItemPatternId: add(ActionCode::Realize, L"Realize", true); break;
		case UIA_SynchronizedInputPatternId:
			add(ActionCode::StartListening, L"StartListening", false, { ChoiceArgument(L"inputType", { { 1, L"KeyUp (1)" }, { 2, L"KeyDown (2)" }, { 4, L"LeftMouseUp (4)" }, { 8, L"LeftMouseDown (8)" }, { 16, L"RightMouseUp (16)" }, { 32, L"RightMouseDown (32)" }, { 3, L"KeyUp | KeyDown (3)" }, { 12, L"LeftMouseUp | LeftMouseDown (12)" }, { 48, L"RightMouseUp | RightMouseDown (48)" } }) }, !listening.Keys().Contains(nodeKey));
			add(ActionCode::CancelListening, L"Cancel", false, {}, listening.Keys().Contains(nodeKey)); break;
		case UIA_ObjectModelPatternId: add(ActionCode::ObjectModel, L"GetUnderlyingObjectModel", false); break;
		case UIA_AnnotationPatternId:
			{
				auto p = PatternInterface<IUIAutomationAnnotationPattern>(pattern.Obj()); READ_INT(p, AnnotationTypeId); READ_STRING(p, AnnotationTypeName); READ_STRING(p, Author); READ_STRING(p, DateTime); READ_ELEMENT(p, Target);
			}
			break;
		case UIA_StylesPatternId:
			{
				auto p = PatternInterface<IUIAutomationStylesPattern>(pattern.Obj());
				auto optionalInt = [&](const wchar_t* name, HRESULT (STDMETHODCALLTYPE IUIAutomationStylesPattern::*getter)(int*)) { int v = 0; auto hr = (p.Obj()->*getter)(&v); auto value = IntegerValue(v); if (hr == UIA_E_NOTSUPPORTED) value->kind = ValueKind::Unsupported; else CheckUia(hr, name); read(name, value); };
				auto optionalString = [&](const wchar_t* name, HRESULT (STDMETHODCALLTYPE IUIAutomationStylesPattern::*getter)(BSTR*)) { BSTR v = nullptr; auto hr = (p.Obj()->*getter)(&v); auto value = StringValue(v ? WString::CopyFrom(v, SysStringLen(v)) : WString()); SysFreeString(v); if (hr == UIA_E_NOTSUPPORTED) value->kind = ValueKind::Unsupported; else CheckUia(hr, name); read(name, value); };
				optionalInt(L"StyleId", &IUIAutomationStylesPattern::get_CurrentStyleId); optionalInt(L"FillColor", &IUIAutomationStylesPattern::get_CurrentFillColor); optionalInt(L"FillPatternColor", &IUIAutomationStylesPattern::get_CurrentFillPatternColor);
				optionalString(L"StyleName", &IUIAutomationStylesPattern::get_CurrentStyleName); optionalString(L"FillPatternStyle", &IUIAutomationStylesPattern::get_CurrentFillPatternStyle); optionalString(L"Shape", &IUIAutomationStylesPattern::get_CurrentShape); optionalString(L"ExtendedProperties", &IUIAutomationStylesPattern::get_CurrentExtendedProperties);
				add(ActionCode::ExtendedProperties, L"GetCurrentExtendedPropertiesAsArray", false);
			}
			break;
		case UIA_SpreadsheetPatternId: add(ActionCode::SpreadsheetItem, L"GetItemByName", false, { TextArgument(L"name") }); break;
		case UIA_SpreadsheetItemPatternId:
			{
				auto p = PatternInterface<IUIAutomationSpreadsheetItemPattern>(pattern.Obj()); READ_STRING(p, Formula);
				READ_ELEMENTS(p, GetCurrentAnnotationObjects);
				SAFEARRAY* types = nullptr; CheckUia(p->GetCurrentAnnotationTypes(&types), L"GetCurrentAnnotationTypes"); read(L"GetCurrentAnnotationTypes", ConvertArray(types, VT_I4)); SafeArrayDestroy(types);
				add(ActionCode::AnnotationObjects, L"GetCurrentAnnotationObjects", false); add(ActionCode::AnnotationTypes, L"GetCurrentAnnotationTypes", false);
			}
			break;
		case UIA_TextChildPatternId:
			{
				auto p = PatternInterface<IUIAutomationTextChildPattern>(pattern.Obj()); IUIAutomationElement* container = nullptr;
				CheckUia(p->get_TextContainer(&container), L"TextContainer"); ComPtr<IUIAutomationElement> keep(container); read(L"TextContainer", ConvertObject(container)); add(ActionCode::ChildRange, L"TextRange", false);
				if (container) { auto document = RetainElement(keep); READ_RANGE(p, get_TextRange, document); }
			}
			break;
		case UIA_DragPatternId:
			{
				auto p = PatternInterface<IUIAutomationDragPattern>(pattern.Obj()); READ_BOOL(p, IsGrabbed); READ_STRING(p, DropEffect);
				READ_ELEMENTS(p, GetCurrentGrabbedItems);
				SAFEARRAY* v = nullptr; CheckUia(p->get_CurrentDropEffects(&v), L"DropEffects"); read(L"DropEffects", ConvertArray(v, VT_BSTR)); SafeArrayDestroy(v); add(ActionCode::GrabbedItems, L"GetCurrentGrabbedItems", false);
			}
			break;
		case UIA_DropTargetPatternId:
			{
				auto p = PatternInterface<IUIAutomationDropTargetPattern>(pattern.Obj()); READ_STRING(p, DropTargetEffect);
				SAFEARRAY* v = nullptr; CheckUia(p->get_CurrentDropTargetEffects(&v), L"DropTargetEffects"); read(L"DropTargetEffects", ConvertArray(v, VT_BSTR)); SafeArrayDestroy(v);
			}
			break;
		case UIA_CustomNavigationPatternId: add(ActionCode::Navigate, L"Navigate", false, { ChoiceArgument(L"direction", { { NavigateDirection_Parent, L"Parent" }, { NavigateDirection_NextSibling, L"NextSibling" }, { NavigateDirection_PreviousSibling, L"PreviousSibling" }, { NavigateDirection_FirstChild, L"FirstChild" }, { NavigateDirection_LastChild, L"LastChild" } }) }); break;
		default: CHECK_FAIL(L"A standard pattern has no adapter");
		}
#undef UIA_WIDE
#undef UIA_WIDE_INNER
#undef READ_INT
#undef READ_BOOL
#undef READ_DOUBLE
#undef READ_ENUM
#undef READ_STRING
#undef READ_ELEMENT
#undef READ_ELEMENTS
#undef READ_RANGE
#undef READ_RANGES
		return section;
	}
}

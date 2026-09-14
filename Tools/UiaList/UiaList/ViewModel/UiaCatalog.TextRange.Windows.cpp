#include "UiaCatalog.Actions.Windows.h"

using namespace vl;
using namespace vl::collections;

namespace uialist::native
{
	Ptr<ValueData> UiaSession::ConvertArray(SAFEARRAY* value, VARTYPE type)
	{
		VARIANT v; VariantInit(&v); v.vt = VT_ARRAY | type; v.parray = value;
		return Convert(v);
	}

	Ptr<ValueData> UiaSession::ConvertRange(IUIAutomationTextRange* value, vint document, const WString& name)
	{
		auto data = Ptr(new ValueData); data->type = VT_UNKNOWN;
		if (!value) return data;
		CHECK_ERROR(document && elements.Keys().Contains(document), L"Text range has no document identity");
		data->kind = ValueKind::Range; data->key = nextKey++;
		data->text = name + L" #" + itow(data->key) + L" / IUIAutomationElement #" + itow(document);
		value->AddRef();
		ranges.Add(data->key, { ComPtr<IUIAutomationTextRange>(value), document, data->text });
		return data;
	}

	Ptr<ValueData> UiaSession::ConvertRanges(IUIAutomationTextRangeArray* value, vint document, const WString& name)
	{
		auto result = Ptr(new ValueData); result->type = VT_UNKNOWN;
		if (!value) return result;
		result->kind = ValueKind::Array; result->text = L"IUIAutomationTextRangeArray";
		int count = 0; CheckUia(value->get_Length(&count), L"TextRangeArray.Length");
		for (int i = 0; i < count; i++)
		{
			IUIAutomationTextRange* item = nullptr; CheckUia(value->GetElement(i, &item), L"TextRangeArray.GetElement");
			ComPtr<IUIAutomationTextRange> keep(item); result->items.Add(ConvertRange(item, document, name + L" " + itow(i + 1)));
		}
		return result;
	}

	List<ReferenceData> UiaSession::GetReferences()
	{
		List<ReferenceData> result;
		for (auto key : elements.Keys())
		{
			auto index = referenceNames.Keys().IndexOf(key);
			auto label = index >= 0 ? referenceNames.Values()[index] + L" " : WString();
			result.Add({ ValueKind::Element, key, 0, label + L"IUIAutomationElement #" + itow(key) });
		}
		for (vint i = 0; i < ranges.Count(); i++) result.Add({ ValueKind::Range, ranges.Keys()[i], ranges.Values()[i].documentKey, ranges.Values()[i].name });
		return result;
	}

	void UiaSession::DiscardRanges() { ranges.Clear(); }

	Ptr<ActionSectionData> UiaSession::DescribeRange(vint rangeKey)
	{
		if (!ranges.Keys().Contains(rangeKey)) return nullptr;
		// Attribute conversion can retain additional ranges and grow the dictionary.
		auto record = ranges[rangeKey]; auto range = record.range;
		conversionDocumentKey = record.documentKey;
		auto section = Ptr(new ActionSectionData); section->rangeKey = rangeKey; section->documentKey = record.documentKey; section->name = record.name;
		auto add = [&](ActionCode code, const wchar_t* name, bool mutation, std::initializer_list<Ptr<ArgumentSpec>> args = {}, bool enabled = true) { return AddAction(*section.Obj(), code, name, mutation, args, enabled); };
		auto read = [&](const wchar_t* name, Ptr<ValueData> value) { AddReadout(*section.Obj(), name, value); };
		auto endpoint = [](const wchar_t* name) { return ChoiceArgument(name, { { TextPatternRangeEndpoint_Start, L"Start" }, { TextPatternRangeEndpoint_End, L"End" } }); };
		auto unit = []() { return ChoiceArgument(L"unit", { { TextUnit_Character, L"Character" }, { TextUnit_Format, L"Format" }, { TextUnit_Word, L"Word" }, { TextUnit_Line, L"Line" }, { TextUnit_Paragraph, L"Paragraph" }, { TextUnit_Page, L"Page" }, { TextUnit_Document, L"Document" } }); };
		auto boolean = [](const wchar_t* name) { return ChoiceArgument(name, { { 0, L"false" }, { 1, L"true" } }); };
		auto operand = []() { return ReferenceArgument(L"range", true); };
		auto count = []() { return NumberArgument(L"count", INT_MIN, INT_MAX, 1, true); };
		auto attribute = []() { return CatalogArgument(L"attributeId", AttributeCatalog, AttributeCatalogCount); };
		BSTR text = nullptr; CheckUia(range->GetText(1025, &text), L"TextRange.GetText(1025)");
		auto preview = text ? WString::CopyFrom(text, SysStringLen(text)) : WString(); SysFreeString(text);
		if (preview.Length() == 1025) preview = preview.Left(1024) + L"… (GetText(-1))";
		read(L"GetText(1025)", StringValue(preview));
		SAFEARRAY* bounds = nullptr; CheckUia(range->GetBoundingRectangles(&bounds), L"TextRange.GetBoundingRectangles"); read(L"GetBoundingRectangles", ConvertArray(bounds, VT_R8)); SafeArrayDestroy(bounds);
		for (vint i = 0; i < AttributeCatalogCount; i++)
		{
			VARIANT value; VariantInit(&value); auto&& descriptor = AttributeCatalog[i];
			CheckUia(range->GetAttributeValue(descriptor.id, &value), L"GetAttributeValue(" + itow(descriptor.id) + L")"); read(descriptor.name, Convert(value)); VariantClear(&value);
		}
		add(ActionCode::RangeClone, L"Clone", false);
		add(ActionCode::RangeCompare, L"Compare", false, { operand() });
		add(ActionCode::RangeCompareEndpoints, L"CompareEndpoints", false, { endpoint(L"srcEndPoint"), operand(), endpoint(L"targetEndPoint") });
		add(ActionCode::RangeExpand, L"ExpandToEnclosingUnit", false, { unit() });
		add(ActionCode::RangeMove, L"Move", false, { unit(), count() });
		add(ActionCode::RangeMoveEndpoint, L"MoveEndpointByUnit", false, { endpoint(L"endpoint"), unit(), count() });
		add(ActionCode::RangeTransferEndpoint, L"MoveEndpointByRange", false, { endpoint(L"srcEndPoint"), operand(), endpoint(L"targetEndPoint") });
		add(ActionCode::RangeFindText, L"FindText", false, { TextArgument(L"text", {}, true), boolean(L"backward"), boolean(L"ignoreCase") });
		auto value = TextArgument(L"value"); value->kind = ArgumentKind::Variant;
		add(ActionCode::RangeFindAttribute, L"FindAttribute", false, { attribute(), value, boolean(L"backward") });
		add(ActionCode::RangeAttribute, L"GetAttributeValue", false, { attribute() });
		add(ActionCode::RangeAllAttributes, L"GetAttributeValue [44]", false);
		add(ActionCode::RangeText, L"GetText", false, { NumberArgument(L"maxLength", -1, INT_MAX, -1, true) });
		add(ActionCode::RangeRectangles, L"GetBoundingRectangles", false);
		add(ActionCode::RangeEnclosing, L"GetEnclosingElement", false);
		add(ActionCode::RangeChildren, L"GetChildren", false);
		auto document = elements[record.documentKey];
		auto textPattern = AcquirePattern<IUIAutomationTextPattern>(document.Obj(), UIA_TextPatternId);
		if (!textPattern) textPattern = AcquirePattern<IUIAutomationTextPattern>(document.Obj(), UIA_TextPattern2Id);
		if (!textPattern) textPattern = AcquirePattern<IUIAutomationTextPattern>(document.Obj(), UIA_TextEditPatternId);
		SupportedTextSelection selection = SupportedTextSelection_None;
		if (textPattern) CheckUia(textPattern->get_SupportedTextSelection(&selection), L"SupportedTextSelection");
		add(ActionCode::RangeSelect, L"Select", true, {}, selection != SupportedTextSelection_None);
		add(ActionCode::RangeAdd, L"AddToSelection", true, {}, selection == SupportedTextSelection_Multiple);
		add(ActionCode::RangeRemove, L"RemoveFromSelection", true, {}, selection != SupportedTextSelection_None);
		add(ActionCode::RangeScroll, L"ScrollIntoView", true, { boolean(L"alignToTop") });
		if (QueryInterface<IUIAutomationTextRange2>(range.Obj())) add(ActionCode::RangeMenu, L"IUIAutomationTextRange2.ShowContextMenu", true);
		if (QueryInterface<IUIAutomationTextRange3>(range.Obj()))
		{
			add(ActionCode::RangeEnclosingCache, L"GetEnclosingElementBuildCache", false);
			add(ActionCode::RangeChildrenCache, L"GetChildrenBuildCache", false);
			auto ids = TextArgument(L"attributeIds", L"40000,40001"); ids->kind = ArgumentKind::AttributeList;
			add(ActionCode::RangeAttributes, L"GetAttributeValues", false, { ids });
		}
		return section;
	}

	ActionOutcome UiaSession::ExecuteRange(const ActionSpec& action, const List<ActionArgument>& arguments)
	{
		ActionOutcome result; result.value = Ptr(new ValueData);
		auto section = DescribeRange(action.rangeKey);
		Ptr<ActionSpec> capability;
		if (section) for (auto&& item : section->commands) if (item->code == action.code) capability = item;
		if (!capability || !capability->enabled || arguments.Count() != capability->parameters.Count()) { result.available = false; return result; }
		auto record = ranges[action.rangeKey];
		conversionDocumentKey = record.documentKey;
		for (vint i = 0; i < arguments.Count(); i++)
		{
			ActionArgument verified; verified.value = arguments[i].value;
			if (!ParseArgument(*capability->parameters[i].Obj(), arguments[i].text, arguments[i].reference, verified)) { result.available = false; return result; }
			if (verified.kind == ArgumentKind::Range && (!ranges.Keys().Contains(verified.reference) || ranges[verified.reference].documentKey != record.documentKey)) { result.available = false; return result; }
		}
		auto range = record.range;
		auto integer = [&](vint i) { return arguments[i].integer; };
		auto endpoint = [&](vint i) { return static_cast<TextPatternRangeEndpoint>(integer(i)); };
		auto unit = [&](vint i) { return static_cast<TextUnit>(integer(i)); };
		auto other = [&](vint i) { return ranges[arguments[i].reference].range.Obj(); };
		auto check = [&](HRESULT hr) { CheckUia(hr, action.name + L"; range=" + itow(action.rangeKey) + L"; document=" + itow(record.documentKey)); };
		auto rangeResult = [&](IUIAutomationTextRange* value) { ComPtr<IUIAutomationTextRange> keep(value); result.value = ConvertRange(value, record.documentKey, action.name); };
		result.mutation = capability->mutation;
		switch (action.code)
		{
		case ActionCode::RangeClone: { IUIAutomationTextRange* value = nullptr; check(range->Clone(&value)); rangeResult(value); } break;
		case ActionCode::RangeCompare: { BOOL value = FALSE; check(range->Compare(other(0), &value)); result.value = BooleanValue(value); } break;
		case ActionCode::RangeCompareEndpoints: { int value = 0; check(range->CompareEndpoints(endpoint(0), other(1), endpoint(2), &value)); result.value = IntegerValue(value); } break;
		case ActionCode::RangeExpand: check(range->ExpandToEnclosingUnit(unit(0))); break;
		case ActionCode::RangeMove: { int value = 0; check(range->Move(unit(0), integer(1), &value)); result.value = IntegerValue(value); } break;
		case ActionCode::RangeMoveEndpoint: { int value = 0; check(range->MoveEndpointByUnit(endpoint(0), unit(1), integer(2), &value)); result.value = IntegerValue(value); } break;
		case ActionCode::RangeTransferEndpoint: check(range->MoveEndpointByRange(endpoint(0), other(1), endpoint(2))); break;
		case ActionCode::RangeFindText:
			{
				BSTR text = SysAllocStringLen(arguments[0].text.Buffer(), static_cast<UINT>(arguments[0].text.Length())); if (!text) throw Exception(L"SysAllocStringLen failed");
				IUIAutomationTextRange* value = nullptr; auto hr = range->FindText(text, integer(1), integer(2), &value); SysFreeString(text); check(hr); rangeResult(value);
			}
			break;
		case ActionCode::RangeFindAttribute:
			{
				VARIANT argument; ToVariant(*this, *arguments[1].value.Obj(), argument); IUIAutomationTextRange* value = nullptr;
				auto hr = range->FindAttribute(integer(0), argument, integer(2), &value); VariantClear(&argument); check(hr); rangeResult(value);
			}
			break;
		case ActionCode::RangeAttribute:
			{
				VARIANT value; VariantInit(&value); check(range->GetAttributeValue(integer(0), &value)); result.value = Convert(value); VariantClear(&value);
			}
			break;
		case ActionCode::RangeAllAttributes:
			result.value->kind = ValueKind::Array;
			for (vint i = 0; i < AttributeCatalogCount; i++)
			{
				VARIANT value; VariantInit(&value); check(range->GetAttributeValue(AttributeCatalog[i].id, &value));
				auto pair = Ptr(new ValueData); pair->kind = ValueKind::Array; pair->items.Add(StringValue(AttributeCatalog[i].name)); pair->items.Add(Convert(value)); result.value->items.Add(pair); VariantClear(&value);
			}
			break;
		case ActionCode::RangeText:
			{
				BSTR value = nullptr; check(range->GetText(integer(0), &value)); result.value = StringValue(value ? WString::CopyFrom(value, SysStringLen(value)) : WString()); SysFreeString(value);
			}
			break;
		case ActionCode::RangeRectangles:
			{
				SAFEARRAY* value = nullptr; check(range->GetBoundingRectangles(&value)); result.value = ConvertArray(value, VT_R8); SafeArrayDestroy(value);
			}
			break;
		case ActionCode::RangeEnclosing: case ActionCode::RangeEnclosingCache:
			{
				IUIAutomationElement* value = nullptr;
				check(action.code == ActionCode::RangeEnclosing ? range->GetEnclosingElement(&value) : QueryInterface<IUIAutomationTextRange3>(range.Obj())->GetEnclosingElementBuildCache(cache.Obj(), &value));
				ComPtr<IUIAutomationElement> keep(value); result.value = ConvertObject(value);
			}
			break;
		case ActionCode::RangeChildren: case ActionCode::RangeChildrenCache:
			{
				IUIAutomationElementArray* value = nullptr;
				check(action.code == ActionCode::RangeChildren ? range->GetChildren(&value) : QueryInterface<IUIAutomationTextRange3>(range.Obj())->GetChildrenBuildCache(cache.Obj(), &value));
				ComPtr<IUIAutomationElementArray> keep(value); result.value = ConvertObject(value);
			}
			break;
		case ActionCode::RangeSelect: check(range->Select()); break;
		case ActionCode::RangeAdd: check(range->AddToSelection()); break;
		case ActionCode::RangeRemove: check(range->RemoveFromSelection()); break;
		case ActionCode::RangeScroll: check(range->ScrollIntoView(integer(0))); break;
		case ActionCode::RangeMenu: check(QueryInterface<IUIAutomationTextRange2>(range.Obj())->ShowContextMenu()); break;
		case ActionCode::RangeAttributes:
			{
				Array<TEXTATTRIBUTEID> ids(arguments[0].ids.Count()); for (vint i = 0; i < ids.Count(); i++) ids[i] = arguments[0].ids[i];
				SAFEARRAY* value = nullptr; check(QueryInterface<IUIAutomationTextRange3>(range.Obj())->GetAttributeValues(&ids[0], static_cast<int>(ids.Count()), &value)); result.value = ConvertArray(value, VT_VARIANT); SafeArrayDestroy(value);
			}
			break;
		default: CHECK_FAIL(L"Text range action has no compiled dispatcher");
		}
		result.references = GetReferences();
		return result;
	}
}

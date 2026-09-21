#include "UiaSession.Windows.h"
#include <limits>

using namespace vl;
using namespace vl::collections;

namespace uialist::native
{
	UiaSession::UiaSession(WindowIdentity target)
		: window(target)
	{
		IUIAutomation* client = nullptr;
		CheckUia(CoCreateInstance(__uuidof(CUIAutomation8), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&client)), L"CUIAutomation8");
		automation = client;
		auto timeouts = QueryInterface<IUIAutomation2>(client);
		CheckUia(timeouts->put_ConnectionTimeout(5000), L"ConnectionTimeout");
		CheckUia(timeouts->put_TransactionTimeout(5000), L"TransactionTimeout");
		IUIAutomationCacheRequest* request = nullptr;
		CheckUia(client->CreateCacheRequest(&request), L"CreateCacheRequest");
		cache = request;
		CheckUia(request->put_AutomationElementMode(AutomationElementMode_Full), L"Cache.AutomationElementMode");
		const PROPERTYID properties[] = { UIA_NamePropertyId, UIA_ControlTypePropertyId, UIA_BoundingRectanglePropertyId, UIA_IsOffscreenPropertyId, UIA_ProcessIdPropertyId, UIA_AutomationIdPropertyId, UIA_NativeWindowHandlePropertyId, UIA_IsControlElementPropertyId, UIA_IsContentElementPropertyId };
		for (auto property : properties) CheckUia(request->AddProperty(property), L"Cache.AddProperty(" + itow(property) + L")");
		IUIAutomationTreeWalker* rawWalker = nullptr;
		CheckUia(client->get_RawViewWalker(&rawWalker), L"RawViewWalker");
		walker = rawWalker;
		IUnknown* reserved = nullptr;
		CheckUia(client->get_ReservedNotSupportedValue(&reserved), L"ReservedNotSupportedValue");
		unsupported = reserved;
		CheckUia(client->get_ReservedMixedAttributeValue(&reserved), L"ReservedMixedAttributeValue");
		mixed = reserved;
	}

	UiaSession::~UiaSession()
	{
		for (auto&& request : listening.Values())
		{
			auto hr = request->Cancel();
			if (FAILED(hr))
			{
				auto message = L"SynchronizedInput.Cancel during disposal; HWND=" + Hex(reinterpret_cast<vuint>(window.handle)) + L"; PID=" + itow(window.processId) + L"; HRESULT=" + Hex(static_cast<ULONG>(hr));
				OutputDebugStringW(message.Buffer());
				std::terminate();
			}
		}
	}

	vint UiaSession::RetainElement(ComPtr<IUIAutomationElement> element)
	{
		if (!element) return 0;
		SAFEARRAY* runtimeId = nullptr;
		CheckUia(element->GetRuntimeId(&runtimeId), L"GetRuntimeId");
		WString identity;
		if (runtimeId)
		{
			LONG lower = 0, upper = -1;
			CheckUia(SafeArrayGetLBound(runtimeId, 1, &lower), L"RuntimeId.LBound");
			CheckUia(SafeArrayGetUBound(runtimeId, 1, &upper), L"RuntimeId.UBound");
			for (auto index = lower; index <= upper; index++)
			{
				int part = 0;
				CheckUia(SafeArrayGetElement(runtimeId, &index, &part), L"RuntimeId.Element");
				identity += itow(part) + L";";
			}
			SafeArrayDestroy(runtimeId);
		}
		if (identity.Length())
		{
			auto existing = runtimeIds.Keys().IndexOf(identity);
			if (existing != -1)
			{
				auto key = runtimeIds.Values()[existing];
				elements.Set(key, element);
				return key;
			}
		}
		else
		{
			for (vint i = 0; i < elements.Count(); i++)
			{
				BOOL equal = FALSE;
				CheckUia(automation->CompareElements(element.Obj(), elements.Values()[i].Obj(), &equal), L"CompareElements");
				if (equal) return elements.Keys()[i];
			}
		}
		auto key = nextKey++;
		elements.Add(key, element);
		if (identity.Length()) runtimeIds.Add(identity, key);
		return key;
	}

	Ptr<ValueData> UiaSession::ConvertObject(IUnknown* value)
	{
		auto data = Ptr(new ValueData);
		data->type = VT_UNKNOWN;
		if (!value) return data;
		auto identity = QueryInterface<IUnknown>(value);
		if (identity.Obj() == unsupported.Obj()) data->kind = ValueKind::Unsupported;
		else if (identity.Obj() == mixed.Obj()) data->kind = ValueKind::Mixed;
		else if (auto element = QueryInterface<IUIAutomationElement>(value))
		{
			data->kind = ValueKind::Element;
			data->key = RetainElement(element);
			data->text = L"IUIAutomationElement";
		}
		else if (auto array = QueryInterface<IUIAutomationElementArray>(value))
		{
			data->kind = ValueKind::Array;
			data->text = L"IUIAutomationElementArray";
			int count = 0;
			CheckUia(array->get_Length(&count), L"ElementArray.Length");
			for (int i = 0; i < count; i++)
			{
				IUIAutomationElement* item = nullptr;
				CheckUia(array->GetElement(i, &item), L"ElementArray.GetElement");
				auto owner = ComPtr(item);
				data->items.Add(ConvertObject(item));
			}
		}
		else if (auto range = QueryInterface<IUIAutomationTextRange>(value))
		{
			return ConvertRange(range.Obj(), conversionDocumentKey, L"UIA_LinkAttributeId");
		}
		else
		{
			data->kind = ValueKind::Opaque;
			data->key = nextKey++;
			data->text = QueryInterface<IAccessible>(value) ? L"IAccessible" : L"IUnknown";
			opaqueObjects.Add(data->key, identity);
		}
		return data;
	}

	Ptr<ValueData> UiaSession::Convert(const VARIANT& value)
	{
		auto data = Ptr(new ValueData);
		data->type = value.vt;
		if (value.vt & VT_BYREF)
		{
			VARIANT copy;
			VariantInit(&copy);
			CheckUia(VariantCopyInd(&copy, &value), L"VariantCopyInd");
			data = Convert(copy);
			data->type = value.vt;
			VariantClear(&copy);
			return data;
		}
		if (value.vt & VT_ARRAY)
		{
			if (!value.parray) return data;
			data->kind = ValueKind::Array;
			auto dimensions = SafeArrayGetDim(value.parray);
			Array<LONG> indexes(dimensions);
			vint count = 1;
			for (UINT dimension = 1; dimension <= dimensions; dimension++)
			{
				LONG lower = 0, upper = -1;
				CheckUia(SafeArrayGetLBound(value.parray, dimension, &lower), L"SafeArray.LBound");
				CheckUia(SafeArrayGetUBound(value.parray, dimension, &upper), L"SafeArray.UBound");
				data->dimensions.Add({ lower, upper });
				indexes[dimension - 1] = lower;
				auto size = static_cast<vint64_t>(upper) - lower + 1;
				if (size < 0 || (size && count > (std::numeric_limits<vint>::max)() / size)) throw Exception(L"SAFEARRAY dimensions overflow");
				count *= static_cast<vint>(size);
			}
			for (vint i = 0; i < count; i++)
			{
				VARIANT cell;
				VariantInit(&cell);
				auto type = static_cast<VARTYPE>(value.vt & VT_TYPEMASK);
				if (type != VT_VARIANT) cell.vt = type;
				void* destination = type == VT_VARIANT ? static_cast<void*>(&cell) : type == VT_DECIMAL ? static_cast<void*>(&cell.decVal) : static_cast<void*>(&cell.llVal);
				CheckUia(SafeArrayGetElement(value.parray, &indexes[0], destination), L"SafeArray.GetElement");
				if (type == VT_DECIMAL) cell.vt = VT_DECIMAL;
				data->items.Add(Convert(cell));
				VariantClear(&cell);
				for (vint dimension = 0; dimension < indexes.Count(); dimension++)
				{
					if (indexes[dimension] < data->dimensions[dimension].value) { indexes[dimension]++; break; }
					indexes[dimension] = data->dimensions[dimension].key;
				}
			}
			return data;
		}
		switch (value.vt)
		{
		case VT_EMPTY: case VT_NULL: break;
		case VT_BOOL: data->kind = ValueKind::Boolean; data->signedValue = value.boolVal != VARIANT_FALSE; break;
		case VT_I1: data->kind = ValueKind::Signed; data->signedValue = value.cVal; break;
		case VT_I2: data->kind = ValueKind::Signed; data->signedValue = value.iVal; break;
		case VT_I4: case VT_INT: data->kind = ValueKind::Signed; data->signedValue = value.lVal; break;
		case VT_I8: data->kind = ValueKind::Signed; data->signedValue = value.llVal; break;
		case VT_UI1: data->kind = ValueKind::Unsigned; data->unsignedValue = value.bVal; break;
		case VT_UI2: data->kind = ValueKind::Unsigned; data->unsignedValue = value.uiVal; break;
		case VT_UI4: case VT_UINT: data->kind = ValueKind::Unsigned; data->unsignedValue = value.ulVal; break;
		case VT_UI8: data->kind = ValueKind::Unsigned; data->unsignedValue = value.ullVal; break;
		case VT_R4: data->kind = ValueKind::Real; data->realValue = value.fltVal; break;
		case VT_R8: case VT_DATE: data->kind = ValueKind::Real; data->realValue = value.dblVal; break;
		case VT_BSTR:
			data->kind = ValueKind::String;
			if (value.bstrVal) data->text = WString::CopyFrom(value.bstrVal, SysStringLen(value.bstrVal));
			break;
		case VT_UNKNOWN: case VT_DISPATCH:
			data = ConvertObject(value.vt == VT_UNKNOWN ? value.punkVal : value.pdispVal);
			data->type = value.vt;
			return data;
		case VT_ERROR: data->kind = ValueKind::Signed; data->signedValue = value.scode; break;
		case VT_CY: case VT_DECIMAL:
		{
			VARIANT text;
			VariantInit(&text);
			CheckUia(VariantChangeTypeEx(&text, &value, LOCALE_INVARIANT, 0, VT_BSTR), L"VariantChangeTypeEx");
			data->kind = ValueKind::String;
			data->text = WString::CopyFrom(text.bstrVal, SysStringLen(text.bstrVal));
			VariantClear(&text);
			break;
		}
		default: throw Exception(L"Unexpected VARIANT type: " + itow(value.vt));
		}
		return data;
	}

	Ptr<TreeSnapshot> UiaSession::ReadTree(vint generation, const Func<bool()>& canceled, const Func<void(vint)>& progress)
	{
		if (!IsCurrentWindow(window)) throw Exception(L"Selected HWND/PID no longer exists");
		auto snapshot = Ptr(new TreeSnapshot);
		snapshot->window = window;
		snapshot->generation = generation;
		IUIAutomationElement* root = nullptr;
		CheckUia(automation->ElementFromHandleBuildCache(window.handle, cache.Obj(), &root), L"ElementFromHandleBuildCache");
		struct PendingNode { ComPtr<IUIAutomationElement> element; vint parent; vint depth; };
		List<PendingNode> pending;
		pending.Add({ ComPtr(root), 0, 0 });
		SortedList<vint> visited;
		while (pending.Count())
		{
			if (canceled()) return nullptr;
			auto current = pending[pending.Count() - 1];
			pending.RemoveAt(pending.Count() - 1);
			NodeData node;
			node.key = RetainElement(current.element);
			node.parent = current.parent;
			node.depth = current.depth;
			if (visited.Contains(node.key)) throw Exception(L"Raw View provider contains a repeated element/cycle");
			visited.Add(node.key);
			auto runtime = runtimeIds.Values().IndexOf(node.key);
			if (runtime != -1) node.runtimeId = runtimeIds.Keys()[runtime];
			BSTR name = nullptr;
			CheckUia(current.element->get_CachedName(&name), L"CachedName");
			if (name) node.name = WString::CopyFrom(name, SysStringLen(name));
			SysFreeString(name);
			referenceNames.Set(node.key, node.name);
			CONTROLTYPEID role = 0;
			CheckUia(current.element->get_CachedControlType(&role), L"CachedControlType");
			node.controlType = role;
			node.role = IdName(ControlTypeCatalog, ControlTypeCatalogCount, role);
			CheckUia(current.element->get_CachedBoundingRectangle(&node.bounds), L"CachedBoundingRectangle");
			BOOL offscreen = FALSE;
			CheckUia(current.element->get_CachedIsOffscreen(&offscreen), L"CachedIsOffscreen");
			node.offscreen = offscreen != FALSE;
			const PatternDescriptor versions[] =
			{
				{ 9, &__uuidof(IUIAutomationElement9), L"IUIAutomationElement9", L"" },
				{ 8, &__uuidof(IUIAutomationElement8), L"IUIAutomationElement8", L"" },
				{ 7, &__uuidof(IUIAutomationElement7), L"IUIAutomationElement7", L"" },
				{ 6, &__uuidof(IUIAutomationElement6), L"IUIAutomationElement6", L"" },
				{ 5, &__uuidof(IUIAutomationElement5), L"IUIAutomationElement5", L"" },
				{ 4, &__uuidof(IUIAutomationElement4), L"IUIAutomationElement4", L"" },
				{ 3, &__uuidof(IUIAutomationElement3), L"IUIAutomationElement3", L"" },
				{ 2, &__uuidof(IUIAutomationElement2), L"IUIAutomationElement2", L"" },
				{ 1, &__uuidof(IUIAutomationElement), L"IUIAutomationElement", L"" },
			};
			for (auto&& version : versions)
			{
				IUnknown* acquired = nullptr;
				auto hr = current.element->QueryInterface(*version.iid, reinterpret_cast<void**>(&acquired));
				if (hr == E_NOINTERFACE) continue;
				CheckUia(hr, L"QueryInterface(" + WString(version.client) + L")");
				ComPtr<IUnknown> owner(acquired);
				node.client = version.client;
				break;
			}
			for (vint i = 0; i < PatternCatalogCount; i++)
			{
				if (canceled()) return nullptr;
				auto&& pattern = PatternCatalog[i];
				IUnknown* acquired = nullptr;
				auto hr = current.element->GetCurrentPatternAs(pattern.id, *pattern.iid, reinterpret_cast<void**>(&acquired));
				if (hr == UIA_E_NOTSUPPORTED || hr == E_NOINTERFACE) continue;
				CheckUia(hr, L"GetCurrentPatternAs(" + itow(pattern.id) + L")");
				ComPtr<IUnknown> owner(acquired);
				if (acquired) node.providers += (node.providers.Length() ? WString(L", ") : WString()) + pattern.provider;
			}
			snapshot->nodes.Add(node);
			if (snapshot->nodes.Count() % 128 == 0) progress(snapshot->nodes.Count());
			IUIAutomationElement* first = nullptr;
			CheckUia(walker->GetFirstChildElementBuildCache(current.element.Obj(), cache.Obj(), &first), L"RawView.FirstChild");
			List<ComPtr<IUIAutomationElement>> children;
			ComPtr<IUIAutomationElement> child(first);
			SortedList<vint> siblings;
			while (child)
			{
				if (canceled()) return nullptr;
				auto childKey = RetainElement(child);
				if (siblings.Contains(childKey)) throw Exception(L"Raw View sibling cycle");
				siblings.Add(childKey);
				children.Add(child);
				IUIAutomationElement* next = nullptr;
				CheckUia(walker->GetNextSiblingElementBuildCache(child.Obj(), cache.Obj(), &next), L"RawView.NextSibling");
				child = next;
			}
			for (vint i = children.Count(); i > 0; i--) pending.Add({ children[i - 1], node.key, node.depth + 1 });
		}
		return snapshot;
	}

	Ptr<InspectionSnapshot> UiaSession::Inspect(vint nodeKey)
	{
		conversionDocumentKey = nodeKey;
		auto snapshot = Ptr(new InspectionSnapshot);
		snapshot->nodeKey = nodeKey;
		auto element = elements[nodeKey];
		for (vint i = 0; i < PropertyCatalogCount; i++)
		{
			auto&& descriptor = PropertyCatalog[i];
			VARIANT value;
			VariantInit(&value);
			CheckUia(element->GetCurrentPropertyValueEx(descriptor.id, TRUE, &value), WString(descriptor.name) + L" (" + itow(descriptor.id) + L")");
			snapshot->properties.Add({ descriptor.id, WString(descriptor.name) + L" (" + itow(descriptor.id) + L")", Convert(value), DescribeSetter(nodeKey, descriptor.id) });
			VariantClear(&value);
		}
		snapshot->sections.Add(DescribeActions(nodeKey, 0));
		for (vint i = 0; i < PatternCatalogCount; i++)
		{
			auto&& descriptor = PatternCatalog[i];
			Ptr<ActionSectionData> section;
			try
			{
				section = DescribeActions(nodeKey, descriptor.id);
			}
			catch (const UiaFailure& error)
			{
				if (!error.IsExpected()) throw;
				// A stale pattern interface need not mean the whole element disappeared.
				// Recheck the element so its destruction still terminates inspection.
				int processId = 0;
				CheckUia(element->get_CurrentProcessId(&processId), L"Recheck element after pattern failure");
				section = Ptr(new ActionSectionData);
				section->pattern = descriptor.id;
				section->name = WString(descriptor.provider) + L" / " + descriptor.client + L" (" + itow(descriptor.id) + L")";
				auto value = Ptr(new ValueData);
				value->kind = ValueKind::String;
				value->text = error.Message();
				PropertyData readout;
				readout.name = L"Pattern query failed";
				readout.value = value;
				readout.sourcePattern = descriptor.id;
				section->readouts.Add(readout);
			}
			if (section)
			{
				snapshot->patterns.Add(descriptor.id);
				snapshot->sections.Add(section);
				for (auto&& readout : section->readouts) snapshot->properties.Add(readout);
			}
		}
		for (auto&& section : snapshot->sections)
		{
			for (auto&& command : section->commands)
			{
				if (!command->pureGetter || command->parameters.Count() != 0) continue;
				bool existing = false;
				for (auto&& readout : section->readouts) if (readout.name == command->name) existing = true;
				if (existing) continue;
				List<ActionArgument> arguments;
				Ptr<ValueData> value;
				try
				{
					value = Execute(nodeKey, *command.Obj(), arguments).value;
				}
				catch (const UiaFailure& error)
				{
					if (!error.IsExpected() || error.IsUnavailable()) throw;
					value = Ptr(new ValueData);
					value->kind = ValueKind::Unsupported;
					value->text = error.Message();
				}
				PropertyData readout;
				readout.name = command->name;
				readout.value = value;
				readout.sourcePattern = section->pattern;
				section->readouts.Add(readout);
			}
		}
		snapshot->references = GetReferences();
		return snapshot;
	}

	void UiaWorker::Run()
	{
		CheckUia(CoInitializeEx(nullptr, COINIT_MULTITHREADED), L"CoInitializeEx(MTA)");
		queue.RunTaskQueue();
		CoUninitialize();
	}
}

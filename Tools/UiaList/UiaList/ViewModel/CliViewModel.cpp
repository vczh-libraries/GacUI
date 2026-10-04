#include "CliViewModel.h"
#include "UiaCatalog.Actions.Windows.h"
#include <cmath>
#include <charconv>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;

namespace uialist::cli
{
	Json String(const WString& value) { auto node = Ptr(new json::JsonString); node->content.value = value; return node; }
	Json Null() { auto node = Ptr(new json::JsonLiteral); node->value = json::JsonLiteralValue::Null; return node; }
	Json Boolean(bool value) { auto node = Ptr(new json::JsonLiteral); node->value = value ? json::JsonLiteralValue::True : json::JsonLiteralValue::False; return node; }
	Json JsonMap(std::initializer_list<Pair<WString, Json>> fields)
	{
		auto node = Ptr(new json::JsonObject);
		for (auto&& field : fields) { auto item = Ptr(new json::JsonObjectField); item->name.value = field.key; item->value = field.value; node->fields.Add(item); }
		return node;
	}
	Json Number(double value)
	{
		if (!std::isfinite(value)) return JsonMap({ { L"nonfinite", String(std::isnan(value) ? L"NaN" : value < 0 ? L"-Infinity" : L"Infinity") } });
		char buffer[64]; auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
		CHECK_ERROR(result.ec == std::errc(), L"uialist::cli::Number#Cannot format number.");
		auto node = Ptr(new json::JsonNumber); node->content.value = atow(AString::CopyFrom(buffer, result.ptr - buffer)); return node;
	}
	Json Field(Ptr<json::JsonObject> object, const WString& name)
	{
		Json result;
		for (auto&& field : object->fields) if (field->name.value == name)
		{
			if (result) throw CommandFailure(L"InvalidArguments", L"Duplicate field: " + name);
			result = field->value;
		}
		if (!result) throw CommandFailure(L"InvalidArguments", L"Missing field: " + name);
		return result;
	}
	WString Text(Json value)
	{
		if (auto text = value.Cast<json::JsonString>()) return text->content.value;
		throw CommandFailure(L"InvalidType", L"Expected JSON string.");
	}
	WString NumericText(Json value)
	{
		if (auto number = value.Cast<json::JsonNumber>()) return number->content.value;
		throw CommandFailure(L"InvalidType", L"Expected JSON number.");
	}
	void Fields(Ptr<json::JsonObject> object, std::initializer_list<WString> names)
	{
		if (object->fields.Count() != static_cast<vint>(names.size())) throw CommandFailure(L"InvalidArguments", L"Unexpected or missing arguments. See Help-Command.");
		for (auto&& name : names) Field(object, name);
	}
	Json Bounds(RECT bounds)
	{
		return JsonMap({ { L"left", Number(bounds.left) }, { L"top", Number(bounds.top) }, { L"right", Number(bounds.right) }, { L"bottom", Number(bounds.bottom) } });
	}
	WString OperationName(const native::ActionSpec& spec)
	{
		WString provider = L"IUIAutomationElement", method = spec.name;
		if (spec.rangeKey)
		{
			provider = L"IUIAutomationTextRange";
			if (spec.code == native::ActionCode::RangeAllAttributes) method = L"ReadAllAttributes";
			if (spec.code == native::ActionCode::RangeMenu) { provider = L"IUIAutomationTextRange2"; method = L"ShowContextMenu"; }
			if (spec.code == native::ActionCode::RangeEnclosingCache || spec.code == native::ActionCode::RangeChildrenCache || spec.code == native::ActionCode::RangeAttributes) provider = L"IUIAutomationTextRange3";
		}
		else if (spec.pattern)
		{
			for (vint i = 0; i < native::PatternCatalogCount; i++) if (native::PatternCatalog[i].id == spec.pattern) provider = native::PatternCatalog[i].provider;
		}
		else if (spec.code == native::ActionCode::ElementMenu) provider = L"IUIAutomationElement3";
		else if (spec.code == native::ActionCode::Metadata) provider = L"IUIAutomationElement7";
		return provider + L"::" + method;
	}

	CommandFailure::CommandFailure(const WString& value, const WString& message) : Exception(message), code(value) {}

	CliViewModel::CliViewModel(UiaListViewModel& value) : root(value)
	{
		parser.OnError.Add(Func<void(ErrorArgs&)>([](ErrorArgs&) { throw CommandFailure(L"InvalidJson", L"Arguments must be valid JSON without parser recovery."); }));
		GUID guid; native::CheckUia(CoCreateGuid(&guid), L"CoCreateGuid");
		wchar_t text[40]; StringFromGUID2(guid, text, 40); session = L"uia-" + WString::CopyFrom(text + 1, 36);
		publicationHandler = root.Published.Add([this]() { if (published) { auto callback = published; callback(); } });
		root.failureHandler = [this](const WString& operation, const WString& message, HRESULT result, bool fatal)
		{
			if (root.treeBusy) { root.ClearSelection(); inspection = nullptr; ranges.Clear(); nativeReferences.Clear(); }
			Fail(fatal ? L"Fatal" : L"Unavailable", message, operation, result, fatal);
		};
	}
	CliViewModel::~CliViewModel() { root.Published.Remove(publicationHandler); root.failureHandler = {}; }
	void CliViewModel::Finish(Json result, bool exit)
	{
		published = {};
		busy = false;
		auto response = JsonMap({ { L"command", String(command) }, { L"ok", Boolean(true) }, { L"generation", root.snapshot ? String(session + L"/g" + itow(root.snapshot->generation)) : Null() }, { L"result", result }, { L"error", Null() } });
		output(json::JsonToString(response), exit, false);
	}
	void CliViewModel::Fail(const WString& code, const WString& message, const WString& operation, HRESULT hresult, bool fatal)
	{
		published = {};
		busy = false;
		auto error = JsonMap({ { L"code", String(code) }, { L"message", String(message) }, { L"operation", String(operation) }, { L"target", target.Length() ? String(target) : Null() }, { L"hresult", FAILED(hresult) ? String(native::Hex(static_cast<ULONG>(hresult))) : Null() }, { L"phase", String(phase) } });
		output(json::JsonToString(JsonMap({ { L"command", String(command) }, { L"ok", Boolean(false) }, { L"generation", root.snapshot ? String(session + L"/g" + itow(root.snapshot->generation)) : Null() }, { L"result", Null() }, { L"error", error } })), fatal, fatal);
	}
	WString CliViewModel::Id(native::ValueKind kind, vint key, vint document)
	{
		if (!key) return {};
		auto generation = root.lifetime->generation.load();
		if (generation != currentIdGeneration) { currentIds.Clear(); currentIdGeneration = generation; }
		Pair<vint, vint> identity{ static_cast<vint>(kind), key };
		if (currentIds.Keys().Contains(identity))
		{
			auto id = currentIds[identity];
			if (document) { auto updated = references[id]; updated.document = document; references.Set(id, updated); }
			return id;
		}
		auto id = session + (kind == native::ValueKind::Range ? L"/r" : L"/e") + itow(++nextId);
		references.Add(id, { kind, key, generation, document }); currentIds.Add(identity, id); return id;
	}
	Reference CliViewModel::Resolve(const WString& id, native::ValueKind kind)
	{
		auto index = references.Keys().IndexOf(id);
		if (index == -1) throw CommandFailure(L"InvalidId", L"Unknown reference.");
		auto ref = references.Values()[index];
		if (ref.kind != kind) throw CommandFailure(L"WrongKind", L"Reference has the wrong kind.");
		if (!root.snapshot || ref.generation != root.lifetime->generation) throw CommandFailure(L"ExpiredId", L"Reference expired. Reacquire it after refresh or mutation.");
		return ref;
	}
	Ptr<ProcessNodeViewModel> CliViewModel::ResolveWindow(const WString& id)
	{
		if (!windows.Keys().Contains(id)) throw CommandFailure(L"InvalidId", L"Unknown or expired window ID. Use List-Process.");
		if (!native::IsCurrentWindow(windows[id]->window.identity)) throw CommandFailure(L"ExpiredId", L"The window no longer exists. Use List-Process.");
		return windows[id];
	}
	void CliViewModel::PublishReferences(const List<native::ReferenceData>& values)
	{
		CopyFrom(nativeReferences, values);
		for (auto&& value : values) Id(value.kind, value.key, value.documentKey);
	}
	Json CliViewModel::References()
	{
		auto result = Ptr(new json::JsonArray);
		for (auto&& ref : nativeReferences) result->items.Add(JsonMap({ { L"id", String(Id(ref.kind, ref.key, ref.documentKey)) }, { L"kind", String(ref.kind == native::ValueKind::Range ? L"Range" : L"Element") }, { L"documentId", ref.documentKey ? String(Id(native::ValueKind::Element, ref.documentKey)) : Null() }, { L"treeNodeId", ref.kind == native::ValueKind::Element && root.nodes.Keys().Contains(ref.key) ? String(Id(native::ValueKind::Element, ref.key)) : Null() }, { L"label", String(ref.label) } }));
		return result;
	}
	void CliViewModel::Native(const WString& operation, Func<void(native::UiaSession&)> task)
	{
		auto gate = root.lifetime;
		auto worker = &root.worker;
		root.QueueNative(operation, [this, gate, worker, task]()
		{
			try { task(*worker->session.Obj()); }
			catch (const CommandFailure& error)
			{
				auto code = error.code, message = error.Message();
				UiaListViewModel::Post(gate, [this, code, message](UiaListViewModel&) { Fail(code, message); });
			}
		});
	}
	void CliViewModel::Inspect(vint key, Func<void()> completion)
	{
		auto gate = root.lifetime;
		Native(L"Inspect", [this, gate, key, completion](native::UiaSession& session)
		{
			auto value = session.Inspect(key);
			UiaListViewModel::Post(gate, [this, value, completion](UiaListViewModel&) { inspection = value; PublishReferences(value->references); completion(); });
		});
	}
	void CliViewModel::Refresh(Func<void()> completion)
	{
		inspection = nullptr; ranges.Clear();
		published = [this, completion]() { if (!root.treeBusy) { published = {}; completion(); } };
		root.RefreshWindow();
	}
	Json CliViewModel::Value(Ptr<native::ValueData> value)
	{
		using native::ValueKind;
		if (!value) return Null();
		const wchar_t* names[] = { L"Null", L"Unsupported", L"Mixed", L"Boolean", L"Signed", L"Unsigned", L"Real", L"String", L"Element", L"Range", L"Array", L"Opaque" };
		Json payload = Null();
		switch (value->kind)
		{
		case ValueKind::Boolean: payload = Boolean(value->signedValue != 0); break;
		case ValueKind::Signed: payload = String(i64tow(value->signedValue)); break;
		case ValueKind::Unsigned: payload = String(u64tow(value->unsignedValue)); break;
		case ValueKind::Real: payload = Number(value->realValue); break;
		case ValueKind::String: payload = String(value->text); break;
		case ValueKind::Element: case ValueKind::Range: payload = value->key ? String(Id(value->kind, value->key)) : Null(); break;
		case ValueKind::Array:
			{
				auto array = Ptr(new json::JsonArray); for (auto&& item : value->items) array->items.Add(Value(item)); payload = array;
			}
			break;
		case ValueKind::Opaque: payload = String(value->text); break;
		}
		auto dimensions = Ptr(new json::JsonArray);
		for (auto&& dimension : value->dimensions) dimensions->items.Add(JsonMap({ { L"lower", Number(dimension.key) }, { L"upper", Number(dimension.value) } }));
		return JsonMap({ { L"kind", String(names[static_cast<vint>(value->kind)]) }, { L"vartype", Number(value->type) }, { L"value", payload }, { L"dimensions", dimensions } });
	}
	Json CliViewModel::Property(const native::PropertyData& property)
	{
		Json setter = Null();
		if (property.setter)
		{
			auto&& spec = *property.setter.Obj(); auto choices = Ptr(new json::JsonArray);
			for (auto&& choice : spec.choices) choices->items.Add(JsonMap({ { L"value", Number(choice.id) }, { L"name", String(choice.label) } }));
			const wchar_t* names[] = { L"None", L"Text", L"Number", L"Choice" };
			setter = JsonMap({ { L"kind", String(names[static_cast<vint>(spec.kind)]) }, { L"multiline", Boolean(spec.multiline) }, { L"minimum", Number(spec.minimum) }, { L"maximum", Number(spec.maximum) }, { L"choices", choices } });
		}
		auto enumField = property.name;
		for (vint i = 0; i < native::PropertyCatalogCount; i++) if (native::PropertyCatalog[i].id == property.id) enumField = native::PropertyCatalog[i].name;
		WString enumName;
		if (property.value->kind == native::ValueKind::Signed) enumName = native::EnumerationName(enumField, static_cast<LONG>(property.value->signedValue));
		if (property.value->kind == native::ValueKind::Unsigned) enumName = native::EnumerationName(enumField, static_cast<LONG>(property.value->unsignedValue));
		return JsonMap({ { L"propertyId", Number(property.id) }, { L"name", String(property.name) }, { L"value", Value(property.value) }, { L"enumName", enumName.Length() ? String(enumName) : Null() }, { L"sourcePattern", Number(property.sourcePattern) }, { L"setter", setter } });
	}
	Json CliViewModel::Section(Ptr<native::ActionSectionData> section, bool catalog)
	{
		auto commands = Ptr(new json::JsonArray), readouts = Ptr(new json::JsonArray);
		for (auto&& item : section->readouts) readouts->items.Add(Property(item));
		for (auto&& action : section->commands)
		{
			auto parameters = Ptr(new json::JsonArray);
			for (auto&& parameter : action->parameters)
			{
				const wchar_t* kinds[] = { L"string", L"integer", L"number", L"choice", L"element", L"range", L"variant", L"attributeIds" };
				auto choices = Ptr(new json::JsonArray);
				for (auto&& choice : parameter->choices) choices->items.Add(JsonMap({ { L"value", Number(choice.id) }, { L"name", String(choice.label) } }));
				parameters->items.Add(JsonMap({ { L"name", String(parameter->name) }, { L"type", String(kinds[static_cast<vint>(parameter->kind)]) }, { L"nullable", Boolean(parameter->nullable) }, { L"multiline", Boolean(parameter->multiline) }, { L"minimum", Number(parameter->minimum) }, { L"maximum", Number(parameter->maximum) }, { L"choices", choices } }));
			}
			auto example = Ptr(new json::JsonObject);
			for (auto&& parameter : action->parameters)
			{
				Json value;
				switch (parameter->kind)
				{
				case native::ArgumentKind::Text: value = String(parameter->initial); break;
				case native::ArgumentKind::Element: case native::ArgumentKind::Range: value = parameter->nullable ? Null() : String(parameter->kind == native::ArgumentKind::Range ? L"RANGE_ID" : L"ELEMENT_ID"); break;
				case native::ArgumentKind::Variant: value = Null(); break;
				case native::ArgumentKind::AttributeList: { auto array = Ptr(new json::JsonArray); array->items.Add(Number(40000)); value = array; } break;
				default: value = Number(wtof(parameter->initial)); break;
				}
				auto field = Ptr(new json::JsonObjectField); field->name.value = parameter->name; field->value = value; example->fields.Add(field);
			}
			auto effect = action->mutation ? L"Invokes once; refresh invalidates element/range IDs; returns tree and target readback." : action->pureGetter ? L"Reads the target; retains generation and references." : section->rangeKey ? L"May edit this local range; retains document, generation and workspace operands." : L"Explicit operation; retains generation. StartListening remains active until Cancel or shutdown.";
			commands->items.Add(JsonMap({ { L"command", String(L"Run-" + OperationName(*action.Obj())) }, { L"name", String(action->name) }, { L"enabled", catalog ? Null() : Boolean(action->enabled) }, { L"reason", catalog ? String(L"Catalog support; current capability must be queried on a target.") : action->enabled ? Null() : String(L"Current capability does not permit this operation.") }, { L"mutation", Boolean(action->mutation) }, { L"pureGetter", Boolean(action->pureGetter) }, { L"parameters", parameters }, { L"result", String(action->mutation ? L"{value:Value,readback:Inspection|null,removed?:true,tree:Tree}; Close: {closed:true,tree?:Tree}" : L"{value:Value,range:ProviderSection|null,references:Reference[]}") }, { L"effects", String(effect) }, { L"example", String(L"Run-" + OperationName(*action.Obj()) + (section->rangeKey ? L" RANGE_ID " : L" ELEMENT_ID ") + json::JsonToString(example)) } }));
		}
		WString provider = L"IUIAutomationElement", client = provider;
		for (vint i = 0; i < native::PatternCatalogCount; i++) if (native::PatternCatalog[i].id == section->pattern) { provider = native::PatternCatalog[i].provider; client = native::PatternCatalog[i].client; }
		if (section->rangeKey) provider = client = L"IUIAutomationTextRange";
		return JsonMap({ { L"patternId", Number(section->pattern) }, { L"provider", String(provider) }, { L"client", String(client) }, { L"readouts", readouts }, { L"commands", commands }, { L"references", section->rangeKey && !catalog ? References() : Ptr(new json::JsonArray) }, { L"documentId", section->documentKey ? String(Id(native::ValueKind::Element, section->documentKey)) : Null() } });
	}
	Json CliViewModel::Inspection()
	{
		auto properties = Ptr(new json::JsonArray), sections = Ptr(new json::JsonArray), refs = Ptr(new json::JsonArray);
		for (auto&& property : inspection->properties) if (property.value->kind != native::ValueKind::Unsupported) properties->items.Add(Property(property));
		for (auto&& section : inspection->sections) sections->items.Add(Section(section));
		for (auto&& ref : inspection->references) refs->items.Add(JsonMap({ { L"id", String(Id(ref.kind, ref.key, ref.documentKey)) }, { L"kind", String(ref.kind == native::ValueKind::Range ? L"Range" : L"Element") }, { L"documentId", ref.documentKey ? String(Id(native::ValueKind::Element, ref.documentKey)) : Null() }, { L"treeNodeId", ref.kind == native::ValueKind::Element && root.nodes.Keys().Contains(ref.key) ? String(Id(native::ValueKind::Element, ref.key)) : Null() }, { L"label", String(ref.label) } }));
		return JsonMap({ { L"id", String(Id(native::ValueKind::Element, inspection->nodeKey)) }, { L"properties", properties }, { L"providers", sections }, { L"references", refs } });
	}
	Json CliViewModel::Tree()
	{
		auto nodes = Ptr(new json::JsonArray), roots = Ptr(new json::JsonArray);
		if (root.snapshot) for (auto&& node : root.snapshot->nodes)
		{
			auto id = String(Id(native::ValueKind::Element, node.key)); if (!node.parent) roots->items.Add(id);
			nodes->items.Add(JsonMap({ { L"id", id }, { L"parentId", node.parent ? String(Id(native::ValueKind::Element, node.parent)) : Null() }, { L"runtimeId", String(node.runtimeId) }, { L"controlType", Number(node.controlType) }, { L"role", String(node.role) }, { L"client", String(node.client) }, { L"name", String(node.name) }, { L"automationId", String(node.automationId) }, { L"providers", String(node.providers) }, { L"bounds", Bounds(node.bounds) }, { L"offscreen", Boolean(node.offscreen) } }));
		}
		return JsonMap({ { L"rootIds", roots }, { L"nodes", nodes } });
	}
	Json CliViewModel::Processes()
	{
		if (!root.selectedWindow) { inspection = nullptr; ranges.Clear(); nativeReferences.Clear(); }
		auto result = Ptr(new json::JsonArray);
		Dictionary<WString, Ptr<ProcessNodeViewModel>> currentWindows;
		List<Pair<Ptr<ProcessNodeViewModel>, DWORD>> pending;
		for (auto&& child : root.processRoot->children) pending.Add({ child.Cast<ProcessNodeViewModel>(), 0 });
		for (vint index = 0; index < pending.Count(); index++)
		{
			auto node = pending[index].key; auto parent = pending[index].value;
			auto windowList = Ptr(new json::JsonArray);
			for (auto&& item : node->windows)
			{
				auto window = item.Cast<ProcessNodeViewModel>(); WString id;
				for (vint i = 0; i < windows.Count(); i++) if (windows.Values()[i]->window.identity == window->window.identity
					&& (!windows.Values()[i]->creationTime || !window->creationTime || windows.Values()[i]->creationTime == window->creationTime)) id = windows.Keys()[i];
				if (!id.Length()) id = session + L"/w" + itow(++nextId);
				currentWindows.Add(id, window);
				windowList->items.Add(JsonMap({ { L"id", String(id) }, { L"pid", Number(window->processId) }, { L"hwnd", String(native::Hex(window->GetWindowKey())) }, { L"title", String(window->window.title) }, { L"class", String(window->window.className) } }));
			}
			result->items.Add(JsonMap({ { L"pid", Number(node->processId) }, { L"parentPid", parent ? Number(parent) : Null() }, { L"creationTime", node->creationTime ? String(u64tow(node->creationTime.Value())) : Null() }, { L"executable", String(node->executable) }, { L"windows", windowList } }));
			for (auto&& child : node->children) pending.Add({ child.Cast<ProcessNodeViewModel>(), node->processId });
		}
		windows = std::move(currentWindows); return JsonMap({ { L"processes", result } });
	}
	Json CliViewModel::Preview()
	{
		auto capture = root.preview->capture;
		const wchar_t* names[] = { L"Ready", L"Unavailable", L"GeometryChanged", L"Canceled" };
		WString bitmap;
		if (capture && capture->status == native::CaptureStatus::Ready)
		{
			DWORD length = 0;
			CHECK_ERROR(CryptBinaryToStringW(&capture->bitmap[0], static_cast<DWORD>(capture->bitmap.Count()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &length), L"uialist::cli::Preview#Base64 size failed.");
			Array<wchar_t> buffer(length);
			CHECK_ERROR(CryptBinaryToStringW(&capture->bitmap[0], static_cast<DWORD>(capture->bitmap.Count()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &buffer[0], &length), L"uialist::cli::Preview#Base64 encoding failed.");
			bitmap = WString::CopyFrom(&buffer[0], length);
		}
		return JsonMap({ { L"status", String(capture ? names[static_cast<vint>(capture->status)] : L"Unavailable") }, { L"bounds", capture ? Bounds(capture->bounds) : Null() }, { L"dpi", capture ? Number(capture->dpi) : Null() }, { L"width", capture ? Number(capture->bounds.right - capture->bounds.left) : Number(0) }, { L"height", capture ? Number(capture->bounds.bottom - capture->bounds.top) : Number(0) }, { L"timestamp", capture ? String(capture->timestamp) : Null() }, { L"mediaType", String(L"image/bmp") }, { L"base64", bitmap.Length() ? String(bitmap) : Null() } });
	}
}

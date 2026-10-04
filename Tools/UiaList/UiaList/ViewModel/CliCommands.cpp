#include "CliViewModel.h"
#include "UiaCatalog.Actions.Windows.h"
#include <cmath>

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;

namespace uialist::cli
{
	extern Json Field(Ptr<json::JsonObject> object, const WString& name);
	extern WString Text(Json value);
	extern WString NumericText(Json value);
	extern void Fields(Ptr<json::JsonObject> object, std::initializer_list<WString> names);

	Ptr<native::ValueData> CliViewModel::InputValue(Json value, VARTYPE type, bool elementArray)
	{
		using namespace native;
		if (auto literal = value.Cast<json::JsonLiteral>())
		{
			if (type == VT_EMPTY && literal->value == json::JsonLiteralValue::Null) return Ptr(new ValueData);
			if (type == VT_BOOL && literal->value != json::JsonLiteralValue::Null) return BooleanValue(literal->value == json::JsonLiteralValue::True);
			if (type == VT_UNKNOWN && literal->value == json::JsonLiteralValue::Null) return Ptr(new ValueData);
		}
		if (elementArray || (type & VT_ARRAY))
		{
			auto array = value.Cast<json::JsonArray>();
			if (!array) throw CommandFailure(L"InvalidType", L"Expected JSON array.");
			auto result = Ptr(new ValueData); result->kind = ValueKind::Array; result->type = type;
			for (auto&& item : array->items) result->items.Add(InputValue(item, elementArray ? VT_UNKNOWN : type & VT_TYPEMASK, false));
			result->dimensions.Add({ 0, static_cast<LONG>(result->items.Count()) - 1 }); return result;
		}
		if (type == VT_BSTR)
		{
			auto text = Text(value); for (vint i = 0; i < text.Length(); i++) if (!text[i]) throw CommandFailure(L"InvalidArguments", L"Embedded NUL is not accepted by this string method.");
			return StringValue(text);
		}
		if (type == VT_UNKNOWN)
		{
			auto id = Text(value);
			if (!references.Keys().Contains(id)) throw CommandFailure(L"InvalidId", L"Unknown reference.");
			auto ref = Resolve(id, ValueKind::Element);
			auto result = Ptr(new ValueData); result->kind = ref.kind; result->type = VT_UNKNOWN; result->key = ref.key; return result;
		}
		bool valid = false;
		if (type == VT_I4)
		{
			auto number = wtoi64_test(NumericText(value), valid);
			if (valid && number >= INT_MIN && number <= INT_MAX) return IntegerValue(number);
		}
		if (type == VT_R8)
		{
			auto number = wtof_test(NumericText(value), valid);
			if (valid && std::isfinite(number)) return NumberValue(number);
		}
		throw CommandFailure(L"InvalidType", L"Value does not match the catalog VARTYPE " + itow(type) + L".");
	}
	List<native::ActionArgument> CliViewModel::Arguments(const native::ActionSpec& spec, Ptr<json::JsonObject> args)
	{
		using namespace native;
		if (args->fields.Count() != spec.parameters.Count()) throw CommandFailure(L"InvalidArguments", L"Supply exactly the parameters listed by Query-Providers or Query-Range.");
		List<ActionArgument> result;
		for (auto&& parameter : spec.parameters)
		{
			auto value = Field(args, parameter->name); ActionArgument argument; WString text; vint reference = 0;
			switch (parameter->kind)
			{
			case ArgumentKind::Text: text = Text(value); break;
			case ArgumentKind::Integer: case ArgumentKind::Number: text = NumericText(value); break;
			case ArgumentKind::Choice:
				if (auto literal = value.Cast<json::JsonLiteral>(); literal && literal->value != json::JsonLiteralValue::Null)
				{
					if (parameter->choices.Count() != 2 || parameter->choices[0].label != L"false" || parameter->choices[1].label != L"true") throw CommandFailure(L"InvalidType", L"This enumeration requires an integer.");
					text = literal->value == json::JsonLiteralValue::True ? L"1" : L"0";
				}
				else text = NumericText(value);
				break;
			case ArgumentKind::Element: case ArgumentKind::Range:
				{
					if (auto literal = value.Cast<json::JsonLiteral>(); literal && literal->value == json::JsonLiteralValue::Null && parameter->nullable) break;
					auto ref = Resolve(Text(value), parameter->kind == ArgumentKind::Range ? ValueKind::Range : ValueKind::Element); reference = ref.key;
					if (spec.rangeKey && ref.kind == ValueKind::Range)
					{
						auto own = Resolve(target, ValueKind::Range);
						if (own.document != ref.document) throw CommandFailure(L"WrongDocument", L"Ranges belong to different documents.");
					}
				}
				break;
			case ArgumentKind::AttributeList:
				{
					auto array = value.Cast<json::JsonArray>(); if (!array) throw CommandFailure(L"InvalidType", L"Expected an array of attribute IDs.");
					for (auto&& item : array->items) text += (text.Length() ? L"," : L"") + NumericText(item);
				}
				break;
			case ArgumentKind::Variant:
				{
					auto property = spec.code == ActionCode::FindItem;
					auto id = wtoi(NumericText(Field(args, property ? L"propertyId" : L"attributeId")));
					auto catalog = property ? PropertyCatalog : AttributeCatalog; auto count = property ? PropertyCatalogCount : AttributeCatalogCount;
					VARTYPE type = VT_EMPTY; bool elements = false;
					for (vint i = 0; i < count; i++) if (catalog[i].id == id) { type = catalog[i].expectedType; elements = catalog[i].elementArray; }
					argument.value = InputValue(value, type, elements);
				}
				break;
			}
			if (!ParseArgument(*parameter.Obj(), text, reference, argument)) throw CommandFailure(L"InvalidArguments", L"Invalid value for " + parameter->name);
			result.Add(std::move(argument));
		}
		return result;
	}
	void CliViewModel::Run(Reference reference, const WString& name, Ptr<json::JsonObject> args)
	{
		using namespace native;
		Ptr<ActionSpec> spec;
		List<Ptr<ActionSectionData>> sections;
		if (reference.kind == ValueKind::Range)
		{
			auto section = DescribeActionCatalog(0, true);
			for (auto&& action : section->commands) action->rangeKey = reference.key;
			sections.Add(section);
		}
		else
		{
			sections.Add(DescribeActionCatalog(0));
			for (vint i = 0; i < PatternCatalogCount; i++) sections.Add(DescribeActionCatalog(PatternCatalog[i].id));
		}
		for (auto&& section : sections) for (auto&& item : section->commands) if (OperationName(*item.Obj()) == name) spec = item;
		if (!spec) throw CommandFailure(L"UnsupportedOperation", L"Operation is not supported on this target. See Query-Providers or Query-Range.");
		if (!spec->enabled) throw CommandFailure(L"CapabilityChanged", L"The current capability disables this operation.");
		auto arguments = Ptr(new List<ActionArgument>(Arguments(*spec.Obj(), args)));
		auto gate = root.lifetime;
		phase = L"invocation";
		Native(name, [this, gate, reference, spec, arguments](UiaSession& session)
		{
			auto outcome = Ptr(new ActionOutcome(session.Execute(reference.key, *spec.Obj(), *arguments.Obj())));
			if (!outcome->available) throw CommandFailure(L"CapabilityChanged", L"Capability changed before invocation.");
			if (outcome->mutation) session.DiscardRanges();
			Ptr<ActionSectionData> range;
			if (reference.kind == ValueKind::Range && (spec->code == ActionCode::RangeExpand || spec->code == ActionCode::RangeMove || spec->code == ActionCode::RangeMoveEndpoint || spec->code == ActionCode::RangeTransferEndpoint)) { UiaListViewModel::Post(gate, [this](UiaListViewModel&) { phase = L"readback"; }); range = session.DescribeRange(reference.key); }
			auto refs = Ptr(new List<ReferenceData>(session.GetReferences()));
			UiaListViewModel::Post(gate, [this, reference, outcome, range, refs](UiaListViewModel& root)
			{
				PublishReferences(*refs.Obj());
				if (range) ranges.Set(reference.key, range);
				if (outcome->closedWindow)
				{
					inspection = nullptr; ranges.Clear();
					if (!native::IsCurrentWindow(root.selectedWindow->window.identity)) { root.ClearSelection(); Finish(JsonMap({ { L"closed", Boolean(true) } })); }
					else Refresh([this]() { Finish(JsonMap({ { L"closed", Boolean(true) }, { L"tree", Tree() } })); });
				}
				else if (outcome->mutation)
				{
					phase = L"readback";
					auto key = reference.kind == ValueKind::Range ? reference.document : reference.key;
					auto wasInTree = root.nodes.Keys().Contains(key);
					Refresh([this, key, outcome, wasInTree]()
					{
						if (wasInTree && !this->root.nodes.Keys().Contains(key)) Finish(JsonMap({ { L"value", Value(outcome->value) }, { L"removed", Boolean(true) }, { L"readback", Null() }, { L"tree", Tree() } }));
						else Inspect(key, [this, outcome]() { Finish(JsonMap({ { L"value", Value(outcome->value) }, { L"readback", Inspection() }, { L"tree", Tree() } })); });
					});
				}
				else Finish(JsonMap({ { L"value", Value(outcome->value) }, { L"range", range ? Section(range) : Null() }, { L"references", References() } }));
			});
		});
	}
	void CliViewModel::Execute(const WString& line)
	{
		vint begin = 0;
		auto space = [](wchar_t c) { return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n'; };
		while (begin < line.Length() && space(line[begin])) begin++;
		if (begin == line.Length()) { output({}, false, false); return; }
		CHECK_ERROR(!busy, L"uialist::cli::CliViewModel::Execute#Commands must be sequential.");
		busy = true; command = {}; target = {}; phase = L"validation";
		try
		{
			auto word = [&]() { auto start = begin; while (begin < line.Length() && !space(line[begin])) begin++; auto value = line.Sub(start, begin - start); while (begin < line.Length() && space(line[begin])) begin++; return value; };
			command = word();
			for (vint i = 0; i < line.Length(); i++) if (!line[i]) throw CommandFailure(L"InvalidJson", L"A command line cannot contain a raw NUL character.");
			if (begin < line.Length() && line[begin] != L'{') target = word();
			auto arguments = Ptr(new json::JsonObject);
			if (begin < line.Length())
			{
				try { arguments = json::JsonParse(line.Sub(begin, line.Length() - begin), parser).Cast<json::JsonObject>(); }
				catch (const Exception&) { throw CommandFailure(L"InvalidJson", L"Arguments must be a valid JSON object."); }
				if (!arguments) throw CommandFailure(L"InvalidJson", L"Arguments must be a JSON object.");
			}
			Dispatch(command, target, arguments);
		}
		catch (const CommandFailure& error) { Fail(error.code, error.Message()); }
	}
	void CliViewModel::Dispatch(const WString& verb, const WString& id, Ptr<json::JsonObject> args)
	{
		using namespace native;
		if (verb == L"Help-Command") { Fields(args, {}); Finish(Help(id)); return; }
		if (verb == L"Exit-Application")
		{
			Fields(args, {}); if (id.Length()) throw CommandFailure(L"InvalidArguments", L"Exit-Application takes no ID.");
			root.RequestClose(); Finish(JsonMap({ { L"exited", Boolean(true) } }), true); return;
		}
		if (verb == L"List-Process")
		{
			Fields(args, {}); if (id.Length()) throw CommandFailure(L"InvalidArguments", L"List-Process takes no ID.");
			published = [this]() { if (!root.processBusy) Finish(Processes()); };
			if (!root.initialized) root.Initialize(); else root.RefreshProcesses(); return;
		}
		if (verb == L"Print-Window" || verb == L"Refresh-Window" || verb == L"Query-Preview" || verb == L"HitTest-Preview")
		{
			auto window = ResolveWindow(id);
			if (verb == L"Print-Window")
			{
				Fields(args, {});
				if (root.selectedWindow && root.selectedWindow->window.identity == window->window.identity) { Finish(Tree()); return; }
				inspection = nullptr; ranges.Clear();
				root.SelectProcess(root.processes[window->processId]);
				published = [this]() { if (!root.treeBusy) Finish(Tree()); };
				root.SelectWindow(window); return;
			}
			if (!root.selectedWindow || root.selectedWindow->window.identity != window->window.identity) throw CommandFailure(L"WrongWindow", L"The window is not selected.");
			if (verb == L"Refresh-Window") { Fields(args, {}); Refresh([this]() { Finish(Tree()); }); return; }
			if (verb == L"Query-Preview") { Fields(args, {}); Finish(Preview()); return; }
			Fields(args, { L"x", L"y" });
			bool valid = false;
			auto x = wtoi_test(NumericText(Field(args, L"x")), valid); if (!valid) throw CommandFailure(L"InvalidArguments", L"x must be an integer.");
			auto y = wtoi_test(NumericText(Field(args, L"y")), valid); if (!valid) throw CommandFailure(L"InvalidArguments", L"y must be an integer.");
			auto key = root.preview->HitTest(x, y); root.preview->Click(x, y);
			Json bounds = Null();
			if (key)
			{
				auto b = root.nodes[key]->data.bounds;
				bounds = JsonMap({ { L"left", Number(b.left) }, { L"top", Number(b.top) }, { L"right", Number(b.right) }, { L"bottom", Number(b.bottom) } });
			}
			Finish(JsonMap({ { L"id", key ? String(Id(ValueKind::Element, key)) : Null() }, { L"bounds", bounds } })); return;
		}
		if (verb == L"Help-Provider")
		{
			Fields(args, {});
			auto catalog = Ptr(new json::JsonArray), sections = Ptr(new json::JsonArray);
			for (vint i = 0; i < PatternCatalogCount; i++) if (!id.Length() || id == PatternCatalog[i].provider || id == PatternCatalog[i].client) catalog->items.Add(Section(DescribeActionCatalog(PatternCatalog[i].id), true));
			if (!id.Length() || id == L"IUIAutomationElement" || id == L"IUIAutomationElement3" || id == L"IUIAutomationElement7") catalog->items.Add(Section(DescribeActionCatalog(0), true));
			if (!id.Length() || id == L"IUIAutomationTextRange" || id == L"IUIAutomationTextRange2" || id == L"IUIAutomationTextRange3") catalog->items.Add(Section(DescribeActionCatalog(0, true), true));
			if (!catalog->items.Count()) throw CommandFailure(L"UnknownProvider", L"Unknown provider or client interface name.");
			if (inspection) for (auto&& section : inspection->sections)
			{
				bool include = !id.Length() || (!section->pattern && id == L"IUIAutomationElement");
				for (vint i = 0; i < PatternCatalogCount; i++) if (PatternCatalog[i].id == section->pattern && (id == PatternCatalog[i].provider || id == PatternCatalog[i].client)) include = true;
				if (include) sections->items.Add(Section(section));
			}
			if (!id.Length() || id == L"IUIAutomationTextRange" || id == L"IUIAutomationTextRange2" || id == L"IUIAutomationTextRange3") for (auto&& range : ranges.Values()) sections->items.Add(Section(range));
			auto descriptors = [](const IdDescriptor* items, vint count)
			{
				auto result = Ptr(new json::JsonArray);
				for (vint i = 0; i < count; i++) result->items.Add(JsonMap({ { L"id", Number(items[i].id) }, { L"name", String(items[i].name) }, { L"vartype", Number(items[i].expectedType) }, { L"elementArray", Boolean(items[i].elementArray) } }));
				return result;
			};
			Finish(JsonMap({ { L"catalog", catalog }, { L"currentTarget", sections }, { L"properties", descriptors(PropertyCatalog, PropertyCatalogCount) }, { L"attributes", descriptors(AttributeCatalog, AttributeCatalogCount) }, { L"roles", descriptors(ControlTypeCatalog, ControlTypeCatalogCount) }, { L"metadata", descriptors(MetadataCatalog, MetadataCatalogCount) }, { L"boundary", String(L"Known SDK client operations only; provider names alias client patterns. Opaque object internals are not discoverable.") } })); return;
		}
		if (verb == L"Select-Node")
		{
			Fields(args, {}); auto ref = Resolve(id, ValueKind::Element);
			if (!root.nodes.Keys().Contains(ref.key)) throw CommandFailure(L"OutsideTree", L"Returned element is outside the selected Raw View tree.");
			auto node = root.nodes[ref.key]; for (auto parent = node->data.parent; parent; parent = root.nodes[parent]->data.parent) root.nodes[parent]->SetIsExpanded(true);
			root.SelectNode(node); Finish(JsonMap({ { L"id", String(id) } })); return;
		}
		if (verb == L"Query-Range")
		{
			Fields(args, {}); auto ref = Resolve(id, ValueKind::Range); auto gate = root.lifetime;
			Native(L"DescribeRange", [this, ref, gate](UiaSession& session)
			{
				auto range = session.DescribeRange(ref.key); auto refs = Ptr(new List<ReferenceData>(session.GetReferences()));
				UiaListViewModel::Post(gate, [this, ref, range, refs](UiaListViewModel&) { PublishReferences(*refs.Obj()); ranges.Set(ref.key, range); Finish(Section(range)); });
			}); return;
		}
		if (verb == L"Query-Node" || verb == L"Query-Properties" || verb == L"Query-Providers")
		{
			Fields(args, {}); auto ref = Resolve(id, ValueKind::Element); Inspect(ref.key, [this]() { Finish(Inspection()); }); return;
		}
		if (verb == L"Set-Property")
		{
			Fields(args, { L"propertyId", L"value" }); auto ref = Resolve(id, ValueKind::Element);
			if (!inspection || inspection->nodeKey != ref.key) { Inspect(ref.key, [this, verb, id, args]() { try { Dispatch(verb, id, args); } catch (const CommandFailure& error) { Fail(error.code, error.Message()); } }); return; }
			bool valid = false; auto property = wtoi_test(NumericText(Field(args, L"propertyId")), valid);
			if (!valid || property < 0 || property > INT_MAX) throw CommandFailure(L"InvalidArguments", L"Invalid property ID.");
			Ptr<SetterSpec> setter;
			for (auto&& row : inspection->properties) if (row.id == property) setter = row.setter;
			if (!setter || setter->kind == SetterKind::None) throw CommandFailure(L"ReadOnly", L"No mapped setter is available for this property.");
			auto input = Field(args, L"value"); auto text = setter->kind == SetterKind::Text ? Text(input) : NumericText(input);
			auto parameter = TextArgument(L"value");
			if (setter->kind == SetterKind::Number) parameter = NumberArgument(L"value", setter->minimum, setter->maximum);
			if (setter->kind == SetterKind::Choice) { parameter->kind = ArgumentKind::Choice; CopyFrom(parameter->choices, setter->choices); }
			ActionArgument parsed; if (!ParseArgument(*parameter.Obj(), text, 0, parsed)) throw CommandFailure(L"InvalidArguments", L"Invalid property value.");
			auto gate = root.lifetime; phase = L"invocation";
			Native(L"SetProperty", [this, ref, property, text, gate](UiaSession& session)
			{
				if (!session.SetProperty(ref.key, static_cast<PROPERTYID>(property), text)) throw CommandFailure(L"CapabilityChanged", L"Setter capability changed before invocation.");
				session.DiscardRanges();
				UiaListViewModel::Post(gate, [this, ref](UiaListViewModel& root)
				{
					phase = L"readback";
					auto wasInTree = root.nodes.Keys().Contains(ref.key);
					Refresh([this, ref, wasInTree]()
					{
						if (wasInTree && !this->root.nodes.Keys().Contains(ref.key)) Finish(JsonMap({ { L"removed", Boolean(true) }, { L"readback", Null() }, { L"tree", Tree() } }));
						else Inspect(ref.key, [this]() { Finish(JsonMap({ { L"readback", Inspection() }, { L"tree", Tree() } })); });
					});
				});
			}); return;
		}
		if (verb.Length() > 4 && verb.Left(4) == L"Run-")
		{
			if (!references.Keys().Contains(id)) throw CommandFailure(L"InvalidId", L"Unknown target ID.");
			auto ref = Resolve(id, references[id].kind); auto name = verb.Sub(4, verb.Length() - 4);
			Run(ref, name, args); return;
		}
		throw CommandFailure(L"UnknownCommand", L"Unknown command. Use Help-Command.");
	}
	Json CliViewModel::Help(const WString& name)
	{
		auto result = Ptr(new json::JsonArray);
		auto add = [&](const wchar_t* command, const wchar_t* target, const wchar_t* arguments, const wchar_t* returns, const wchar_t* effects, const wchar_t* example)
		{
			if (!name.Length() || name == command) result->items.Add(JsonMap({
				{ L"command", String(command) }, { L"target", String(target) }, { L"arguments", String(arguments) },
				{ L"result", String(returns) }, { L"effects", String(effects) }, { L"example", String(example) }
			}));
		};
		add(L"Help-Command", L"optional command name", L"{}", L"{commands:CommandSchema[],schemas:object}", L"No target access.", L"Help-Command Set-Property");
		add(L"List-Process", L"none", L"{}", L"{processes:Process[]}", L"Refresh discovery; retain surviving window IDs; remove absent windows and clear their selection.", L"List-Process");
		add(L"Print-Window", L"WindowId", L"{}", L"Tree", L"Select a window and await tree/capture completion; selecting a different window expires elements/ranges. Repeated print returns the cached tree.", L"Print-Window WINDOW_ID");
		add(L"Refresh-Window", L"selected WindowId", L"{}", L"Tree", L"Rebuild selected tree and capture. Expires all element/range IDs. Does not activate or move the target.", L"Refresh-Window WINDOW_ID");
		add(L"Select-Node", L"ElementId in the cached tree", L"{}", L"{id:ElementId}", L"Select/reveal the inspector node without calling target focus/input methods.", L"Select-Node ELEMENT_ID");
		add(L"Query-Node", L"ElementId including external references", L"{}", L"Inspection", L"Read current properties, providers and returned references. Existing generation and references remain valid.", L"Query-Node ELEMENT_ID");
		add(L"Query-Properties", L"ElementId", L"{}", L"Inspection", L"Same current typed snapshot as Query-Node; properties omit Unsupported values and include mapped setters.", L"Query-Properties ELEMENT_ID");
		add(L"Set-Property", L"ElementId", L"{propertyId:integer,value:string|number}", L"{readback:Inspection|null,tree:Tree,removed?:true}", L"Validate mapped setter and current capability, invoke once, refresh and read back. Expires old element/range IDs. Error phase distinguishes invocation from readback.", L"Set-Property ELEMENT_ID {\"propertyId\":30045,\"value\":\"hello\\n\u4e16\u754c\"}");
		add(L"Query-Providers", L"ElementId", L"{}", L"Inspection", L"Same current snapshot as Query-Node; providers report current enabled state and typed readouts.", L"Query-Providers ELEMENT_ID");
		add(L"Help-Provider", L"optional catalog provider or client interface name", L"{}", L"{catalog:ProviderSection[],currentTarget:ProviderSection[],properties:Descriptor[],attributes:Descriptor[],roles:Descriptor[],metadata:Descriptor[],boundary:string}", L"No target calls; catalog enabled=null is independent of live capability. TextRange2/3 use client interface names.", L"Help-Provider IUIAutomationTextRange3");
		add(L"Run-ProviderInterface::Method", L"ElementId or RangeId", L"Exact named fields from Help-Provider; choice=integer (boolean only for false/true choices); variant matches property/attribute VARTYPE; refs are opaque IDs; attributeIds is integer[].", L"{value:Value,range:ProviderSection|null,references:Reference[]} or mutation {value:Value,readback:Inspection|null,tree:Tree,removed?:true}; Close {closed:true,tree?:Tree}", L"See each shared action descriptor. Getter calls retain generation; local range operations retain operands; target mutations expire elements/ranges. No implicit listening or input.", L"Run-IUIAutomationTextRange::FindText RANGE_ID {\"text\":\"hello\",\"backward\":false,\"ignoreCase\":true}");
		add(L"Query-Range", L"RangeId", L"{}", L"ProviderSection", L"Read range preview, attributes, geometry and current operations. GetText(-1) returns the complete text. Retains workspace references.", L"Query-Range RANGE_ID");
		add(L"Query-Preview", L"selected WindowId", L"{}", L"Preview", L"Read current-generation capture status and optional BMP; unavailable capture leaves inspection usable.", L"Query-Preview WINDOW_ID");
		add(L"HitTest-Preview", L"selected WindowId", L"{x:integer,y:integer}", L"{id:ElementId|null,bounds:Bounds|null}", L"Coordinates are physical pixels relative to preview origin. Uses cached geometry once; selects/reveals matched node without target input.", L"HitTest-Preview WINDOW_ID {\"x\":100,\"y\":80}");
		add(L"Exit-Application", L"none", L"{}", L"{exited:true}", L"Drain workers, cancel listening and release UIA on MTA, then emit final response and exit zero. EOF drains without a response.", L"Exit-Application");
		if (!result->items.Count()) throw CommandFailure(L"UnknownCommand", L"Unknown command name.");
		auto schemas = JsonMap({
			{ L"Response", String(L"{command:string,ok:boolean,generation:string|null,result:any|null,error:Error|null}") },
			{ L"Error", String(L"{code:string,message:string,operation:string,target:string|null,hresult:hex-string|null,phase:validation|invocation|readback}") },
			{ L"Process", String(L"{pid:integer,parentPid:integer|null,creationTime:decimal-string|null,executable:string,windows:Window[]}") },
			{ L"Window", String(L"{id:WindowId,pid:integer,hwnd:hex-string,title:string,class:string}") },
			{ L"Tree", String(L"{rootIds:ElementId[],nodes:Node[]} in complete flat preorder; parent precedes child, sibling order is retained") },
			{ L"Node", String(L"{id:ElementId,parentId:ElementId|null,runtimeId:string,controlType:integer,role:string,client:string,name:string,automationId:string,providers:string,bounds:Bounds,offscreen:boolean}") },
			{ L"Inspection", String(L"{id:ElementId,properties:Property[],providers:ProviderSection[],references:Reference[]}") },
			{ L"Property", String(L"{propertyId:integer,name:string,value:Value,enumName:string|null,sourcePattern:integer,setter:Setter|null}; propertyId=0 denotes a pattern readout") },
			{ L"Setter", String(L"{kind:None|Text|Number|Choice,multiline:boolean,minimum:number,maximum:number,choices:{value:integer,name:string}[]}") },
			{ L"Value", String(L"{kind:Null|Unsupported|Mixed|Boolean|Signed|Unsigned|Real|String|Element|Range|Array|Opaque,vartype:integer,value:any,dimensions:{lower:integer,upper:integer}[]}; null/unsupported/mixed payload=null; Boolean=boolean; Signed/Unsigned=decimal string; Real=number or {nonfinite:NaN|Infinity|-Infinity}; String=full string including escaped NUL; Element/Range=opaque ID; Array=Value[]; Opaque=interface description, not a callable reference") },
			{ L"Reference", String(L"{id:ElementId|RangeId,kind:Element|Range,documentId:ElementId|null,treeNodeId:ElementId|null,label:string}; treeNodeId exists only for matching actual tree nodes; range operands must have equal documentId") },
			{ L"ProviderSection", String(L"{patternId:integer,provider:string,client:string,readouts:Property[],commands:ActionSchema[],references:Reference[],documentId:ElementId|null}") },
			{ L"ActionSchema", String(L"{command:string,name:string,enabled:boolean|null,reason:string|null,mutation:boolean,pureGetter:boolean,parameters:Parameter[],result:string,effects:string,example:string}") },
			{ L"Parameter", String(L"{name:string,type:string|integer|number|choice|element|range|variant|attributeIds,nullable:boolean,multiline:boolean,minimum:number,maximum:number,choices:{value:integer,name:string}[]}; all parameters required; no extra or duplicate fields") },
			{ L"Preview", String(L"{status:Ready|Unavailable|GeometryChanged|Canceled,bounds:Bounds|null,dpi:integer|null,width:integer,height:integer,timestamp:string|null,mediaType:image/bmp,base64:string|null}") },
			{ L"Descriptor", String(L"{id:integer,name:string,vartype:integer,elementArray:boolean}; SDK catalog type for Variant inputs") },
			{ L"Bounds", String(L"{left:integer,top:integer,right:integer,bottom:integer} in physical screen coordinates") },
			{ L"CommandSchema", String(L"{command:string,target:string,arguments:string,result:string,effects:string,example:string}") }
		});
		return JsonMap({ { L"commands", result }, { L"schemas", schemas }, { L"syntax", String(L"Verb-Target [ID] [JSON arguments object]; exact case; UTF-8 JSON lines; blank lines ignored; one flushed response after completion.") }, { L"identity", String(L"Opaque IDs belong to this CLI session. Window IDs survive discovery only while the same window survives. Element/range IDs expire on refresh, mutation or window switch; no stale ID retargets another object.") } });
	}
}

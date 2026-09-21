#include "ActionViewModel.h"
#include "TextRangeViewModel.h"
#include "UiaCatalog.Actions.Windows.h"
#include <cmath>

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection::description;

namespace uialist
{
	WString CompactValue(const WString& text)
	{
		vint length = (std::min)(text.Length(), static_cast<vint>(200));
		for (vint i = 0; i < length; i++) if (!text[i] || text[i] == L'\r' || text[i] == L'\n') { length = i; break; }
		return length < text.Length() ? text.Left(length) + L"… (" + itow(text.Length()) + L")" : text;
	}

	vint ReferenceViewModel::GetKey() { return data.key; }
	WString ReferenceViewModel::GetLabel() { return data.label; }
	bool ReferenceViewModel::GetIsRange() { return data.kind == native::ValueKind::Range; }
	bool ReferenceViewModel::GetCanInspect() { return owner->open && !owner->busy && !owner->owner->treeBusy && generation == owner->generation; }
	bool ReferenceViewModel::GetCanReveal() { return GetCanInspect() && !GetIsRange() && owner->owner->nodes.Keys().Contains(data.key); }
	WString ReferenceViewModel::GetStatus() { return GetIsRange() || GetCanReveal() ? WString() : owner->owner->strings->OutsideTree(); }
	void ReferenceViewModel::Inspect() { if (GetCanInspect()) owner->InspectReference(data); }
	void ReferenceViewModel::Reveal()
	{
		if (!GetCanReveal()) return;
		auto root = owner->owner; auto node = root->nodes[data.key]; auto parent = node->data.parent;
		while (parent) { auto p = root->nodes[parent]; p->SetIsExpanded(true); parent = p->data.parent; }
		root->SelectNode(node); root->SetActiveTab(2); owner->Close();
	}

	WString ActionParameterViewModel::GetLabel() { return label; }
	vint ActionParameterViewModel::GetKind()
	{
		using native::ArgumentKind;
		if (spec->kind == ArgumentKind::Variant && variantElements) return 2;
		return spec->kind == ArgumentKind::Choice || spec->kind == ArgumentKind::Element || spec->kind == ArgumentKind::Range || (spec->kind == ArgumentKind::Variant && (variantType == VT_BOOL || variantType == VT_UNKNOWN)) ? 1 : 0;
	}
	WString ActionParameterViewModel::GetDraftText() { return draft; }
	void ActionParameterViewModel::SetDraftText(const WString& value)
	{
		if (owner->owner->busy || draft == value) return;
		draft = value; owner->Validate(); DraftTextChanged();
	}
	Ptr<IValueList> ActionParameterViewModel::GetChoices() { return UnboxValue<Ptr<IValueList>>(BoxParameter(choices)); }
	vint ActionParameterViewModel::GetChoiceIndex()
	{
		if (spec->kind == native::ArgumentKind::Element || spec->kind == native::ArgumentKind::Range || (spec->kind == native::ArgumentKind::Variant && variantType == VT_UNKNOWN)) return choiceValues.IndexOf(reference);
		bool valid = false; auto value = wtoi64_test(draft, valid); return valid ? choiceValues.IndexOf(static_cast<vint>(value)) : -1;
	}
	void ActionParameterViewModel::SetChoiceIndex(vint value)
	{
		if (owner->owner->busy || value < 0 || value >= choiceValues.Count()) return;
		if (value == GetChoiceIndex()) return;
		reference = choiceValues[value]; draft = itow(reference); owner->Validate(); DraftTextChanged(); ChoiceIndexChanged(); Commit();
	}
	bool ActionParameterViewModel::GetIsMultiline() { return spec->multiline || (spec->kind == native::ArgumentKind::Variant && (variantType == VT_BSTR || variantType == (VT_ARRAY | VT_BSTR))); }
	WString ActionParameterViewModel::GetSelectedReferences() { return selectedReferenceLabels; }
	void ActionParameterViewModel::UpdateSelectedReferences()
	{
		WString result;
		for (auto key : selectedReferences) for (auto&& value : owner->owner->referenceData) if (value.key == key)
			result += (result.Length() ? L"\r\n" : L"") + value.label;
		selectedReferenceLabels = result.Length() ? result : L"[]";
		SelectedReferencesChanged();
	}
	void ActionParameterViewModel::AddReference()
	{
		if (owner->owner->busy || !reference || !choiceValues.Contains(reference)) return;
		selectedReferences.Add(reference); UpdateSelectedReferences(); owner->Validate();
	}
	void ActionParameterViewModel::ClearReferences()
	{
		if (owner->owner->busy) return;
		selectedReferences.Clear(); UpdateSelectedReferences(); owner->Validate();
	}
	WString ActionParameterViewModel::GetValidationMessage() { return validation; }
	void ActionParameterViewModel::EditText()
	{
		auto dialog = owner->owner;
		if (dialog->busy || !dialog->open || (dialog->textDialog && dialog->textDialog->open)) return;
		auto text = Ptr(new TextDialogViewModel); text->title = spec->name; text->original = text->draft = draft; text->readOnly = false;
		auto parameter = Ptr(this);
		text->accept = [parameter, dialog](const WString& value) { parameter->SetDraftText(value); parameter->Commit(); dialog->textDialog->Cancel(); };
		dialog->textDialog = text; dialog->TextDialogChanged();
	}
	void ActionParameterViewModel::UpdateChoices()
	{
		label = spec->name;
		if (spec->kind == native::ArgumentKind::Number || spec->kind == native::ArgumentKind::Integer) label += L" [" + ftow(spec->minimum) + L".." + ftow(spec->maximum) + L"]";
		if (spec->kind == native::ArgumentKind::Variant) label += L" (VT=" + itow(variantType) + L")";
		choices.Clear(); choiceValues.Clear();
		using native::ArgumentKind;
		if (spec->kind == ArgumentKind::Choice) for (auto&& choice : spec->choices) { choices.Add(choice.label); choiceValues.Add(choice.id); }
		else if (spec->kind == ArgumentKind::Variant && variantType == VT_BOOL) { choices.Add(L"false"); choices.Add(L"true"); choiceValues.Add(0); choiceValues.Add(1); if (!draft.Length()) draft = L"0"; }
		else if (GetKind() >= 1)
		{
			if (spec->nullable || spec->kind == ArgumentKind::Variant) { choices.Add(owner->owner->owner->strings->Null()); choiceValues.Add(0); }
			vint document = 0;
			for (auto&& value : owner->owner->referenceData) if (value.kind == native::ValueKind::Range && value.key == owner->spec->rangeKey) document = value.documentKey;
			bool wantRange = spec->kind == ArgumentKind::Range;
			if (spec->kind == ArgumentKind::Variant && owner->spec->code == native::ActionCode::RangeFindAttribute)
			{
				auto attribute = owner->parameters[0].Cast<ActionParameterViewModel>();
				wantRange = wtoi(attribute->draft) == UIA_LinkAttributeId;
			}
			for (auto&& value : owner->owner->referenceData)
			{
				if ((value.kind == native::ValueKind::Range) != wantRange) continue;
				if (wantRange && document && value.documentKey != document) continue;
				choices.Add(value.label); choiceValues.Add(value.key);
			}
		}
		UpdateSelectedReferences();
		ChoicesChanged(); ChoiceIndexChanged(); KindChanged(); LabelChanged();
	}
	void ActionParameterViewModel::Commit()
	{
		if (owner->spec->pureGetter) owner->Query();
	}
	bool ActionParameterViewModel::Parse(native::ActionArgument& result)
	{
		using namespace native;
		if (spec->kind == ArgumentKind::Variant)
		{
			bool valid = true;
			if (owner->spec->code == ActionCode::FindItem && wtoi(owner->parameters[1].Cast<ActionParameterViewModel>()->draft) == 0) result.value = Ptr(new ValueData);
			else if (variantType == VT_BSTR) { result.value = StringValue(draft); for (vint i = 0; i < draft.Length(); i++) if (!draft[i]) valid = false; }
			else if (variantType == VT_BOOL) { auto n = wtoi_test(draft, valid); valid = valid && (n == 0 || n == 1); result.value = BooleanValue(n != 0); }
			else if (variantType == VT_I4) { auto n = wtoi64_test(draft, valid); valid = valid && n >= INT_MIN && n <= INT_MAX; result.value = IntegerValue(n); }
			else if (variantType == VT_R8) { auto n = wtof_test(draft, valid); valid = valid && std::isfinite(n); result.value = NumberValue(n); }
			else if (variantElements)
			{
				result.value = Ptr(new ValueData); result.value->type = VT_UNKNOWN; result.value->kind = ValueKind::Array;
				for (auto key : selectedReferences)
				{
					if (!choiceValues.Contains(key)) valid = false;
					auto item = Ptr(new ValueData); item->type = VT_UNKNOWN; item->kind = ValueKind::Element; item->key = key; result.value->items.Add(item);
				}
			}
			else if (variantType == VT_UNKNOWN)
			{
				result.value = Ptr(new ValueData); result.value->type = VT_UNKNOWN; result.value->kind = ValueKind::Element; result.value->key = reference;
				for (auto&& r : owner->owner->referenceData) if (r.key == reference) result.value->kind = r.kind;
				valid = choiceValues.Contains(reference);
			}
			else if (variantType & VT_ARRAY)
			{
				result.value = Ptr(new ValueData); result.value->kind = ValueKind::Array; result.value->type = variantType;
				vint begin = 0;
				auto type = variantType & VT_TYPEMASK;
				if (draft.Length()) for (vint i = 0; i <= draft.Length(); i++) if (i == draft.Length() || draft[i] == (type == VT_BSTR ? L'\n' : L','))
				{
					auto part = draft.Sub(begin, i - begin); begin = i + 1;
					if (type == VT_I4) { auto n = wtoi64_test(part, valid); valid = valid && n >= INT_MIN && n <= INT_MAX; result.value->items.Add(IntegerValue(n)); }
					else if (type == VT_R8) { auto n = wtof_test(part, valid); valid = valid && std::isfinite(n); result.value->items.Add(NumberValue(n)); }
					else if (type == VT_BSTR)
					{
						if (part.Length() && part[part.Length() - 1] == L'\r') part = part.Left(part.Length() - 1);
						for (vint j = 0; j < part.Length(); j++) if (!part[j]) valid = false;
						result.value->items.Add(StringValue(part));
					}
					else valid = false;
					if (!valid) break;
				}
				result.value->dimensions.Add({ 0, static_cast<LONG>(result.value->items.Count()) - 1 });
			}
			else valid = false;
			if (!valid) return false;
		}
		return native::ParseArgument(*spec.Obj(), draft, reference, result);
	}

	WString ActionResultViewModel::GetOperation() { return operation; }
	WString ActionResultViewModel::GetDisplayValue() { return display; }
	Ptr<IValueList> ActionResultViewModel::GetReferences() { return UnboxValue<Ptr<IValueList>>(BoxParameter(references)); }
	void ActionResultViewModel::OpenDetails() { owner->OpenDetails(operation, details); }

	WString ActionCommandViewModel::GetKey() { return key; }
	WString ActionCommandViewModel::GetLabel() { return spec->name; }
	Ptr<IValueList> ActionCommandViewModel::GetParameters() { return UnboxValue<Ptr<IValueList>>(BoxParameter(parameters)); }
	bool ActionCommandViewModel::GetCanExecute()
	{
		if (!owner->open || owner->busy || owner->owner->treeBusy || generation != owner->generation || !spec->enabled) return false;
		for (auto&& value : parameters) if (!value.Cast<ActionParameterViewModel>()->argumentValid) return false;
		return true;
	}
	bool ActionCommandViewModel::GetIsBusy() { return executing; }
	bool ActionCommandViewModel::GetIsGetter() { return spec->pureGetter; }
	WString ActionCommandViewModel::GetStatus() { return executing ? owner->owner->strings->Loading() : status; }
	void ActionCommandViewModel::Validate(bool invalidateQuery)
	{
		if (invalidateQuery)
		{
			querySerial++;
			if (spec->pureGetter) executing = false;
		}
		for (auto&& item : parameters)
		{
			auto p = item.Cast<ActionParameterViewModel>();
			if (p->spec->kind == native::ArgumentKind::Variant)
			{
				auto descriptorParameter = parameters[spec->code == native::ActionCode::FindItem ? 1 : 0].Cast<ActionParameterViewModel>();
				auto id = wtoi(descriptorParameter->draft);
				auto catalog = spec->code == native::ActionCode::FindItem ? native::PropertyCatalog : native::AttributeCatalog;
				auto count = spec->code == native::ActionCode::FindItem ? native::PropertyCatalogCount : native::AttributeCatalogCount;
				VARTYPE type = VT_EMPTY; bool elements = false;
				for (vint i = 0; i < count; i++) if (catalog[i].id == id) { type = catalog[i].expectedType; elements = catalog[i].elementArray; }
				if (type != p->variantType || elements != p->variantElements)
				{
					p->variantType = type; p->variantElements = elements; p->selectedReferences.Clear();
					p->draft = {}; p->reference = 0; p->DraftTextChanged(); p->IsMultilineChanged(); p->UpdateChoices();
				}
			}
			native::ActionArgument value;
			p->argumentValid = p->Parse(value);
			auto message = p->argumentValid ? WString() : owner->owner->strings->InvalidValue();
			if (message != p->validation) { p->validation = message; p->ValidationMessageChanged(); }
		}
		CanExecuteChanged();
	}

	void ActionCommandViewModel::Query()
	{
		Validate();
		if (!spec->pureGetter || !GetCanExecute()) return;
		auto arguments = Ptr(new List<native::ActionArgument>);
		for (auto&& value : parameters)
		{
			native::ActionArgument argument;
			CHECK_ERROR(value.Cast<ActionParameterViewModel>()->Parse(argument), L"Validated getter argument changed during acceptance");
			arguments->Add(std::move(argument));
		}
		executing = true;
		StatusChanged();
		auto command = Ptr(this);
		auto gate = owner->owner->lifetime;
		auto epoch = generation, request = owner->serial, query = querySerial, key = owner->nodeKey;
		auto descriptor = spec;
		auto worker = &owner->owner->worker;
		owner->owner->QueueNative(spec->name + L"; element=" + itow(key), [command, gate, epoch, request, query, key, descriptor, arguments, worker]()
		{
			auto outcome = Ptr(new native::ActionOutcome);
			try
			{
				*outcome.Obj() = worker->session->Execute(key, *descriptor.Obj(), *arguments.Obj());
			}
			catch (const native::UiaFailure& error)
			{
				if (!error.IsExpected()) throw;
				outcome->available = false;
				outcome->value = native::StringValue(error.Message());
			}
			UiaListViewModel::Post(gate, [command, outcome, epoch, request, query](UiaListViewModel& root)
			{
				if (root.lifetime->generation != epoch || !root.propertyDialog || !root.propertyDialog->open || root.propertyDialog->serial != request || command->querySerial != query) return;
				command->executing = false;
				command->status = FormatValue(*outcome->value.Obj(), *root.strings.Obj());
				command->StatusChanged();
				root.propertyDialog->PublishReferences(outcome->references);
			});
		});
	}

	void ActionCommandViewModel::Execute()
	{
		Validate(); if (!GetCanExecute()) return;
		auto command = Ptr(this);
		auto arguments = Ptr(new List<native::ActionArgument>);
		for (auto&& value : parameters) { native::ActionArgument argument; CHECK_ERROR(value.Cast<ActionParameterViewModel>()->Parse(argument), L"Validated argument changed during acceptance"); arguments->Add(std::move(argument)); }
		executing = true; owner->busy = true; owner->NotifyAvailability(); IsBusyChanged(); StatusChanged();
		auto gate = owner->owner->lifetime;
		auto epoch = generation, request = owner->serial, key = owner->nodeKey;
		auto descriptor = spec; auto worker = &owner->owner->worker;
		owner->owner->QueueNative(spec->name + L"; element=" + itow(key), [command, gate, epoch, request, key, descriptor, arguments, worker]()
		{
			// FIFO retains the original worker session until every accepted operation ahead of a selection runs.
			auto outcome = Ptr(new native::ActionOutcome);
			WString failure;
			try { *outcome.Obj() = worker->session->Execute(key, *descriptor.Obj(), *arguments.Obj()); }
			catch (const native::UiaFailure& error)
			{
				if (!error.IsExpected()) throw;
				failure = error.Message();
			}
			Ptr<native::InspectionSnapshot> inspection;
			Ptr<native::ActionSectionData> range;
			if (failure.Length() == 0 && !outcome->closedWindow)
			{
				if (outcome->mutation) worker->session->DiscardRanges();
				try
				{
					inspection = worker->session->Inspect(key);
				}
				catch (const native::UiaFailure& error)
				{
					if (!error.IsUnavailable()) throw;
					outcome->closedWindow = true;
				}
				if (!outcome->closedWindow && descriptor->rangeKey && !outcome->mutation) range = worker->session->DescribeRange(descriptor->rangeKey);
			}
			UiaListViewModel::Post(gate, [command, failure, outcome, inspection, range, epoch, request, descriptor](UiaListViewModel& root)
			{
				if (root.lifetime->generation != epoch || !root.propertyDialog || !root.propertyDialog->open || root.propertyDialog->serial != request) return;
				auto dialog = root.propertyDialog;
				command->executing = false;
				if (failure.Length())
				{
					command->status = failure;
					command->StatusChanged(); command->IsBusyChanged();
					dialog->busy = false; dialog->status = failure;
					dialog->StatusChanged(); dialog->NotifyAvailability();
					return;
				}
				if (outcome->closedWindow)
				{
					dialog->Close(); if (!native::IsCurrentWindow(root.selectedWindow->window.identity)) root.ClearSelection(); else root.RefreshWindow(); return;
				}
				dialog->status = outcome->available ? root.strings->Completed() : root.strings->CapabilityChanged();
				if (outcome->mutation) dialog->textRanges.Clear();
				dialog->Publish(inspection);
				auto result = Ptr(new ActionResultViewModel); result->owner = dialog.Obj(); result->operation = descriptor->name;
				result->details = outcome->value->kind == native::ValueKind::Null && descriptor->mutation ? dialog->status : FormatValue(*outcome->value.Obj(), *root.strings.Obj());
				result->display = CompactValue(result->details);
				result->details = L"VARTYPE=" + itow(outcome->value->type) + L"\r\n" + result->details;
				List<Ptr<native::ValueData>> pending; pending.Add(outcome->value);
				for (vint i = 0; i < pending.Count(); i++)
				{
					auto value = pending[i]; for (auto&& child : value->items) pending.Add(child);
					if (value->kind == native::ValueKind::Element || value->kind == native::ValueKind::Range)
					{
						for (auto&& data : outcome->references) if (data.kind == value->kind && data.key == value->key)
						{
							auto reference = Ptr(new ReferenceViewModel); reference->owner = dialog.Obj(); reference->data = data; reference->generation = epoch; result->references.Add(reference); break;
						}
					}
				}
				dialog->results.Add(result);
				if (range) for (auto&& item : dialog->textRanges)
				{
					auto vm = item.Cast<TextRangeViewModel>(); if (vm->key == range->rangeKey) vm->SetSection(Ptr(new ActionSectionViewModel(*dialog.Obj(), range)));
				}
				if (outcome->mutation) root.RefreshWindowInternal(false);
			});
		});
	}

	ActionSectionViewModel::ActionSectionViewModel(PropertyDialogViewModel& dialog, Ptr<native::ActionSectionData> value)
		: data(value)
	{
		for (auto&& readout : data->readouts) readouts.Add(dialog.CreateRow(readout));
		for (auto&& command : data->commands)
		{
			if (command->pureGetter && command->parameters.Count() == 0) continue;
			auto vm = Ptr(new ActionCommandViewModel); vm->owner = &dialog; vm->spec = command; vm->generation = dialog.generation;
			vm->key = itow(command->pattern) + L"/" + itow(command->rangeKey) + L"/" + itow(static_cast<vint>(command->code));
			for (auto&& argument : command->parameters)
			{
				auto p = Ptr(new ActionParameterViewModel); p->owner = vm.Obj(); p->spec = argument; p->draft = argument->initial; vm->parameters.Add(p);
				p->UpdateChoices();
			}
			vm->Validate(); commands.Add(vm);
		}
	}
	WString ActionSectionViewModel::GetHeading() { return data->name; }
	vint ActionSectionViewModel::GetPatternId() { return data->pattern; }
	Ptr<IValueList> ActionSectionViewModel::GetReadouts() { return UnboxValue<Ptr<IValueList>>(BoxParameter(readouts)); }
	Ptr<IValueList> ActionSectionViewModel::GetCommands() { return UnboxValue<Ptr<IValueList>>(BoxParameter(commands)); }
}

#include "PropertyViewModel.h"
#include "ActionViewModel.h"
#include "TextRangeViewModel.h"
#include <cmath>

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection::description;

namespace uialist
{
	WString FormatValue(const native::ValueData& value, IStringsStrings& strings)
	{
		using native::ValueKind;
		switch (value.kind)
		{
		case ValueKind::Null: return strings.Null() + L" (VT=" + itow(value.type) + L")";
		case ValueKind::Unsupported: return value.text.Length() ? strings.NotSupported() + L": " + value.text : strings.NotSupported();
		case ValueKind::Mixed: return strings.Mixed();
		case ValueKind::Boolean: return value.signedValue ? L"true" : L"false";
		case ValueKind::Signed: return i64tow(value.signedValue);
		case ValueKind::Unsigned: return u64tow(value.unsignedValue);
		case ValueKind::Real: return ftow(value.realValue);
		case ValueKind::String:
			{
				if (!value.text.Length()) return L"BSTR (UTF-16=0) \"\"";
				bool embeddedNull = false;
				for (vint i = 0; i < value.text.Length(); i++) if (!value.text[i]) embeddedNull = true;
				if (!embeddedNull) return value.text;
				WString result = L"BSTR (UTF-16=" + itow(value.text.Length()) + L")\r\n\"";
				for (vint i = 0; i < value.text.Length(); i++)
				{
					switch (value.text[i])
					{
					case 0: result += L"\\0"; break;
					case L'\r': result += L"\\r"; break;
					case L'\n': result += L"\\n"; break;
					case L'\t': result += L"\\t"; break;
					case L'\\': result += L"\\\\"; break;
					case L'\"': result += L"\\\""; break;
					default: result += value.text.Sub(i, 1);
					}
				}
				return result + L"\"";
			}
		case ValueKind::Element: return L"IUIAutomationElement #" + itow(value.key);
		case ValueKind::Range: return L"IUIAutomationTextRange #" + itow(value.key);
		case ValueKind::Opaque: return value.text + L" #" + itow(value.key);
		case ValueKind::Array:
			{
				WString result = (value.text.Length() ? value.text : WString(L"SAFEARRAY")) + L" (VT=" + itow(value.type) + L", " + itow(value.items.Count()) + L")";
				for (auto&& dimension : value.dimensions) result += L" [" + itow(dimension.key) + L".." + itow(dimension.value) + L"]";
				for (vint i = 0; i < value.items.Count(); i++) result += L"\r\n[" + itow(i) + L"] " + FormatValue(*value.items[i].Obj(), strings);
				return result;
			}
		default: CHECK_FAIL(L"Unrecognized UIA value kind.");
		}
	}

	WString TextDialogViewModel::GetTitle() { return title; }
	WString TextDialogViewModel::GetOriginalText() { return original; }
	WString TextDialogViewModel::GetDraftText() { return draft; }
	void TextDialogViewModel::SetDraftText(const WString& value)
	{
		if (!readOnly && !busy && draft != value)
		{
			draft = value; validation = {};
			DraftTextChanged(); ValidationMessageChanged(); CanAcceptChanged();
		}
	}
	bool TextDialogViewModel::GetIsReadOnly() { return readOnly; }
	bool TextDialogViewModel::GetCanAccept() { return !readOnly && open && !busy && validation.Length() == 0; }
	WString TextDialogViewModel::GetValidationMessage() { return validation; }
	bool TextDialogViewModel::GetIsOpen() { return open; }
	void TextDialogViewModel::Accept() { if (GetCanAccept() && accept) accept(draft); }
	void TextDialogViewModel::Cancel() { if (open) { open = false; accept = {}; IsOpenChanged(); } }

	WString PropertyRowViewModel::GetKey() { return key; }
	WString PropertyRowViewModel::GetLabel() { return data.name; }
	WString PropertyRowViewModel::GetDisplayValue() { return display; }
	bool PropertyRowViewModel::GetCanEdit() { return owner->open && !owner->busy && !owner->owner->treeBusy && data.setter && data.setter->kind != native::SetterKind::None; }
	bool PropertyRowViewModel::GetIsMultiline() { return data.setter && data.setter->multiline; }
	bool PropertyRowViewModel::GetIsSelected() { return selected; }
	bool PropertyRowViewModel::GetIsEditing() { return editing; }
	vint PropertyRowViewModel::GetEditorKind() { return data.setter ? static_cast<vint>(data.setter->kind) : 0; }
	Ptr<IValueList> PropertyRowViewModel::GetChoices() { return UnboxValue<Ptr<IValueList>>(BoxParameter(choices)); }
	vint PropertyRowViewModel::GetChoiceIndex()
	{
		return choiceValues.IndexOf(draft);
	}
	void PropertyRowViewModel::SetChoiceIndex(vint value)
	{
		if (editing && data.setter && value >= 0 && value < data.setter->choices.Count()) SetDraftText(itow(data.setter->choices[value].id));
	}
	WString PropertyRowViewModel::GetDraftText() { return draft; }
	void PropertyRowViewModel::SetDraftText(const WString& value) { if (editing && !owner->busy && draft != value) { draft = value; DraftTextChanged(); ChoiceIndexChanged(); Validate(); } }
	WString PropertyRowViewModel::GetValidationMessage() { return validation; }
	void PropertyRowViewModel::BeginEdit() { owner->BeginEdit(Ptr(this)); }
	void PropertyRowViewModel::Commit() { owner->CommitEdit(Ptr(this)); }
	void PropertyRowViewModel::Cancel() { owner->CancelEdit(Ptr(this)); }
	void PropertyRowViewModel::OpenDetails() { owner->OpenText(Ptr(this)); }
	bool PropertyRowViewModel::Validate()
	{
		bool valid = true;
		if (!data.setter || data.setter->kind == native::SetterKind::None) valid = false;
		else if (data.setter->kind == native::SetterKind::Number)
		{
			auto value = wtof_test(draft, valid);
			valid = valid && std::isfinite(value) && value >= data.setter->minimum && value <= data.setter->maximum;
		}
		else if (data.setter->kind == native::SetterKind::Choice) valid = GetChoiceIndex() >= 0;
		else if (data.setter->kind == native::SetterKind::Text) for (vint i = 0; i < draft.Length(); i++) if (draft[i] == 0) valid = false;
		validation = valid ? WString() : owner->owner->strings->InvalidValue();
		ValidationMessageChanged();
		owner->StatusChanged();
		return valid;
	}

	PropertyDialogViewModel::PropertyDialogViewModel(UiaListViewModel& root, vint key, const WString& name)
		: owner(&root), nodeKey(key), generation(root.lifetime->generation), serial(++root.lifetime->dialogSerial), title(name)
	{
	}
	WString PropertyDialogViewModel::GetTitle() { return title; }
	Ptr<IValueList> PropertyDialogViewModel::GetRows() { return UnboxValue<Ptr<IValueList>>(BoxParameter(rows)); }
	Ptr<IValueList> PropertyDialogViewModel::GetSections() { return UnboxValue<Ptr<IValueList>>(BoxParameter(sections)); }
	Ptr<IValueList> PropertyDialogViewModel::GetReferences() { return UnboxValue<Ptr<IValueList>>(BoxParameter(references)); }
	Ptr<IValueList> PropertyDialogViewModel::GetResults() { return UnboxValue<Ptr<IValueList>>(BoxParameter(results)); }
	Ptr<IValueList> PropertyDialogViewModel::GetTextRanges() { return UnboxValue<Ptr<IValueList>>(BoxParameter(textRanges)); }
	Ptr<vm::ITextDialogViewModel> PropertyDialogViewModel::GetTextDialog() { return textDialog; }
	bool PropertyDialogViewModel::GetIsBusy() { return busy; }
	WString PropertyDialogViewModel::GetStatus() { return busy ? owner->strings->Loading() : selected && selected->validation.Length() ? selected->validation : status.Length() ? status : owner->strings->Ready(); }
	bool PropertyDialogViewModel::GetIsOpen() { return open; }
	vint PropertyDialogViewModel::GetSelectedRowIndex() { return selected ? rows.IndexOf(selected.Obj()) : -1; }

	void PropertyDialogViewModel::Refresh()
	{
		if (!open || busy) return;
		busy = true;
		IsBusyChanged();
		StatusChanged();
		auto gate = owner->lifetime;
		auto request = serial;
		auto epoch = generation;
		auto key = nodeKey;
		auto worker = &owner->worker;
		owner->QueueNative(L"Read properties; element=" + itow(key), [worker, gate, request, epoch, key]()
		{
			if (gate->closed || gate->generation != epoch || gate->dialogSerial != request) return;
			auto result = worker->session->Inspect(key);
			UiaListViewModel::Post(gate, [result, request, epoch](UiaListViewModel& root)
			{
				if (root.lifetime->generation == epoch && root.propertyDialog && root.propertyDialog->open && root.propertyDialog->serial == request)
					root.propertyDialog->Publish(result);
			});
		});
	}

	void PropertyDialogViewModel::Publish(Ptr<native::InspectionSnapshot> result)
	{
		auto selectedKey = pendingKey.Length() ? pendingKey : selected ? selected->GetKey() : WString();
		pendingKey = {};
		rows.Clear();
		selected = nullptr;
		for (auto&& property : result->properties)
		{
			if (property.value->kind == native::ValueKind::Unsupported) continue;
			auto row = CreateRow(property);
			rows.Add(row);
			if (row->GetKey() == selectedKey) { selected = row; row->selected = true; }
		}
		PublishReferences(result->references);
		sections.Clear();
		for (auto&& section : result->sections) sections.Add(Ptr(new ActionSectionViewModel(*this, section)));
		busy = false;
		NotifyAvailability();
		SelectedRowIndexChanged();
	}

	Ptr<PropertyRowViewModel> PropertyDialogViewModel::CreateRow(const native::PropertyData& property)
	{
		auto row = Ptr(new PropertyRowViewModel); row->owner = this; row->data = property;
		row->key = property.id ? itow(property.id) : itow(property.sourcePattern) + L"/" + property.name;
		row->details = FormatValue(*property.value.Obj(), *owner->strings.Obj()); row->display = CompactValue(row->details);
		if (property.value->kind == native::ValueKind::Signed)
		{
			auto field = property.name;
			for (vint i = 0; property.id && i < native::PropertyCatalogCount; i++) if (native::PropertyCatalog[i].id == property.id) field = native::PropertyCatalog[i].name;
			auto name = native::EnumerationName(field, static_cast<LONG>(property.value->signedValue));
			if (name.Length()) row->details = row->display = name;
		}
		if (property.id == UIA_ControlTypePropertyId && property.value->kind == native::ValueKind::Signed) row->display = native::IdName(native::ControlTypeCatalog, native::ControlTypeCatalogCount, static_cast<LONG>(property.value->signedValue));
		if (property.setter) for (auto&& choice : property.setter->choices) { row->choices.Add(choice.label); row->choiceValues.Add(itow(choice.id)); }
		row->details = L"VARTYPE=" + itow(property.value->type) + L"\r\n" + row->details;
		return row;
	}
	void PropertyDialogViewModel::PublishReferences(const List<native::ReferenceData>& value)
	{
		CopyFrom(referenceData, value); references.Clear();
		for (auto&& data : value)
		{
			auto reference = Ptr(new ReferenceViewModel); reference->owner = this; reference->data = data; reference->generation = generation; references.Add(reference);
		}
	}
	void PropertyDialogViewModel::NotifyAvailability()
	{
		for (auto&& row : rows) row->CanEditChanged();
		auto notify = [&](Ptr<ActionSectionViewModel> section)
		{
			for (auto&& command : section->commands)
			{
				auto value = command.Cast<ActionCommandViewModel>(); value->generation = generation;
				for (auto&& parameter : value->parameters) parameter.Cast<ActionParameterViewModel>()->UpdateChoices();
				value->Validate(false);
			}
		};
		for (auto&& section : sections) notify(section.Cast<ActionSectionViewModel>());
		for (auto&& range : textRanges) notify(range.Cast<TextRangeViewModel>()->section);
		for (auto&& reference : references) { reference->CanInspectChanged(); reference->CanRevealChanged(); }
		IsBusyChanged(); StatusChanged();
	}
	void PropertyDialogViewModel::OpenDetails(const WString& name, const WString& value)
	{
		if (!open || busy || (textDialog && textDialog->open)) return;
		textDialog = Ptr(new TextDialogViewModel); textDialog->title = name; textDialog->original = textDialog->draft = value; TextDialogChanged();
	}
	void PropertyDialogViewModel::InspectReference(const native::ReferenceData& reference)
	{
		if (!open || busy || owner->treeBusy) return;
		if (reference.kind == native::ValueKind::Range) { OpenRange(reference.key); return; }
		auto root = owner; auto key = reference.key; auto name = reference.label; Close();
		root->propertyDialog = Ptr(new PropertyDialogViewModel(*root, key, name));
		root->PropertyDialogChanged(); root->propertyDialog->Refresh();
	}

	bool PropertyDialogViewModel::SelectRow(Ptr<vm::IPropertyRowViewModel> value)
	{
		if (busy) return false;
		auto row = value.Cast<PropertyRowViewModel>();
		if (row == selected) return true;
		if (selected && selected->editing)
		{
			if (!selected->Validate()) return false;
			pendingKey = row ? row->GetKey() : WString();
			CommitEdit(selected);
			return false;
		}
		if (selected) { selected->selected = false; selected->IsSelectedChanged(); }
		selected = row;
		if (selected) { selected->selected = true; selected->IsSelectedChanged(); }
		SelectedRowIndexChanged();
		return true;
	}
	void PropertyDialogViewModel::BeginEdit(Ptr<vm::IPropertyRowViewModel> value)
	{
		auto row = value.Cast<PropertyRowViewModel>();
		if (!row || !row->GetCanEdit() || !SelectRow(row) || row->editing) return;
		if (row->GetIsMultiline()) return;
		row->draft = row->data.value->kind == native::ValueKind::String ? row->data.value->text : FormatValue(*row->data.value.Obj(), *owner->strings.Obj());
		row->editing = true;
		row->validation = {};
		row->DraftTextChanged(); row->ChoiceIndexChanged(); row->ValidationMessageChanged(); row->IsEditingChanged();
	}
	void PropertyDialogViewModel::CommitEdit(Ptr<vm::IPropertyRowViewModel> value)
	{
		auto row = value.Cast<PropertyRowViewModel>();
		if (!open || busy || !row || !row->editing || !row->Validate()) return;
		busy = true;
		status = {};
		IsBusyChanged(); StatusChanged();
		for (auto&& item : rows) item->CanEditChanged();
		if (textDialog) { textDialog->busy = true; textDialog->CanAcceptChanged(); }
		auto gate = owner->lifetime;
		auto request = serial, epoch = generation, key = nodeKey;
		auto property = row->data.id;
		auto draft = row->draft;
		auto worker = &owner->worker;
		owner->QueueNative(L"Set property " + itow(property) + L"; element=" + itow(key), [worker, gate, request, epoch, key, property, draft]()
		{
			// Once queued, the command belongs to the original session and must run once.
			auto written = worker->session->SetProperty(key, property, draft);
			auto result = worker->session->Inspect(key);
			UiaListViewModel::Post(gate, [written, result, request, epoch](UiaListViewModel& root)
			{
				if (root.lifetime->generation != epoch || !root.propertyDialog || !root.propertyDialog->open || root.propertyDialog->serial != request) return;
				auto dialog = root.propertyDialog;
				if (dialog->textDialog) dialog->textDialog->Cancel();
				dialog->status = written ? WString() : root.strings->CapabilityChanged();
				dialog->Publish(result);
				if (written) root.RefreshWindowInternal(false);
			});
		});
	}
	void PropertyDialogViewModel::CancelEdit(Ptr<vm::IPropertyRowViewModel> row)
	{
		auto value = row.Cast<PropertyRowViewModel>();
		if (value && !busy)
		{
			value->editing = false;
			value->validation = {};
			value->IsEditingChanged(); value->ValidationMessageChanged(); StatusChanged();
		}
	}
	void PropertyDialogViewModel::OpenText(Ptr<vm::IPropertyRowViewModel> value)
	{
		auto row = value.Cast<PropertyRowViewModel>();
		if (!open || busy || !row || (textDialog && textDialog->open)) return;
		textDialog = Ptr(new TextDialogViewModel);
		textDialog->title = row->data.name;
		textDialog->original = textDialog->draft = row->details;
		textDialog->readOnly = !row->GetCanEdit() || row->data.setter->kind != native::SetterKind::Text;
		if (!textDialog->readOnly && row->data.value->kind == native::ValueKind::String) textDialog->original = textDialog->draft = row->data.value->text;
		if (!textDialog->readOnly)
		{
			textDialog->accept = [this, row](const WString& value)
			{
				row->draft = value;
				row->editing = true;
				if (!row->Validate())
				{
					textDialog->validation = row->validation;
					textDialog->ValidationMessageChanged(); textDialog->CanAcceptChanged();
					return;
				}
				CommitEdit(row);
			};
		}
		TextDialogChanged();
	}
	void PropertyDialogViewModel::Close()
	{
		if (!open) return;
		open = false;
		if (textDialog) textDialog->Cancel();
		IsOpenChanged();
	}
}

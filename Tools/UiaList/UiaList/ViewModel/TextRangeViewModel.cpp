#include "TextRangeViewModel.h"

using namespace vl;
using namespace vl::reflection::description;

namespace uialist
{
	WString TextRangeViewModel::GetLabel() { return section->GetHeading(); }
	Ptr<vm::IActionSectionViewModel> TextRangeViewModel::GetSection() { return section; }
	Ptr<IValueList> TextRangeViewModel::GetSections() { return sections; }
	void TextRangeViewModel::SetSection(Ptr<ActionSectionViewModel> value)
	{
		section = value;
		sections = IValueList::Create();
		sections->Add(BoxValue(Ptr<vm::IActionSectionViewModel>(section)));
		SectionChanged(); SectionsChanged();
	}

	void PropertyDialogViewModel::OpenRange(vint key)
	{
		if (!open || busy || owner->treeBusy) return;
		for (auto&& range : textRanges) if (range.Cast<TextRangeViewModel>()->key == key) return;
		busy = true; NotifyAvailability();
		auto gate = owner->lifetime; auto epoch = generation, request = serial; auto worker = &owner->worker;
		owner->QueueNative(L"Read text range #" + itow(key), [gate, epoch, request, key, worker]()
		{
			if (gate->closed || gate->generation != epoch || gate->dialogSerial != request) return;
			auto result = worker->session->DescribeRange(key);
			auto references = Ptr(new vl::collections::List<native::ReferenceData>(worker->session->GetReferences()));
			UiaListViewModel::Post(gate, [result, references, epoch, request, key](UiaListViewModel& root)
			{
				if (root.lifetime->generation != epoch || !root.propertyDialog || !root.propertyDialog->open || root.propertyDialog->serial != request) return;
				auto dialog = root.propertyDialog;
				dialog->PublishReferences(*references.Obj());
				if (result)
				{
					auto range = Ptr(new TextRangeViewModel); range->key = key; range->SetSection(Ptr(new ActionSectionViewModel(*dialog.Obj(), result))); dialog->textRanges.Add(range);
				}
				dialog->busy = false; dialog->NotifyAvailability();
			});
		});
	}
}

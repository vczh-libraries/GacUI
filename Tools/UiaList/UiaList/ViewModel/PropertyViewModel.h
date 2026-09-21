#ifndef VCZH_UIALIST_PROPERTYVIEWMODEL
#define VCZH_UIALIST_PROPERTYVIEWMODEL

#include "UiaListViewModel.h"

namespace uialist
{
	class TextDialogViewModel : public vl::Object, public virtual vm::ITextDialogViewModel
	{
	public:
		vl::WString title, original, draft, validation;
		bool readOnly = true, open = true, busy = false;
		vl::Func<void(const vl::WString&)> accept;
		vl::WString GetTitle() override;
		vl::WString GetOriginalText() override;
		vl::WString GetDraftText() override;
		void SetDraftText(const vl::WString& value) override;
		bool GetIsReadOnly() override;
		bool GetCanAccept() override;
		vl::WString GetValidationMessage() override;
		bool GetIsOpen() override;
		void Accept() override;
		void Cancel() override;
	};

	class PropertyRowViewModel : public vl::Object, public virtual vm::IPropertyRowViewModel
	{
	public:
		native::PropertyData data;
		PropertyDialogViewModel* owner = nullptr;
		vl::collections::ObservableList<vl::WString> choices;
		vl::collections::List<vl::WString> choiceValues;
		vl::WString key, display, details, draft, validation;
		bool selected = false, editing = false;
		vl::WString GetKey() override;
		vl::WString GetLabel() override;
		vl::WString GetDisplayValue() override;
		bool GetCanEdit() override;
		bool GetIsMultiline() override;
		bool GetIsSelected() override;
		bool GetIsEditing() override;
		vl::vint GetEditorKind() override;
		vl::Ptr<vl::reflection::description::IValueList> GetChoices() override;
		vl::vint GetChoiceIndex() override;
		void SetChoiceIndex(vl::vint value) override;
		vl::WString GetDraftText() override;
		void SetDraftText(const vl::WString& value) override;
		vl::WString GetValidationMessage() override;
		void BeginEdit() override;
		void Commit() override;
		void Cancel() override;
		void OpenDetails() override;
		bool Validate();
	};

	class PropertyDialogViewModel : public vl::Object, public virtual vm::IPropertyDialogViewModel
	{
	public:
		UiaListViewModel* owner;
		vl::vint nodeKey, generation, serial;
		vl::WString title, pendingKey, status;
		bool open = true, busy = false;
		vl::collections::ObservableList<vl::Ptr<vm::IPropertyRowViewModel>> rows;
		vl::collections::ObservableList<vl::Ptr<vm::IActionSectionViewModel>> sections;
		vl::collections::ObservableList<vl::Ptr<vm::IReferenceViewModel>> references;
		vl::collections::ObservableList<vl::Ptr<vm::IActionResultViewModel>> results;
		vl::collections::ObservableList<vl::Ptr<vm::ITextRangeViewModel>> textRanges;
		vl::collections::List<native::ReferenceData> referenceData;
		vl::Ptr<PropertyRowViewModel> selected;
		vl::Ptr<TextDialogViewModel> textDialog;
		PropertyDialogViewModel(UiaListViewModel& root, vl::vint key, const vl::WString& name);
		vl::WString GetTitle() override;
		vl::Ptr<vl::reflection::description::IValueList> GetRows() override;
		vl::Ptr<vl::reflection::description::IValueList> GetSections() override;
		vl::Ptr<vl::reflection::description::IValueList> GetReferences() override;
		vl::Ptr<vl::reflection::description::IValueList> GetResults() override;
		vl::Ptr<vl::reflection::description::IValueList> GetTextRanges() override;
		vl::Ptr<vm::ITextDialogViewModel> GetTextDialog() override;
		bool GetIsBusy() override;
		vl::WString GetStatus() override;
		bool GetIsOpen() override;
		vl::vint GetSelectedRowIndex() override;
		void Refresh() override;
		bool SelectRow(vl::Ptr<vm::IPropertyRowViewModel> row) override;
		void BeginEdit(vl::Ptr<vm::IPropertyRowViewModel> row) override;
		void CommitEdit(vl::Ptr<vm::IPropertyRowViewModel> row) override;
		void CancelEdit(vl::Ptr<vm::IPropertyRowViewModel> row) override;
		void OpenText(vl::Ptr<vm::IPropertyRowViewModel> row) override;
		void Close() override;
		void Publish(vl::Ptr<native::InspectionSnapshot> result);
		vl::Ptr<PropertyRowViewModel> CreateRow(const native::PropertyData& property);
		void PublishReferences(const vl::collections::List<native::ReferenceData>& value);
		void NotifyAvailability();
		void OpenDetails(const vl::WString& title, const vl::WString& value);
		void InspectReference(const native::ReferenceData& reference);
		void OpenRange(vl::vint key);
	};

	extern vl::WString FormatValue(const native::ValueData& value, IStringsStrings& strings);
}

#endif

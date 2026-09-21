#ifndef VCZH_UIALIST_ACTIONVIEWMODEL
#define VCZH_UIALIST_ACTIONVIEWMODEL

#include "PropertyViewModel.h"

namespace uialist
{
	extern vl::WString CompactValue(const vl::WString& text);
	class ActionCommandViewModel;

	class ReferenceViewModel : public vl::Object, public virtual vm::IReferenceViewModel
	{
	public:
		PropertyDialogViewModel* owner;
		native::ReferenceData data;
		vl::vint generation = 0;
		vl::vint GetKey() override;
		vl::WString GetLabel() override;
		bool GetIsRange() override;
		bool GetCanInspect() override;
		bool GetCanReveal() override;
		vl::WString GetStatus() override;
		void Inspect() override;
		void Reveal() override;
	};

	class ActionParameterViewModel : public vl::Object, public virtual vm::IActionParameterViewModel
	{
	public:
		ActionCommandViewModel* owner;
		vl::Ptr<native::ArgumentSpec> spec;
		vl::WString label, draft, validation, selectedReferenceLabels;
		vl::vint reference = 0;
		VARTYPE variantType = VT_BSTR;
		bool variantElements = false;
		bool argumentValid = false;
		vl::collections::List<vl::vint> selectedReferences;
		vl::collections::ObservableList<vl::WString> choices;
		vl::collections::List<vl::vint> choiceValues;
		vl::WString GetLabel() override;
		vl::vint GetKind() override;
		vl::WString GetDraftText() override;
		void SetDraftText(const vl::WString& value) override;
		vl::Ptr<vl::reflection::description::IValueList> GetChoices() override;
		vl::vint GetChoiceIndex() override;
		void SetChoiceIndex(vl::vint value) override;
		bool GetIsMultiline() override;
		vl::WString GetValidationMessage() override;
		void EditText() override;
		vl::WString GetSelectedReferences() override;
		void AddReference() override;
		void ClearReferences() override;
		void Commit() override;
		bool Parse(native::ActionArgument& result);
		void UpdateChoices();
		void UpdateSelectedReferences();
	};

	class ActionResultViewModel : public vl::Object, public virtual vm::IActionResultViewModel
	{
	public:
		PropertyDialogViewModel* owner;
		vl::WString operation, display, details;
		vl::collections::ObservableList<vl::Ptr<vm::IReferenceViewModel>> references;
		vl::WString GetOperation() override;
		vl::WString GetDisplayValue() override;
		vl::Ptr<vl::reflection::description::IValueList> GetReferences() override;
		void OpenDetails() override;
	};

	class ActionCommandViewModel : public vl::Object, public virtual vm::IActionCommandViewModel
	{
	public:
		PropertyDialogViewModel* owner;
		vl::Ptr<native::ActionSpec> spec;
		vl::vint generation;
		vl::vint querySerial = 0;
		vl::collections::ObservableList<vl::Ptr<vm::IActionParameterViewModel>> parameters;
		vl::WString key, status;
		bool executing = false;
		vl::WString GetKey() override;
		vl::WString GetLabel() override;
		vl::Ptr<vl::reflection::description::IValueList> GetParameters() override;
		bool GetCanExecute() override;
		bool GetIsBusy() override;
		bool GetIsGetter() override;
		vl::WString GetStatus() override;
		void Execute() override;
		void Validate(bool invalidateQuery = true);
		void Query();
	};

	class ActionSectionViewModel : public vl::Object, public virtual vm::IActionSectionViewModel
	{
	public:
		vl::Ptr<native::ActionSectionData> data;
		vl::collections::ObservableList<vl::Ptr<vm::IPropertyRowViewModel>> readouts;
		vl::collections::ObservableList<vl::Ptr<vm::IActionCommandViewModel>> commands;
		ActionSectionViewModel(PropertyDialogViewModel& owner, vl::Ptr<native::ActionSectionData> data);
		vl::WString GetHeading() override;
		vl::vint GetPatternId() override;
		vl::Ptr<vl::reflection::description::IValueList> GetReadouts() override;
		vl::Ptr<vl::reflection::description::IValueList> GetCommands() override;
	};
}

#endif

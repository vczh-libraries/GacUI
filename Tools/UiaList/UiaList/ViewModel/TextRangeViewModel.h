#ifndef VCZH_UIALIST_TEXTRANGEVIEWMODEL
#define VCZH_UIALIST_TEXTRANGEVIEWMODEL

#include "ActionViewModel.h"

namespace uialist
{
	class TextRangeViewModel : public vl::Object, public virtual vm::ITextRangeViewModel
	{
	public:
		vl::vint key;
		vl::Ptr<ActionSectionViewModel> section;
		vl::WString GetLabel() override;
		vl::Ptr<vm::IActionSectionViewModel> GetSection() override;
		vl::Ptr<vl::reflection::description::IValueList> GetSections() override;
	};
}

#endif

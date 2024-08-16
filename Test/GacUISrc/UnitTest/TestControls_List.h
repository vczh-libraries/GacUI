#include "TestControls.h"

namespace gacui_unittest_template
{
	extern void GuiListControl_SingleSelect_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiSelectableListControl_SingleSelect_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiSelectableListControl_MultiSelect_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiListItemTemplate_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiListItemTemplate_WithAxis_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiVirtualTextList_TestCases(
		WString resourceXml,
		WString pathFragment,
		bool setTextListView,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	);

	extern void GuiTextListItemTemplate_TestCases(
		WString resourceXml,
		WString pathFragment,
		bool setTextListView,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	);
}
#include "TestControls.h"

namespace gacui_unittest_template
{
	extern void GuiTextList_TestCases(
		const wchar_t* resourceXml,
		WString pathFragment,
		bool setTextListView,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	);

	extern void GuiTextListItemTemplate_TestCases(
		const wchar_t* resourceXml,
		WString pathFragment,
		bool setTextListView,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	);
}
#include "TestControls.h"

namespace gacui_unittest_template
{
	extern void GuiTextList_TestCases(
		const wchar_t* resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	);
}
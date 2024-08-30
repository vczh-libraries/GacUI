#include "TestControls.h"

namespace gacui_unittest_template
{
	extern NativePoint	GetListItemLocation(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX);
	extern void			HoverListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX = -1);
	extern void			LClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX = -1);
	extern void			LClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, bool ctrl, bool shift, bool alt, vint index, vint offsetX = -1);
	extern void			LDBClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX = -1);
	extern void			RClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX = -1);
	extern void			RClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, bool ctrl, bool shift, bool alt, vint index, vint offsetX = -1);

	// GuiListControl

	extern void GuiListControl_TestCases(
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

	// TextList

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

	// ListView

	extern void GuiVirtualListView_ViewAndImages_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiVirtualListView_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	);

	// TreeView

	extern void GuiVirtualTreeListControl_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiVirtualTreeView_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	);

	extern void GuiVirtualTreeView_Images_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiTreeItemTemplate1_TestCases(
		WString resourceXml,
		WString pathFragment
	);

	extern void GuiTreeItemTemplate2_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	);
}
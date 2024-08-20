#include "TestControls_List.h"

namespace gacui_unittest_template
{
	static void InitializeItems(GuiWindow* window, vint count)
	{
		Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(count)));
	}

	static Value MakeItem(GuiWindow* window, WString name)
	{
		return Value::From(window).Invoke(L"MakeItem", (Value_xs(), BoxValue(name), BoxValue<vint>(-1)));
	}

	// TODO:
	// ExpandItem and specify (Expand/Collapse, index, path)
	// verify if the node at the path is rendered in the index-th position
	// verify Expanded
	// click
	// verify Expanded

	void GuiTreeItemTemplate_Shared_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	);

	void GuiVirtualTreeView_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	)
	{
		TEST_CASE(L"MakeVisibleItems")
		{
		});

		TEST_CASE(L"MakeInvisibleItems")
		{
		});

		TEST_CASE(L"NavigateByClickAndKey")
		{
		});

		GuiTreeItemTemplate_Shared_TestCases(
			resourceXml,
			pathFragment,
			getRootItems,
			getChildItems,
			updateText,
			notifyNodeDataModified
		);
	}

	void GuiTreeItemTemplate_Shared_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	)
	{
		TEST_CASE(L"UpdateVisibleItems")
		{
		});

		TEST_CASE(L"UpdateInvisibleItems")
		{
		});

		TEST_CASE(L"ClickAndExpandCollapseItems")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 5);
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle);
						protocol->LClick(location);
					}
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(3);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 3);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8 + 12, 0);
						protocol->LClick(location);
					}
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(6);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 6);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8 + 12, 0);
						protocol->LClick(location);
					}
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
				});
				protocol->OnNextIdleFrame(L"Collapse 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/ClickAndExpandCollapseItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}

	void GuiTreeItemTemplate_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	)
	{
		TEST_CASE(L"MouseVisualEffect")
		{
		});

		TEST_CASE(L"Context")
		{
		});

		TEST_CASE(L"Font")
		{
		});

		TEST_CASE(L"DisplayItemBackground")
		{
		});

		TEST_CASE(L"Image")
		{
		});

		GuiTreeItemTemplate_Shared_TestCases(
			resourceXml,
			pathFragment,
			getRootItems,
			getChildItems,
			updateText,
			notifyNodeDataModified
		);
	}
}
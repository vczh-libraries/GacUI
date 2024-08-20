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
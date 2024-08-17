#include "TestControls_List.h"

namespace gacui_unittest_template
{
	void GuiVirtualListView_WithView_TestCases(
		WString resourceXml,
		WString pathFragment,
		ListViewView view,
		WString viewName,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	)
	{
		TEST_CATEGORY(viewName)
		{
			TEST_CASE(L"MakeVisibleItems")
			{
			});

			TEST_CASE(L"MakeInvisibleItems")
			{
			});

			TEST_CASE(L"UpdateVisibleItems")
			{
			});

			TEST_CASE(L"UpdateInvisibleItems")
			{
			});

			TEST_CASE(L"ClickVisibleItems")
			{
			});

			TEST_CASE(L"NavigateByKey")
			{
			});
		});
	}

	void GuiVirtualListView_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	)
	{
		TEST_CASE(L"SwitchViews")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
					listControl->SetView(ListViewView::BigIcon);
				});
				protocol->OnNextIdleFrame(L"BigIcon", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
					listControl->SetView(ListViewView::SmallIcon);
				});
				protocol->OnNextIdleFrame(L"SmallIcon", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
					listControl->SetView(ListViewView::List);
				});
				protocol->OnNextIdleFrame(L"List", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
					listControl->SetView(ListViewView::Tile);
				});
				protocol->OnNextIdleFrame(L"Tile", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
					listControl->SetView(ListViewView::Information);
				});
				protocol->OnNextIdleFrame(L"Information", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
					listControl->SetView(ListViewView::Detail);
				});
				protocol->OnNextIdleFrame(L"Detail", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/SwitchViews"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		ListViewView views[] = {
			ListViewView::BigIcon,
			ListViewView::SmallIcon,
			ListViewView::List,
			ListViewView::Tile,
			ListViewView::Information,
			ListViewView::Detail,
			};

		WString viewNames[] = {
			WString::Unmanaged(L"BigIcon"),
			WString::Unmanaged(L"SmallIcon"),
			WString::Unmanaged(L"List"),
			WString::Unmanaged(L"Tile"),
			WString::Unmanaged(L"Information"),
			WString::Unmanaged(L"Detail"),
			};

		for (vint i = 0; i < 6; i++)
		{
			GuiVirtualListView_WithView_TestCases(
				resourceXml,
				pathFragment,
				views[i],
				viewNames[i],
				getItems,
				notifyItemDataModified
				);
		}
	}
}
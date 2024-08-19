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
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Ready", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto listControl = FindObjectByName<GuiVirtualListView>(window, L"list");
						listControl->SetView(view);
						InitializeItems(window, 5);
					});
					protocol->OnNextIdleFrame(L"5 Items", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto items = getItems(window);
						items->Insert(0, MakeItem(window, L"First Item"));
					});
					protocol->OnNextIdleFrame(L"Add to Top", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto items = getItems(window);
						items->Add(MakeItem(window, L"Last Item"));
					});
					protocol->OnNextIdleFrame(L"Add to Last", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto items = getItems(window);
						items->RemoveAt(0);
						items->RemoveAt(items->GetCount() - 1);
					});
					protocol->OnNextIdleFrame(L"Remove Added Items", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
						listControl->SetSelected(2, true);
					});
					protocol->OnNextIdleFrame(L"Select 3rd", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto items = getItems(window);
						items->Set(2, MakeItem(window, L"Updated Item"));
					});
					protocol->OnNextIdleFrame(L"Update 3rd", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
						listControl->SetSelected(2, true);
					});
					protocol->OnNextIdleFrame(L"Select 3rd", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/") + viewName + WString::Unmanaged(L"/MakeVisibleItems"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resourceXml
					);
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

	void GuiVirtualListView_ViewAndImages_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"SwitchViews")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 20);
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
	}

	void GuiVirtualListView_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getItems,
		Func<void(GuiWindow*, vint, vint)> notifyItemDataModified
	)
	{
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
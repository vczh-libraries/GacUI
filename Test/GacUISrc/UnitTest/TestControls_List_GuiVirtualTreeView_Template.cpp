#include "TestControls_List.h"

namespace gacui_unittest_template
{
	static void InitializeItems(GuiWindow* window, vint count)
	{
		Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(count)));
	}

	static Value MakeItem(GuiWindow* window, WString name, vint number)
	{
		return Value::From(window).Invoke(L"MakeItem", (Value_xs(), BoxValue(name), BoxValue(number)));
	}

	template<typename ...TPaths>
	Ptr<INodeProvider> GetNodeProvider(
		GuiWindow* window,
		GuiVirtualTreeListControl* listControl,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		vint visibleIndex,
		TPaths ...paths
	)
	{
		static_assert(sizeof...(paths) >= 1);
		vint pathIndex[] = { paths... };
		Value node = getRootItems(window)->Get(pathIndex[0]);
		for (vint i = 1; i < sizeof...(paths); i++)
		{
			node = getChildItems(node)->Get(pathIndex[i]);
		}

		auto nodeItemView = dynamic_cast<INodeItemView*>(listControl->GetItemProvider()->RequestView(WString::Unmanaged(INodeItemView::Identifier)));
		auto nodeProvider = nodeItemView->RequestNode(visibleIndex);
		if (dynamic_cast<GuiTreeView*>(listControl))
		{
			TEST_ASSERT(nodeProvider == node.GetSharedPtr());
		}
		else
		{
			auto nodeData = listControl->GetNodeRootProvider()->GetBindingValue(nodeProvider.Obj());
			TEST_ASSERT(nodeData == node);
		}

		return nodeProvider;
	}

	template<typename ...TPaths>
	void ClickExpandItem(
		UnitTestRemoteProtocol* protocol,
		GuiWindow* window,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		bool toExpand,
		vint visibleIndex,
		TPaths ...paths
	)
	{
		auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
		auto nodeProvider = GetNodeProvider(window, listControl, getRootItems, getChildItems, visibleIndex, paths...);
		TEST_ASSERT(nodeProvider->GetExpanding() != toExpand);
		LClickListItem(protocol, listControl, visibleIndex, 8 + 12 * (sizeof...(paths) - 1));
		TEST_ASSERT(nodeProvider->GetExpanding() == toExpand);
	}

	template<typename ...TPaths>
	void DBClickExpandItem(
		UnitTestRemoteProtocol* protocol,
		GuiWindow* window,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		bool toExpand,
		vint visibleIndex,
		TPaths ...paths
	)
	{
		auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
		auto nodeProvider = GetNodeProvider(window, listControl, getRootItems, getChildItems, visibleIndex, paths...);
		TEST_ASSERT(nodeProvider->GetExpanding() != toExpand);
		LDBClickListItem(protocol, listControl, visibleIndex);
		TEST_ASSERT(nodeProvider->GetExpanding() == toExpand);
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
					auto items = getRootItems(window);
					items->Insert(0, MakeItem(window, L"First Item", 101));
				});
				protocol->OnNextIdleFrame(L"Add to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getRootItems(window);
					items->Add(MakeItem(window, L"Last Item", 102));
				});
				protocol->OnNextIdleFrame(L"Add to Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getRootItems(window);
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
					auto items = getRootItems(window);
					items->Set(2, MakeItem(window, L"Updated Item", 103));
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
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MakeVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"MakeVisibleChildItems")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 5);

					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					listControl->GetNodeRootProvider()->GetRootNode()->GetChild(1)->SetExpanding(true);
				});
				protocol->OnNextIdleFrame(L"5 Items with 2nd Expanded", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getChildItems(getRootItems(window)->Get(1));
					items->Insert(0, MakeItem(window, L"First Item", 101));
				});
				protocol->OnNextIdleFrame(L"Add to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getChildItems(getRootItems(window)->Get(1));
					items->Add(MakeItem(window, L"Last Item", 102));
				});
				protocol->OnNextIdleFrame(L"Add to Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getChildItems(getRootItems(window)->Get(1));
					items->RemoveAt(0);
					items->RemoveAt(items->GetCount() - 1);
				});
				protocol->OnNextIdleFrame(L"Remove Added Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetSelected(3, true);
				});
				protocol->OnNextIdleFrame(L"Select 2nd/2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getChildItems(getRootItems(window)->Get(1));
					items->Set(1, MakeItem(window, L"Updated Item", 103));
				});
				protocol->OnNextIdleFrame(L"Update 2nd/2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetSelected(3, true);
				});
				protocol->OnNextIdleFrame(L"Select 2nd/2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MakeVisibleChildItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"MakeInvisibleItems")
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
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					auto items = getRootItems(window);
					items->Insert(0, MakeItem(window, L"First Item", 101));
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Add to Top and Select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					auto items = getRootItems(window);
					items->Add(MakeItem(window, L"Last Item", 102));
					listControl->SetSelected(21, true);
				});
				protocol->OnNextIdleFrame(L"Add to Last and Select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(21);
				});
				protocol->OnNextIdleFrame(L"Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getRootItems(window);
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
					auto items = getRootItems(window);
					items->Set(2, MakeItem(window, L"Updated Item", 103));
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
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MakeInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"MakeInvisibleChildItems")
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
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					getChildItems(getRootItems(window)->Get(1))->Clear();
				});
				protocol->OnNextIdleFrame(L"Remove all children of 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					auto children = getChildItems(getRootItems(window)->Get(2));
					children->Insert(0, MakeItem(window, L"First Item", 101));
					children->Insert(4, MakeItem(window, L"Last Item", 102));
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Add children to 3rd and select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					auto node = listControl->GetNodeRootProvider()->GetRootNode()->GetChild(2);
					node->SetExpanding(true);
				});
				protocol->OnNextIdleFrame(L"Expand 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MakeInvisibleChildItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
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
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto node = getRootItems(window)->Get(1);
					updateText(node, L"Updated Text");
					notifyNodeDataModified(window, node);
				});
				protocol->OnNextIdleFrame(L"Change 2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto node = getRootItems(window)->Get(0);
					updateText(node, L"New Text");
					notifyNodeDataModified(window, node);
				});
				protocol->OnNextIdleFrame(L"Change 1st Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/UpdateVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"UpdateVisibleChildItems")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					InitializeItems(window, 5);
					listControl->GetNodeRootProvider()->GetRootNode()->GetChild(1)->SetExpanding(true);
				});
				protocol->OnNextIdleFrame(L"5 Items and Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 2nd/1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto node = getChildItems(getRootItems(window)->Get(1))->Get(1);
					updateText(node, L"Updated Text");
					notifyNodeDataModified(window, node);
				});
				protocol->OnNextIdleFrame(L"Change 2nd/2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto node = getChildItems(getRootItems(window)->Get(1))->Get(0);
					updateText(node, L"New Text");
					notifyNodeDataModified(window, node);
				});
				protocol->OnNextIdleFrame(L"Change 2nd/1st Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/UpdateVisibleChildItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"UpdateInvisibleItems")
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
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetSelected(0, true);
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Select 1st and Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto items = getRootItems(window);
					updateText(items->Get(0), L"New Text");
					updateText(items->Get(1), L"Updated Text");
					updateText(items->Get(2), L"Whatever");
					notifyNodeDataModified(window, items->Get(0));
					notifyNodeDataModified(window, items->Get(1));
					notifyNodeDataModified(window, items->Get(2));
				});
				protocol->OnNextIdleFrame(L"Change 1st, 2nd, 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/UpdateInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"UpdateInvisibleChildItems")
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
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					listControl->GetNodeRootProvider()->GetRootNode()->GetChild(1)->SetExpanding(true);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					listControl->GetNodeRootProvider()->GetRootNode()->GetChild(1)->SetExpanding(false);
				});
				protocol->OnNextIdleFrame(L"Collapse 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					auto items = getChildItems(getRootItems(window)->Get(1));
					updateText(items->Get(0), L"New Text");
					updateText(items->Get(1), L"Updated Text");
					updateText(items->Get(2), L"Whatever");
					listControl->SetSelected(1, true);
				});
				protocol->OnNextIdleFrame(L"Select 2nd and Change 2nd/1st, 2nd/2nd, 2nd/3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					listControl->GetNodeRootProvider()->GetRootNode()->GetChild(1)->SetExpanding(true);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/UpdateInvisibleChildItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
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
					LClickListItem(protocol, listControl, 0);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						1,
						1
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						3,
						1, 1
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						6,
						1, 2
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						false,
						1,
						1
					);
				});
				protocol->OnNextIdleFrame(L"Collapse 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						1,
						1
					);
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

		TEST_CASE(L"DBClickAndExpandCollapseItems")
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
					LClickListItem(protocol, listControl, 0);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					DBClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						1,
						1
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					DBClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						3,
						1, 1
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					DBClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						6,
						1, 2
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					DBClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						false,
						1,
						1
					);
				});
				protocol->OnNextIdleFrame(L"Collapse 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					DBClickExpandItem(
						protocol, window, getRootItems, getChildItems,
						true,
						1,
						1
					);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/DBClickAndExpandCollapseItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"KeyAndExpandCollapseItems")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					InitializeItems(window, 5);
					listControl->SetFocused();
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					protocol->KeyPress(VKEY::KEY_HOME);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->KeyPress(VKEY::KEY_RIGHT);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->KeyPress(VKEY::KEY_RIGHT);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/2nd", [=]()
				{
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->KeyPress(VKEY::KEY_DOWN);
					protocol->KeyPress(VKEY::KEY_RIGHT);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd/3rd", [=]()
				{
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->KeyPress(VKEY::KEY_UP);
					protocol->KeyPress(VKEY::KEY_LEFT);
				});
				protocol->OnNextIdleFrame(L"Collapse 2nd", [=]()
				{
					protocol->KeyPress(VKEY::KEY_RIGHT);
				});
				protocol->OnNextIdleFrame(L"Expand 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/KeyAndExpandCollapseItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}

	void GuiVirtualTreeView_Images_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"Image")
		{
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 5);

					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					listControl->GetNodeRootProvider()->GetRootNode()->GetChild(2)->SetExpanding(true);
				});
				protocol->OnNextIdleFrame(L"5 Items with 3rd Expanded", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/Image"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}

	void GuiTreeItemTemplate1_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"MouseVisualEffect")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->LClick();
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetEnabled(false);
				});
				protocol->OnNextIdleFrame(L"Disabled", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetEnabled(true);
				});
				protocol->OnNextIdleFrame(L"Enabled", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MouseVisualEffect"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Context")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetContext(BoxValue(WString(L"This")));
				});
				protocol->OnNextIdleFrame(L"This", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetContext(BoxValue(WString(L"That")));
				});
				protocol->OnNextIdleFrame(L"That", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetContext({});
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/Context"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Font")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					FontProperties font;
					font.fontFamily = L"This Font";
					font.size = 18;
					listControl->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"18", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					FontProperties font;
					font.fontFamily = L"That Font";
					font.size = 24;
					listControl->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"24", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetFont({});
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
					listControl->SetSelected(19, true);
				});
				protocol->OnNextIdleFrame(L"Scroll to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/Font"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"DisplayItemBackground")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
					protocol->LClick();
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
				});
				protocol->OnNextIdleFrame(L"Select 1st and Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetDisplayItemBackground(false);
				});
				protocol->OnNextIdleFrame(L"DisplayItemBackground = false", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
					protocol->LClick();
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
				});
				protocol->OnNextIdleFrame(L"Select 3rd and Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetDisplayItemBackground(true);
				});
				protocol->OnNextIdleFrame(L"DisplayItemBackground = true", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove({ location.x + 1,location.y });
					}
				});
				protocol->OnNextIdleFrame(L"Move", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/DisplayItemBackground"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}

	void GuiTreeItemTemplate2_TestCases(
		WString resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getRootItems,
		Func<Ptr<IValueList>(Value)> getChildItems,
		Func<void(Value, WString)> updateText,
		Func<void(GuiWindow*, Value)> notifyNodeDataModified
	)
	{
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
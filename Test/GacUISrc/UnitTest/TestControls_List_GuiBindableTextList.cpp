#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::unittest;

TEST_FILE
{
	const auto resourceTextList = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func InitializeItems(items:observe TextItem^[], count:int) : void
        {
          list.ItemSource = items;
          for (item in range[1, count])
          {
            items.Add(new TextItem^($"Item $(item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiBindableTextList [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItemText)]" ClientSize="x:320 y:240">
        <GuiBindableTextList ref.Name="list" env.ItemType="TextItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.TextProperty>Text</att.TextProperty>
          <att.CheckedProperty>Checked</att.CheckedProperty>
        </BindableTextList>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiBindableTextList")
	{
		TEST_CASE(L"MakeVisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Insert(0, Ptr(new TextItem(L"First Item")));
				});
				protocol->OnNextIdleFrame(L"Add to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Add(Ptr(new TextItem(L"Last Item")));
				});
				protocol->OnNextIdleFrame(L"Add to Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->RemoveAt(0);
					items->RemoveAt(items->Count() - 1);
				});
				protocol->OnNextIdleFrame(L"Remove Added Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Set(2, Ptr(new TextItem(L"Updated Item")));
				});
				protocol->OnNextIdleFrame(L"Update 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/MakeVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"MakeInvisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Insert(0, Ptr(new TextItem(L"First Item")));
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Add to Top and Select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Add(Ptr(new TextItem(L"Last Item")));
					listControl->SetSelected(21, true);
				});
				protocol->OnNextIdleFrame(L"Add to Last and Select", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(21);
				});
				protocol->OnNextIdleFrame(L"Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->RemoveAt(0);
					items->RemoveAt(items->Count() - 1);
				});
				protocol->OnNextIdleFrame(L"Remove Added Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Set(2, Ptr(new TextItem(L"Updated Item")));
				});
				protocol->OnNextIdleFrame(L"Update 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/MakeInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"UpdateVisibleItems (View = Check)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(1)->SetText(L"Updated Text");
					listControl->NotifyItemDataModified(1, 1);
				});
				protocol->OnNextIdleFrame(L"Change 2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(2)->SetChecked(true);
					listControl->NotifyItemDataModified(2, 1);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(0)->SetText(L"New Text");
					items->Get(0)->SetChecked(true);
					listControl->NotifyItemDataModified(0, 1);
				});
				protocol->OnNextIdleFrame(L"Change 1st Text and Check", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/UpdateVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"UpdateInvisibleItems (View = Radio)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Radio);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Select 1st and Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(0)->SetText(L"New Text");
					items->Get(0)->SetChecked(true);
					items->Get(1)->SetText(L"Updated Text");
					items->Get(2)->SetChecked(true);
					listControl->NotifyItemDataModified(0, 3);
				});
				protocol->OnNextIdleFrame(L"Change 1st, 2nd, 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/UpdateInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});
	});

	TEST_CATEGORY(L"Accessibility")
	{
		TEST_CASE(L"ClickVisibleItems (View = Check)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");

					TEST_ASSERT(items->Get(2)->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 2);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(items->Get(2)->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");

					TEST_ASSERT(items->Get(0)->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(items->Get(0)->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/ClickVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});
	});

	TEST_CATEGORY(L"GuiTextListItemTemplate")
	{
		TEST_CASE(L"UpdateVisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(1)->SetText(L"Updated Text");
					listControl->NotifyItemDataModified(1, 1);
				});
				protocol->OnNextIdleFrame(L"Change 2nd Text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(2)->SetChecked(true);
					listControl->NotifyItemDataModified(2, 1);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(0)->SetText(L"New Text");
					items->Get(0)->SetChecked(true);
					listControl->NotifyItemDataModified(0, 1);
				});
				protocol->OnNextIdleFrame(L"Change 1st Text and Check", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/GuiTextListItemTemplate/UpdateVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});

		TEST_CASE(L"UpdateInvisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Radio);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Select 1st and Scroll to Bottom", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					items->Get(0)->SetText(L"New Text");
					items->Get(0)->SetChecked(true);
					items->Get(1)->SetText(L"Updated Text");
					items->Get(2)->SetChecked(true);
					listControl->NotifyItemDataModified(0, 3);
				});
				protocol->OnNextIdleFrame(L"Change 1st, 2nd, 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->EnsureItemVisible(0);
				});
				protocol->OnNextIdleFrame(L"Scroll to Top", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/GuiTextListItemTemplate/UpdateInvisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});
		
		TEST_CASE(L"ClickVisibleItems")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto items = Ptr(new collections::ObservableList<Ptr<TextItem>>);
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetView(TextListView::Check);
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxParameter(*items.Obj()), BoxValue<vint>(5)));
				});
				protocol->OnNextIdleFrame(L"5 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");

					TEST_ASSERT(items->Get(2)->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 2);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(items->Get(2)->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiBindableTextList>(window, L"list");

					TEST_ASSERT(items->Get(0)->GetChecked() == false);
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						TEST_ASSERT(itemStyle != nullptr);
						TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
						auto location = protocol->LocationOf(itemStyle, 0.0, 0.5, 8, 0);
						protocol->LClick(location);
					}
					TEST_ASSERT(items->Get(0)->GetChecked() == true);
				});
				protocol->OnNextIdleFrame(L"Check 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableTextList/GuiTextListItemTemplate/ClickVisibleItems"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTextList
				);
		});
	});
}
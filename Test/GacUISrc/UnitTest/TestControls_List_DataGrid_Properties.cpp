#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const WString resourceDataGridStringProperty = LR"GacUISrc(
<Resource>
  <Script name="DataGridItemResource"><Workflow><![CDATA[
    module datagriditem;
    using system::*;

    class DataGridItem
    {
      prop Language:string = "" {not observe}
      prop IDE:string = "" {not observe}
      prop Company:string = "" {not observe}

      new(){}
      new(language:string, ide:string, company:string)
      {
        Language=language;
        IDE=ide;
        Company=company;
      }
    }
  ]]></Workflow></Script>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items : observe DataGridItem^[] = {
          new DataGridItem^("C++", "Visual Studio", "Microsoft");
          new DataGridItem^("C#", "Visual Studio", "Microsoft");
          new DataGridItem^("F#", "Visual Studio", "Microsoft");
          new DataGridItem^("TypeScript", "Visual Studio Code", "Microsoft");
          new DataGridItem^("Java", "Eclipse", "IBM");
        };
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableDataGrid" ClientSize="x:640 y:320">
        <BindableDataGrid ref.Name="dataGrid" env.ItemType="DataGridItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.ItemSource-eval>self.items</att.ItemSource-eval>
          <att.Columns>
            <_ Text="Language" TextProperty="Language"/>
            <_ Text="IDE" TextProperty="IDE"/>
            <_ Text="Company" TextProperty="Company"/>
          </att.Columns>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiBindableDataGrid")
	{
		TEST_CASE(L"SelectCell")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == -1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(-1, -1));
					dataGrid->SelectCell({ 1,1 }, false);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 1));
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,2 }, false);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 2));
				});
				protocol->OnNextIdleFrame(L"1,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 2,2 }, false);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 2));
				});
				protocol->OnNextIdleFrame(L"2,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 2,1 }, false);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 1));
				});
				protocol->OnNextIdleFrame(L"2,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,1 }, false);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 1));
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Properties/SelectCell"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridStringProperty
				);
		});

		TEST_CASE(L"SelectCellOpenEditor")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == -1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(-1, -1));
					dataGrid->SelectCell({ 1,1 }, true);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 1));
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,2 }, true);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 2));
				});
				protocol->OnNextIdleFrame(L"1,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 2,2 }, true);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 2));
				});
				protocol->OnNextIdleFrame(L"2,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 2,1 }, true);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 1));
				});
				protocol->OnNextIdleFrame(L"2,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,1 }, true);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 1));
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Properties/SelectCellOpenEditor"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridStringProperty
				);
		});

		TEST_CASE(L"SelectCellByClick")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,1 }, false);
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					LClickDataCell(protocol, dataGrid, 1, 2);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 2));
				});
				protocol->OnNextIdleFrame(L"1,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					LClickDataCell(protocol, dataGrid, 2, 2);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 2));
				});
				protocol->OnNextIdleFrame(L"2,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					LClickDataCell(protocol, dataGrid, 2, 1);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 1));
				});
				protocol->OnNextIdleFrame(L"2,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					LClickDataCell(protocol, dataGrid, 1, 1);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 1));
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Properties/SelectCellByClick"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridStringProperty
				);
		});

		TEST_CASE(L"SelectCellByKey")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,1 }, false);
					dataGrid->SetFocused();
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					protocol->KeyPress(VKEY::KEY_RIGHT);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 2));
				});
				protocol->OnNextIdleFrame(L"1,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					protocol->KeyPress(VKEY::KEY_DOWN);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 2));
				});
				protocol->OnNextIdleFrame(L"2,2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					protocol->KeyPress(VKEY::KEY_LEFT);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 2);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(2, 1));
				});
				protocol->OnNextIdleFrame(L"2,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					protocol->KeyPress(VKEY::KEY_UP);
					TEST_ASSERT(dataGrid->GetSelectedItemIndex() == 1);
					TEST_ASSERT(dataGrid->GetSelectedCell() == GridPos(1, 1));
				});
				protocol->OnNextIdleFrame(L"1,1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Properties/SelectCellByKey"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridStringProperty
				);
		});
	});
}
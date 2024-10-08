#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const WString resourceDataGrid = LR"GacUISrc(
<Resource>
  <Script name="DataGridItemResource"><Workflow><![CDATA[
    module datagriditem;
    using system::*;

    enum Companies
    {
      Microsoft=0,
      Borland=1,
      IBM=2,
    }

    class DataGridItem
    {
      prop Language:string = "" {not observe}
      prop MetaProgramming:bool = false {not observe}
      prop IDEs:int = 0 {not observe}
      prop Company:Companies = Microsoft {not observe}

      new(){}
      new(language:string, metaProgramming:bool, ides:int, company:Companies)
      {
        Language=language;
        MetaProgramming=metaProgramming;
        IDEs=ides;
        Company=company;
      }
    }

    func ToString(value:Companies) : string
    {
      switch(value)
      {
        case Microsoft: { return "Microsoft"; }
        case Borland: { return "Borland"; }
        case IBM: { return "IBM"; }
        default: { return "<ERROR>"; }
      }
    }
  ]]></Workflow></Script>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items : observe DataGridItem^[] = {
          new DataGridItem^("C++", true, 4, Microsoft);
          new DataGridItem^("C#", false, 3, Microsoft);
          new DataGridItem^("F#", false, 2, Microsoft);
          new DataGridItem^("TypeScript", true, 1, Microsoft);
          new DataGridItem^("Java", false, 3, IBM);
          new DataGridItem^("Object Pascal", false, 1, Borland);
        };
        var companySorterById : IDataSorter^ = null;
        var companySorterByName : IDataSorter^ = null;
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{
        companySorterById = dataGrid.Columns[3].Sorter;
        companySorterByName = new IDataSorter^
        {
          override func SetCallback(value:IDataProcessorCallback*) : void
          {
          }
          override func Compare(row1:object, row2:object) : int
          {
            return Sys::Compare(
              ToString((cast DataGridItem^ row1).Company),
              ToString((cast DataGridItem^ row1).Company)
              );
          }
        };
      }]]></ref.Ctor>
      <Window ref.Name="self" Text="GuiBindableDataGrid" ClientSize="x:640 y:320">
        <BindableDataGrid ref.Name="dataGrid" env.ItemType="DataGridItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.ItemSource-eval>self.items</att.ItemSource-eval>
          <att.Columns>
            <_ Text="Language" Size="150" TextProperty="Language">
              <att.Sorter>[Sys::Compare($1.Language, $2.Language)]</att.Sorter>
            </_>
            <_ Text="Meta Programming" Size="150" TextProperty="MetaProgramming">
            </_>
            <_ Text="IDE Count" Size="150" TextProperty="IDEs">
            </_>
            <_ Text="Company" Size="150" TextProperty-eval="[ToString((cast DataGridItem^ $1).Company)]">
              <att.Sorter>[Sys::Compare(cast UInt64 $1.Company, cast UInt64 $2.Company)]</att.Sorter>
            </_>
          </att.Columns>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiBindableDataGrid")
	{
		TEST_CASE(L"SortByColumn")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->IsColumnSortable(0) == true);
					TEST_ASSERT(dataGridView->IsColumnSortable(1) == false);
					TEST_ASSERT(dataGridView->IsColumnSortable(2) == false);
					TEST_ASSERT(dataGridView->IsColumnSortable(3) == true);
					TEST_ASSERT(dataGridView->GetSortedColumn() == -1);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					dataGridView->SortByColumn(0, true);
				});
				protocol->OnNextIdleFrame(L"+Language", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 0);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					dataGridView->SortByColumn(0, false);
				});
				protocol->OnNextIdleFrame(L"-Language", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 0);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					dataGridView->SortByColumn(1, true);
				});
				protocol->OnNextIdleFrame(L"+MP", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 1);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					dataGridView->SortByColumn(1, false);
				});
				protocol->OnNextIdleFrame(L"-MP", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 1);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					dataGridView->SortByColumn(2, true);
				});
				protocol->OnNextIdleFrame(L"+IDEs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 2);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					dataGridView->SortByColumn(2, false);
				});
				protocol->OnNextIdleFrame(L"-IDEs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 2);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					dataGridView->SortByColumn(3, true);
				});
				protocol->OnNextIdleFrame(L"+Company", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 3);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					dataGridView->SortByColumn(3, false);
				});
				protocol->OnNextIdleFrame(L"-Company", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 3);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					dataGrid->GetColumns()[3]->SetSorter(UnboxValue<Ptr<IDataSorter>>(Value::From(window).GetProperty(L"companySorterByName")));
				});
				protocol->OnNextIdleFrame(L"-Company by Name", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 3);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					dataGridView->SortByColumn(-1, false);
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == -1);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/ColumnApi/SortByColumn"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});

		TEST_CASE(L"FilterByColumn")
		{
		});

		TEST_CASE(L"ResetDataSource")
		{
		});

		TEST_CASE(L"ReplaceDataSource")
		{
		});

		TEST_CASE(L"ChangeDataSource")
		{
		});
	});
}
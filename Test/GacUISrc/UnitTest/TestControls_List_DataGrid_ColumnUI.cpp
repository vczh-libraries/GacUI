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
    <Instance ref.Class="gacuisrc_unittest::MainWindow" xmlns:x="presentation::controls::GuiSelectableButton::*">
      <ref.Members><![CDATA[
        var items : observe DataGridItem^[] = {
          new DataGridItem^("C++", true, 4, Microsoft);
          new DataGridItem^("C#", false, 3, Microsoft);
          new DataGridItem^("F#", false, 2, Microsoft);
          new DataGridItem^("TypeScript", true, 1, Microsoft);
          new DataGridItem^("Object Pascal", false, 1, Borland);
          new DataGridItem^("Java", false, 3, IBM);
        };
        var filterByIDEs : IDataFilter^ = null;
        var filterByCompaniesCallback : IDataProcessorCallback* = null;
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableDataGrid" env.ItemType="DataGridItem^" ClientSize="x:640 y:320">
        <att.filterByIDEs>[$1.IDEs &gt; 1]</att.filterByIDEs>
        <x:MutexGroupController ref.Name="mutexIDEs"/>
        <x:MutexGroupController ref.Name="mutexCompanies"/>
        <BindableDataGrid ref.Name="dataGrid" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.ItemSource-eval>self.items</att.ItemSource-eval>
          <att.Columns>
            <_ Text="Language" Size="150" TextProperty="Language">
              <att.Sorter>[Sys::Compare($1.Language, $2.Language)]</att.Sorter>
            </_>
            <_ Text="Meta Programming" Size="150" TextProperty="MetaProgramming">
            </_>
            <_ Text="IDE Count" Size="150" TextProperty="IDEs">
              <att.Popup>
                <ToolstripMenu>
                  <Stack Direction="Vertical" Padding="5" AlignmentToParent="left:5 top:5 right:5 bottom:5" MinSizeLimitation="LimitToElementAndChildren">
                    <StackItem>
                      <RadioButton Text="All" Selected="true" GroupController-ref="mutexIDEs">
                        <ev.SelectedChanged-eval><![CDATA[{
                          if ((cast GuiSelectableButton* (sender.RelatedControl)).Selected)
                          {
                            dataGrid.Columns[2].Filter = null;
                          }
                        }]]></ev.SelectedChanged-eval>
                      </RadioButton>
                    </StackItem>
                    <StackItem>
                      <RadioButton Text="Multiple IDEs" GroupController-ref="mutexIDEs">
                        <ev.SelectedChanged-eval><![CDATA[{
                          if ((cast GuiSelectableButton* (sender.RelatedControl)).Selected)
                          {
                            dataGrid.Columns[2].Filter = self.filterByIDEs;
                          }
                        }]]></ev.SelectedChanged-eval>
                      </RadioButton>
                    </StackItem>
                  </Stack>
                </ToolstripMenu>
              </att.Popup>
            </_>
            <_ ref.Name="columnCompanies" Text="Company" Size="150" TextProperty-eval="[ToString((cast DataGridItem^ $1).Company)]">
              <att.Sorter>[Sys::Compare(ToString($1.Company), ToString($2.Company))]</att.Sorter>
              <att.Filter-eval><![CDATA[
                new IDataFilter^
                {
                  override func SetCallback(value: IDataProcessorCallback*): void
                  {
                    self.filterByCompaniesCallback = value;
                  }
                  override func Filter(row: object): bool
                  {
                    if (radioMicrosoft.Selected)
                    {
                      return (cast DataGridItem^ row).Company == Companies::Microsoft;
                    }
                    else
                    {
                      return true;
                    }
                  }
                }
              ]]></att.Filter-eval>
              <att.Popup>
                <ToolstripMenu>
                  <Stack Direction="Vertical" Padding="5" AlignmentToParent="left:5 top:5 right:5 bottom:5" MinSizeLimitation="LimitToElementAndChildren">
                    <StackItem>
                      <RadioButton ref.Name="radioAllCompanies" Text="All" Selected="true" GroupController-ref="mutexCompanies">
                        <ev.SelectedChanged-eval><![CDATA[{
                          if (radioAllCompanies.Selected and self.filterByCompaniesCallback is not null)
                          {
                            self.filterByCompaniesCallback.OnProcessorChanged();
                          }
                        }]]></ev.SelectedChanged-eval>
                      </RadioButton>
                    </StackItem>
                    <StackItem>
                      <RadioButton ref.Name="radioMicrosoft" Text="Microsoft" GroupController-ref="mutexCompanies">
                        <ev.SelectedChanged-eval><![CDATA[{
                          if (radioMicrosoft.Selected and self.filterByCompaniesCallback is not null)
                          {
                            self.filterByCompaniesCallback.OnProcessorChanged();
                          }
                        }]]></ev.SelectedChanged-eval>
                      </RadioButton>
                    </StackItem>
                  </Stack>
                </ToolstripMenu>
              </att.Popup>
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
					ClickListViewColumn(protocol, dataGrid, 0);
				});
				protocol->OnNextIdleFrame(L"+Language", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 0);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					ClickListViewColumn(protocol, dataGrid, 0);
				});
				protocol->OnNextIdleFrame(L"-Language", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 0);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					ClickListViewColumn(protocol, dataGrid, 1);
				});
				protocol->OnNextIdleFrame(L"MP does not sort", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 0);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					ClickListViewColumn(protocol, dataGrid, 2);
				});
				protocol->OnNextIdleFrame(L"IDEs does not sort", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 0);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					ClickListViewColumn(protocol, dataGrid, 3);
				});
				protocol->OnNextIdleFrame(L"+Company", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 3);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == true);
					ClickListViewColumn(protocol, dataGrid, 3);
				});
				protocol->OnNextIdleFrame(L"-Company", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					TEST_ASSERT(dataGridView->GetSortedColumn() == 3);
					TEST_ASSERT(dataGridView->IsSortOrderAscending() == false);
					ClickListViewColumn(protocol, dataGrid, 3);
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
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/ColumnUI/SortByColumn"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});

		TEST_CASE(L"FilterByColumn")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					ClickListViewColumnDropdown(protocol, dataGrid, 2);
				});
				protocol->OnNextIdleFrame(L"Expand IDEs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto radio = FindControlByText<GuiSelectableButton>(dataGrid->GetColumns()[2]->GetPopup(), L"Multiple IDEs");
					auto location = protocol->LocationOf(radio);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Multiple IDEs", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto location = protocol->LocationOf(dataGrid, 1.0, 1.0);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Close Popup", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/ColumnUI/FilterByColumn"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});
	});
}
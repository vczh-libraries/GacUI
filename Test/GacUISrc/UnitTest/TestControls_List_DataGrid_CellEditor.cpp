#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const WString resourceDataGrid = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">DataGrid/DataGridImagesData.xml</Folder>
  <Folder name="DataGridImages" content="Link">DataGrid/DataGridImagesFolder.xml</Folder>
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
  <Instance name="CompanyEditorResource">
    <Instance ref.CodeBehind="false" ref.Class="gacuisrc_unittest::CompanyEditor" xmlns:demo="demo::*">
      <ref.Members>
        <![CDATA[
          var items : Companies[] = 
          {
            Microsoft of Companies
            Borland of Companies
            IBM of Companies
          };
        ]]>
      </ref.Members>
      <GridEditorTemplate ref.Name="self" CellValue-bind="comboBox.SelectedItem" FocusControl-ref="comboBox">
        <ComboBox ref.Name="comboBox" SelectedIndex-bind="self.items.IndexOf(self.CellValue)">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
          <att.ListControl>
            <BindableTextList ItemSource-eval="self.items" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
              <att.TextProperty-eval>[ToString(cast Companies $1)]</att.TextProperty-eval>
            </BindableTextList>
          </att.ListControl>
        </ComboBox>
      </GridEditorTemplate>
    </Instance>
  </Instance>
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
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableDataGrid" env.ItemType="DataGridItem^" ClientSize="x:640 y:320">
        <att.filterByIDEs>[$1.IDEs &gt; 1]</att.filterByIDEs>
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
            </_>
            <_ ref.Name="columnCompanies" Text="Company" Size="150" ValueProperty="Company">
              <att.TextProperty-eval>[ToString((cast DataGridItem^ $1).Company)]</att.TextProperty-eval>
              <att.EditorFactory>gacuisrc_unittest::CompanyEditor</att.EditorFactory>
              <att.Sorter>[Sys::Compare(ToString($1.Company), ToString($2.Company))]</att.Sorter>
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
		TEST_CASE(L"ComboEditor")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					LClickDataCell(protocol, dataGrid, 0, 3);
				});
				protocol->OnNextIdleFrame(L"Open Editor in First Row", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					TEST_ASSERT(dataGrid->GetOpenedEditor());
					auto combo = FindObjectByName<GuiComboBoxListControl>(dataGrid->GetOpenedEditor()->GetTemplate(), L"comboBox");
					auto location = protocol->LocationOf(combo);
					location.x.value += 1; // TODO: GuiButton::OnMouseEnter is not called when the editor is opened right under the mouse
					protocol->LClick(location);
					TEST_ASSERT(combo->GetSubMenuOpening() == true);
				});
				protocol->OnNextIdleFrame(L"Open Combo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					TEST_ASSERT(dataGrid->GetOpenedEditor());
					auto combo = FindObjectByName<GuiComboBoxListControl>(dataGrid->GetOpenedEditor()->GetTemplate(), L"comboBox");
					LClickListItem(protocol, combo->GetContainedListControl(), 2);
				});
				protocol->OnNextIdleFrame(L"Select IBM", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					LClickDataCell(protocol, dataGrid, 0, 0);
					TEST_ASSERT(!dataGrid->GetOpenedEditor());
				});
				protocol->OnNextIdleFrame(L"Exit Editor", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 0,3 }, true);
					TEST_ASSERT(dataGrid->GetOpenedEditor());
				});
				protocol->OnNextIdleFrame(L"Start Editing by SelectCell", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell(dataGrid->GetSelectedCell(), false);
					TEST_ASSERT(!dataGrid->GetOpenedEditor());
				});
				protocol->OnNextIdleFrame(L"Stop Editing by SelectCell", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/CellEditor/ComboEditor"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});

		TEST_CASE(L"ComboEditorWithSorterAndFilter")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SetAdditionalFilter(UnboxValue<Ptr<IDataFilter>>(Value::From(window).GetProperty(L"filterByIDEs")));
					ClickListViewColumn(protocol, dataGrid, 3);
				});
				protocol->OnNextIdleFrame(L"IDEs > 1 && Sort by Company", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SelectCell({ 1,3 }, true);
				});
				protocol->OnNextIdleFrame(L"Start Edit the Second Row", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					TEST_ASSERT(dataGrid->GetOpenedEditor());
					auto combo = FindObjectByName<GuiComboBoxListControl>(dataGrid->GetOpenedEditor()->GetTemplate(), L"comboBox");
					combo->SetSelectedIndex(2);
				});
				protocol->OnNextIdleFrame(L"Change from Microsoft to IBM", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SetAdditionalFilter(nullptr);
				});
				protocol->OnNextIdleFrame(L"Reset Filter", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					auto dataGridView = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					dataGridView->SortByColumn(-1, false);
				});
				protocol->OnNextIdleFrame(L"Reset Sorting", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
					dataGrid->SetAdditionalFilter(UnboxValue<Ptr<IDataFilter>>(Value::From(window).GetProperty(L"filterByIDEs")));
					ClickListViewColumn(protocol, dataGrid, 3);
				});
				protocol->OnNextIdleFrame(L"IDEs > 1 && Sort by Company", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/CellEditor/ComboEditorWithSorterAndFilter"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});
	});
}
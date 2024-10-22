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
  <Instance name="LanguageVisualizerResource">
    <Instance ref.CodeBehind="false" ref.Class="gacuisrc_unittest::LanguageVisualizer">
      <GridVisualizerTemplate ref.Name="self">
        <Stack Padding="2" Direction="Horizontal" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:2 top:0 right:2 bottom:0">
          <StackItem>
            <ImageFrame HorizontalAlignment="Left" VerticalAlignment="Center">
              <att.Image-bind>self.Text == "C++" ? (cast (GuiImageData^) self.ResolveResource("res", $"DataGridImages/Male", true)).Image : null</att.Image-bind>
            </ImageFrame>
          </StackItem>
          <StackItem>
            <SolidLabel Font-bind="self.Font" Text-bind="self.Text" Color-bind="self.PrimaryTextColor" HorizontalAlignment="Left" VerticalAlignment="Center"/>
          </StackItem>
        </Stack>
      </GridVisualizerTemplate>
    </Instance>
  </Instance>
  <Instance name="CompanyVisualizerResource">
    <Instance ref.CodeBehind="false" ref.Class="gacuisrc_unittest::CompanyVisualizer">
      <GridVisualizerTemplate ref.Name="self">
        <Stack Padding="2" Direction="Horizontal" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:2 top:0 right:2 bottom:0">
          <StackItem>
            <ImageFrame HorizontalAlignment="Left" VerticalAlignment="Center">
              <att.Image-bind>(cast Companies self.CellValue == Companies::Microsoft ? (cast (GuiImageData^) self.ResolveResource("res", $"DataGridImages/Female", true)).Image : null) ?? null</att.Image-bind>
            </ImageFrame>
          </StackItem>
          <StackItem>
            <SolidLabel Font-bind="self.Font" Text-bind="ToString((cast DataGridItem^ self.RowValue).Company) ?? ''" Color-bind="self.PrimaryTextColor" HorizontalAlignment="Left" VerticalAlignment="Center"/>
          </StackItem>
        </Stack>
      </GridVisualizerTemplate>
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
              <att.VisualizerFactory>gacuisrc_unittest::LanguageVisualizer;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate</att.VisualizerFactory>
              <att.Sorter>[Sys::Compare($1.Language, $2.Language)]</att.Sorter>
            </_>
            <_ Text="Meta Programming" Size="150" TextProperty="MetaProgramming">
            </_>
            <_ Text="IDE Count" Size="150" TextProperty="IDEs">
            </_>
            <_ ref.Name="columnCompanies" Text="Company" Size="150" ValueProperty="Company">
              <att.VisualizerFactory>gacuisrc_unittest::CompanyVisualizer;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate</att.VisualizerFactory>
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
		TEST_CASE(L"SorterAndFilter")
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
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/CellVisualizer/SorterAndFilter"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});
	});
}
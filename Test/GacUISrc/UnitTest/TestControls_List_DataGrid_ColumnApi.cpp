#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const WString resourceDataGrid = LR"GacUISrc(
<Resource>
  <Script name="DataGridItemResource"><Workflow><![CDATA[
    module datagriditem;
    using system::*;

    enum Companys
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
      prop Company:Companys = Microsoft {not observe}

      new(){}
      new(language:string, metaProgramming:bool, ides:int, company:Companys)
      {
        Language=language;
        MetaProgramming=metaProgramming;
        IDEs=ides;
        Company=company;
      }
    }

    func ToString(value:Companys) : string
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
          new DataGridItem^("Object Pascal", false, 1, Borland);
          new DataGridItem^("Java", false, 3, IBM);
        };
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableDataGrid" ClientSize="x:640 y:320">
        <BindableDataGrid ref.Name="dataGrid" env.ItemType="DataGridItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.ItemSource-eval>self.items</att.ItemSource-eval>
          <att.Columns>
            <_ Text="Language" Size="150" TextProperty="Language"/>
            <_ Text="Meta Programming" Size="150" TextProperty="MetaProgramming"/>
            <_ Text="IDE Count" Size="150" TextProperty="IDEs"/>
            <_ Text="Company" Size="150" TextProperty-eval="[ToString((cast DataGridItem^ $1).Company)]"/>
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
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/ColumnApi/SortByColumn"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});
	});
}
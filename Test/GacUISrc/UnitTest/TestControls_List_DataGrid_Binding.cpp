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

	const WString resourceDataGridMixedProperty = LR"GacUISrc(
<Resource>
  <Script name="DataGridItemResource"><Workflow><![CDATA[
    module datagriditem;
    using system::*;

    class DataGridItem
    {
      prop Language:string = "" {not observe}
      prop MetaProgramming:bool = false {not observe}
      prop IDEs:int = 0 {not observe}

      new(){}
      new(language:string, metaProgramming:bool, ides:int)
      {
        Language=language;
        MetaProgramming=metaProgramming;
        IDEs=ides;
      }
    }
  ]]></Workflow></Script>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items : observe DataGridItem^[] = {
          new DataGridItem^("C++", true, 4);
          new DataGridItem^("C#", false, 3);
          new DataGridItem^("F#", false, 2);
          new DataGridItem^("TypeScript", true, 1);
          new DataGridItem^("Java", false, 3);
        };
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableDataGrid" ClientSize="x:640 y:320">
        <BindableDataGrid ref.Name="dataGrid" env.ItemType="DataGridItem^" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.ItemSource-eval>self.items</att.ItemSource-eval>
          <att.Columns>
            <_ Text="Language" TextProperty="Language"/>
            <_ Text="Meta Programming" TextProperty="MetaProgramming"/>
            <_ Text="IDE Count" TextProperty="IDEs"/>
          </att.Columns>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";
	
	const WString resourceDataGridEnumProperty = LR"GacUISrc(
<Resource>
  <Script name="DataGridItemResource"><Workflow><![CDATA[
    module datagriditem;
    using system::*;

    enum IDEs
    {
      VisualStudio=0,
      VSCode=1,
      Eclipse=2,
    }

    enum compare
    {
      Microsoft=0,
      IBM=1,
    }

    class DataGridItem
    {
      prop Language:string = "" {not observe}
      prop IDE:IDEs = VisualStudio {not observe}
      prop Company:compare = Microsoft {not observe}

      new(){}
      new(language:string)
      {
        Language=language;
      }
      new(language:string, ide:IDEs, company:compare)
      {
        Language=language;
        IDE=ide;
        Company=company;
      }
    }

    func ToString(value:IDEs) : string
    {
      switch(value)
      {
        case VisualStudio: { return "Visual Studio"; }
        case VSCode: { return "Visual Studio Code"; }
        case Eclipse: { return "Eclipse"; }
        default: { return "<ERROR>"; }
      }
    }

    func ToString(value:compare) : string
    {
      switch(value)
      {
        case Microsoft: { return "Microsoft"; }
        case IBM: { return "IBM"; }
        default: { return "<ERROR>"; }
      }
    }
  ]]></Workflow></Script>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var items : observe DataGridItem^[] = {
          new DataGridItem^("C++");
          new DataGridItem^("C#");
          new DataGridItem^("F#");
          new DataGridItem^("TypeScript", VSCode, Microsoft);
          new DataGridItem^("Java", Eclipse, IBM);
        };
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableDataGrid" ClientSize="x:640 y:320">
        <BindableDataGrid ref.Name="dataGrid" env.ItemType="DataGridItem^" env.ItemName="itemToBind" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.ItemSource-eval>self.items</att.ItemSource-eval>
          <att.Columns>
            <_ Text="Language" TextProperty="Language"/>
            <_ Text="IDE" TextProperty="ToString(itemToBind.IDE)"/>
            <_ Text="Company" TextProperty-eval="[ToString((cast DataGridItem^ $1).Company)]"/>
          </att.Columns>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiBindableDataGrid")
	{
		TEST_CASE(L"DisplayStringProperties")
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
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Binding/DisplayStringProperties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridStringProperty
				);
		});
		
		TEST_CASE(L"DisplayMixedProperties")
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
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Binding/DisplayMixedProperties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridMixedProperty
				);
		});

		TEST_CASE(L"DisplayEmumProperties")
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
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Binding/DisplayEmumProperties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGridEnumProperty
				);
		});
	});

	TEST_CASE(L"GuiBindableDataGrid/PropertyBinding")
	{
		GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto listControl = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
				listControl->SetItemSource(nullptr);
			});
			protocol->OnNextIdleFrame(L"Reset ItemSource", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto listControl = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
				auto items = UnboxValue<Ptr<IValueEnumerable>>(Value::From(window).GetProperty(L"items"));
				listControl->SetItemSource(items);
			});
			protocol->OnNextIdleFrame(L"Set ItemSource", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto listControl = FindObjectByName<GuiBindableDataGrid>(window, L"dataGrid");
				listControl->GetColumns()[0]->SetTextProperty([](const Value& value)
				{
					auto text = UnboxValue<WString>(value.GetProperty(L"Language"));
					return text + L"*";
				});
			});
			protocol->OnNextIdleFrame(L"Change TextProperty on Language", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Binding/PropertyBinding"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceDataGridStringProperty
			);
	});
}
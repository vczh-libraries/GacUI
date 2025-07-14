#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTabNavigate = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="TabNavigate" ClientSize="x:480 y:320">
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren" CellPadding="5" BorderVisible="true">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          
          <Cell Site="row:0 column:0">
            <Button ref.Name="button1" Text="First">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </Button>
          </Cell>
          
          <Cell Site="row:1 column:0">
            <Button ref.Name="button2" Text="Second">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </Button>
          </Cell>
          
          <Cell Site="row:2 column:0">
            <Button ref.Name="button3" Text="Third">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Application")
	{
		TEST_CASE(L"TabNavigate")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");
					
					button1->SetFocused();
					TEST_ASSERT(button1->GetFocused());
				});
				protocol->OnNextIdleFrame(L"Focused button1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button2 = FindObjectByName<GuiButton>(window, L"button2");
					
					protocol->KeyPress(VKEY::KEY_TAB, false, false, false);
					TEST_ASSERT(button2->GetFocused());
				});
				protocol->OnNextIdleFrame(L"[TAB]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button3 = FindObjectByName<GuiButton>(window, L"button3");
					
					protocol->KeyPress(VKEY::KEY_TAB, false, false, false);
					TEST_ASSERT(button3->GetFocused());
				});
				protocol->OnNextIdleFrame(L"[TAB]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");
					
					protocol->KeyPress(VKEY::KEY_TAB, false, false, false);
					TEST_ASSERT(button1->GetFocused());
				});
				protocol->OnNextIdleFrame(L"[TAB]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button3 = FindObjectByName<GuiButton>(window, L"button3");
					
					protocol->KeyPress(VKEY::KEY_TAB, false, true, false);
					TEST_ASSERT(button3->GetFocused());
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[TAB]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button2 = FindObjectByName<GuiButton>(window, L"button2");
					
					protocol->KeyPress(VKEY::KEY_TAB, false, true, false);
					TEST_ASSERT(button2->GetFocused());
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[TAB]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");
					
					protocol->KeyPress(VKEY::KEY_TAB, false, true, false);
					TEST_ASSERT(button1->GetFocused());
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[TAB]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/TabNavigate"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabNavigate
			);
		});
	});
}
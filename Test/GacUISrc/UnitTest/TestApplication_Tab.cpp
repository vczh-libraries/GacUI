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

	const auto resourceTabNavigateWithContainer = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="TabNavigateWithContainer" ClientSize="x:480 y:320">
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren" CellPadding="5" BorderVisible="true">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
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
          
          <Cell Site="row:0 column:1 rowSpan:4">
            <GroupBox Text="Group 4th">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:-1"/>
              <Table AlignmentToParent="left:5 top:5 right:5 bottom:5" MinSizeLimitation="LimitToElementAndChildren" CellPadding="2" BorderVisible="false">
                <att.Rows>
                  <_>composeType:MinSize</_>
                  <_>composeType:MinSize</_>
                  <_>composeType:Percentage percentage:1.0</_>
                </att.Rows>
                <att.Columns>
                  <_>composeType:Percentage percentage:1.0</_>
                </att.Columns>
                
                <Cell Site="row:0 column:0">
                  <Button ref.Name="button4" Text="Fourth">
                    <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
                  </Button>
                </Cell>
                
                <Cell Site="row:1 column:0">
                  <GroupBox Text="Group 5th">
                    <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
                    <Button ref.Name="button5" Text="Fifth">
                      <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
                    </Button>
                  </GroupBox>
                </Cell>
              </Table>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Application")
	{
		auto navibateButtons = []<typename TLastCallback>(UnitTestRemoteProtocol* protocol, const WString& firstFrame, vint count, TLastCallback && lastCallback)
		{
			protocol->OnNextIdleFrame(firstFrame, [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button1 = FindObjectByName<GuiButton>(window, L"button1");
				
				button1->SetFocused();
				TEST_ASSERT(button1->GetFocused());
			});

			for (vint i = 0; i < count; i++)
			{
				protocol->OnNextIdleFrame((i == 0 ? L"Focused button1" : L"[TAB]"), [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button" + itow((i + 1) % count + 1));

					protocol->KeyPress(VKEY::KEY_TAB, false, false, false);
					TEST_ASSERT(button->GetFocused());
				});
			}

			for (vint i = 0; i < count; i++)
			{
				protocol->OnNextIdleFrame((i == 0 ? L"[TAB]" : L"[SHIFT]+[TAB]"), [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button" + itow(count - i));

					protocol->KeyPress(VKEY::KEY_TAB, false, true, false);
					TEST_ASSERT(button->GetFocused());
				});
			}

			protocol->OnNextIdleFrame(L"[SHIFT]+[TAB]", std::forward<TLastCallback&&>(lastCallback));
		};

		TEST_CASE(L"TabNavigate")
		{
			GacUIUnitTest_SetGuiMainProxy([&navibateButtons](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				navibateButtons(protocol, L"Ready", 3, [=]()
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

		TEST_CASE(L"TabNavigateWithContainer")
		{
			GacUIUnitTest_SetGuiMainProxy([&navibateButtons](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				navibateButtons(protocol, L"Ready", 5, [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/TabNavigateWithContainer"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabNavigateWithContainer
			);
		});

		TEST_CASE(L"FocusedAndDisable")
		{
			GacUIUnitTest_SetGuiMainProxy([&navibateButtons](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
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
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");

					button1->SetEnabled(false);
				});
				protocol->OnNextIdleFrame(L"Deleted button1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/FocusedAndDisable"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabNavigate
			);
		});

		TEST_CASE(L"FocusedAndHide")
		{
			GacUIUnitTest_SetGuiMainProxy([&navibateButtons](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
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
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");

					button1->SetVisible(false);
				});
				protocol->OnNextIdleFrame(L"Deleted button1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/FocusedAndHide"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabNavigate
			);
		});

		TEST_CASE(L"FocusedAndDelete")
		{
			GacUIUnitTest_SetGuiMainProxy([&navibateButtons](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
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
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");

					SafeDeleteControl(button1);
				});
				protocol->OnNextIdleFrame(L"Deleted button1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/FocusedAndDelete"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabNavigate
			);
		});
	});
}
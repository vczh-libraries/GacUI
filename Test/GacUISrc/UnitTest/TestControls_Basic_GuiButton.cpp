#include "../../../Source//UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;

TEST_FILE
{
	const auto resourceSingleButton = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiButton" ClientSize="x:320 y:240">
        <Button ref.Name="button" Text="This is a Button">
          <ev.Clicked-eval><![CDATA[{
            button.Text = "Clicked!";
          }]]></ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceTwoButtons = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiButton" ClientSize="x:320 y:240">
        <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <StackItem>
            <Button ref.Name="button1" Text="This is Button 1">
              <ev.Clicked-eval><![CDATA[{
                button1.Text = "Clicked!";
              }]]></ev.Clicked-eval>
            </Button>
          </StackItem>
          <StackItem>
            <Button ref.Name="button2" Text="This is Button 2" ClickOnMouseUp="false">
              <ev.Clicked-eval><![CDATA[{
                button2.Text = "Clicked!";
              }]]></ev.Clicked-eval>
            </Button>
          </StackItem>
        </Stack>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiButton")
	{
		TEST_CASE(L"ClickOnMouseUp")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					TEST_ASSERT(button->GetAutoFocus() == true);
					TEST_ASSERT(button->GetClickOnMouseUp() == true);
					button->SetAutoFocus(false);
					auto location = protocol->LocationOf(button);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->_LDown(location);
				});
				protocol->OnNextIdleFrame(L"MouseDown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->_LUp(location);
				});
				protocol->OnNextIdleFrame(L"MouseUp", [=]()
				{
					protocol->MouseMove({ 0,0 });
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiButton/ClickOnMouseUp"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceSingleButton
				);
		});

		TEST_CASE(L"ClickOnMouseUp")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					TEST_ASSERT(button->GetAutoFocus() == true);
					TEST_ASSERT(button->GetClickOnMouseUp() == true);
					button->SetAutoFocus(false);
					button->SetClickOnMouseUp(false);
					TEST_ASSERT(button->GetClickOnMouseUp() == false);
					auto location = protocol->LocationOf(button);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->_LDown(location);
				});
				protocol->OnNextIdleFrame(L"MouseDown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->_LUp(location);
				});
				protocol->OnNextIdleFrame(L"MouseUp", [=]()
				{
					protocol->MouseMove({ 0,0 });
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiButton/ClickOnMouseDown"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceSingleButton
				);
		});

		TEST_CASE(L"AutoFocus")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button1");
					auto location = protocol->LocationOf(button);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button1");
					auto location = protocol->LocationOf(button);
					protocol->_LDown(location);
				});
				protocol->OnNextIdleFrame(L"MouseDown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button1");
					auto location = protocol->LocationOf(button);
					protocol->_LUp(location);
				});
				protocol->OnNextIdleFrame(L"MouseUp", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button2");
					auto location = protocol->LocationOf(button);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button2");
					auto location = protocol->LocationOf(button);
					protocol->_LDown(location);
				});
				protocol->OnNextIdleFrame(L"MouseDown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button2");
					auto location = protocol->LocationOf(button);
					protocol->_LUp(location);
				});
				protocol->OnNextIdleFrame(L"MouseUp", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->MouseMove({ 0,0 });
				});
				protocol->OnNextIdleFrame(L"Close", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiButton/AutoFocus"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTwoButtons
				);
		});
	});
}
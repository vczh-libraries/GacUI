#include "TestControls.h"

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

	const auto resourceNestedButtons = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiButton" ClientSize="x:320 y:240">
        <Button ref.Name="button1">
          <ev.Clicked-eval><![CDATA[{
            self.Text = "Clicked!";
          }]]></ev.Clicked-eval>
          <Button ref.Name="button2" Text="This is a Button">
            <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:10 bottom:10"/>
            <ev.Clicked-eval><![CDATA[{
              button2.Text = "Clicked!";
            }]]></ev.Clicked-eval>
          </Button>
        </Button>
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

		TEST_CASE(L"ClickOnMouseDown")
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

		TEST_CASE(L"Press Enter")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					button->SetFocused();
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Pressed");
					protocol->_KeyDown(VKEY::KEY_RETURN);
				});
				protocol->OnNextIdleFrame(L"Pressed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Released");
					protocol->_KeyUp(VKEY::KEY_RETURN);
				});
				protocol->OnNextIdleFrame(L"Released", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiButton/PressEnter"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceSingleButton
				);
		});

		TEST_CASE(L"Press Space")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					button->SetFocused();
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Pressed");
					protocol->_KeyDown(VKEY::KEY_SPACE);
				});
				protocol->OnNextIdleFrame(L"Pressed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Released");
					protocol->_KeyUp(VKEY::KEY_SPACE);
				});
				protocol->OnNextIdleFrame(L"Released", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiButton/PressSpace"),
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

		TEST_CASE(L"IgnoreChildControlMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");
					auto button2 = FindObjectByName<GuiButton>(window, L"button2");
					TEST_ASSERT(button1->GetIgnoreChildControlMouseEvents() == true);
					auto location = protocol->LocationOf(button2);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Clicked", [=]()
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
				WString::Unmanaged(L"Controls/Basic/GuiButton/IgnoreChildControlMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceNestedButtons
				);
		});

		TEST_CASE(L"AcknowledgeChildControlMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiButton>(window, L"button1");
					auto button2 = FindObjectByName<GuiButton>(window, L"button2");
					TEST_ASSERT(button1->GetIgnoreChildControlMouseEvents() == true);
					button1->SetIgnoreChildControlMouseEvents(false);
					TEST_ASSERT(button1->GetIgnoreChildControlMouseEvents() == false);
					auto location = protocol->LocationOf(button2);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Clicked", [=]()
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
				WString::Unmanaged(L"Controls/Basic/GuiButton/AcknowledgeChildControlMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceNestedButtons
				);
		});

		TEST_CASE(L"Disabled")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Hover");
					auto button = FindObjectByName<GuiButton>(window, L"button");
					TEST_ASSERT(button->GetEnabled() == true);
					button->SetEnabled(false);
					TEST_ASSERT(button->GetEnabled() == false);
					auto location = protocol->LocationOf(button);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"MouseDown");
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->_LDown(location);
				});
				protocol->OnNextIdleFrame(L"MouseDown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"MouseUp");
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->_LUp(location);
				});
				protocol->OnNextIdleFrame(L"MouseUp", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Focused");
					auto button = FindObjectByName<GuiButton>(window, L"button");
					button->SetEnabled(true);
					button->SetFocused();
					protocol->MouseMove({ 0,0 });
				});
				protocol->OnNextIdleFrame(L"Re-enabled and Focused", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Disabled");
					auto button = FindObjectByName<GuiButton>(window, L"button");
					button->SetEnabled(false);
				});
				protocol->OnNextIdleFrame(L"Disabled", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetText(L"Pressed");
					protocol->KeyPress(VKEY::KEY_RETURN);
				});
				protocol->OnNextIdleFrame(L"Pressed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiButton/Disabled"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceSingleButton
				);
		});
	});

	const auto resourceTwoCheckBoxes = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSelectableButton" ClientSize="x:320 y:240">
        <GroupBox Text="Options">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:-1 bottom:-1"/>
          <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:5 bottom:5">
            <StackItem>
              <CheckBox ref.Name="button1" Text="Option 1"/>
            </StackItem>
            <StackItem>
              <CheckBox ref.Name="button2" Text="Option 2"/>
            </StackItem>
          </Stack>
        </GroupBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceTwoRadioButtons = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow" xmlns:x="presentation::controls::GuiSelectableButton::*">
      <Window ref.Name="self" Text="GuiSelectableButton" ClientSize="x:320 y:240">
        <x:MutexGroupController ref.Name="mutex"/>
        <GroupBox Text="Options">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:-1 bottom:-1"/>
          <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:5 bottom:5">
            <StackItem>
              <RadioButton ref.Name="button1" Text="Option 1" GroupController-ref="mutex"/>
            </StackItem>
            <StackItem>
              <RadioButton ref.Name="button2" Text="Option 2" GroupController-ref="mutex"/>
            </StackItem>
          </Stack>
        </GroupBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceFourRadioButtons = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow" xmlns:x="presentation::controls::GuiSelectableButton::*">
      <Window ref.Name="self" Text="GuiSelectableButton" ClientSize="x:320 y:240">
        <x:MutexGroupController ref.Name="mutex1"/>
        <x:MutexGroupController ref.Name="mutex2"/>
        <Stack Direction="Horizontal" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:0 top:5 right:-1 bottom:-1">
          <StackItem>
            <GroupBox Text="Opt1">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:5 bottom:5">
                <StackItem>
                  <RadioButton ref.Name="button1" Text="Option 1" GroupController-ref="mutex1"/>
                </StackItem>
                <StackItem>
                  <RadioButton ref.Name="button2" Text="Option 2" GroupController-ref="mutex1"/>
                </StackItem>
              </Stack>
            </GroupBox>
          </StackItem>
          <StackItem>
            <GroupBox Text="Opt2">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:5 bottom:5">
                <StackItem>
                  <RadioButton ref.Name="button3" Text="Option 1" GroupController-ref="mutex2"/>
                </StackItem>
                <StackItem>
                  <RadioButton ref.Name="button4" Text="Option 2" GroupController-ref="mutex2"/>
                </StackItem>
              </Stack>
            </GroupBox>
          </StackItem>
        </Stack>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSelectableButton")
	{
		TEST_CASE(L"AutoSelection")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiSelectableButton>(window, L"button1");
					auto button2 = FindObjectByName<GuiSelectableButton>(window, L"button2");
					TEST_ASSERT(button1->GetAutoSelection() == true);
					TEST_ASSERT(button2->GetAutoSelection() == true);
					TEST_ASSERT(button1->GetSelected() == false);
					TEST_ASSERT(button2->GetSelected() == false);
					button2->SetAutoSelection(false);
					TEST_ASSERT(button2->GetAutoSelection() == false);

					button1->SetSelected(true);
					button2->SetSelected(true);
					TEST_ASSERT(button1->GetSelected() == true);
					TEST_ASSERT(button2->GetSelected() == true);
				});
				protocol->OnNextIdleFrame(L"Both Selected", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button1 = FindObjectByName<GuiSelectableButton>(window, L"button1");
					auto button2 = FindObjectByName<GuiSelectableButton>(window, L"button2");
					button1->SetSelected(false);
					button2->SetSelected(false);
					TEST_ASSERT(button1->GetSelected() == false);
					TEST_ASSERT(button2->GetSelected() == false);
				});
				protocol->OnNextIdleFrame(L"Both Unselected", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button1");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button2");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiSelectableButton/AutoSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTwoCheckBoxes
				);
		});

		TEST_CASE(L"MutexSelection")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button1");
					button->SetSelected(true);
				});
				protocol->OnNextIdleFrame(L"Select Option 1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button2");
					button->SetSelected(true);
				});
				protocol->OnNextIdleFrame(L"Select Option 2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button1");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button2");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiSelectableButton/MutexSelection"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTwoRadioButtons
				);
		});

		TEST_CASE(L"MutexGroup")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button1");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button2");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 2", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button3");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiSelectableButton>(window, L"button4");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Option 4", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiSelectableButton/MutexGroup"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFourRadioButtons
				);
		});
	});
}
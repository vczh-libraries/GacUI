#include "TestControls.h"

TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MyControlTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::MyControlTemplate">
      <ControlTemplate ref.Name="self" ContainerComposition-ref="container" MinSizeLimitation="LimitToElementAndChildren">
        <SolidBorder Color="#FFFFFF"/>
        <Table CellPadding="5" BorderVisible="true" AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0 columnSpan:2"> <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.Text"/>                                     </Cell>
          <Cell Site="row:1 column:0 columnSpan:2"> <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.Font.fontFamily"/>                          </Cell>
          <Cell Site="row:2 column:0">              <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.VisuallyEnabled ? 'Enabled' : 'Disabled'"/> </Cell>
          <Cell Site="row:3 column:0">              <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.Focused ? 'Focused' : 'Unfocused'"/>        </Cell>

          <Cell Site="row:2 column:1 rowSpan:3">
            <SolidBorder Color="#FFFFFF"/>
            <Bounds ref.Name="container" AlignmentToParent="left:1 top:1 right:1 bottom:1" MinSizeLimitation="LimitToElementAndChildren"/>
          </Cell>
        </Table>
      </ControlTemplate>
    </Instance>
  </Instance>

  <Instance name="MyControlResource">
    <Instance ref.Class="gacuisrc_unittest::MyControl" xmlns:ut="gacuisrc_unittest::*">
      <CustomControl ref.Name="self" FocusableComposition-eval="self.BoundsComposition">
        <att.ControlTemplate>ut:MyControlTemplate</att.ControlTemplate>
      </CustomControl>
    </Instance>
  </Instance>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow" xmlns:ut="gacuisrc_unittest::*">
      <Window ref.Name="self" Text="MyControlTemplate" ClientSize="x:320 y:240">
        <att.ContainerComposition-set InternalMargin="left:0 top:5 right:0 bottom:0"/>
        <ut:MyControl ref.Name="my" Text="This is a control with a template"/>
      </Window>
    </Instance>
  </Instance>

  <Instance name="MainWindow2Resource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow2" xmlns:ut="gacuisrc_unittest::*">
      <Window ref.Name="self" Text="MyControlTemplate" ClientSize="x:320 y:240">
        <att.ContainerComposition-set InternalMargin="left:0 top:5 right:0 bottom:0"/>
        <ut:MyControl ref.Name="a" Text="A" Alt="A">
          <Stack Direction="Vertical" ExtraMargin="left:5 top:5 right:5 bottom:5" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:0 top:0 right:0 bottom:0">
            <StackItem>
              <ut:MyControl ref.Name="b" Text="B">
                <ut:MyControl ref.Name="c" Text="C">
                  <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                </ut:MyControl>
              </ut:MyControl>
            </StackItem>
            <StackItem>
              <ut:MyControl ref.Name="d" Text="D">
                <ut:MyControl ref.Name="e" Text="E">
                  <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                </ut:MyControl>
              </ut:MyControl>
            </StackItem>
          </Stack>
        </ut:MyControl>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"Control Template")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				auto font = my->GetDisplayFont();
				font.fontFamily = L"Another Font";
				TEST_ASSERT(my->GetDisplayFont() == window->GetDisplayFont());
				my->SetFont(font);
				TEST_ASSERT(my->GetDisplayFont() == font);
			});
			protocol->OnNextIdleFrame(L"Changed font", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				TEST_ASSERT(my->GetEnabled() == true);
				TEST_ASSERT(my->GetVisuallyEnabled() == true);
				my->SetEnabled(false);
				TEST_ASSERT(my->GetEnabled() == false);
				TEST_ASSERT(my->GetVisuallyEnabled() == false);
			});
			protocol->OnNextIdleFrame(L"Disabled", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				my->SetEnabled(true);
				window->SetEnabled(false);
				TEST_ASSERT(my->GetEnabled() == true);
				TEST_ASSERT(my->GetVisuallyEnabled() == false);
			});
			protocol->OnNextIdleFrame(L"Disabled window", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				window->SetEnabled(true);
				TEST_ASSERT(my->GetFocused() == false);
				my->SetFocused();
				TEST_ASSERT(my->GetFocused() == true);
			});
			protocol->OnNextIdleFrame(L"Focused", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/MyControlTemplate"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});

	TEST_CASE(L"Visible")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");

				a->SetVisible(false);
				TEST_ASSERT(a->GetVisible() == false);
			});
			protocol->OnNextIdleFrame(L"Hide A", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");

				a->SetVisible(true);
				b->SetVisible(false);
				TEST_ASSERT(a->GetVisible() == true);
				TEST_ASSERT(b->GetVisible() == false);
			});
			protocol->OnNextIdleFrame(L"Hide B", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");

				b->SetVisible(true);
				c->SetVisible(false);
				TEST_ASSERT(b->GetVisible() == true);
				TEST_ASSERT(c->GetVisible() == false);
			});
			protocol->OnNextIdleFrame(L"Hide C", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");

				c->SetVisible(true);
				d->SetVisible(false);
				TEST_ASSERT(c->GetVisible() == true);
				TEST_ASSERT(d->GetVisible() == false);
			});
			protocol->OnNextIdleFrame(L"Hide D", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				d->SetVisible(true);
				e->SetVisible(false);
				TEST_ASSERT(d->GetVisible() == true);
				TEST_ASSERT(e->GetVisible() == false);
			});
			protocol->OnNextIdleFrame(L"Hide E", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/Visible"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow2"),
			resource
			);
	});

	TEST_CASE(L"Enabled and VisuallyEnabled")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				a->SetEnabled(false);
				TEST_ASSERT(a->GetEnabled() == false);
				TEST_ASSERT(b->GetEnabled() == true);
				TEST_ASSERT(c->GetEnabled() == true);
				TEST_ASSERT(d->GetEnabled() == true);
				TEST_ASSERT(e->GetEnabled() == true);
				TEST_ASSERT(a->GetVisuallyEnabled() == false);
				TEST_ASSERT(b->GetVisuallyEnabled() == false);
				TEST_ASSERT(c->GetVisuallyEnabled() == false);
				TEST_ASSERT(d->GetVisuallyEnabled() == false);
				TEST_ASSERT(e->GetVisuallyEnabled() == false);
			});
			protocol->OnNextIdleFrame(L"Disable A", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				a->SetEnabled(true);
				b->SetEnabled(false);
				e->SetEnabled(false);
				TEST_ASSERT(a->GetEnabled() == true);
				TEST_ASSERT(b->GetEnabled() == false);
				TEST_ASSERT(c->GetEnabled() == true);
				TEST_ASSERT(d->GetEnabled() == true);
				TEST_ASSERT(e->GetEnabled() == false);
				TEST_ASSERT(a->GetVisuallyEnabled() == true);
				TEST_ASSERT(b->GetVisuallyEnabled() == false);
				TEST_ASSERT(c->GetVisuallyEnabled() == false);
				TEST_ASSERT(d->GetVisuallyEnabled() == true);
				TEST_ASSERT(e->GetVisuallyEnabled() == false);
			});
			protocol->OnNextIdleFrame(L"Disable B/E", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				b->SetEnabled(true);
				d->SetEnabled(false);
				TEST_ASSERT(a->GetEnabled() == true);
				TEST_ASSERT(b->GetEnabled() == true);
				TEST_ASSERT(c->GetEnabled() == true);
				TEST_ASSERT(d->GetEnabled() == false);
				TEST_ASSERT(e->GetEnabled() == false);
				TEST_ASSERT(a->GetVisuallyEnabled() == true);
				TEST_ASSERT(b->GetVisuallyEnabled() == true);
				TEST_ASSERT(c->GetVisuallyEnabled() == true);
				TEST_ASSERT(d->GetVisuallyEnabled() == false);
				TEST_ASSERT(e->GetVisuallyEnabled() == false);
			});
			protocol->OnNextIdleFrame(L"Disable D/E", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/Enable"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow2"),
			resource
			);
	});

	TEST_CASE(L"Font and DisplayFont")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				auto font = window->GetDisplayFont();
				font.fontFamily = L"Font A";
				a->SetFont(font);
				font.fontFamily = L"Font B";
				b->SetFont(font);
				font.fontFamily = L"Font E";
				e->SetFont(font);

				TEST_ASSERT(a->GetFont() == true);
				TEST_ASSERT(b->GetFont() == true);
				TEST_ASSERT(c->GetFont() == false);
				TEST_ASSERT(d->GetFont() == false);
				TEST_ASSERT(e->GetFont() == true);

				TEST_ASSERT(a->GetFont().Value().fontFamily == L"Font A");
				TEST_ASSERT(b->GetFont().Value().fontFamily == L"Font B");
				TEST_ASSERT(e->GetFont().Value().fontFamily == L"Font E");

				TEST_ASSERT(a->GetDisplayFont().fontFamily == L"Font A");
				TEST_ASSERT(b->GetDisplayFont().fontFamily == L"Font B");
				TEST_ASSERT(c->GetDisplayFont().fontFamily == L"Font B");
				TEST_ASSERT(d->GetDisplayFont().fontFamily == L"Font A");
				TEST_ASSERT(e->GetDisplayFont().fontFamily == L"Font E");
			});
			protocol->OnNextIdleFrame(L"Change fonts", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/Font"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow2"),
			resource
			);
	});

	TEST_CASE(L"Focus")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				a->SetFocused();
				TEST_ASSERT(a->GetFocused() == true);
				TEST_ASSERT(b->GetFocused() == false);
				TEST_ASSERT(c->GetFocused() == false);
				TEST_ASSERT(d->GetFocused() == false);
				TEST_ASSERT(e->GetFocused() == false);
			});
			protocol->OnNextIdleFrame(L"Focus A", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				b->SetFocused();
				TEST_ASSERT(a->GetFocused() == false);
				TEST_ASSERT(b->GetFocused() == true);
				TEST_ASSERT(c->GetFocused() == false);
				TEST_ASSERT(d->GetFocused() == false);
				TEST_ASSERT(e->GetFocused() == false);
			});
			protocol->OnNextIdleFrame(L"Focus B", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				c->SetFocused();
				TEST_ASSERT(a->GetFocused() == false);
				TEST_ASSERT(b->GetFocused() == false);
				TEST_ASSERT(c->GetFocused() == true);
				TEST_ASSERT(d->GetFocused() == false);
				TEST_ASSERT(e->GetFocused() == false);
			});
			protocol->OnNextIdleFrame(L"Focus C", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				d->SetFocused();
				TEST_ASSERT(a->GetFocused() == false);
				TEST_ASSERT(b->GetFocused() == false);
				TEST_ASSERT(c->GetFocused() == false);
				TEST_ASSERT(d->GetFocused() == true);
				TEST_ASSERT(e->GetFocused() == false);
			});
			protocol->OnNextIdleFrame(L"Focus D", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				e->SetFocused();
				TEST_ASSERT(a->GetFocused() == false);
				TEST_ASSERT(b->GetFocused() == false);
				TEST_ASSERT(c->GetFocused() == false);
				TEST_ASSERT(d->GetFocused() == false);
				TEST_ASSERT(e->GetFocused() == true);
			});
			protocol->OnNextIdleFrame(L"Focus E", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/Focus"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow2"),
			resource
			);
	});

	TEST_CASE(L"Alt with Labels")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				protocol->KeyPress(VKEY::KEY_MENU);
			});
			protocol->OnNextIdleFrame(L"[ALT]", [=]()
			{
				protocol->KeyPress(VKEY::KEY_ESCAPE);
			});
			protocol->OnNextIdleFrame(L"[ESC]", [=]()
			{
				protocol->KeyPress(VKEY::KEY_MENU);
			});
			protocol->OnNextIdleFrame(L"[ALT]", [=]()
			{
				protocol->KeyPress(VKEY::KEY_A);
			});
			protocol->OnNextIdleFrame(L"[A]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/AltLabel"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow2"),
			resource
			);
	});

	TEST_CASE(L"Alt")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto a = FindObjectByName<GuiControl>(window, L"a");
				auto b = FindObjectByName<GuiControl>(window, L"b");
				auto c = FindObjectByName<GuiControl>(window, L"c");
				auto d = FindObjectByName<GuiControl>(window, L"d");
				auto e = FindObjectByName<GuiControl>(window, L"e");

				protocol->KeyPress(VKEY::KEY_MENU);
				protocol->KeyPress(VKEY::KEY_A);
				TEST_ASSERT(a->GetFocused() == true);
				TEST_ASSERT(b->GetFocused() == false);
				TEST_ASSERT(c->GetFocused() == false);
				TEST_ASSERT(d->GetFocused() == false);
				TEST_ASSERT(e->GetFocused() == false);
			});
			protocol->OnNextIdleFrame(L"Focus A", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/AltFocus"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow2"),
			resource
			);
	});

	const auto resourceContext = LR"GacUISrc(
<Resource>
  <Instance name="MyControlResource">
    <Instance ref.Class="gacuisrc_unittest::MyControl">
      <CustomControl ref.Name="self">
        <Label Text-bind="(cast string self.Context) ?? '&lt;null&gt;'"/>
      </CustomControl>
    </Instance>
  </Instance>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow" xmlns:ut="gacuisrc_unittest::*">
      <Window ref.Name="self" Text="MyControlTemplate" ClientSize="x:320 y:240">
        <ut:MyControl ref.Name="my">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
        </ut:MyControl>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"Context")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				my->SetContext(BoxValue<WString>(L"Hello"));
			});
			protocol->OnNextIdleFrame(L"Hello", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				my->SetContext(BoxValue<WString>(L"World"));
			});
			protocol->OnNextIdleFrame(L"World", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto my = FindObjectByName<GuiControl>(window, L"my");

				my->SetContext({});
			});
			protocol->OnNextIdleFrame(L"Restored", [=]()
			{
				auto window = GetApplication()->GetMainWindow();

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/Context"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceContext
			);
	});
}
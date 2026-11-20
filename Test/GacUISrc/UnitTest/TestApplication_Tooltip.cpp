#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTooltipWindows = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Tooltip" ClientSize="x:480 y:320">
        <Table CellPadding="5" AlignmentToParent="left:0 top:0 right:0 bottom:0">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          <Cell Site="row:0 column:0">
            <Button ref.Name="buttonTooltipA" Text="TooltipA">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <att.TooltipControl>
                <Label Text="This is Tooltip A"/>
              </att.TooltipControl>
            </Button>
          </Cell>
          <Cell Site="row:1 column:0">
            <Button ref.Name="buttonTooltipB" Text="TooltipB">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <att.TooltipControl>
                <Label Text="This is Tooltip B"/>
              </att.TooltipControl>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Tooltip")
	{
		for (auto closeOwner : { false,true })
		TEST_CASE(closeOwner ? L"CloseTooltipWindowOwner" : L"HideTooltipWindowOwner")
		{
			GuiWindow* ownerWindow = nullptr;
			GuiTooltip* tooltipWindow = nullptr;
			vint closed = 0;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&]()
				{
					ownerWindow = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::MainWindow"));
					ownerWindow->SetText(L"Tooltip owner");
					ownerWindow->SetClientSize(Size(240, 160));
					ownerWindow->SetLocation({ {100},{80} });
					ownerWindow->Show();
				});
				protocol->OnNextIdleFrame(L"Secondary owner visible", [&]()
				{
					auto mainOwner = FindObjectByName<GuiButton>(GetApplication()->GetMainWindow(), L"buttonTooltipA");
					TEST_ASSERT(mainOwner->DisplayTooltip(Point(0, 0)));
					auto content = mainOwner->GetTooltipControl();
					auto contentDisposed = content->GetDisposedFlag();
					auto frameDisposed = content->GetRelatedControlHost()->GetDisposedFlag();
					auto owner = FindObjectByName<GuiButton>(ownerWindow, L"buttonTooltipA");
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					TEST_ASSERT(frameDisposed->IsDisposed());
					TEST_ASSERT(!contentDisposed->IsDisposed() && !content->GetParent());
					TEST_ASSERT(mainOwner->GetTooltipControl() == content);
					tooltipWindow = dynamic_cast<GuiTooltip*>(owner->GetTooltipControl()->GetRelatedControlHost());
					TEST_ASSERT(tooltipWindow);
					tooltipWindow->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(closed++ == 0);
					});
					ownerWindow->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(closed == 1);
						TEST_ASSERT(!tooltipWindow->GetOpening());
						TEST_ASSERT(!tooltipWindow->GetTemporaryContentControl());
						TEST_ASSERT(!GetApplication()->GetTooltipOwner());
						closed++;
					});
				});
				protocol->OnNextIdleFrame(L"Tooltip visible over secondary owner", [&, protocol]()
				{
					protocol->MouseMove(protocol->LocationOf(tooltipWindow->GetTemporaryContentControl()));
					if (closeOwner) ownerWindow->Close();
					else ownerWindow->Hide();
				});
				protocol->OnNextIdleFrame(L"Main window survives owner and tooltip", [&]()
				{
					TEST_ASSERT(closed == 2);
					SafeDeleteControl(ownerWindow);
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				closeOwner ? L"Application/Tooltip/CloseWindowOwner" : L"Application/Tooltip/HideWindowOwner",
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});

		TEST_CASE(L"DestroyVisibleTooltipOwner")
		{
			GuiTooltip* tooltipWindow = nullptr;
			Ptr<GuiDisposedFlag> disposed;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto owner = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					tooltipWindow = dynamic_cast<GuiTooltip*>(owner->GetTooltipControl()->GetRelatedControlHost());
					TEST_ASSERT(tooltipWindow);
					disposed = tooltipWindow->GetDisposedFlag();
				});
				protocol->OnNextIdleFrame(L"Tooltip before owner deletion", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto owner = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					protocol->MouseMove(protocol->LocationOf(owner->GetTooltipControl()));
					SafeDeleteControl(owner);
					TEST_ASSERT(tooltipWindow->GetTemporaryContentControl() == nullptr);
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == nullptr);
					window->SetText(L"Tooltip owner removed");
					GetApplication()->RefreshThemes();
				});
				protocol->OnNextIdleFrame(L"Tooltip closed after owner deletion", [&, protocol]()
				{
					TEST_ASSERT(disposed->IsDisposed());
					auto window = GetApplication()->GetMainWindow();
					auto owner = FindObjectByName<GuiButton>(window, L"buttonTooltipB");
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == owner);
					tooltipWindow = dynamic_cast<GuiTooltip*>(owner->GetTooltipControl()->GetRelatedControlHost());
					TEST_ASSERT(tooltipWindow);
				});
				protocol->OnNextIdleFrame(L"Surviving owner shows its tooltip", [&, protocol]()
				{
					TEST_ASSERT(tooltipWindow->GetOpening());
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/DestroyVisibleTooltipOwner"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});

		TEST_CASE(L"RefreshVisibleTooltip")
		{
			TooltipTimer timer;
			GuiButton* owner = nullptr;
			GuiLabel* visibleContent = nullptr;
			GuiLabel* hiddenContent = nullptr;
			vint closed = 0;
			const Color ownerBackground(40, 70, 100);
			const Color themeBackground(220, 230, 240);
			const Color themeText(10, 20, 30);
			auto tooltipTemplate = [](Color color) -> TemplateProperty<templates::GuiWindowTemplate>
			{
				return [=](const Value&)
				{
					auto ct = new darkskin::TooltipTemplate;
					ct->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->SetColor(color);
					return ct;
				};
			};
			auto background = [](GuiTooltip* tooltip)
			{
				return tooltip->GetControlTemplateObject()->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor();
			};
			auto alternate = Ptr(new theme::ThemeTemplates);
			alternate->Name = L"TooltipRefresh";
			alternate->Tooltip = tooltipTemplate(themeBackground);
			alternate->Label = [=](const Value&)
			{
				auto ct = new darkskin::LabelTemplate;
				ct->SetDefaultTextColor(themeText);
				return ct;
			};
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Owner window tooltip template", [&]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetControlThemeName(theme::ThemeName::SystemFrameWindow);
					window->TypedControlTemplateObject(true)->SetTooltipTemplate(tooltipTemplate(ownerBackground));
					owner = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					visibleContent = dynamic_cast<GuiLabel*>(owner->GetTooltipControl());
					hiddenContent = dynamic_cast<GuiLabel*>(FindObjectByName<GuiButton>(window, L"buttonTooltipB")->GetTooltipControl());
					TEST_ASSERT(visibleContent && hiddenContent);
				});
				protocol->OnNextIdleFrame(L"Hover over the first button", [&, protocol]()
				{
					protocol->MouseMove(protocol->LocationOf(owner));
				});
				protocol->OnNextIdleFrame([&]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"Tooltip uses the owner window template", [&, protocol]()
				{
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == owner);
					auto tooltip = dynamic_cast<GuiTooltip*>(visibleContent->GetRelatedControlHost());
					TEST_ASSERT(tooltip && background(tooltip) == ownerBackground);
					TEST_ASSERT(visibleContent->GetTextColor() != themeText && !hiddenContent->GetParent());
					tooltip->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(!GetApplication()->GetTooltipOwner());
						closed++;
					});
					protocol->MouseMove({ 0, 0 });
				});
				protocol->OnNextIdleFrame([&]()
				{
					auto disposed = visibleContent->GetRelatedControlHost()->GetDisposedFlag();
					theme::RegisterTheme(alternate);
					GetApplication()->RefreshThemes();
					TEST_ASSERT(disposed->IsDisposed() && closed == 1);
					TEST_ASSERT(!visibleContent->GetParent() && !hiddenContent->GetParent());
					TEST_ASSERT(visibleContent->GetTextColor() == themeText && hiddenContent->GetTextColor() == themeText);
				});
				protocol->OnNextIdleFrame(L"Theme change dismissed the tooltip", [&]()
				{
					TEST_ASSERT(!GetApplication()->GetTooltipOwner());
					TEST_ASSERT(owner->DisplayTooltip(Point(0, 0)));
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"Previous leave timer preserves the new tooltip", [&, protocol]()
				{
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == owner);
					owner->CloseTooltip();
					protocol->MouseMove(protocol->LocationOf(owner));
				});
				protocol->OnNextIdleFrame([&]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"Hover opens the current theme tooltip", [&]()
				{
					TEST_ASSERT(GetApplication()->GetTooltipOwner() == owner);
					auto tooltip = dynamic_cast<GuiTooltip*>(visibleContent->GetRelatedControlHost());
					TEST_ASSERT(tooltip && !tooltip->GetControlTemplate() && background(tooltip) == themeBackground);
					TEST_ASSERT(visibleContent->GetTextColor() == themeText);
					theme::UnregisterTheme(alternate->Name);
					GetApplication()->RefreshThemes();
					TEST_ASSERT(!GetApplication()->GetTooltipOwner());
					TEST_ASSERT(visibleContent->GetTextColor() != themeText && hiddenContent->GetTextColor() != themeText);
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/RefreshVisibleTooltip"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});

		TEST_CASE(L"ShowTooltipAndWaitForClosing")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					timer.WaitForClosing();
				});
				protocol->OnNextIdleFrame(L"After timer.WaitForClosing", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndWaitForClosing"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
		
		TEST_CASE(L"ShowTooltipAndLeave")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					protocol->MouseMove({0, 0});
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After leaving tooltip", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndLeave"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
		
		TEST_CASE(L"ShowTooltipAndSwitchToAnother")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipB = FindObjectByName<GuiButton>(window, L"buttonTooltipB");
					auto location = protocol->LocationOf(buttonTooltipB);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipB", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip for A", [&, protocol]()
				{
					timer.WaitForClosing();
				});
				protocol->OnNextIdleFrame(L"After timer.WaitForClosing", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndSwitchToAnother"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
	});
}

#include "TestControls.h"

TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiLabel" ClientSize="x:320 y:240">
        <Label ref.Name="label" Text="This is a label"/>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"GuiLabel synchronizes colors when replacing templates")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				auto checkColor = [&](Color expected)
				{
					auto ct = dynamic_cast<templates::GuiLabelTemplate*>(label->GetControlTemplateObject());
					TEST_ASSERT(label->GetTextColor() == expected);
					TEST_ASSERT(ct->GetTextColor() == expected);
					TEST_ASSERT(ct->GetOwnedElement().Cast<GuiSolidLabelElement>()->GetColor() == expected);
				};
				auto defaultColor = dynamic_cast<templates::GuiLabelTemplate*>(label->GetControlTemplateObject())->GetDefaultTextColor();
				checkColor(defaultColor);
				for (vint i = 0; i < 2; i++)
				{
					GetApplication()->RefreshThemes();
					checkColor(defaultColor);
				}

				auto overrideColor = Color(10, 200, 30);
				label->SetTextColor(overrideColor);
				GetApplication()->RefreshThemes();
				checkColor(overrideColor);
				label->SetTextColor(defaultColor);

				auto changedColor = Color(200, 30, 10);
				auto createTemplate = [=](const Value&)
				{
					auto ct = new darkskin::LabelTemplate;
					ct->SetDefaultTextColor(changedColor);
					return ct;
				};
				label->SetControlTemplate(createTemplate);
				checkColor(changedColor);
				label->SetControlTemplate(createTemplate);
				checkColor(changedColor);
				label->SetTextColor(overrideColor);
				label->SetControlTemplate({});
				checkColor(overrideColor);
				label->SetTextColor(defaultColor);
				label->RefreshThemes();
				checkColor(defaultColor);
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiLabel/RefreshThemes"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});

	TEST_CASE(L"GuiLabel")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				TEST_ASSERT(label->GetText() == L"This is a label");
				{
					// such font and measuring only applied in unit test environment
					auto font = label->GetDisplayFont();
					TEST_ASSERT(font.fontFamily == L"GacUI Default Font");
					TEST_ASSERT(font.size == 12);
					auto size = label->GetBoundsComposition()->GetCachedMinSize();
					TEST_ASSERT(size == Size(107, 16)); // 4+12*(15x1)
				}
				{
					auto labelCT = dynamic_cast<templates::GuiLabelTemplate*>(label->GetControlTemplateObject());
					TEST_ASSERT(labelCT);
					TEST_ASSERT(label->GetTextColor() == labelCT->GetDefaultTextColor());
				}
				TEST_ASSERT(label->GetFocused() == false);
				label->SetFocused();
				TEST_ASSERT(label->GetFocused() == false);

				label->TextChanged.AttachLambda([=](auto&&...)
				{
					window->SetText(L"TextChanged");
				});
				label->EnabledChanged.AttachLambda([=](auto&&...)
				{
					window->SetText(label->GetEnabled() ? L"Enabled" : L"Disabled");
				});

				label->SetText(L"The text is changed");
			});
			protocol->OnNextIdleFrame(L"The text is changed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto label = FindObjectByName<GuiLabel>(window, L"label");

				label->SetEnabled(false);
			});
			protocol->OnNextIdleFrame(L"Disabled", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto label = FindObjectByName<GuiLabel>(window, L"label");

				label->SetEnabled(true);
				label->SetText(L"The color is changed");
				label->SetTextColor(Color(0, 255, 0));
			});
			protocol->OnNextIdleFrame(L"The text is changed", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				TEST_ASSERT(label->GetTextColor() == Color(0, 255, 0));

				label->SetEnabled(false);
			});
			protocol->OnNextIdleFrame(L"Disabled again", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto label = FindObjectByName<GuiLabel>(window, L"label");
				TEST_ASSERT(label->GetTextColor() == Color(0, 255, 0));

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/GuiLabel"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});
}

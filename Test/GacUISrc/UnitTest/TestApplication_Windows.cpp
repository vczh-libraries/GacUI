#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceEmptyWindows = LR"GacUISrc(
<Resource>
  <Instance name="SubWindowResource">
    <Instance ref.Class="gacuisrc_unittest::SubWindow">
      <Window ref.Name="self" Text="SubWindow" ClientSize="x:200 y:200">
        <Label Text-format="This is $(self.Text)">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </Label>
      </Window>
    </Instance>
  </Instance>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="WindowManagement" ClientSize="x:480 y:320">
        <Label Text="This is the main window">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </Label>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Windows")
	{
		TEST_CASE(L"Order")
		{
			GuiWindow* subWindowA = nullptr;
			GuiWindow* subWindowB = nullptr;

			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					{
						subWindowA = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
						subWindowA->SetText(L"SubWindow A");
						subWindowA->SetLocation({ {100},{50} });
						subWindowA->Show();
					}
					{
						subWindowB = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
						subWindowB->SetText(L"SubWindow B");
						subWindowB->SetLocation({ {200},{100} });
						subWindowB->Show();
					}
				});
				protocol->OnNextIdleFrame(L"Created two SubWindows", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.0, 0.0, 10, 10);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click SubWindowA", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowB, 1.0, 1.0, -10, -10);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click SubWindowB", [&, protocol]()
				{
					subWindowA->Hide();
					SafeDeleteControl(subWindowA);
					subWindowA = nullptr;

					subWindowB->Hide();
					SafeDeleteControl(subWindowB);
					subWindowB = nullptr;

					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/Order"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});
	});
}

#include "TestControls.h"

TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiWindow" ClientSize="x:320 y:240">
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"Window Features")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetText() == L"GuiWindow");
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->SetMaximizedBox(false);
				window->SetText(L"No Maximized Box");
			});
			protocol->OnNextIdleFrame(L"No Maximized Box", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == false);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->SetMaximizedBox(true);
				window->SetMinimizedBox(false);
				window->SetText(L"No Minimized Box");
			});
			protocol->OnNextIdleFrame(L"No Minimized Box", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == false);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->SetMinimizedBox(true);
				window->SetBorder(false);
				window->SetText(L"No Border");
			});
			protocol->OnNextIdleFrame(L"No Border", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == false);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->SetBorder(true);
				window->SetSizeBox(false);
				window->SetText(L"No Size Box");
			});
			protocol->OnNextIdleFrame(L"No Size Box", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == false);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->SetSizeBox(true);
				window->SetIconVisible(false);
				window->SetText(L"No Icon");
			});
			protocol->OnNextIdleFrame(L"No Icon", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == false);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->SetIconVisible(true);
				window->SetTitleBar(false);
				window->SetText(L"No Title Bar");
			});
			protocol->OnNextIdleFrame(L"No Title Bar", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == false);

				window->SetTitleBar(true);
				window->SetText(L"Restored");
			});
			protocol->OnNextIdleFrame(L"Restored", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetMaximizedBox() == true);
				TEST_ASSERT(window->GetMinimizedBox() == true);
				TEST_ASSERT(window->GetBorder() == true);
				TEST_ASSERT(window->GetSizeBox() == true);
				TEST_ASSERT(window->GetIconVisible() == true);
				TEST_ASSERT(window->GetTitleBar() == true);

				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/WindowFeatures"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});

	TEST_CASE(L"Window Sizing")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->ShowMaximized();
			});
			protocol->OnNextIdleFrame(L"Maximized", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->ShowMinimized();
			});
			protocol->OnNextIdleFrame(L"Minimized", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->ShowRestored();
			});
			protocol->OnNextIdleFrame(L"Restored", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->SetClientSize({ 640,480 });
			});
			protocol->OnNextIdleFrame(L"Resized", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->ShowMaximized();
			});
			protocol->OnNextIdleFrame(L"Maximized", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->ShowMinimized();
			});
			protocol->OnNextIdleFrame(L"Minimized", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->ShowRestored();
			});
			protocol->OnNextIdleFrame(L"Restored", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/CoreApplication/WindowSizing"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});

	TEST_CASE(L"Window Maximized at Startup")
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
			WString::Unmanaged(L"Controls/CoreApplication/WindowMaximized"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource,
			[](GuiWindow* window)
			{
				window->WindowOpened.AttachLambda([=](auto&&...)
				{
					window->ShowMaximized();
				});
			});
	});
}
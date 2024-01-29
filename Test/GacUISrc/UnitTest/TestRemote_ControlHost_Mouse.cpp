#include "TestRemote_ControlHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"MouseMove")
	{
		ControlHostProtocol protocol;
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});
		SetGuiMainProxy([&]()
		{
			protocol.events->OnControllerConnect();
			TEST_CASE(L"Create and destroy a control host")
			{
				auto theme = Ptr(new EmptyControlTheme);
				theme::RegisterTheme(theme);

				GuiWindow window(theme::ThemeName::Window);
				window.SetClientSize({ 640,480 });
				controlHost = &window;
				GetApplication()->Run(&window);
				controlHost = nullptr;

				theme::UnregisterTheme(theme->Name);
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	// TODO:
	//   Mouse event propogation and enter/leave/moving with @DropConsecutive/dbclick
	//   Mouse capturing
}
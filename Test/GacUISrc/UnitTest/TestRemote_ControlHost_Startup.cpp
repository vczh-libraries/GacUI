#include "TestRemote_ControlHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Create one window and exit immediately")
	{
		ControlHostProtocol protocol;
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetCachedBounds() == Rect(0, 0, 100, 200));
			TEST_ASSERT(controlHost->GetClientSize() == Size(100, 200));
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
				window.SetClientSize({ 100,200 });
				window.SetText(L"EmptyControlHost");
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
	//   Mouse enter/leave/capturing
	//   [TAB] and keyboard focus
	//   [ALT]
	//   Shortcut Key
	//   Other events
}
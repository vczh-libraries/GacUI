#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Create one window and exit immediately")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetCachedBounds() == Rect(0, 0, 100, 200));
			TEST_ASSERT(controlHost->GetClientSize() == Size(100, 200));
			controlHost->Hide();
		});

		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
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
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Test nested <Bounds> associated resources")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiControl* control = nullptr;

		protocol.OnNextFrame([&]()
		{
			control = new GuiControl(theme::ThemeName::CustomControl);
			control->GetBoundsComposition()->SetExpectedBounds(Rect({ 10,10 }, { 50,50 }));
			controlHost->AddChild(control);
			controlHost->ForceCalculateSizeImmediately();

			TEST_ASSERT(controlHost->GetBoundsComposition()->GetCachedBounds() == Rect(0, 0, 100, 200));
			TEST_ASSERT(controlHost->GetClientSize() == Size(100, 200));
			TEST_ASSERT(control->GetBoundsComposition()->GetCachedBounds() == Rect(10, 10, 60, 60));

			TEST_ASSERT(control->GetContainerComposition()->GetAssociatedControl() == nullptr);
			TEST_ASSERT(control->GetContainerComposition()->GetAssociatedHost() == nullptr);
			TEST_ASSERT(control->GetContainerComposition()->GetRelatedControl() == control);
			TEST_ASSERT(control->GetContainerComposition()->GetRelatedControlHost() == controlHost);
			TEST_ASSERT(control->GetContainerComposition()->GetRelatedGraphicsHost() == controlHost->GetGraphicsHost());

			TEST_ASSERT(control->GetBoundsComposition()->GetAssociatedControl() == control);
			TEST_ASSERT(control->GetBoundsComposition()->GetAssociatedHost() == nullptr);
			TEST_ASSERT(control->GetBoundsComposition()->GetRelatedControl() == control);
			TEST_ASSERT(control->GetBoundsComposition()->GetRelatedControlHost() == controlHost);
			TEST_ASSERT(control->GetBoundsComposition()->GetRelatedGraphicsHost() == controlHost->GetGraphicsHost());

			TEST_ASSERT(controlHost->GetContainerComposition()->GetAssociatedControl() == nullptr);
			TEST_ASSERT(controlHost->GetContainerComposition()->GetAssociatedHost() == nullptr);
			TEST_ASSERT(controlHost->GetContainerComposition()->GetRelatedControl() == controlHost);
			TEST_ASSERT(controlHost->GetContainerComposition()->GetRelatedControlHost() == controlHost);
			TEST_ASSERT(controlHost->GetContainerComposition()->GetRelatedGraphicsHost() == controlHost->GetGraphicsHost());

			TEST_ASSERT(controlHost->GetBoundsComposition()->GetAssociatedControl() == controlHost);
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetAssociatedHost() == nullptr);
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetRelatedControl() == controlHost);
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetRelatedControlHost() == controlHost);
			TEST_ASSERT(controlHost->GetBoundsComposition()->GetRelatedGraphicsHost() == controlHost->GetGraphicsHost());

			TEST_ASSERT(controlHost->GetGraphicsHost()->GetMainComposition()->GetAssociatedControl() == nullptr);
			TEST_ASSERT(controlHost->GetGraphicsHost()->GetMainComposition()->GetAssociatedHost() == controlHost->GetGraphicsHost());
			TEST_ASSERT(controlHost->GetGraphicsHost()->GetMainComposition()->GetRelatedControl() == nullptr);
			TEST_ASSERT(controlHost->GetGraphicsHost()->GetMainComposition()->GetRelatedControlHost() == nullptr);
			TEST_ASSERT(controlHost->GetGraphicsHost()->GetMainComposition()->GetRelatedGraphicsHost() == controlHost->GetGraphicsHost());

			controlHost->Hide();
		});

		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
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
		StartRemoteControllerTest(protocol);
	});
}
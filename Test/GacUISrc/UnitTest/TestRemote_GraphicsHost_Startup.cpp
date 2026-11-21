#include "TestRemote_GraphicsHost_Shared.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_BasicElements.h"
#include "../../../Source/PlatformProviders/Hosted/GuiHostedApplication.h"

TEST_FILE
{
	TEST_CATEGORY(L"Detached labels refresh font measurements through their retained remote target")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		protocol.OnNextFrame([&]()
		{
			auto target = dynamic_cast<GuiRemoteGraphicsRenderTarget*>(GetGuiGraphicsResourceManager()->GetRenderTarget(GetHostedApplication()->GetNativeWindowHost()));
			auto label = Ptr(GuiSolidLabelElement::Create());
			FontProperties font;
			font.fontFamily = L"Detached label font";
			font.size = 16;
			label->SetFont(font);
			label->SetText(L"Retained document heading");
			label->SetEllipse(true);
			auto renderer = dynamic_cast<elements_remoteprotocol::GuiSolidLabelElementRenderer*>(label->GetRenderer());
			renderer->SetRenderTarget(target);
			renderer->SetRenderTarget(nullptr);

			List<remoteprotocol::OrdinaryElementDescVariant> updates;
			renderer->NotifyMinSizeCacheInvalidated();
			renderer->SendUpdateElementMessages(true, updates);
			TEST_ASSERT(renderer->NeedUpdateMinSizeFromCache());
			target->fontHeights.Set({ font.fontFamily,font.size }, 37);
			renderer->TryFetchMinSizeFromCache();
			TEST_ASSERT(renderer->GetMinSize() == Size(37, 37));
			TEST_ASSERT(!renderer->NeedUpdateMinSizeFromCache());

			target->fontHeights.Set({ font.fontFamily,font.size }, 19);
			renderer->NotifyMinSizeCacheInvalidated();
			renderer->SendUpdateElementMessages(true, updates);
			TEST_ASSERT(renderer->GetMinSize() == Size(19, 19));
			renderer->SetRenderTarget(target);
			TEST_ASSERT(renderer->GetMinSize() == Size(19, 19));
			renderer->SetRenderTarget(nullptr);
			label = nullptr;
			controlHost->Hide();
		});
		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

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
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
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

	TEST_CATEGORY(L"Update image min size when only image metadata is returned")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;
		Ptr<GuiImageFrameElement> imageElement;

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(imageElement->GetRenderer()->GetMinSize() == Size(0, 0));
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(imageElement->GetRenderer()->GetMinSize() == Size(16, 24));
			controlHost->Hide();
		});

		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
			auto theme = Ptr(new EmptyControlTheme);
			theme::RegisterTheme(theme);

			GuiWindow window(theme::ThemeName::Window);
			window.SetClientSize({ 100,200 });
			controlHost = &window;

			char8_t imageData[] = u8"16x24";
			auto image = GetCurrentController()->ImageService()->CreateImageFromMemory(imageData, sizeof(imageData) - 1);
			imageElement = Ptr(GuiImageFrameElement::Create());
			imageElement->SetImage(image, 0);

			auto bounds = new GuiBoundsComposition;
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
			bounds->SetOwnedElement(imageElement);
			window.GetContainerComposition()->AddChild(bounds);

			GetApplication()->Run(&window);
			imageElement = nullptr;
			controlHost = nullptr;

			theme::UnregisterTheme(theme->Name);
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Nested compositions resolve their graphics host and publish bounds changes")
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
		});

		protocol.OnNextFrame([&]()
		{
			auto bounds = control->GetBoundsComposition();
			auto& hostEvent = controlHost->GetGraphicsHost()->CompositionCachedBoundsChanged;
			Rect expected(20, 30, 70, 80);
			vint localChanges = 0;
			vint hostChanges = 0;
			auto local = bounds->CachedBoundsChanged.AttachLambda([&](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				TEST_ASSERT(sender == bounds && arguments.compositionSource == bounds && arguments.eventSource == bounds);
				TEST_ASSERT(bounds->GetCachedBounds() == expected && hostChanges == 0);
				localChanges++;
			});
			auto onHostChange = [&](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				if (sender != bounds) return;
				TEST_ASSERT(arguments.compositionSource == bounds && arguments.eventSource == bounds);
				TEST_ASSERT(bounds->GetCachedBounds() == expected && localChanges == 1);
				hostChanges++;
			};
			auto first = hostEvent.AttachLambda(onHostChange);
			auto second = hostEvent.AttachLambda(onHostChange);
			bounds->SetExpectedBounds(expected);
			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(localChanges == 1 && hostChanges == 2);
			bounds->SetExpectedBounds(expected);
			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(localChanges == 1 && hostChanges == 2);
			hostEvent.Detach(first);
			hostEvent.Detach(second);
			bounds->CachedBoundsChanged.Detach(local);
			controlHost->Hide();
		});

		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
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

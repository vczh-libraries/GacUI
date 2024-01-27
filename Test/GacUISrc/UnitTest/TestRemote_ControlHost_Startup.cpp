#include "TestRemote.h"

using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;

namespace remote_control_host_tests
{
	class ControlHostProtocol : public SingleScreenProtocol
	{
	public:
		static SingleScreenConfig MakeSingleScreenConfig()
		{
			SingleScreenConfig config;

			config.customFramePadding = { 8,8,8,8 };

			config.fontConfig.defaultFont.fontFamily = L"One";
			config.fontConfig.supportedFonts = Ptr(new List<WString>());
			config.fontConfig.supportedFonts->Add(L"One");
			config.fontConfig.supportedFonts->Add(L"Two");
			config.fontConfig.supportedFonts->Add(L"Three");

			config.screenConfig.bounds = { 0,0,640,480 };
			config.screenConfig.clientBounds = { 0,0,640,440 };
			config.screenConfig.scalingX = 1;
			config.screenConfig.scalingY = 1;

			return config;
		}

		ControlHostProtocol()
			: SingleScreenProtocol(MakeSingleScreenConfig())
		{
		}
	};

	class EmptyControlHost : public GuiWindow
	{
	public:
		EmptyControlHost()
			: GuiWindow(theme::ThemeName::Window, INativeWindow::Normal)
		{
		}
	};

	class EmptyControlTheme : public theme::ThemeTemplates
	{
	public:
		EmptyControlTheme()
		{
			Name = WString::Unmanaged(L"EmptyControlTheme");
			PreferCustomFrameWindow = true;
			CustomFrameWindow = [](auto)
			{
				auto ct = new templates::GuiWindowTemplate;
				ct->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				ct->SetContainerComposition(ct);
				ct->SetFocusableComposition(ct);
				return ct;
			};
		}
	};
}
using namespace remote_control_host_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Create one window and exit immediately")
	{
		ControlHostProtocol protocol;
		EmptyControlHost* controlHost = nullptr;

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

				EmptyControlHost window;
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
}
#include "TestRemote.h"

using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;

namespace remote_graphics_host_tests
{
	template<typename TProtocol>
	class GraphicsHostProtocolBase : public TProtocol
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

		template<typename ...TArgs>
		GraphicsHostProtocolBase(TArgs&& ...args)
			: TProtocol(MakeSingleScreenConfig(), std::forward<TArgs&&>(args)...)
		{
		}
	};

	class GraphicsHostProtocol : public GraphicsHostProtocolBase<SingleScreenProtocol>
	{
	};

	class GraphicsHostRenderingProtocol : public GraphicsHostProtocolBase<SingleScreenRenderingProtocol>
	{
	public:
		GraphicsHostRenderingProtocol(List<WString>& _eventLogs)
			:GraphicsHostProtocolBase<SingleScreenRenderingProtocol>(_eventLogs)
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

			CustomControl = [](auto)
			{
				auto ct = new templates::GuiControlTemplate;
				ct->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				ct->SetContainerComposition(ct);
				ct->SetFocusableComposition(ct);
				return ct;
			};

			ShortcutKey = [](auto)
			{
				auto ct = new templates::GuiLabelTemplate;
				ct->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				ct->SetContainerComposition(ct);
				ct->SetFocusableComposition(ct);
				return ct;
			};

			Button = [](auto)
			{
				auto ct = new templates::GuiButtonTemplate;
				ct->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				ct->SetContainerComposition(ct);
				ct->SetFocusableComposition(ct);
				return ct;
			};
		}
	};

	template<typename TProtocol>
	Func<void()> MakeGuiMain(TProtocol& protocol, List<WString>& eventLogs, GuiWindow*& controlHost)
	{
		return [&]()
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
				TEST_ASSERT(protocol.nextEventIndex == protocol.processRemoteEvents.Count());
				AssertEventLogs(eventLogs);
			});
		};
	}

	void									AttachAndLogEvents(GuiGraphicsComposition* sender, const wchar_t* name, List<WString>& eventLogs);

	NativeWindowKeyInfo						MakeKeyInfo(bool ctrl, bool shift, bool alt, VKEY code);
	NativeWindowCharInfo					MakeCharInfo(bool ctrl, bool shift, bool alt, wchar_t code);
	NativeWindowMouseInfo					MakeMouseInfo(bool left, bool middle, bool right, vint x, vint y, vint wheel);
	remoteprotocol::IOMouseInfoWithButton	MakeMouseInfoWithButton(remoteprotocol::IOMouseButton button, bool left, bool middle, bool right, vint x, vint y, vint wheel);
}
using namespace remote_graphics_host_tests;
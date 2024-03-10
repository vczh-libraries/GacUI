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
		static UnitTestScreenConfig MakeUnitTestScreenConfig()
		{
			UnitTestScreenConfig config;
			config.FastInitialize(640, 480, 40);

			config.fontConfig.defaultFont.fontFamily = WString::Unmanaged(L"One");
			config.fontConfig.supportedFonts->Clear();
			config.fontConfig.supportedFonts->Add(config.fontConfig.defaultFont.fontFamily);

			return config;
		}

		template<typename ...TArgs>
		GraphicsHostProtocolBase(TArgs&& ...args)
			: TProtocol(MakeUnitTestScreenConfig(), std::forward<TArgs&&>(args)...)
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
			protocol.GetEvents()->OnControllerConnect();
			auto theme = Ptr(new EmptyControlTheme);
			theme::RegisterTheme(theme);

			GuiWindow window(theme::ThemeName::Window);
			window.SetClientSize({ 640,480 });
			controlHost = &window;
			GetApplication()->Run(&window);
			controlHost = nullptr;

			theme::UnregisterTheme(theme->Name);

			TEST_CASE(L"Test if all expected frames are executed")
			{
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
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

	extern Func<void()>						MakeGuiMain(ControlHostProtocol& protocol, List<WString>& eventLogs, GuiWindow*& controlHost);

	void									AttachMouseEvents(GuiGraphicsComposition* sender, const wchar_t* name, List<WString>& eventLogs);

	NativeWindowMouseInfo					MakeMouseInfo(bool left, bool middle, bool right, vint x, vint y, vint wheel);
	remoteprotocol::IOMouseInfoWithButton	MakeMouseInfoWithButton(remoteprotocol::IOMouseButton button, bool left, bool middle, bool right, vint x, vint y, vint wheel);
}
using namespace remote_control_host_tests;
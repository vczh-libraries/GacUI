/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_SHARED
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_SHARED

#include "../GacUI.h"
#include "../PlatformProviders/Remote/GuiRemoteController.h"

namespace vl::presentation::unittest
{
	struct WindowStyleConfig
	{
		WString						title;
		bool						enabled = true;
		bool						topMost = false;
		bool						showInTaskBar = true;

		bool						customFrameMode = false;
		bool						maximizedBox = true;
		bool						minimizedBox = true;
		bool						border = true;
		bool						sizeBox = true;
		bool						iconVisible = true;
		bool						titleBar = true;
		bool						activated = false;

		auto operator<=>(const WindowStyleConfig&) const = default;
	};

	struct UnitTestScreenConfig
	{
		using FontConfig = vl::presentation::remoteprotocol::FontConfig;
		using ScreenConfig = vl::presentation::remoteprotocol::ScreenConfig;

		WString						executablePath;
		NativeMargin				customFramePadding;
		FontConfig					fontConfig;
		ScreenConfig				screenConfig;

		void						FastInitialize(vint width, vint height, vint taskBarHeight = 0);
	};

	class UnitTestRemoteProtocolBase : public Object, protected virtual IGuiRemoteProtocol
	{
	protected:
		IGuiRemoteProtocolEvents*	events = nullptr;
		UnitTestScreenConfig		globalConfig;

	public:
		UnitTestRemoteProtocolBase(UnitTestScreenConfig _globalConfig)
			: globalConfig(_globalConfig)
		{
		}

		IGuiRemoteProtocol* GetProtocol()
		{
			return this;
		}

		IGuiRemoteProtocolEvents* GetEvents() const
		{
			return events;
		}

		const UnitTestScreenConfig& GetGlobalConfig() const
		{
			return globalConfig;
		}

	protected:

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		void Initialize(IGuiRemoteProtocolEvents* _events) override
		{
			events = _events;
		}

		WString GetExecutablePath() override
		{
			return globalConfig.executablePath;
		}
	};
}

#endif
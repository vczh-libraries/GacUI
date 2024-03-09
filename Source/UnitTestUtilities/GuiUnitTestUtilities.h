/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTUTILITIES
#define VCZH_PRESENTATION_GUIUNITTESTUTILITIES

#include "../GacUI.h"
#include "../PlatformProviders/Remote/GuiRemoteController.h"

namespace vl::presentation::unittest
{
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

	class UnitTestContext : public Object
	{
	public:
		virtual IGuiRemoteProtocolEvents*		GetEvents() = 0;
	};

	using UnitTestMainFunc = vl::Func<void(UnitTestContext*)>;
}

extern void GacUIUnitTest_Initialize();
extern void GacUIUnitTest_Finalize();
extern void GacUIUnitTest_SetGuiMainProxy(const vl::presentation::unittest::UnitTestMainFunc& proxy);
extern void GacUIUnitTest_Start(vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config);

#endif
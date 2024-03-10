#include "GuiUnitTestProtocol_Shared.h"

namespace vl::presentation::unittest
{
	using namespace vl::collections;

/***********************************************************************
UnitTestScreenConfig
***********************************************************************/

	void UnitTestScreenConfig::FastInitialize(vint width, vint height, vint taskBarHeight)
	{
		executablePath = WString::Unmanaged(L"/GacUI/Remote/Protocol/UnitTest.exe");
		customFramePadding = { 8,8,8,8 };

		fontConfig.defaultFont.fontFamily = WString::Unmanaged(L"GacUI Default Font");
		fontConfig.defaultFont.size = 12;
		fontConfig.supportedFonts = Ptr(new List<WString>());
		fontConfig.supportedFonts->Add(fontConfig.defaultFont.fontFamily);

		screenConfig.bounds = { 0,0,width,height };
		screenConfig.clientBounds = { 0,0,width,(height - taskBarHeight) };
		screenConfig.scalingX = 1;
		screenConfig.scalingY = 1;
	}
}
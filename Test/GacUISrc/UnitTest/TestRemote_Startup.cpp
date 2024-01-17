#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

extern void SetGuiMainProxy(void(*proxy)());

TEST_FILE
{
	TEST_CASE(L"Start and Stop")
	{
		SetGuiMainProxy([]()
		{
		});
		SetupRemoteNativeController(nullptr);
		SetGuiMainProxy(nullptr);
	});
}
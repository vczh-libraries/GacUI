#include "../../../Source//UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif

using namespace vl;
using namespace vl::stream;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;

TEST_FILE
{
	TEST_CATEGORY(L"DOM Recovery")
	{
		// TODO: test dom recovery from logging
	});

	TEST_CATEGORY(L"Hello, world!")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->GetEvents()->OnControllerConnect();
			auto darkskinTheme = Ptr(new darkskin::Theme);
			theme::RegisterTheme(darkskinTheme);

			GuiWindow window(theme::ThemeName::Window);
			window.SetText(L"Hello, world!");
			window.SetClientSize({ 640,480 });
			window.MoveToScreenCenter();

			protocol->OnNextIdleFrame([&]()
			{
				TEST_ASSERT(protocol->GetLoggedRenderingResults().Count() == 1);
				window.Hide();
			});

			GetApplication()->Run(&window);
			theme::UnregisterTheme(darkskinTheme->Name);
		});
		GacUIUnitTest_Start(WString::Unmanaged(L"UnitTestFramework_HelloWorld"));
	});
}
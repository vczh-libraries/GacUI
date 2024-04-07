#include "../../../Source//UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif

using namespace vl;
using namespace vl::stream;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;

namespace unittest_framework_tests
{
	class EmptyWindowTheme : public theme::ThemeTemplates
	{
	public:
		EmptyWindowTheme()
		{
			Name = WString::Unmanaged(L"EmptyWindowTheme");
			PreferCustomFrameWindow = true;

			CustomFrameWindow = [](auto)
			{
				auto ct = new templates::GuiWindowTemplate;
				ct->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				ct->SetContainerComposition(ct);
				ct->SetFocusableComposition(ct);

				auto background = Ptr(GuiSolidBackgroundElement::Create());
				background->SetColor({ 255,255,255 });
				ct->SetOwnedElement(background);
				return ct;
			};
		}
	};
}
using namespace unittest_framework_tests;

TEST_FILE
{
	TEST_CATEGORY(L"DOM Recovery")
	{
		TEST_CATEGORY(L"Empty Window")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->GetEvents()->OnControllerConnect();
				auto emptyWindowTheme = Ptr(new EmptyWindowTheme);
				theme::RegisterTheme(emptyWindowTheme);

				GuiWindow window(theme::ThemeName::Window);
				window.SetClientSize({ 640,480 });
				window.MoveToScreenCenter();

				protocol->OnNextIdleFrame([&]()
				{
					TEST_ASSERT(protocol->GetLoggedRenderingResults().Count() == 1);
					window.Hide();
				});

				GetApplication()->Run(&window);
				theme::UnregisterTheme(emptyWindowTheme->Name);
			});
			GacUIUnitTest_Start(WString::Unmanaged(L"DomRecovery/EmptyWindow"));
		});

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
		GacUIUnitTest_Start(WString::Unmanaged(L"HelloWorld"));
	});
}
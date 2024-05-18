#include "../../../Source//UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
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

			protocol->OnNextIdleFrame([=, &window]()
			{
				TEST_ASSERT(protocol->GetLoggedTrace().frames->Count() == 1);
				window.Hide();
			});

			GetApplication()->Run(&window);
			theme::UnregisterTheme(darkskinTheme->Name);
		});
		GacUIUnitTest_Start(WString::Unmanaged(L"HelloWorld"));
	});

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

				protocol->OnNextIdleFrame([=, &window]()
				{
					TEST_ASSERT(protocol->GetLoggedTrace().frames->Count() == 1);
					window.Hide();
				});

				GetApplication()->Run(&window);
				theme::UnregisterTheme(emptyWindowTheme->Name);
			});
			GacUIUnitTest_Start(WString::Unmanaged(L"DomRecovery/EmptyWindow"));
		});

		TEST_CATEGORY(L"Clipping")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->GetEvents()->OnControllerConnect();
				auto emptyWindowTheme = Ptr(new EmptyWindowTheme);
				theme::RegisterTheme(emptyWindowTheme);

				GuiWindow window(theme::ThemeName::Window);
				{
					auto bounds = new GuiBoundsComposition;
					bounds->SetExpectedBounds({ {20,-20},{100,520} });
					window.GetContainerComposition()->AddChild(bounds);

					auto element = Ptr(GuiSolidBorderElement::Create());
					element->SetColor(Color(255, 0, 0));
					bounds->SetOwnedElement(element);
				}
				{
					auto bounds = new GuiBoundsComposition;
					bounds->SetExpectedBounds({ {520,-20},{100,520} });
					window.GetContainerComposition()->AddChild(bounds);

					auto element = Ptr(GuiSolidBorderElement::Create());
					element->SetColor(Color(0, 255, 0));
					bounds->SetOwnedElement(element);
				}
				{
					auto bounds = new GuiBoundsComposition;
					bounds->SetExpectedBounds({ {10,10},{620,460} });
					window.GetContainerComposition()->AddChild(bounds);
				}
				{
					auto bounds = new GuiBoundsComposition;
					bounds->SetExpectedBounds({ {-30,10},{680,100} });
					window.GetContainerComposition()->Children()[2]->AddChild(bounds);

					auto element = Ptr(GuiSolidBorderElement::Create());
					element->SetColor(Color(255, 0, 255));
					bounds->SetOwnedElement(element);
				}
				{
					auto bounds = new GuiBoundsComposition;
					bounds->SetExpectedBounds({ {-30,350},{680,100} });
					window.GetContainerComposition()->Children()[2]->AddChild(bounds);

					auto element = Ptr(GuiSolidBorderElement::Create());
					element->SetColor(Color(0, 255, 0));
					bounds->SetOwnedElement(element);
				}

				window.SetClientSize({ 640,480 });
				window.MoveToScreenCenter();

				protocol->OnNextIdleFrame([=, &window]()
				{
					TEST_ASSERT(protocol->GetLoggedTrace().frames->Count() == 1);
					window.Hide();
				});

				GetApplication()->Run(&window);
				theme::UnregisterTheme(emptyWindowTheme->Name);
			});
			GacUIUnitTest_Start(WString::Unmanaged(L"DomRecovery/Clipping"));
		});
	});

	TEST_CATEGORY(L"Resources and Operations")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Hello, world!" ClientSize="x:640 y:480">
        <Button ref.Name="buttonOK" Text="OK">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
          <ev.Clicked-eval><![CDATA[ {self.Close();} ]]></ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CATEGORY(L"Window with OK Button")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame([=]()
				{
					auto window = GetApplication()->GetMainWindow();
					{
						auto buttonOKValue = window->GetNamedObject(L"buttonOK").GetRawPtr();
						TEST_ASSERT(buttonOKValue);
						auto buttonOK = buttonOKValue->SafeAggregationCast<GuiButton>();
						TEST_ASSERT(buttonOK);
						TEST_ASSERT(buttonOK->GetText() == L"OK");
					}
					TEST_ASSERT(protocol->GetLoggedTrace().frames->Count() == 1);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"UnitTestFramework/WindowWithOKButton"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				{},
				resource
				);
		});

		TEST_CATEGORY(L"Click Button and Close")
		{
		});
	});
}
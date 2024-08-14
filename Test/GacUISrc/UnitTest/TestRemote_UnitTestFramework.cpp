#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
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
	TEST_CASE(L"Hello, world!")
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
		TEST_CASE(L"Empty Window")
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

		TEST_CASE(L"Clipping")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->GetEvents()->OnControllerConnect();
				auto emptyWindowTheme = Ptr(new EmptyWindowTheme);
				theme::RegisterTheme(emptyWindowTheme);

				GuiWindow window(theme::ThemeName::Window);
				window.GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
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
					element->SetColor(Color(0, 0, 255));
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

	TEST_CATEGORY(L"Window with OK Button")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Hello, world!" ClientSize="x:320 y:240">
        <Button ref.Name="buttonOK" Text="OK">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
          <ev.Clicked-eval><![CDATA[ {
            Application::GetApplication().InvokeInMainThread(self, func():void{self.Hide();});
          } ]]></ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		TEST_CASE(L"Open and Close")
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

						TEST_ASSERT(buttonOK == TryFindObjectByName<GuiButton>(window, L"buttonOK"));
						TEST_ASSERT(buttonOK == FindObjectByName<GuiButton>(window, L"buttonOK"));
						TEST_ASSERT(buttonOK == TryFindControlByText<GuiButton>(window, L"OK"));
						TEST_ASSERT(buttonOK == FindControlByText<GuiButton>(window, L"OK"));
					}
					{
						TEST_EXCEPTION(TryFindObjectByName<GuiWindow>(window, L"buttonOK"), Error, [](const Error& error)
						{
							auto expected = WString::Unmanaged(L"#The object assigned by the name is not in the specified type.");
							auto actual = WString::Unmanaged(error.Description());
							TEST_ASSERT(actual.Right(expected.Length()) == expected);
						});
						TEST_EXCEPTION(FindObjectByName<GuiWindow>(window, L"buttonOK"), Error, [](const Error& error)
						{
							auto expected = WString::Unmanaged(L"#The object assigned by the name is not in the specified type.");
							auto actual = WString::Unmanaged(error.Description());
							TEST_ASSERT(actual.Right(expected.Length()) == expected);
						});
						TEST_EXCEPTION(TryFindControlByText<GuiWindow>(window, L"OK"), Error, [](const Error& error)
						{
							auto expected = WString::Unmanaged(L"#The object with the specified text is not in the specified type.");
							auto actual = WString::Unmanaged(error.Description());
							TEST_ASSERT(actual.Right(expected.Length()) == expected);
						});
						TEST_EXCEPTION(FindControlByText<GuiWindow>(window, L"OK"), Error, [](const Error& error)
						{
							auto expected = WString::Unmanaged(L"#The object with the specified text is not in the specified type.");
							auto actual = WString::Unmanaged(error.Description());
							TEST_ASSERT(actual.Right(expected.Length()) == expected);
						});
					}
					{
						TEST_ASSERT(nullptr == TryFindObjectByName<GuiButton>(window, L"Whatever"));
						TEST_ASSERT(nullptr == TryFindControlByText<GuiButton>(window, L"Whatever"));
						TEST_EXCEPTION(FindObjectByName<GuiButton>(window, L"Whatever"), Error, [](const Error& error)
						{
							auto expected = WString::Unmanaged(L"#The name has not been used.");
							auto actual = WString::Unmanaged(error.Description());
							TEST_ASSERT(actual.Right(expected.Length()) == expected);
						});
						TEST_EXCEPTION(FindControlByText<GuiButton>(window, L"Whatever"), Error, [](const Error& error)
						{
							auto expected = WString::Unmanaged(L"#The control with the specified text does not exist.");
							auto actual = WString::Unmanaged(error.Description());
							TEST_ASSERT(actual.Right(expected.Length()) == expected);
						});
					}
					TEST_ASSERT(protocol->GetLoggedTrace().frames->Count() == 1);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"UnitTestFramework/WindowWithOKButton"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
				);
		});

		TEST_CASE(L"Click Button and Close in Separated IO Commands")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonOK = TryFindObjectByName<GuiButton>(window, L"buttonOK");
					protocol->MouseMove(protocol->LocationOf(buttonOK));
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					protocol->_LDown();
				});
				protocol->OnNextIdleFrame(L"Press", [=]()
				{
					protocol->_LUp();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"UnitTestFramework/WindowWithOKButton_ClickInSteps"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
				);
		});

		TEST_CASE(L"Click Button and Close in Combined IO Commands")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame([=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonOK = TryFindObjectByName<GuiButton>(window, L"buttonOK");
					protocol->LClick(protocol->LocationOf(buttonOK));
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"UnitTestFramework/WindowWithOKButton_Click"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
				);
		});
	});

	TEST_CASE(L"Display an Image")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig">
    <Xml name="ImageData">
      <ImageData>
        <Image Path="Cake" Format="Jpeg" Width="291" Height="212"/>
      </ImageData>
    </Xml>
  </Folder>
  <Image name="Cake" content="File">cake.jpeg</Image>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Image" ClientSize="x:320 y:240">
        <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElement">
          <ImageFrame Image-uri="res://Cake"/>
        </Bounds>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame([=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"UnitTestFramework/SingleImage"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});
}
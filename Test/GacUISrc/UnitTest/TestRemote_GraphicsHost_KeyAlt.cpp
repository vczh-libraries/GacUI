#include "TestRemote_GraphicsHost_Shared.h"

namespace remote_graphics_host_tests
{
	template<typename ...TArgs>
	void AssertAltLabels(List<GuiControl*>& altControls, TArgs&& ...args)
	{
		const wchar_t* expected[] = { args... };
		TEST_ASSERT(altControls.Count() == sizeof...(args));
		for (vint i = 0; i < altControls.Count(); i++)
		{
			GuiLabel* altLabel = nullptr;
			auto altAction = altControls[i]->QueryTypedService<IGuiAltAction>();
			auto altComposition = altAction->GetAltComposition();
			for (auto candidate : altComposition->Children())
			{
				if (auto label = dynamic_cast<GuiLabel*>(candidate->GetRelatedControl()))
				{
					if (label->GetVisible() && label->GetControlThemeName() == theme::ThemeName::ShortcutKey)
					{
						altLabel = label;
						break;
					}
				}
			}

			if (altLabel)
			{
				TEST_ASSERT(expected[i] != nullptr);
				TEST_ASSERT(altLabel->GetText() == expected[i]);
			}
			else
			{
				TEST_ASSERT(expected[i] == nullptr);
			}
		}
	}

	class AltContainer : public GuiCustomControl, protected GuiAltActionHostBase
	{
	protected:

		IGuiAltActionHost* GetActivatingAltHost() override
		{
			return this;
		}

	public:

		AltContainer()
			: GuiCustomControl(theme::ThemeName::CustomControl)
		{
			SetFocusableComposition(boundsComposition);
			SetAltComposition(boundsComposition);
			SetAltControl(this, false);
		}
	};

	AltContainer* CreateContainer(List<WString>& eventLogs, List<GuiControl*>& altControls, GuiWindow* controlHost, const wchar_t* alt, const wchar_t* name)
	{
		auto control = new AltContainer;
		control->SetAlt(alt);
		controlHost->AddChild(control);
		altControls.Add(control);
		return control;
	}

	GuiButton* CreateButton(List<WString>& eventLogs, List<GuiControl*>& altControls, GuiWindow* controlHost, const wchar_t* alt, const wchar_t* name)
	{
		auto control = new GuiButton(theme::ThemeName::Button);
		control->SetAlt(alt);
		control->Clicked.AttachLambda([&eventLogs, name](GuiGraphicsComposition*, GuiEventArgs&)
		{
			eventLogs.Add(WString::Unmanaged(name));
		});
		controlHost->AddChild(control);
		altControls.Add(control);
		return control;
	}

	void Contain(GuiControl* container, GuiControl* child)
	{
		if (auto parent = child->GetBoundsComposition()->GetParent())
		{
			parent->RemoveChild(child->GetBoundsComposition());
		}
		container->AddChild(child);
	}
}
using namespace remote_graphics_host_tests;

TEST_FILE
{
#define CREATE_CONTAINER(ALT, NAME) CreateContainer(eventLogs, altControls, controlHost, ALT, NAME)
#define CREATE_BUTTON(ALT, NAME) CreateButton(eventLogs, altControls, controlHost, ALT, NAME)

	TEST_CATEGORY(L"Alt single level and escape / activate")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			CREATE_BUTTON(L"A", L"A");
			CREATE_BUTTON(L"B", L"B");
			CREATE_BUTTON(L"C", L"C");
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", L"[B]", L"[C]");
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]", L"[B]", L"[C]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", L"[B]", L"[C]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"A");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", L"[B]", L"[C]");
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"B");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", L"[B]", L"[C]");
			pressKey(VKEY::KEY_D);
			AssertAltLabels(altControls, L"[A]", L"[B]", L"[C]");
			pressKey(VKEY::KEY_C);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"C");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Alt single level with conflict")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			CREATE_BUTTON(L"A", L"A");
			CREATE_BUTTON(L"A", L"B");
			CREATE_BUTTON(L"B", L"C");
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", L"A[1]", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", L"A[1]", nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", L"A[1]", nullptr);
			pressKey(VKEY::KEY_0);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"A");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", L"A[1]", nullptr);
			pressKey(VKEY::KEY_NUMPAD1);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"B");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", L"[A]1", L"[B]");
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"C");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Alt fake multiple level and escape / activate")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			auto a = CREATE_BUTTON(L"A", L"A");
			auto b = CREATE_BUTTON(L"B", L"B");
			auto c = CREATE_BUTTON(L"C", L"C");
			Contain(a, b);
			Contain(a, c);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"A");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Alt multiple level and escape / activate")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			auto a = CREATE_CONTAINER(L"A", L"A");
			auto b = CREATE_BUTTON(L"B", L"B");
			auto c = CREATE_BUTTON(L"C", L"C");
			Contain(a, b);
			Contain(a, c);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_C);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"B");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, L"[B]", L"[C]");
			pressKey(VKEY::KEY_C);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"C");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Alt multiple level with conflict and cancel")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			auto a = CREATE_CONTAINER(L"A", L"A");
			auto b = CREATE_BUTTON(L"A", L"B");
			auto c = CREATE_BUTTON(L"A", L"C");
			auto d = CREATE_BUTTON(L"B", L"D");
			Contain(a, b);
			Contain(a, c);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr, L"[B]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr, L"[B]");
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, nullptr, L"[A]0", L"[A]1", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr, L"[B]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]", nullptr, nullptr, L"[B]");
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"D");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Alt multiple level with conflict and going back")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			auto a = CREATE_CONTAINER(L"A", L"A");
			auto b = CREATE_BUTTON(L"B", L"B");
			auto c = CREATE_BUTTON(L"B", L"C");
			auto d = CREATE_CONTAINER(L"A", L"D");
			auto e = CREATE_BUTTON(L"B", L"E");
			Contain(a, b);
			Contain(a, c);
			Contain(d, e);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", nullptr, nullptr, L"A[1]", nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", nullptr, nullptr, L"A[1]", nullptr);
			pressKey(VKEY::KEY_0);
			AssertAltLabels(altControls, nullptr, L"[B]0", L"[B]1", nullptr, nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, L"B[0]", L"B[1]", nullptr, nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, nullptr, L"[B]0", L"[B]1", nullptr, nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", nullptr, nullptr, L"A[1]", nullptr);
			pressKey(VKEY::KEY_0);
			AssertAltLabels(altControls, nullptr, L"[B]0", L"[B]1", nullptr, nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, L"B[0]", L"B[1]", nullptr, nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, nullptr, L"[B]0", L"[B]1", nullptr, nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[0]", nullptr, nullptr, L"A[1]", nullptr);
			pressKey(VKEY::KEY_NUMPAD1);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, L"[B]");
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]0", nullptr, nullptr, L"[A]1", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Alt multiple level with long keys")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
		};

		protocol.OnNextFrame([&]()
		{
			auto a = CREATE_CONTAINER(L"AB", L"A");
			auto b = CREATE_BUTTON(L"XY", L"B");
			auto c = CREATE_BUTTON(L"XZ", L"C");
			auto d = CREATE_CONTAINER(L"AB", L"D");
			auto e = CREATE_BUTTON(L"B", L"E");
			Contain(a, b);
			Contain(a, c);
			Contain(d, e);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"AB[0]", nullptr, nullptr, L"AB[1]", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"AB[0]", nullptr, nullptr, L"AB[1]", nullptr);
			pressKey(VKEY::KEY_0);
			AssertAltLabels(altControls, nullptr, L"[X]Y", L"[X]Z", nullptr, nullptr);
			pressKey(VKEY::KEY_X);
			AssertAltLabels(altControls, nullptr, L"X[Y]", L"X[Z]", nullptr, nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_ESCAPE);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs);

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"AB[0]", nullptr, nullptr, L"AB[1]", nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"AB[0]", nullptr, nullptr, L"AB[1]", nullptr);
			pressKey(VKEY::KEY_0);
			AssertAltLabels(altControls, nullptr, L"[X]Y", L"[X]Z", nullptr, nullptr);
			pressKey(VKEY::KEY_X);
			AssertAltLabels(altControls, nullptr, L"X[Y]", L"X[Z]", nullptr, nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, nullptr, L"[X]Y", L"[X]Z", nullptr, nullptr);
			pressKey(VKEY::KEY_BACK);
			AssertAltLabels(altControls, nullptr, L"[X]Y", L"[X]Z", nullptr, nullptr);
			pressKey(VKEY::KEY_X);
			AssertAltLabels(altControls, nullptr, L"X[Y]", L"X[Z]", nullptr, nullptr);
			pressKey(VKEY::KEY_Y);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"B");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"AB[0]", nullptr, nullptr, L"AB[1]", nullptr);
			pressKey(VKEY::KEY_0);
			AssertAltLabels(altControls, nullptr, L"[X]Y", L"[X]Z", nullptr, nullptr);
			pressKey(VKEY::KEY_X);
			AssertAltLabels(altControls, nullptr, L"X[Y]", L"X[Z]", nullptr, nullptr);
			pressKey(VKEY::KEY_Z);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"C");

			pressKey(VKEY::KEY_MENU);
			AssertAltLabels(altControls, L"[A]B0", nullptr, nullptr, L"[A]B1", nullptr);
			pressKey(VKEY::KEY_A);
			AssertAltLabels(altControls, L"A[B]0", nullptr, nullptr, L"A[B]1", nullptr);
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, L"AB[0]", nullptr, nullptr, L"AB[1]", nullptr);
			pressKey(VKEY::KEY_1);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, L"[B]");
			pressKey(VKEY::KEY_B);
			AssertAltLabels(altControls, nullptr, nullptr, nullptr, nullptr, nullptr);
			AssertEventLogs(eventLogs, L"E");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

#undef CREATE_BUTTON
#undef CREATE_CONTAINER
}
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
}
using namespace remote_graphics_host_tests;

TEST_FILE
{
#define CREATE_BUTTON(ALT, NAME) CreateButton(eventLogs, altControls, controlHost, ALT, NAME)

	TEST_CATEGORY(L"Alt single level and escape / activate")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
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
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Alt single level with conflict")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		auto pressKey = [&](VKEY key)
		{
			protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, key));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, key));
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
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Alt multiple level with conflict and cancel")
	{
	});

	TEST_CATEGORY(L"Alt multiple level with conflict and going back")
	{
	});

	TEST_CATEGORY(L"Alt multiple level with conflict and activate")
	{
	});

#undef CREATE_BUTTON
}
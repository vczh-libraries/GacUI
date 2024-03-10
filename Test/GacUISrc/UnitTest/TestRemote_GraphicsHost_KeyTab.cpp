#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
#define WITH(KEYS) ) + WString::Unmanaged(KEYS) + WString::Unmanaged(

#define ASSERT_FOCUS(KEYS)\
	(WString::Unmanaged(to) + WString::Unmanaged(L".GotFocus()")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyUp(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyUp(" WITH(KEYS) L":Tab)")).Buffer()\

#define ASSERT_FOCUS_CONTAINER(KEYS)\
	(WString::Unmanaged(to) + WString::Unmanaged(L".GotFocus()")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->") + WString::Unmanaged(container) + WString::Unmanaged(L".KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyUp(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->") + WString::Unmanaged(container) + WString::Unmanaged(L".KeyUp(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyUp(" WITH(KEYS) L":Tab)")).Buffer()\

#define ASSERT_NO_FOCUS(KEYS)\
	(WString::Unmanaged(L"host.bounds.KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(L"host.bounds.KeyDown(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(L"host.bounds.KeyPreview(" WITH(KEYS) L":Tab)")).Buffer(),\
	(WString::Unmanaged(L"host.bounds.KeyUp(" WITH(KEYS) L":Tab)")).Buffer()\

	List<WString> eventLogs;

	auto assertFocusOn = [&](const wchar_t* to, bool shift = false)
	{
		AssertEventLogs(
			eventLogs,
			ASSERT_FOCUS(shift ? L"S" : L"")
			);
	};

	auto assertFocusTransition = [&](const wchar_t* from, const wchar_t* to, bool shift = false)
	{
		AssertEventLogs(
			eventLogs,
			(WString::Unmanaged(from) + WString::Unmanaged(L".LostFocus()")).Buffer(),
			ASSERT_FOCUS(shift ? L"S" : L"")
			);
	};

	auto assertFocusTransitionWithContainer = [&](const wchar_t* from, const wchar_t* to, const wchar_t* container, bool shift = false)
	{
		AssertEventLogs(
			eventLogs,
			(WString::Unmanaged(from) + WString::Unmanaged(L".LostFocus()")).Buffer(),
			ASSERT_FOCUS_CONTAINER(shift ? L"S" : L"")
			);
	};

	auto assertFocusNoFocus = [&](bool shift = false)
	{
		AssertEventLogs(
			eventLogs,
			ASSERT_NO_FOCUS(shift ? L"S" : L"")
			);
	};

	TEST_CATEGORY(L"Tab through all buttons")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			for (auto& button : buttons)
			{
				button = new GuiButton(theme::ThemeName::Button);;
				controlHost->AddChild(button);
			}

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(buttons[0]->GetFocusableComposition(), L"0", eventLogs);
			AttachAndLogEvents(buttons[1]->GetFocusableComposition(), L"1", eventLogs);
			AttachAndLogEvents(buttons[2]->GetFocusableComposition(), L"2", eventLogs);
			AttachAndLogEvents(buttons[3]->GetFocusableComposition(), L"3", eventLogs);
			AttachAndLogEvents(buttons[4]->GetFocusableComposition(), L"4", eventLogs);

			buttons[1]->SetEnabled(false);
			buttons[3]->SetVisible(false);
		});

		protocol.OnNextFrame([&]()
		{
			pressTab();
			assertFocusOn(L"0");

			pressTab();
			assertFocusTransition(L"0", L"2");

			pressTab();
			assertFocusTransition(L"2", L"4");

			pressTab();
			assertFocusTransition(L"4", L"0");

			buttons[1]->SetEnabled(true);
			buttons[3]->SetVisible(true);

			pressTab();
			assertFocusTransition(L"0", L"1");

			pressTab();
			assertFocusTransition(L"1", L"2");

			pressTab();
			assertFocusTransition(L"2", L"3");

			pressTab();
			assertFocusTransition(L"3", L"4");

			pressTab();
			assertFocusTransition(L"4", L"0");
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Shift+Tab through all buttons")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, true, false, VKEY::KEY_TAB));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_TAB));
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			for (auto& button : buttons)
			{
				button = new GuiButton(theme::ThemeName::Button);;
				controlHost->AddChild(button);
			}

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(buttons[0]->GetFocusableComposition(), L"0", eventLogs);
			AttachAndLogEvents(buttons[1]->GetFocusableComposition(), L"1", eventLogs);
			AttachAndLogEvents(buttons[2]->GetFocusableComposition(), L"2", eventLogs);
			AttachAndLogEvents(buttons[3]->GetFocusableComposition(), L"3", eventLogs);
			AttachAndLogEvents(buttons[4]->GetFocusableComposition(), L"4", eventLogs);

			buttons[1]->SetEnabled(false);
			buttons[3]->SetVisible(false);
		});

		protocol.OnNextFrame([&]()
		{
			pressTab();
			assertFocusOn(L"0", true);

			pressTab();
			assertFocusTransition(L"0", L"4", true);

			pressTab();
			assertFocusTransition(L"4", L"2", true);

			pressTab();
			assertFocusTransition(L"2", L"0", true);

			buttons[1]->SetEnabled(true);
			buttons[3]->SetVisible(true);

			pressTab();
			assertFocusTransition(L"0", L"4", true);

			pressTab();
			assertFocusTransition(L"4", L"3", true);

			pressTab();
			assertFocusTransition(L"3", L"2", true);

			pressTab();
			assertFocusTransition(L"2", L"1", true);

			pressTab();
			assertFocusTransition(L"1", L"0", true);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Tab and delete focused buttons")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			for (auto& button : buttons)
			{
				button = new GuiButton(theme::ThemeName::Button);
				controlHost->AddChild(button);
			}

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(buttons[0]->GetFocusableComposition(), L"0", eventLogs);
			AttachAndLogEvents(buttons[1]->GetFocusableComposition(), L"1", eventLogs);
			AttachAndLogEvents(buttons[2]->GetFocusableComposition(), L"2", eventLogs);
			AttachAndLogEvents(buttons[3]->GetFocusableComposition(), L"3", eventLogs);
			AttachAndLogEvents(buttons[4]->GetFocusableComposition(), L"4", eventLogs);

			buttons[1]->SetEnabled(false);
			buttons[3]->SetVisible(false);
		});

		protocol.OnNextFrame([&]()
		{
			pressTab();
			assertFocusOn(L"0");

			SafeDeleteControl(buttons[0]);
			pressTab();
			assertFocusOn(L"2");

			SafeDeleteControl(buttons[2]);
			pressTab();
			assertFocusOn(L"4");

			SafeDeleteControl(buttons[4]);
			pressTab();
			assertFocusNoFocus();

			buttons[1]->SetEnabled(true);
			buttons[3]->SetVisible(true);

			pressTab();
			assertFocusOn(L"1");

			SafeDeleteControl(buttons[1]);
			pressTab();
			assertFocusOn(L"3");

			SafeDeleteControl(buttons[3]);
			pressTab();
			assertFocusNoFocus();
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Tab and delete unfocused buttons")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			for (auto& button : buttons)
			{
				button = new GuiButton(theme::ThemeName::Button);
				controlHost->AddChild(button);
			}

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(buttons[0]->GetFocusableComposition(), L"0", eventLogs);
			AttachAndLogEvents(buttons[1]->GetFocusableComposition(), L"1", eventLogs);
			AttachAndLogEvents(buttons[2]->GetFocusableComposition(), L"2", eventLogs);
			AttachAndLogEvents(buttons[3]->GetFocusableComposition(), L"3", eventLogs);
			AttachAndLogEvents(buttons[4]->GetFocusableComposition(), L"4", eventLogs);

			buttons[1]->SetEnabled(false);
			buttons[3]->SetVisible(false);
		});

		protocol.OnNextFrame([&]()
		{
			pressTab();
			assertFocusOn(L"0");

			SafeDeleteControl(buttons[2]);
			SafeDeleteControl(buttons[4]);
			AssertEventLogs(eventLogs);

			buttons[1]->SetEnabled(true);
			buttons[3]->SetVisible(true);

			pressTab();
			assertFocusTransition(L"0", L"1");

			SafeDeleteControl(buttons[0]);
			SafeDeleteControl(buttons[3]);
			AssertEventLogs(eventLogs);

			SafeDeleteControl(buttons[1]);
			pressTab();
			assertFocusNoFocus();
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Tab and delete container of focused button")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			for (auto& button : buttons)
			{
				button = new GuiButton(theme::ThemeName::Button);
			}

			buttons[2]->AddChild(buttons[0]);
			buttons[2]->AddChild(buttons[1]);
			buttons[2]->AddChild(buttons[3]);
			controlHost->AddChild(buttons[2]);
			controlHost->AddChild(buttons[4]);

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(buttons[0]->GetFocusableComposition(), L"0", eventLogs);
			AttachAndLogEvents(buttons[1]->GetFocusableComposition(), L"1", eventLogs);
			AttachAndLogEvents(buttons[2]->GetFocusableComposition(), L"2", eventLogs);
			AttachAndLogEvents(buttons[3]->GetFocusableComposition(), L"3", eventLogs);
			AttachAndLogEvents(buttons[4]->GetFocusableComposition(), L"4", eventLogs);

			buttons[1]->SetEnabled(false);
			buttons[3]->SetVisible(false);
		});

		protocol.OnNextFrame([&]()
		{
			pressTab();
			assertFocusOn(L"2");

			pressTab();
			assertFocusTransitionWithContainer(L"2", L"0", L"2");

			pressTab();
			assertFocusTransition(L"0", L"4");

			pressTab();
			assertFocusTransition(L"4", L"2");

			pressTab();
			assertFocusTransitionWithContainer(L"2", L"0", L"2");

			SafeDeleteControl(buttons[2]);
			pressTab();
			assertFocusOn(L"4");

			SafeDeleteControl(buttons[4]);
			pressTab();
			assertFocusNoFocus();
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Shift+Tab and delete container of focused button")
	{
		GraphicsHostProtocol protocol;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(false, true, false, VKEY::KEY_TAB));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_TAB));
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			for (auto& button : buttons)
			{
				button = new GuiButton(theme::ThemeName::Button);
			}

			buttons[2]->AddChild(buttons[0]);
			buttons[2]->AddChild(buttons[1]);
			buttons[2]->AddChild(buttons[3]);
			controlHost->AddChild(buttons[2]);
			controlHost->AddChild(buttons[4]);

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(buttons[0]->GetFocusableComposition(), L"0", eventLogs);
			AttachAndLogEvents(buttons[1]->GetFocusableComposition(), L"1", eventLogs);
			AttachAndLogEvents(buttons[2]->GetFocusableComposition(), L"2", eventLogs);
			AttachAndLogEvents(buttons[3]->GetFocusableComposition(), L"3", eventLogs);
			AttachAndLogEvents(buttons[4]->GetFocusableComposition(), L"4", eventLogs);

			buttons[1]->SetEnabled(false);
			buttons[3]->SetVisible(false);
		});

		protocol.OnNextFrame([&]()
		{
			pressTab();
			assertFocusOn(L"2", true);

			pressTab();
			assertFocusTransition(L"2", L"4", true);

			pressTab();
			assertFocusTransitionWithContainer(L"4", L"0", L"2", true);

			pressTab();
			assertFocusTransition(L"0", L"2", true);

			pressTab();
			assertFocusTransition(L"2", L"4", true);

			pressTab();
			assertFocusTransitionWithContainer(L"4", L"0", L"2", true);

			SafeDeleteControl(buttons[2]);
			pressTab();
			assertFocusOn(L"4", true);

			SafeDeleteControl(buttons[4]);
			pressTab();
			assertFocusNoFocus(true);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

#undef ASSERT_NO_FOCUS
#undef ASSERT_FOCUS_CONTAINER
#undef ASSERT_FOCUS
#undef WITH
}
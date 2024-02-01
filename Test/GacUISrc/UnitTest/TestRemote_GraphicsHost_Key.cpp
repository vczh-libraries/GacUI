#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Focus and press keys on buttons")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiButton* x = nullptr, * y = nullptr, * z = nullptr;

		auto pressKeys = [&]()
		{
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_A));
			protocol.events->OnIOChar(MakeCharInfo(false, true, false, L'B'));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, true, VKEY::KEY_C));
		};

		auto assertKeysOnX = [&]()
		{
			AssertEventLogs(
				eventLogs,

				L"x->host.bounds.KeyPreview(C:A)",
				L"x.KeyPreview(C:A)",
				L"x.KeyDown(C:A)",
				L"x->host.bounds.KeyDown(C:A)",

				L"x->host.bounds.CharPreview(S:B)",
				L"x.CharPreview(S:B)",
				L"x.Char(S:B)",
				L"x->host.bounds.Char(S:B)",

				L"x->host.bounds.KeyPreview(A:C)",
				L"x.KeyPreview(A:C)",
				L"x.KeyUp(A:C)",
				L"x->host.bounds.KeyUp(A:C)"
				);
		};

		auto assertKeysOnY = [&]()
		{
			AssertEventLogs(
				eventLogs,

				L"y->host.bounds.KeyPreview(C:A)",
				L"y->x.KeyPreview(C:A)",
				L"y.KeyPreview(C:A)",
				L"y.KeyDown(C:A)",
				L"y->x.KeyDown(C:A)",
				L"y->host.bounds.KeyDown(C:A)",

				L"y->host.bounds.CharPreview(S:B)",
				L"y->x.CharPreview(S:B)",
				L"y.CharPreview(S:B)",
				L"y.Char(S:B)",
				L"y->x.Char(S:B)",
				L"y->host.bounds.Char(S:B)",

				L"y->host.bounds.KeyPreview(A:C)",
				L"y->x.KeyPreview(A:C)",
				L"y.KeyPreview(A:C)",
				L"y.KeyUp(A:C)",
				L"y->x.KeyUp(A:C)",
				L"y->host.bounds.KeyUp(A:C)"
				);
		};

		auto assertKeysOnZ = [&]()
		{
			AssertEventLogs(
				eventLogs,

				L"z->host.bounds.KeyPreview(C:A)",
				L"z.KeyPreview(C:A)",
				L"z.KeyDown(C:A)",
				L"z->host.bounds.KeyDown(C:A)",

				L"z->host.bounds.CharPreview(S:B)",
				L"z.CharPreview(S:B)",
				L"z.Char(S:B)",
				L"z->host.bounds.Char(S:B)",

				L"z->host.bounds.KeyPreview(A:C)",
				L"z.KeyPreview(A:C)",
				L"z.KeyUp(A:C)",
				L"z->host.bounds.KeyUp(A:C)"
				);
		};

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();
			x = new GuiButton(theme::ThemeName::Button);
			y = new GuiButton(theme::ThemeName::Button);
			z = new GuiButton(theme::ThemeName::Button);

			x->AddChild(y);
			controlHost->AddChild(x);
			controlHost->AddChild(z);

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(x->GetFocusableComposition(), L"x", eventLogs);
			AttachAndLogEvents(y->GetFocusableComposition(), L"y", eventLogs);
			AttachAndLogEvents(z->GetFocusableComposition(), L"z", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			y->SetFocused();
			AssertEventLogs(
				eventLogs,
				L"y.GotFocus()"
				);

			y->SetFocused();
			pressKeys();
			assertKeysOnY();

			z->SetFocused();
			AssertEventLogs(
				eventLogs,
				L"y.LostFocus()",
				L"z.GotFocus()"
				);

			z->SetFocused();
			pressKeys();
			assertKeysOnZ();

			y->SetEnabled(false);
			y->SetFocused();
			pressKeys();
			assertKeysOnZ();

			y->SetEnabled(true);
			x->SetEnabled(false);
			y->SetFocused();
			pressKeys();
			assertKeysOnZ();

			x->SetEnabled(true);
			y->SetEnabled(false);
			x->SetFocused();
			y->SetFocused();
			AssertEventLogs(
				eventLogs,
				L"z.LostFocus()",
				L"x.GotFocus()"
				);
			pressKeys();
			assertKeysOnX();

			y->SetEnabled(true);
			y->SetFocused();
			AssertEventLogs(
				eventLogs,
				L"x.LostFocus()",
				L"y.GotFocus()"
				);

			pressKeys();
			assertKeysOnY();
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

#define ASSERT_FOCUS\
	(WString::Unmanaged(to) + WString::Unmanaged(L".GotFocus()")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyPreview(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyPreview(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyUp(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyUp(:TAB)")).Buffer()\

#define ASSERT_FOCUS_CONTAINER\
	(WString::Unmanaged(to) + WString::Unmanaged(L".GotFocus()")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyPreview(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->") + WString::Unmanaged(container) + WString::Unmanaged(L".KeyPreview(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyPreview(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L".KeyUp(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->") + WString::Unmanaged(container) + WString::Unmanaged(L".KeyUp(:TAB)")).Buffer(),\
	(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyUp(:TAB)")).Buffer()\

#define ASSERT_NO_FOCUS\
	L"host.bounds.KeyPreview(:TAB)",\
	L"host.bounds.KeyDown(:TAB)",\
	L"host.bounds.KeyPreview(:TAB)",\
	L"host.bounds.KeyUp(:TAB)"\

	{
		List<WString> eventLogs;

		auto assertFocusOn = [&](const wchar_t* to)
		{
			AssertEventLogs(
				eventLogs,
				ASSERT_FOCUS
				);
		};

		auto assertFocusTransition = [&](const wchar_t* from, const wchar_t* to)
		{
			AssertEventLogs(
				eventLogs,
				(WString::Unmanaged(from) + WString::Unmanaged(L".LostFocus()")).Buffer(),
				ASSERT_FOCUS
				);
		};

		auto assertFocusTransitionWithContainer = [&](const wchar_t* from, const wchar_t* to, const wchar_t* container)
		{
			AssertEventLogs(
				eventLogs,
				(WString::Unmanaged(from) + WString::Unmanaged(L".LostFocus()")).Buffer(),
				ASSERT_FOCUS_CONTAINER
				);
		};

		auto assertFocusNoFocus = [&]()
		{
			AssertEventLogs(
				eventLogs,
				ASSERT_NO_FOCUS
				);
		};

		TEST_CATEGORY(L"Tab through all buttons")
		{
			GraphicsHostProtocol protocol;
			GuiWindow* controlHost = nullptr;
			GuiButton* buttons[5];

			auto pressTab = [&]()
			{
				protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
				protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
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
			BatchedProtocol batchedProtocol(&protocol);
			SetupRemoteNativeController(&batchedProtocol);
			SetGuiMainProxy({});
		});

		TEST_CATEGORY(L"Delete focused buttons")
		{
			GraphicsHostProtocol protocol;
			GuiWindow* controlHost = nullptr;
			GuiButton* buttons[5];

			auto pressTab = [&]()
			{
				protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
				protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
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
			BatchedProtocol batchedProtocol(&protocol);
			SetupRemoteNativeController(&batchedProtocol);
			SetGuiMainProxy({});
		});

		TEST_CATEGORY(L"Delete unfocused buttons")
		{
			GraphicsHostProtocol protocol;
			GuiWindow* controlHost = nullptr;
			GuiButton* buttons[5];

			auto pressTab = [&]()
			{
				protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
				protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
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
			BatchedProtocol batchedProtocol(&protocol);
			SetupRemoteNativeController(&batchedProtocol);
			SetGuiMainProxy({});
		});

		TEST_CATEGORY(L"Delete focused buttons")
		{
			GraphicsHostProtocol protocol;
			GuiWindow* controlHost = nullptr;
			GuiButton* buttons[5];

			auto pressTab = [&]()
			{
				protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
				protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
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
			BatchedProtocol batchedProtocol(&protocol);
			SetupRemoteNativeController(&batchedProtocol);
			SetGuiMainProxy({});
		});

		TEST_CATEGORY(L"Delete container of focused button")
		{
			GraphicsHostProtocol protocol;
			GuiWindow* controlHost = nullptr;
			GuiButton* buttons[5];

			auto pressTab = [&]()
			{
				protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
				protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
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
			BatchedProtocol batchedProtocol(&protocol);
			SetupRemoteNativeController(&batchedProtocol);
			SetGuiMainProxy({});
		});
	}

#undef ASSERT_NO_FOCUS
#undef ASSERT_FOCUS_CONTAINER
#undef ASSERT_FOCUS
}
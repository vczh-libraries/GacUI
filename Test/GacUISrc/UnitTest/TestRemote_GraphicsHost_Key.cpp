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
	TEST_CATEGORY(L"Tab through all buttons")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiButton* buttons[5];

		auto pressTab = [&]()
		{
			protocol.events->OnIOKeyDown(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_TAB));
		};

		auto assertFocusOn = [&](const wchar_t* to)
		{
			AssertEventLogs(
				eventLogs,
				(WString::Unmanaged(to) + WString::Unmanaged(L".GotFocus()")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyPreview(:TAB)")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L".KeyPreview(:TAB)")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L".KeyUp(:TAB)")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyUp(:TAB)")).Buffer()
				);
		};

		auto assertFocusTransition = [&](const wchar_t* from, const wchar_t* to)
		{
			AssertEventLogs(
				eventLogs,
				(WString::Unmanaged(from) + WString::Unmanaged(L".LostFocus()")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L".GotFocus()")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyPreview(:TAB)")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L".KeyPreview(:TAB)")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L".KeyUp(:TAB)")).Buffer(),
				(WString::Unmanaged(to) + WString::Unmanaged(L"->host.bounds.KeyUp(:TAB)")).Buffer()
				);
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

	// TODO:
	//   Delete focused control, until nothing. One control is invisible, another control is disabled.
	//   Delete unfocused control, until nothing. One control is invisible, another control is disabled.
	//   Delete container of the focused control. One control is invisible, another control is disabled.
}
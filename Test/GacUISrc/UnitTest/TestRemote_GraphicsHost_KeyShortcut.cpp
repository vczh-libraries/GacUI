#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Trigger local shortcut key")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiToolstripCommand* c1 = nullptr, * c2 = nullptr;

		protocol.OnNextFrame([&]()
		{
			c1 = new GuiToolstripCommand;
			c1->SetShortcutBuilder(L"Ctrl+X");
			c1->Executed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { eventLogs.Add((L"Ctrl+X")); });
			controlHost->AddComponent(c1);

			c2 = new GuiToolstripCommand;
			c2->SetShortcutBuilder(L"Ctrl+Shift+Y");
			c2->Executed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { eventLogs.Add((L"Ctrl+Shift+Y")); });
			controlHost->AddComponent(c2);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs, L"Ctrl+X");

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs, L"Ctrl+Shift+Y");

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			c1->SetEnabled(false);
			c2->SetEnabled(false);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
	TEST_CATEGORY(L"Trigger global shortcut key")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiToolstripCommand* c1 = nullptr, * c2 = nullptr;

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.globalShortcutKeys.Count() == 0);

			c1 = new GuiToolstripCommand;
			c1->SetShortcutBuilder(L"global:Ctrl+X");
			c1->Executed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { eventLogs.Add((L"Ctrl+X")); });
			controlHost->AddComponent(c1);

			TEST_ASSERT(protocol.globalShortcutKeys.Count() == 1);
			TEST_ASSERT(protocol.globalShortcutKeys[0].id == 1);
			TEST_ASSERT(protocol.globalShortcutKeys[0].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[0].shift == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].code == VKEY::KEY_X);

			c2 = new GuiToolstripCommand;
			c2->SetShortcutBuilder(L"global:Ctrl+Shift+Y");
			c2->Executed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { eventLogs.Add((L"Ctrl+Shift+Y")); });
			controlHost->AddComponent(c2);

			TEST_ASSERT(protocol.globalShortcutKeys.Count() == 2);
			TEST_ASSERT(protocol.globalShortcutKeys[0].id == 1);
			TEST_ASSERT(protocol.globalShortcutKeys[0].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[0].shift == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].code == VKEY::KEY_X);
			TEST_ASSERT(protocol.globalShortcutKeys[1].id == 2);
			TEST_ASSERT(protocol.globalShortcutKeys[1].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].shift == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[1].code == VKEY::KEY_Y);

			protocol.globalShortcutKeys.Clear();
			protocol.events->OnControllerDisconnect();
			protocol.events->OnControllerConnect();

			TEST_ASSERT(protocol.globalShortcutKeys.Count() == 2);
			TEST_ASSERT(protocol.globalShortcutKeys[0].id == 1);
			TEST_ASSERT(protocol.globalShortcutKeys[0].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[0].shift == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].code == VKEY::KEY_X);
			TEST_ASSERT(protocol.globalShortcutKeys[1].id == 2);
			TEST_ASSERT(protocol.globalShortcutKeys[1].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].shift == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[1].code == VKEY::KEY_Y);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.events->OnIOGlobalShortcutKey(0);
			AssertEventLogs(eventLogs);
			protocol.events->OnIOGlobalShortcutKey(1);
			AssertEventLogs(eventLogs, L"Ctrl+X");
			protocol.events->OnIOGlobalShortcutKey(2);
			AssertEventLogs(eventLogs, L"Ctrl+Shift+Y");
			protocol.events->OnIOGlobalShortcutKey(3);
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			c1->SetEnabled(false);
			c2->SetEnabled(false);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.events->OnIOGlobalShortcutKey(0);
			AssertEventLogs(eventLogs);
			protocol.events->OnIOGlobalShortcutKey(1);
			AssertEventLogs(eventLogs);
			protocol.events->OnIOGlobalShortcutKey(2);
			AssertEventLogs(eventLogs);
			protocol.events->OnIOGlobalShortcutKey(3);
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}
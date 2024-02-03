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
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	// TODO:
	//   Global shortcut key, and recover state when connected/disconnected
}
#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Trigger local shortcut key")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiToolstripCommand* c1 = nullptr, * c2 = nullptr, * c3 = nullptr;

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

			c3 = new GuiToolstripCommand;
			c3->SetShortcutBuilder(L"Ctrl+Win+Z");
			c3->Executed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { eventLogs.Add((L"Ctrl+Super+Z")); });
			auto detachedSuperShortcut = c3->GetShortcut();
			TEST_ASSERT(detachedSuperShortcut != nullptr);
			TEST_ASSERT(detachedSuperShortcut->GetName() == L"Ctrl+Super+Z");
			auto detachedShortcutManager = detachedSuperShortcut->GetManager();
			vint shortcutMigrationCount = 0;
			auto shortcutMigrationHandler = c3->DescriptionChanged.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
			{
				shortcutMigrationCount++;
				TEST_ASSERT(c3->GetShortcut() != nullptr);
			});
			controlHost->AddComponent(c3);

			auto superShortcut = c3->GetShortcut();
			TEST_ASSERT(superShortcut != nullptr);
			TEST_ASSERT(superShortcut->GetManager() != detachedShortcutManager);
			TEST_ASSERT(superShortcut->GetName() == L"Ctrl+Super+Z");
			TEST_ASSERT(shortcutMigrationCount == 1);
			c3->SetShortcutBuilder(L"Ctrl+Command+Z");
			TEST_ASSERT(c3->GetShortcut() == superShortcut);
			c3->SetShortcutBuilder(L"Ctrl+Super+Z");
			TEST_ASSERT(c3->GetShortcut() == superShortcut);

			vint descriptionChangedCount = 0;
			auto descriptionChangedHandler = c3->DescriptionChanged.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
			{
				descriptionChangedCount++;
			});
			auto windowsConfig = MakeGlobalConfig();
			windowsConfig.osSuperKeyName = WString::Unmanaged(L"Win");
			protocol.GetEvents()->OnControllerDisconnect();
			protocol.GetEvents()->OnControllerConnect(windowsConfig);
			TEST_ASSERT(c3->GetShortcut() == superShortcut);
			TEST_ASSERT(c3->GetShortcut()->GetName() == L"Ctrl+Win+Z");
			TEST_ASSERT(descriptionChangedCount >= 1);
			c3->DescriptionChanged.Detach(descriptionChangedHandler);
			c3->DescriptionChanged.Detach(shortcutMigrationHandler);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs, L"Ctrl+X");

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs, L"Ctrl+Shift+Y");

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, true, VKEY::KEY_Z));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, true, VKEY::KEY_Z));
			AssertEventLogs(eventLogs, L"Ctrl+Super+Z");

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Z));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Z));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			c1->SetEnabled(false);
			c2->SetEnabled(false);
			c3->SetEnabled(false);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, true, VKEY::KEY_Z));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, true, VKEY::KEY_Z));
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
		GuiToolstripCommand* c1 = nullptr, * c2 = nullptr, * c3 = nullptr;

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
			TEST_ASSERT(protocol.globalShortcutKeys[0].osSuper == false);
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
			TEST_ASSERT(protocol.globalShortcutKeys[0].osSuper == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].code == VKEY::KEY_X);
			TEST_ASSERT(protocol.globalShortcutKeys[1].id == 2);
			TEST_ASSERT(protocol.globalShortcutKeys[1].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].shift == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[1].osSuper == false);
			TEST_ASSERT(protocol.globalShortcutKeys[1].code == VKEY::KEY_Y);

			c3 = new GuiToolstripCommand;
			c3->SetShortcutBuilder(L"global:Ctrl+Shift+Alt+Command+Z");
			c3->Executed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { eventLogs.Add((L"Ctrl+Shift+Alt+Super+Z")); });
			controlHost->AddComponent(c3);

			TEST_ASSERT(protocol.globalShortcutKeys.Count() == 3);
			TEST_ASSERT(protocol.globalShortcutKeys[2].id == 3);
			TEST_ASSERT(protocol.globalShortcutKeys[2].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].shift == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].alt == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].osSuper == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].code == VKEY::KEY_Z);
			TEST_ASSERT(c3->GetShortcut()->GetName() == L"{Ctrl+Shift+Alt+Super+Z}");

			protocol.globalShortcutKeys.Clear();
			protocol.GetEvents()->OnControllerDisconnect();
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());

			TEST_ASSERT(protocol.globalShortcutKeys.Count() == 3);
			TEST_ASSERT(protocol.globalShortcutKeys[0].id == 1);
			TEST_ASSERT(protocol.globalShortcutKeys[0].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[0].shift == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].osSuper == false);
			TEST_ASSERT(protocol.globalShortcutKeys[0].code == VKEY::KEY_X);
			TEST_ASSERT(protocol.globalShortcutKeys[1].id == 2);
			TEST_ASSERT(protocol.globalShortcutKeys[1].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].shift == true);
			TEST_ASSERT(protocol.globalShortcutKeys[1].alt == false);
			TEST_ASSERT(protocol.globalShortcutKeys[1].osSuper == false);
			TEST_ASSERT(protocol.globalShortcutKeys[1].code == VKEY::KEY_Y);
			TEST_ASSERT(protocol.globalShortcutKeys[2].id == 3);
			TEST_ASSERT(protocol.globalShortcutKeys[2].ctrl == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].shift == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].alt == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].osSuper == true);
			TEST_ASSERT(protocol.globalShortcutKeys[2].code == VKEY::KEY_Z);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOGlobalShortcutKey(0);
			AssertEventLogs(eventLogs);
			protocol.GetEvents()->OnIOGlobalShortcutKey(1);
			AssertEventLogs(eventLogs, L"Ctrl+X");
			protocol.GetEvents()->OnIOGlobalShortcutKey(2);
			AssertEventLogs(eventLogs, L"Ctrl+Shift+Y");
			protocol.GetEvents()->OnIOGlobalShortcutKey(3);
			AssertEventLogs(eventLogs, L"Ctrl+Shift+Alt+Super+Z");
			protocol.GetEvents()->OnIOGlobalShortcutKey(4);
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			c1->SetEnabled(false);
			c2->SetEnabled(false);
			c3->SetEnabled(false);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_CONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(true, false, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_CONTROL));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_Y));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_Y));
			AssertEventLogs(eventLogs);

			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyDown(MakeKeyInfo(true, true, false, VKEY::KEY_X));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, true, false, VKEY::KEY_LCONTROL));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_RSHIFT));
			protocol.GetEvents()->OnIOKeyUp(MakeKeyInfo(false, false, false, VKEY::KEY_X));
			AssertEventLogs(eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnIOGlobalShortcutKey(0);
			AssertEventLogs(eventLogs);
			protocol.GetEvents()->OnIOGlobalShortcutKey(1);
			AssertEventLogs(eventLogs);
			protocol.GetEvents()->OnIOGlobalShortcutKey(2);
			AssertEventLogs(eventLogs);
			protocol.GetEvents()->OnIOGlobalShortcutKey(3);
			AssertEventLogs(eventLogs);
			protocol.GetEvents()->OnIOGlobalShortcutKey(4);
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

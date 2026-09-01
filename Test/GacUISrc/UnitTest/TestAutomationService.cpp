#include "../../../Source/GacUI.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService.h"
#include "TestRemote_GraphicsHost_Shared.h"

using namespace vl;
using namespace vl::presentation;
using namespace remote_graphics_host_tests;

class AutomationServiceTestDouble : public AutomationServiceBase
{
protected:
	Nullable<WString> GetNativeWindowId(INativeWindow* window) override
	{
		return {};
	}

	INativeWindow* GetNativeWindow(Nullable<WString> windowId) override
	{
		return nullptr;
	}

	WString RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) override
	{
		handledCount++;
		lastWindowId = windowId;
		lastCommand = ioCommand;
		return WString::Unmanaged(L"Handled: ") + ioCommand;
	}

public:
	INativeAutomationService::IOCommandAvailability availability = INativeAutomationService::IOCommandAvailability::Disabled;
	vint handledCount = 0;
	Nullable<WString> lastWindowId;
	WString lastCommand;

	INativeAutomationService::IOCommandAvailability CanRunIOCommands() override
	{
		return availability;
	}
};

class AutomationMouseListener : public Object, public virtual INativeWindowListener
{
public:
	vint mouse4Down = 0;
	vint mouse4Up = 0;
	vint mouse4DoubleClick = 0;
	vint mouse5Down = 0;
	vint mouse5Up = 0;
	vint mouse5DoubleClick = 0;
	vint mouseMove = 0;
	vint verticalWheel = 0;
	bool allSuper = true;

	void MouseDown(NativeMouseButton button, const NativeWindowMouseInfo& info) override
	{
		allSuper &= info.osSuper;
		if (button == NativeMouseButton::Mouse4) mouse4Down++;
		if (button == NativeMouseButton::Mouse5) mouse5Down++;
	}

	void MouseUp(NativeMouseButton button, const NativeWindowMouseInfo& info) override
	{
		allSuper &= info.osSuper;
		if (button == NativeMouseButton::Mouse4) mouse4Up++;
		if (button == NativeMouseButton::Mouse5) mouse5Up++;
	}

	void MouseDoubleClick(NativeMouseButton button, const NativeWindowMouseInfo& info) override
	{
		allSuper &= info.osSuper;
		if (button == NativeMouseButton::Mouse4) mouse4DoubleClick++;
		if (button == NativeMouseButton::Mouse5) mouse5DoubleClick++;
	}

	void MouseMoving(const NativeWindowMouseInfo& info) override
	{
		allSuper &= info.osSuper;
		mouseMove++;
	}

	void VerticalWheel(const NativeWindowMouseInfo& info) override
	{
		allSuper &= info.osSuper;
		verticalWheel++;
	}
};

TEST_FILE
{
	TEST_CASE(L"AutomationServiceBase: Disabled rejects all IO commands")
	{
		AutomationServiceTestDouble service;
		service.availability = INativeAutomationService::IOCommandAvailability::Disabled;

		TEST_ASSERT(service.RunIOCommand(Nullable<WString>(), L"!Type:x") == L"");
		TEST_ASSERT(service.handledCount == 0);
	});

	TEST_CASE(L"AutomationServiceBase: Enabled forwards arbitrary IO commands")
	{
		AutomationServiceTestDouble service;
		service.availability = INativeAutomationService::IOCommandAvailability::Enabled;

		TEST_ASSERT(service.RunIOCommand(Nullable<WString>(WString::Unmanaged(L"window-1")), L"!Type:x") == L"Handled: !Type:x");
		TEST_ASSERT(service.handledCount == 1);
		TEST_ASSERT(service.lastWindowId);
		TEST_ASSERT(service.lastWindowId.Value() == L"window-1");
		TEST_ASSERT(service.lastCommand == L"!Type:x");
	});

	TEST_CASE(L"AutomationServiceBase: ExitOnly forwards exact Exit and rejects everything else")
	{
		AutomationServiceTestDouble service;
		service.availability = INativeAutomationService::IOCommandAvailability::ExitOnly;

		TEST_ASSERT(service.RunIOCommand(Nullable<WString>(), L"!Exit") == L"Handled: !Exit");
		TEST_ASSERT(service.handledCount == 1);
		TEST_ASSERT(service.lastCommand == L"!Exit");

		TEST_ASSERT(service.RunIOCommand(Nullable<WString>(), L"!Exit ") == L"!Application stopped responding.");
		TEST_ASSERT(service.RunIOCommand(Nullable<WString>(), L"!Type:x") == L"!Application stopped responding.");
		TEST_ASSERT(service.RunIOCommand(Nullable<WString>(), L"???") == L"!Application stopped responding.");
		TEST_ASSERT(service.handledCount == 1);
	});

	TEST_CATEGORY(L"Shared automation: extended mouse buttons and Super aliases")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		IoCommandState state;
		AutomationMouseListener listener;
		List<INativeWindowListener*> listeners;
		listeners.Add(&listener);

		protocol.OnNextFrame([&]()
		{
			auto controller = GetCurrentController();
			auto nativeWindow = controlHost->GetNativeWindow();
			auto run = [&](const wchar_t* command)
			{
				TEST_ASSERT(RunIOCommandOnNativeWindow(&state, controller, nativeWindow, listeners, WString::Unmanaged(command)) == L"Queued");
			};

			run(L"!Mouse4Down:10,20,Win");
			run(L"!Mouse4Up:10,20,Command");
			run(L"!Mouse4Click:10,20,Super");
			run(L"!Mouse4DbClick:10,20,Win");
			run(L"!Mouse5Down:10,20,Command");
			run(L"!Mouse5Up:10,20,Super");
			run(L"!Mouse5Click:10,20,Win");
			run(L"!Mouse5DbClick:10,20,Command");
			run(L"!MouseMove:30,40,Super");
			run(L"!MouseWheelUp:1,Win");
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(listener.mouse4Down == 4);
			TEST_ASSERT(listener.mouse4Up == 4);
			TEST_ASSERT(listener.mouse4DoubleClick == 1);
			TEST_ASSERT(listener.mouse5Down == 4);
			TEST_ASSERT(listener.mouse5Up == 4);
			TEST_ASSERT(listener.mouse5DoubleClick == 1);
			TEST_ASSERT(listener.mouseMove == 2);
			TEST_ASSERT(listener.verticalWheel == 1);
			TEST_ASSERT(listener.allSuper);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}

#include "../../../Source/GacUI.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService.h"

using namespace vl;
using namespace vl::presentation;

namespace
{
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
}

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
}

#include "GuiSharedAutomationService_Controls.h"
#include "../../PlatformProviders/Hosted/GuiHostedController.h"
#include "../../PlatformProviders/Remote/GuiRemoteController.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace controls;
		using namespace remoteprotocol;

/***********************************************************************
AutomationService
***********************************************************************/

		WString AutomationService::DumpControlTreeInternal()
		{
			auto app = GetApplication();
			auto mainWindow = app->GetMainWindow();

			auto dumpRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(dumpRoot, L"WindowManagement", WString::Unmanaged(L"MultiWindow"));
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"MainWindow";
				field->value = DumpWindowClientArea(mainWindow, GetNativeWindowId(mainWindow->GetNativeWindow()));
			}
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"SubWindows";

				auto subWindows = Ptr(new glr::json::JsonArray);
				field->value = subWindows;

				for (auto subWindow : app->windows)
				{
					if (subWindow->GetVisible() && !dynamic_cast<GuiPopup*>(subWindow))
					{
						subWindows->items.Add(DumpWindowClientArea(subWindow, GetNativeWindowId(subWindow->GetNativeWindow())));
					}
				}
			}
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"Popups";

				auto popups = Ptr(new glr::json::JsonArray);
				field->value = popups;

				for (auto popup : app->openingPopups)
				{
					popups->items.Add(DumpWindowClientArea(popup, GetNativeWindowId(popup->GetNativeWindow())));
				}
			}
			return DumpJsonToString(dumpRoot);
		}

		AutomationService::AutomationService()
		{
		}

		AutomationService::~AutomationService()
		{
		}

		bool AutomationService::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
AutomationServiceHosted
***********************************************************************/

		Nullable<WString> AutomationServiceHosted::GetNativeWindowId(INativeWindow* window)
		{
			return {};
		}

		INativeWindow* AutomationServiceHosted::GetNativeWindow(Nullable<WString> windowId)
		{
			return GetHostedApplication()->GetNativeWindowHost();
		}

		WString AutomationServiceHosted::DumpControlTreeInternal()
		{
			auto app = GetApplication();
			auto mainWindow = app->GetMainWindow();

			auto dumpRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(dumpRoot, L"WindowManagement", WString::Unmanaged(L"MultiWindow"));
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"MainWindow";
				field->value = DumpWindowClientArea(mainWindow, {});

				auto hostedController = dynamic_cast<GuiHostedController*>(GetHostedApplication());
				auto windowsInOrder = From(hostedController->wmManager->topMostedWindowsInOrder).Concat(hostedController->wmManager->ordinaryWindowsInOrder).Reverse();
			}
			return DumpJsonToString(dumpRoot);
		}

		AutomationServiceHosted::AutomationServiceHosted()
		{
		}

		AutomationServiceHosted::~AutomationServiceHosted()
		{
		}

		bool AutomationServiceHosted::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
RemoteProtocolAutomationService
***********************************************************************/

		WString RemoteProtocolAutomationService::RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand)
		{
			auto window = dynamic_cast<GuiRemoteWindow*>(this->GetNativeWindow(windowId));
			if (!window)
			{
				return L"!Invalid window.";
			}

			return RunIOCommandOnNativeWindow(GetHostedApplication()->GetNativeController(), window->listeners, ioCommand);
		}

		RemoteProtocolAutomationService::RemoteProtocolAutomationService()
		{
			windowManagement = WString::Unmanaged(L"HostedRemoteProtocol");
		}

		RemoteProtocolAutomationService::~RemoteProtocolAutomationService()
		{
		}

		bool RemoteProtocolAutomationService::CanRunIOCommands()
		{
			return true;
		}

/***********************************************************************
DumpWindowClientArea
***********************************************************************/

		Ptr<glr::json::JsonNode> DumpWindowClientArea(controls::GuiWindow* window, Nullable<WString> windowId)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}
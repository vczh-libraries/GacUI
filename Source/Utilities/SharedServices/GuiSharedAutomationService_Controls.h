/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE_CONTROLS
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE_CONTROLS

#include "../../Application/Controls/GuiApplication.h"
#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

		class AutomationService : public AutomationServiceBase
		{
		protected:
			WString								DumpControlTreeInternal() override;

		public:
			AutomationService();
			~AutomationService();

			bool								CanDumpControlTree() override;
		};

		class AutomationServiceHosted : public AutomationServiceBase
		{
		protected:
			WString								windowManagement = WString::Unmanaged(L"Hosted");

			WString								DumpControlTreeInternal() override;

		public:
			AutomationServiceHosted();
			~AutomationServiceHosted();

			bool								CanDumpControlTree() override;
		};

		class RemoteProtocolAutomationService : public AutomationServiceHosted
		{
		protected:
			Nullable<WString>					GetNativeWindowId(INativeWindow* window) override;
			INativeWindow*						GetNativeWindow(Nullable<WString> windowId) override;
			WString								RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) override;

		public:
			RemoteProtocolAutomationService();
			~RemoteProtocolAutomationService();

			bool								CanRunIOCommands() override;
		};

		/*
		* Schema of /Controls:
		* {
		*   WindowManagement: "MultiWindow" | "Hosted" | "HostedRemoteProtocol";
		*   MainWindow: WindowDump;
		* 
		*   // no ordering
		*   // available for "MultiWindow"
		*   // otherwise sub windows become child controls of the main window
		*   SubWindows?: WindowDump[];
		*
		*   // in the order of ownership
		*   // the first one is the bottom level popup
		*   Popus: WindowDump[];
		* }
		* 
		* 
		* interface WindowDump
		* {
		* }
		*/
		extern Ptr<glr::json::JsonNode>			DumpWindowClientArea(controls::GuiWindow* window, Nullable<WString> windowId, Point offsetLogical, double scaleX, double scaleY);
	}
}

#endif
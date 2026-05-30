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
		*   // sub windows are normal window
		*   // no ordering
		*   // available for "MultiWindow"
		*   // otherwise sub windows become child controls of the main window
		*   SubWindows?: WindowDump[];
		*
		*   // popups are usually dropdowns, tooltips or menus
		*   // no ordering
		*   // available for "MultiWindow"
		*   // otherwise sub windows become child controls of the main window
		*   Popups?: WindowDump[];
		* }
		* 
		* 
		* interface WindowDump
		* {
		*   // windowId is used to identify a window
		*   // for "MultiWindow"
		*   //   URL `.../IO/<windowId>` is used to send commands to a specific window
		*   // for other modes
		*   //   URL `.../IO` is used to send commands to the main window
		*   //   since sub windows and popups are all child controls of the main window
		*   //   the main window is responsible for window management, dispatching IO commands to the correct target
		*   windowId?: string;
		* 
		*   // offset + logicalSize defines the valid coordinate space for IO commands
		*   // IO commands use the client area of a native window as the coordinate space
		*   // such native window is a OS native window
		*   // for a window that owns a OS native window, the valid coordinate space is the client area
		*   // for a window that doesn't own a OS native window, the valid coordinate space is the partial rectangle of the main window client area
		*   // native coordinate = logical coordinate * scale
		* 
		*   // offset always (0,0) for main window or when "MultiWindow"
		*   offset: { x: number, y: number };
		*   logicalSize: { x: number, y: number };
		* 
		*   // properties describing a OS native window
		*   // available for main window or when "MultiWindow"
		*   // when "Hosted" or "HostedRemoteProtocol"
		*   //   sub windows or popups are controls in main window
		*   //   so such window does not own its own coordinate space, it uses the main window's
		*   scale?: { x: number, y: number };
		*   nativeSize?: { x: number, y: number };
		* }
		*/
		extern Ptr<glr::json::JsonNode>			DumpWindowClientArea(controls::GuiWindow* window, Nullable<WString> windowId);
	}
}

#endif
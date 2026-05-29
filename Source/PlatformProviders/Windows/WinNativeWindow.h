/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_WINNATIVEWINDOW
#define VCZH_PRESENTATION_WINDOWS_WINNATIVEWINDOW

#include "..\..\NativeWindow\GuiNativeWindow.h"
#include "..\..\Utilities\SharedServices\GuiSharedAutomationService.h"
#include "WinNativeDpiAwareness.h"

namespace vl
{
	namespace presentation
	{
		class AutomationService;
		class AutomationServiceHosted;

		namespace windows
		{

/***********************************************************************
Windows Platform Native Controller
***********************************************************************/

			class INativeMessageHandler : public Interface
			{
			public:
				virtual void								BeforeHandle(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& skip) = 0;
				virtual void								AfterHandle(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& skip, LRESULT& result) = 0;
			};

			class IWindowsForm : public Interface
			{
			public:
				virtual HWND								GetWindowHandle() = 0;
				virtual Interface*							GetGraphicsHandler() = 0;
				virtual void								SetGraphicsHandler(Interface* handler) = 0;
				virtual bool								InstallMessageHandler(Ptr<INativeMessageHandler> handler) = 0;
				virtual bool								UninstallMessageHandler(Ptr<INativeMessageHandler> handler) = 0;
			};

			extern void										SetWindowDefaultIcon(UINT resourceId);
			extern void										StartWindowsNativeController(HINSTANCE hInstance);
			extern INativeController*						GetWindowsNativeController();
			extern IWindowsForm*							GetWindowsFormFromHandle(HWND hwnd);
			extern IWindowsForm*							GetWindowsForm(INativeWindow* window);
			extern void										GetAllCreatedWindows(collections::List<IWindowsForm*>& windows, bool rootWindowOnly);
			extern void										StopWindowsNativeController();
			extern void										EnableCrossKernelCrashing();

			struct WindowsAutomationServiceHelper
			{
				static WString								GetNativeWindowId(INativeWindow* window);
				static INativeWindow*						GetNativeWindow(Nullable<WString> windowId);
				static WString								RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand);
			};

			extern void										StartWindowsHttpAutomationService(const WString& applicationName, vint port);
			extern void										StopWindowsHttpAutomationService();

			template<typename TBase>
			struct WindowsAutomationService_ : public TBase
			{
				WString GetNativeWindowId(INativeWindow* window) override
				{
					return WindowsAutomationServiceHelper::GetNativeWindowId(window);
				}

				INativeWindow* GetNativeWindow(Nullable<WString> windowId) override
				{
					return WindowsAutomationServiceHelper::GetNativeWindow(windowId);
				}

				WString RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) override
				{
					return WindowsAutomationServiceHelper::RunIOCommandInternal(windowId, ioCommand);
				}

			public:

				void Stop() override
				{
					TBase::Stop();
					StopWindowsHttpAutomationService();
				}

				bool CanRunIOCommands() override
				{
					return true;
				}
			};

			using WindowsAutomationService = WindowsAutomationService_<AutomationService>;
			using WindowsAutomationServiceHosted = WindowsAutomationService_<AutomationServiceHosted>;
		}
	}
}

#endif
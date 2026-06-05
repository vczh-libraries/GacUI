/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_WINNATIVEWINDOW
#define VCZH_PRESENTATION_WINDOWS_WINNATIVEWINDOW

#include "..\..\NativeWindow\GuiNativeWindow.h"
#include "..\..\Utilities\SharedServices\GuiSharedAutomationService_Controls.h"
#include "WinNativeDpiAwareness.h"
#include <VlppOS.Windows.h>

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

			template<typename TBase>
			class WindowsAutomationServiceBase : public TBase
			{
			protected:

				WString										RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) override;

			public:
				template<typename ...TArgs>
				WindowsAutomationServiceBase(TArgs&& ...args)
					:TBase(std::forward<TArgs>(args)...)
				{
				}

				void										Stop() override;
				bool										CanRunIOCommands() override;
			};

			class WindowsAutomationService : public WindowsAutomationServiceBase<AutomationService>
			{
			protected:
				Nullable<WString>							GetNativeWindowId(INativeWindow* window) override;
				INativeWindow*								GetNativeWindow(Nullable<WString> windowId) override;

			public:
				WindowsAutomationService();
				~WindowsAutomationService();
			};

			class WindowsAutomationServiceHosted : public WindowsAutomationServiceBase<AutomationServiceHosted>
			{
			public:
				WindowsAutomationServiceHosted();
				~WindowsAutomationServiceHosted();
			};

			class WindowsAutomationServiceRenderer : public WindowsAutomationServiceBase<AutomationServiceRenderer>
			{
			public:
				WindowsAutomationServiceRenderer(remote_renderer::GuiRemoteRendererSingle* _renderer);
				~WindowsAutomationServiceRenderer();
			};

			extern void										StartWindowsHttpAutomationService(const WString& applicationName, vint port);
			extern void										StopWindowsHttpAutomationService();
		}
	}
}

#endif

/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_WINNATIVEWINDOW
#define VCZH_PRESENTATION_WINDOWS_WINNATIVEWINDOW

#include "..\..\NativeWindow\GuiNativeWindow.h"
#include "ServicesImpl\WindowsImageService.h"

namespace vl
{
	namespace presentation
	{
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
		}
	}
}

#endif
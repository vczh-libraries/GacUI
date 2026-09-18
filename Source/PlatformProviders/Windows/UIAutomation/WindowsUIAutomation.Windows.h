#ifndef VCZH_PRESENTATION_WINDOWS_UIAUTOMATION
#define VCZH_PRESENTATION_WINDOWS_UIAUTOMATION

#include "../WinNativeWindow.h"

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	class WindowsUIAutomationContext;

	class WindowsUIAutomationListener : public Object, public INativeControllerListener, public INativeMessageHandler
	{
	protected:
		Ptr<WindowsUIAutomationContext>		context;
		Ptr<INativeMessageHandler>			messageHandler;
	public:
		WindowsUIAutomationListener(bool isHostedMode);
		~WindowsUIAutomationListener();
		void Start(Ptr<WindowsUIAutomationListener> self);
		void Stop();
		void NativeWindowCreated(INativeWindow* window)override;
		void NativeWindowDestroying(INativeWindow* window)override;
		void BeforeHandle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)override;
		void AfterHandle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip, LRESULT& result)override;
	};
}
#endif
#endif

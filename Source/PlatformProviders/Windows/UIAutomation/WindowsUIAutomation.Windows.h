#ifndef VCZH_PRESENTATION_WINDOWS_UIAUTOMATION
#define VCZH_PRESENTATION_WINDOWS_UIAUTOMATION

#include "../WinNativeWindow.h"

#ifdef VCZH_MSVC
namespace vl::presentation::controls
{
	class GuiControl;
}
namespace vl::presentation::windows
{
	class WindowsUIAutomationContext;

	extern void SetWindowsUIAutomationName(controls::GuiControl* control, const WString& name);
	// Stable application identity; omit it for dynamically generated controls.
	extern void SetWindowsUIAutomationId(controls::GuiControl* control, const WString& id);
	extern void SetWindowsUIAutomationLabel(controls::GuiControl* control, controls::GuiControl* label);
	extern void SetWindowsUIAutomationText(controls::GuiControl* control, const WString& key, const WString& text);

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

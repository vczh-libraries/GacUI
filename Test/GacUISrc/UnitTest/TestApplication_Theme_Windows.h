#ifndef VCZH_TEST_APPLICATION_THEME_WINDOWS
#define VCZH_TEST_APPLICATION_THEME_WINDOWS

#ifdef _MSC_VER
#include "../../../Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.h"
#include "../../../Source/PlatformProviders/Windows/GDI/WinGDIApplication.h"

extern void SetGuiMainProxy(const vl::Func<void()>& proxy);

namespace theme_observation_tests
{
	using namespace vl;
	using namespace vl::presentation::windows;

	class Listener : public WindowsUIAutomationListener
	{
	public:
		Listener(Ptr<WindowsUIAutomationContext> probe)
			: WindowsUIAutomationListener(probe->hosted)
		{
			context->Stop();
			context = probe;
		}

		void AfterHandle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip, LRESULT& result)override
		{
			// Desktop accessibility clients may query a shown window at any time.
			// The fixture acquires its root directly on the UI thread.
			if (message == WM_GETOBJECT && static_cast<LONG>(lParam) == UiaRootObjectId)
			{
				skip = true;
				result = 0;
				return;
			}
			WindowsUIAutomationListener::AfterHandle(hwnd, message, wParam, lParam, skip, result);
		}
	};

	template<typename TContext, typename TTest>
	void Run(bool hosted, TTest test)
	{
		Ptr<TContext> context;
		SetGuiMainProxy([&]() { test(context); });
		SetupWindowsGDIRendererInternal(hosted, false, [&]() -> Ptr<WindowsUIAutomationListener>
		{
			context = Ptr(new TContext);
			return Ptr(new Listener(context));
		});
		SetGuiMainProxy({});
	}
}

#endif
#endif

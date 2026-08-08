#ifndef VCZH_PRESENTATION_REMOTING_WINDOWSAUTOMATIONSERVICE
#define VCZH_PRESENTATION_REMOTING_WINDOWSAUTOMATIONSERVICE

#include "../../../GacUI.h"
#include "../../../PlatformProviders/Windows/WinNativeWindow.h"
#include "../../../PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"

namespace vl::presentation::windows
{
	template<typename TBase>
	class WindowsAutomationServiceBase : public TBase
	{
	protected:
		WString RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) override;

	public:
		template<typename ...TArgs>
		WindowsAutomationServiceBase(TArgs&& ...args)
			: TBase(std::forward<TArgs>(args)...)
		{
		}

		INativeAutomationService::IOCommandAvailability CanRunIOCommands() override;
	};

	class WindowsAutomationService : public WindowsAutomationServiceBase<AutomationService>
	{
	protected:
		Nullable<WString> GetNativeWindowId(INativeWindow* window) override;
		INativeWindow* GetNativeWindow(Nullable<WString> windowId) override;
	};

	class WindowsAutomationServiceHosted : public WindowsAutomationServiceBase<AutomationServiceHosted>
	{
	};

	class WindowsAutomationServiceRenderer : public WindowsAutomationServiceBase<AutomationServiceRenderer>
	{
	public:
		WindowsAutomationServiceRenderer(remote_renderer::GuiRemoteRendererSingle* renderer);

		INativeAutomationService::IOCommandAvailability CanRunIOCommands() override;
	};

	extern void StartWindowsHttpAutomationService(const WString& applicationName, vint port);
	extern void StopWindowsHttpAutomationService();
}

#endif

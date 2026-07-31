#ifndef VCZH_PRESENTATION_REMOTING_WINDOWSAUTOMATIONSERVICE
#define VCZH_PRESENTATION_REMOTING_WINDOWSAUTOMATIONSERVICE

#include "../MiniHttpAutomationService.h"
#include "../../../PlatformProviders/Windows/WinNativeWindow.h"

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

		void Stop() override;
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

	enum class WindowsAutomationServiceType
	{
		Normal,
		Hosted,
		Renderer,
	};

	extern void StartWindowsAutomationService(
		WindowsAutomationServiceType serviceType,
		remote_renderer::GuiRemoteRendererSingle* renderer = nullptr
		);
	extern void StopWindowsAutomationService();
	extern INativeAutomationService* GetWindowsAutomationService();

	extern void StartWindowsHttpAutomationService(const WString& applicationName, vint port);
	extern void StopWindowsHttpAutomationService();

	class WindowsAutomationServiceSubstitutionScope
	{
	public:
		WindowsAutomationServiceSubstitutionScope(
			WindowsAutomationServiceType serviceType,
			remote_renderer::GuiRemoteRendererSingle* renderer = nullptr
			);
		~WindowsAutomationServiceSubstitutionScope();

		WindowsAutomationServiceSubstitutionScope(const WindowsAutomationServiceSubstitutionScope&) = delete;
		WindowsAutomationServiceSubstitutionScope& operator=(const WindowsAutomationServiceSubstitutionScope&) = delete;
	};

	class AutomationServiceEndpointScope
	{
	private:
		remoting::RemotingAutomationService					automationService;

	public:
		AutomationServiceEndpointScope(
			remoting::RemotingAutomationService service,
			const WString& applicationName,
			vint port,
			Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer = nullptr
			);
		~AutomationServiceEndpointScope();

		AutomationServiceEndpointScope(const AutomationServiceEndpointScope&) = delete;
		AutomationServiceEndpointScope& operator=(const AutomationServiceEndpointScope&) = delete;
	};

	class WindowsAutomationServiceScope
	{
	private:
		WindowsAutomationServiceSubstitutionScope				substitution;
		AutomationServiceEndpointScope							endpoint;

	public:
		WindowsAutomationServiceScope(
			WindowsAutomationServiceType serviceType,
			remoting::RemotingAutomationService endpointType,
			const WString& applicationName,
			vint port,
			Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer = nullptr,
			remote_renderer::GuiRemoteRendererSingle* renderer = nullptr
			);

		AutomationServiceRenderer* GetRendererAutomationService();

		WindowsAutomationServiceScope(const WindowsAutomationServiceScope&) = delete;
		WindowsAutomationServiceScope& operator=(const WindowsAutomationServiceScope&) = delete;
	};
}

#endif

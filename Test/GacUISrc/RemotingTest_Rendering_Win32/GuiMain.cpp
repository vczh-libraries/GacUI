#if defined __APPLE__ && __has_include(<GacUI.h>)
#include <GacUI.h>
#include "../Mac/NativeWindow/CocoaAutomationService.h"
#include "../Mac/NativeWindow/OSX/CoreGraphics/CoreGraphicsApp.h"
#include <dispatch/dispatch.h>
#else
#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#endif
#include <VlppOS.h>
#if defined VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#endif

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

extern void StartMiniHttpAutomationService(Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer);
extern void StartMiniHttpAutomationService(Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> socketServer, const WString& applicationName);
extern void StopMiniHttpAutomationService();

namespace
{
#if defined VCZH_MSVC
	constexpr const wchar_t* GacUIRemoteProtocolNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
#endif
	constexpr const wchar_t* GacUIRemoteProtocolHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
	constexpr vint GacUIRemoteProtocolHttpPort = 8888;
	constexpr vint GacUIAutomationHttpPort = 8889;
#if defined VCZH_MSVC
	constexpr const wchar_t* GacUIAutomationApplicationName = L"RemotingTest_Rendering_Win32";
#endif
#if defined VCZH_GCC && !defined VCZH_APPLE
	constexpr const wchar_t* GacUIAutomationApplicationName = L"RemotingTest_Renderer_Linux";
#endif
#if defined VCZH_GCC && defined VCZH_APPLE
	constexpr const wchar_t* GacUIAutomationApplicationName = L"RemotingTest_Renderer_macOS";
#endif
}

GuiRemoteRendererSingle* renderer = nullptr;
GuiRemoteProtocolAsyncJsonChannelRenderer* asyncChannel = nullptr;
AutomationServiceRenderer* rendererAutomationService = nullptr;
#if defined VCZH_MSVC
bool useWindowsHttpAutomationService = true;
#endif
Ptr<inter_process::async_tcp_socket::IAsyncSocketServer>* miniHttpAutomationSocketServer = nullptr;

class RemotingTestChannelClient : public GuiRemoteProtocolChannelClient
{
	using Base = GuiRemoteProtocolChannelClient;
private:
	SpinLock									lockFatalError; // covers triggeredFatalError, fatalTitle, fatalMessage
	bool										triggeredFatalError = false;
	WString										fatalTitle;
	WString										fatalMessage;
	GuiRemoteRendererSingle*					renderer = nullptr;
	GuiRemoteProtocolAsyncJsonChannelRenderer*	asyncRendererChannel = nullptr;

	bool ClaimFatalError(const WString& title, const WString& errorMessage)
	{
		bool claimed = false;
		SPIN_LOCK(lockFatalError)
		{
			if (!triggeredFatalError && (!renderer || !renderer->IsDisconnectedFromCore()))
			{
				triggeredFatalError = true;
				fatalTitle = title;
				fatalMessage = errorMessage;
				claimed = true;
			}
		}
		return claimed;
	}

	bool HasFatalError()
	{
		bool hasFatalError = false;
		SPIN_LOCK(lockFatalError)
		{
			hasFatalError = triggeredFatalError;
		}
		return hasFatalError;
	}

	void QueueFatalPrompt()
	{
		WString title;
		WString message;
		SPIN_LOCK(lockFatalError)
		{
			title = fatalTitle;
			message = fatalMessage;
		}

		auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
		auto targetRenderer = renderer;
		auto targetAutomationService = rendererAutomationService;
		GetCurrentController()->AsyncService()->InvokeInMainThread(
			mainWindow,
			[=]()
			{
				auto result = GetCurrentController()->DialogService()->ShowMessageBox(
					mainWindow,
					message + WString::Unmanaged(L"\r\n\r\nDo you want to close the renderer?"),
					title,
					INativeDialogService::DisplayYesNo,
					INativeDialogService::DefaultFirst,
					INativeDialogService::IconError
				);
				if (result == INativeDialogService::SelectYes)
				{
					if (targetRenderer)
					{
						targetRenderer->ForceExitByFatelError();
					}
				}
				else
				{
					if (targetRenderer)
					{
						targetRenderer->RetainByFatalError(message);
					}
					if (targetAutomationService)
					{
						targetAutomationService->SetFatalError(Nullable<WString>(message));
					}
				}
			});
	}

public:
	RemotingTestChannelClient(Ptr<inter_process::INetworkProtocolClient> client, Ptr<glr::json::Parser> parser)
		: Base(client, parser)
	{
	}

	void SetRenderer(GuiRemoteRendererSingle* _renderer)
	{
		renderer = _renderer;
	}

	void SetAsyncRendererChannel(GuiRemoteProtocolAsyncJsonChannelRenderer* _asyncRendererChannel)
	{
		asyncRendererChannel = _asyncRendererChannel;
	}

	void OnReadError(const WString& errorMessage) override
	{
		if (ClaimFatalError(L"ERROR from GacUI Core", errorMessage))
		{
			QueueFatalPrompt();
		}
	}

	void OnLocalError(const WString& errorMessage, bool fatal) override
	{
		if (fatal)
		{
			if (ClaimFatalError(L"ERROR from Renderer Transport", errorMessage))
			{
				if (renderer)
				{
					renderer->RequestCoreForceExitByFatalError();
				}
				QueueFatalPrompt();
			}
		}
	}

	void OnDisconnected() override
	{
		Base::OnDisconnected();
		if (asyncRendererChannel)
		{
			asyncRendererChannel->Detach();
		}
		if (renderer && !HasFatalError())
		{
			auto targetRenderer = renderer;
#if defined VCZH_MSVC
			targetRenderer->ForceExitByFatelError();
#endif
#if defined VCZH_GCC && !defined VCZH_APPLE
			auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
			GetCurrentController()->AsyncService()->InvokeInMainThread(
				mainWindow,
				[targetRenderer]()
				{
					targetRenderer->ForceExitByFatelError();
				});
#endif
#if defined VCZH_GCC && defined VCZH_APPLE
			dispatch_async_f(
				dispatch_get_main_queue(),
				targetRenderer,
				[](void* context)
				{
					static_cast<GuiRemoteRendererSingle*>(context)->ForceExitByFatelError();
				});
#endif
		}
	}
};

class GuiMainAsyncRendererInvoker : public Object, public virtual IGuiRemoteProtocolAsyncRendererInvoker
{
public:
	void InvokeInMainThread(const Func<void()>& proc) override
	{
#if defined VCZH_MSVC
		GetApplication()->InvokeInMainThread(nullptr, proc);
#endif
#if defined VCZH_GCC && !defined VCZH_APPLE
		GetCurrentController()->AsyncService()->InvokeInMainThread(nullptr, proc);
#endif
#if defined VCZH_GCC && defined VCZH_APPLE
		auto queuedProc = new Func<void()>(proc);
		dispatch_async_f(
			dispatch_get_main_queue(),
			queuedProc,
			[](void* context)
			{
				auto callback = static_cast<Func<void()>*>(context);
				(*callback)();
				delete callback;
			});
#endif
	}
};

void GuiMain()
{
	auto mainWindow = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
	mainWindow->SetTitle(L"Connecting ...");
	{
		auto size = mainWindow->Convert(Size(320, 240));
		auto screen = GetCurrentController()->ScreenService()->GetScreen((vint)0);
		auto client = screen->GetClientBounds();
		auto x = client.Left() + (client.Width() - size.x) / 2;
		auto y = client.Top() + (client.Height() - size.y) / 2;
		mainWindow->SetBounds({ {x,y},size });
	}
	GuiMainAsyncRendererInvoker invoker;
	renderer->RegisterMainWindow(mainWindow);
	asyncChannel->SetInvokeInMainThread(&invoker);

	{
#if defined VCZH_MSVC
		windows::WindowsAutomationServiceRenderer automationService(renderer);
#endif
#if defined VCZH_GCC && !defined VCZH_APPLE
		wayland::WGacAutomationServiceRenderer automationService(renderer);
#endif
#if defined VCZH_GCC && defined VCZH_APPLE
		osx::CocoaAutomationServiceRenderer automationService(renderer);
#endif
		rendererAutomationService = &automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		auto cleanup = [&]()
		{
#if defined VCZH_MSVC
			if (!useWindowsHttpAutomationService)
			{
				StopMiniHttpAutomationService();
			}
#else
			StopMiniHttpAutomationService();
#endif
			GetCurrentController()->AutomationService()->Stop();
#if defined VCZH_MSVC
			if (useWindowsHttpAutomationService)
			{
				windows::StopWindowsHttpAutomationService();
			}
#endif
			GetNativeServiceSubstitution()->Unsubstitute(&automationService);
			rendererAutomationService = nullptr;
		};
		try
		{
#if defined VCZH_MSVC
			if (useWindowsHttpAutomationService)
			{
				windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/RemotingTest_Rendering_Win32"), GacUIAutomationHttpPort);
			}
			else
#endif
			{
				StartMiniHttpAutomationService(
					*miniHttpAutomationSocketServer,
					WString::Unmanaged(GacUIAutomationApplicationName)
					);
			}
			GetCurrentController()->WindowService()->Run(mainWindow);
		}
		catch (...)
		{
			cleanup();
			throw;
		}
		cleanup();
	}

	asyncChannel->SetInvokeInMainThread(nullptr);
	renderer->UnregisterMainWindow();
}

int StartClient(Ptr<inter_process::INetworkProtocolClient> networkClient)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	RemotingTestChannelClient channelClient(networkClient, jsonParser);
	GuiRemoteProtocolAsyncJsonChannelRenderer asyncRendererChannel(channelClient.GetProtocolChannel());
	GuiRemoteRendererSingle remoteRenderer(true); // true to enable automation data collection
	GuiRemoteProtocolRendererChannel rendererChannel(&asyncRendererChannel, &remoteRenderer);
	channelClient.SetRenderer(&remoteRenderer);
	channelClient.SetAsyncRendererChannel(&asyncRendererChannel);
	channelClient.WaitForServer();

	asyncChannel = &asyncRendererChannel;
	renderer = &remoteRenderer;
#if defined VCZH_MSVC
	int result = SetupRawWindowsDirect2DRenderer();
#endif
#if defined VCZH_GCC && !defined VCZH_APPLE
	int result = elements::wgac::SetupWGacRenderer();
#endif
#if defined VCZH_GCC && defined VCZH_APPLE
	int result = SetupRawOSXCoreGraphicsRenderer();
#endif
	networkClient->GetConnection()->Stop();
	renderer = nullptr;
	asyncChannel = nullptr;
	channelClient.SetAsyncRendererChannel(nullptr);
	channelClient.SetRenderer(nullptr);

	return result;
}

#if defined VCZH_MSVC
int StartNamedPipeClient()
{
	useWindowsHttpAutomationService = true;
	return StartClient(Ptr(new inter_process::named_pipe::NamedPipeClient(WString::Unmanaged(GacUIRemoteProtocolNamedPipeName))));
}

int StartHttpClient()
{
	useWindowsHttpAutomationService = true;
	return StartClient(Ptr(new inter_process::windows_http::HttpClient(WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl), GacUIRemoteProtocolHttpPort)));
}
#endif

int StartMiniHttpClient()
{
#if defined VCZH_MSVC
	useWindowsHttpAutomationService = false;
#endif
	auto socketServer = inter_process::async_tcp_socket::CreateDefaultAsyncSocketServer(GacUIAutomationHttpPort);
	auto socketClient = inter_process::async_tcp_socket::CreateDefaultAsyncSocketClient(GacUIRemoteProtocolHttpPort);
	miniHttpAutomationSocketServer = &socketServer;
	try
	{
		auto result = StartClient(Ptr(new inter_process::async_tcp_socket::SocketHttpClient(
			socketClient,
			WString::Unmanaged(L"localhost"),
			WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl)
			)));
		miniHttpAutomationSocketServer = nullptr;
		return result;
	}
	catch (...)
	{
		miniHttpAutomationSocketServer = nullptr;
		throw;
	}
}

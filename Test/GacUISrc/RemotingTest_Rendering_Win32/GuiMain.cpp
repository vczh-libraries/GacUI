#if defined __linux__ && __has_include(<GacUI.h>) && __has_include("../WGac/Services/WGacAutomationService.h") && __has_include("../WGac/Renderers/WGacRenderer.h")
#include <GacUI.h>
#include "../WGac/Services/WGacAutomationService.h"
#include "../WGac/Renderers/WGacRenderer.h"
#elif defined __APPLE__ && __has_include(<GacUI.h>)
#include <GacUI.h>
#include "../Mac/NativeWindow/CocoaAutomationService.h"
#include "../Mac/NativeWindow/OSX/CoreGraphics/CoreGraphicsApp.h"
#include <dispatch/dispatch.h>
#else
#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#endif
#include "../../../Source/RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/RemotingHelpers/RemotingClient/RemotingChannelClient.h"
#include <VlppOS.h>
#if defined VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
#endif

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

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
#elif defined VCZH_GCC && !defined VCZH_APPLE
	constexpr const wchar_t* GacUIAutomationApplicationName = L"RemotingTest_Renderer_Wayland";
#else
	constexpr const wchar_t* GacUIAutomationApplicationName = L"RemotingTest_Renderer_macOS";
#endif

	struct RendererGuiContext
	{
		RemotingChannelClient*								channelClient = nullptr;
		GuiRemoteProtocolAsyncJsonChannelRenderer*			asyncChannel = nullptr;
		GuiRemoteRendererSingle*							renderer = nullptr;
		RemotingHostContext									host;
	};

	RendererGuiContext* currentGuiContext = nullptr;

	class GuiMainAsyncRendererInvoker : public Object, public virtual IGuiRemoteProtocolAsyncRendererInvoker
	{
	public:
		void InvokeInMainThread(const Func<void()>& proc) override
		{
#if defined VCZH_MSVC
			GetApplication()->InvokeInMainThread(nullptr, proc);
#elif defined VCZH_GCC && !defined VCZH_APPLE
			GetCurrentController()->AsyncService()->InvokeInMainThread(nullptr, proc);
#else
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

	RemotingChannelClientConfiguration CreateClientConfiguration()
	{
		RemotingChannelClientConfiguration configuration;
		configuration.fatalTitle = WString::Unmanaged(L"ERROR from GacUI Core");
#if !defined VCZH_GCC || defined VCZH_APPLE
		configuration.retainFatalError = Func<bool(const WString&, const WString&)>(
			[](const WString& title, const WString& errorMessage)
			{
				auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
				auto result = GetCurrentController()->DialogService()->ShowMessageBox(
					mainWindow,
					errorMessage + WString::Unmanaged(L"\r\n\r\nDo you want to close the renderer?"),
					title,
					INativeDialogService::DisplayYesNo,
					INativeDialogService::DefaultFirst,
					INativeDialogService::IconError
					);
				return result != INativeDialogService::SelectYes;
			});
#endif
		return configuration;
	}
}

void GuiMain()
{
	CHECK_ERROR(currentGuiContext, L"GuiMain()#The renderer GUI context is null.");
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
	auto invoker = Ptr(new GuiMainAsyncRendererInvoker);
	currentGuiContext->renderer->RegisterMainWindow(mainWindow);

#if defined VCZH_MSVC
	windows::WindowsAutomationServiceScope automation(
		windows::WindowsAutomationServiceType::Renderer,
		currentGuiContext->host.automationService,
		WString::Unmanaged(GacUIAutomationApplicationName),
		GacUIAutomationHttpPort,
		currentGuiContext->host.miniHttpSocketServer,
		currentGuiContext->renderer
		);
	auto rendererAutomationService = automation.GetRendererAutomationService();
#elif defined VCZH_GCC && !defined VCZH_APPLE
	wayland::WGacAutomationServiceRenderer rendererAutomationServiceObject(currentGuiContext->renderer);
	NativeAutomationServiceScope substitution(&rendererAutomationServiceObject);
	MiniHttpAutomationServiceScope endpoint(
		currentGuiContext->host.miniHttpSocketServer,
		WString::Unmanaged(GacUIAutomationApplicationName)
		);
	auto rendererAutomationService = &rendererAutomationServiceObject;
#else
	osx::CocoaAutomationServiceRenderer rendererAutomationServiceObject(currentGuiContext->renderer);
	NativeAutomationServiceScope substitution(&rendererAutomationServiceObject);
	MiniHttpAutomationServiceScope endpoint(
		currentGuiContext->host.miniHttpSocketServer,
		WString::Unmanaged(GacUIAutomationApplicationName)
		);
	auto rendererAutomationService = &rendererAutomationServiceObject;
#endif
	currentGuiContext->channelClient->SetRendererAutomationService(rendererAutomationService);

#if defined VCZH_GCC && !defined VCZH_APPLE
	currentGuiContext->channelClient->WaitForServer();
#endif
	currentGuiContext->asyncChannel->SetInvokeInMainThread(invoker);
	currentGuiContext->asyncChannel->ProcessPendingMessages();
	if (
		!currentGuiContext->renderer->IsDisconnectedFromCore() ||
		currentGuiContext->channelClient->IsFatalErrorRetained()
		)
	{
		GetCurrentController()->WindowService()->Run(mainWindow);
	}

	currentGuiContext->channelClient->BeginStopping();
	currentGuiContext->asyncChannel->SetInvokeInMainThread(nullptr);
	currentGuiContext->channelClient->SetRendererAutomationService(nullptr);
	currentGuiContext->renderer->UnregisterMainWindow();
}

int StartClient(
	Ptr<inter_process::INetworkProtocolClient> networkClient,
	RemotingHostContext host
	)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	RemotingChannelClient channelClient(networkClient, jsonParser, CreateClientConfiguration());
	GuiRemoteProtocolAsyncJsonChannelRenderer asyncRendererChannel(channelClient.GetProtocolChannel());
	GuiRemoteRendererSingle remoteRenderer(true);
	GuiRemoteProtocolRendererChannel rendererChannel(&asyncRendererChannel, &remoteRenderer);
	channelClient.SetRenderer(&remoteRenderer);
	channelClient.SetAsyncRendererChannel(&asyncRendererChannel);
#if defined VCZH_MSVC || (defined VCZH_GCC && defined VCZH_APPLE)
	channelClient.WaitForServer();
#endif

	RendererGuiContext context{ &channelClient, &asyncRendererChannel, &remoteRenderer, host };
	CHECK_ERROR(!currentGuiContext, L"StartClient(...)#The GUI context has already been bound.");
	currentGuiContext = &context;
#if defined VCZH_MSVC
	auto result = SetupRawWindowsDirect2DRenderer();
#elif defined VCZH_GCC && !defined VCZH_APPLE
	auto result = elements::wgac::SetupRawWGacRenderer();
#else
	auto result = SetupRawOSXCoreGraphicsRenderer();
#endif
	currentGuiContext = nullptr;

	channelClient.BeginStopping();
	networkClient->GetConnection()->Stop();
	channelClient.SetAsyncRendererChannel(nullptr);
	channelClient.SetRenderer(nullptr);
	return result;
}

#if defined VCZH_MSVC
int StartNamedPipeClient()
{
	return StartClient(
		Ptr(new inter_process::named_pipe::NamedPipeClient(WString::Unmanaged(GacUIRemoteProtocolNamedPipeName))),
		{ RemotingAutomationService::WindowsHttp, nullptr }
		);
}

int StartHttpClient()
{
	return StartClient(
		Ptr(new inter_process::windows_http::HttpClient(
			WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl),
			GacUIRemoteProtocolHttpPort
			)),
		{ RemotingAutomationService::WindowsHttp, nullptr }
		);
}
#endif

int StartMiniHttpClient()
{
	auto socketServer = inter_process::async_tcp_socket::CreateDefaultAsyncSocketServer(GacUIAutomationHttpPort);
	auto socketClient = inter_process::async_tcp_socket::CreateDefaultAsyncSocketClient(GacUIRemoteProtocolHttpPort);
	return StartClient(
		Ptr(new inter_process::async_tcp_socket::SocketHttpClient(
			socketClient,
			WString::Unmanaged(L"localhost"),
			WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl)
			)),
		{ RemotingAutomationService::MiniHttp, socketServer }
		);
}

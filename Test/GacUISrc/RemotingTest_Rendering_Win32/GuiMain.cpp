#if defined __linux__ && __has_include(<GacUI.h>) && __has_include("../WGac/Services/WGacAutomationService.h") && __has_include("../WGac/Renderers/WGacRenderer.h")
#include <GacUI.h>
#include <Test.RemotingHelpers.h>
#include "../WGac/Services/WGacAutomationService.h"
#include "../WGac/Renderers/WGacRenderer.h"
#elif defined __APPLE__ && __has_include(<GacUI.h>)
#include <GacUI.h>
#include <Test.RemotingHelpers.h>
#include "../Mac/NativeWindow/CocoaAutomationService.h"
#include "../Mac/NativeWindow/OSX/CoreGraphics/CoreGraphicsApp.h"
#include <dispatch/dispatch.h>
#else
#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../../../Source/Utilities/AutomationService/MiniHttpAutomationService.h"
#include "../../RemotingHelpers/RendererClient/RemoteProtocolRendererClient.h"
#endif
#include <VlppOS.h>
#if defined VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/Utilities/AutomationService/Windows/WindowsAutomationService.Windows.h"
#endif

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

constexpr const wchar_t* GacUIRemoteProtocolNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
constexpr const wchar_t* GacUIRemoteProtocolHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
constexpr vint GacUIRemoteProtocolHttpPort = 8888;
constexpr const wchar_t* GacUIAutomationApplicationName = L"RemotingTest_Rendering_Native";

struct RendererGuiContext
{
	RemoteProtocolRendererClient*						channelClient = nullptr;
	GuiRemoteProtocolAsyncJsonChannelRenderer*			asyncChannel = nullptr;
	GuiRemoteRendererSingle*							renderer = nullptr;
	Ptr<inter_process::async_tcp_socket::IAsyncSocketServer>
											miniHttpSocketServer;
	vint												automationHttpPort = 0;
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
	windows::WindowsAutomationServiceRenderer rendererAutomationServiceObject(currentGuiContext->renderer);
#elif defined VCZH_GCC && !defined VCZH_APPLE
	wayland::WGacAutomationServiceRenderer rendererAutomationServiceObject(currentGuiContext->renderer);
#elif defined VCZH_GCC && defined VCZH_APPLE
	osx::CocoaAutomationServiceRenderer rendererAutomationServiceObject(currentGuiContext->renderer);
#endif
	GetNativeServiceSubstitution()->Substitute(&rendererAutomationServiceObject, false);
#if defined VCZH_MSVC
	if (!currentGuiContext->miniHttpSocketServer)
	{
		windows::StartWindowsHttpAutomationService(
			WString::Unmanaged(L"Automation/") + WString::Unmanaged(GacUIAutomationApplicationName),
			currentGuiContext->automationHttpPort
			);
	}
	else
#endif
	{
		StartMiniHttpAutomationService(
			currentGuiContext->miniHttpSocketServer,
			WString::Unmanaged(GacUIAutomationApplicationName)
			);
	}
	currentGuiContext->channelClient->SetRendererAutomationService(&rendererAutomationServiceObject);
	currentGuiContext->asyncChannel->SetInvokeInMainThread(invoker);
	currentGuiContext->asyncChannel->ProcessPendingMessages();
	if (
		!currentGuiContext->renderer->IsDisconnectedFromCore() ||
		currentGuiContext->channelClient->IsFatalErrorRetained()
		)
	{
		GetCurrentController()->WindowService()->Run(mainWindow);
	}

#if defined VCZH_MSVC
	if (!currentGuiContext->miniHttpSocketServer)
	{
		windows::StopWindowsHttpAutomationService();
	}
	else
#endif
	{
		StopMiniHttpAutomationService();
	}

	currentGuiContext->channelClient->SetRendererAutomationService(nullptr);
	rendererAutomationServiceObject.Stop();
	GetNativeServiceSubstitution()->Unsubstitute(&rendererAutomationServiceObject);
	currentGuiContext->asyncChannel->SetInvokeInMainThread(nullptr);
	currentGuiContext->renderer->UnregisterMainWindow();
}

int StartClient(
	Ptr<inter_process::INetworkProtocolClient> networkClient,
	Ptr<inter_process::async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer,
	vint automationHttpPort
	)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	RemoteProtocolRendererClient channelClient(
		networkClient,
		jsonParser,
		WString::Unmanaged(L"ERROR from GacUI Core")
		);
	GuiRemoteProtocolAsyncJsonChannelRenderer asyncRendererChannel(channelClient.GetProtocolChannel());
	GuiRemoteRendererSingle remoteRenderer(true);
	GuiRemoteProtocolRendererChannel rendererChannel(&asyncRendererChannel, &remoteRenderer);
	channelClient.SetRenderer(&remoteRenderer);
	channelClient.SetAsyncRendererChannel(&asyncRendererChannel);
	channelClient.WaitForServer();

	RendererGuiContext context{ &channelClient, &asyncRendererChannel, &remoteRenderer, miniHttpSocketServer, automationHttpPort };
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
int StartNamedPipeClient(vint automationHttpPort)
{
	return StartClient(
		Ptr(new inter_process::named_pipe::NamedPipeClient(WString::Unmanaged(GacUIRemoteProtocolNamedPipeName))),
		nullptr,
		automationHttpPort
		);
}

int StartHttpClient(vint automationHttpPort)
{
	return StartClient(
		Ptr(new inter_process::windows_http::HttpClient(
			WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl),
			GacUIRemoteProtocolHttpPort
			)),
		nullptr,
		automationHttpPort
		);
}
#endif

int StartMiniHttpClient(vint automationHttpPort)
{
	auto socketServer = inter_process::async_tcp_socket::CreateDefaultAsyncSocketServer(automationHttpPort);
	auto socketClient = inter_process::async_tcp_socket::CreateDefaultAsyncSocketClient(GacUIRemoteProtocolHttpPort);
	return StartClient(
		Ptr(new inter_process::async_tcp_socket::SocketHttpClient(
			socketClient,
			WString::Unmanaged(L"localhost"),
			WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl)
		)),
		socketServer,
		automationHttpPort
		);
}

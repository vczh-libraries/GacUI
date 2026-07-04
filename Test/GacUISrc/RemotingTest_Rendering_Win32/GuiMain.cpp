#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include <VlppOS.Windows.h>
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

namespace
{
	constexpr const wchar_t* GacUIRemoteProtocolNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
	constexpr const wchar_t* GacUIRemoteProtocolHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
	constexpr vint GacUIRemoteProtocolHttpPort = 8888;
}

GuiRemoteRendererSingle* renderer = nullptr;
GuiRemoteProtocolAsyncJsonChannelRenderer* asyncChannel = nullptr;

class RemotingTestChannelClient : public GuiRemoteProtocolChannelClient
{
	using Base = GuiRemoteProtocolChannelClient;
private:
	bool										triggeredFatalError = false;
	GuiRemoteRendererSingle*					renderer = nullptr;
	GuiRemoteProtocolAsyncJsonChannelRenderer*	asyncRendererChannel = nullptr;

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
		triggeredFatalError = true;
		auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
		GetCurrentController()->AsyncService()->InvokeInMainThread(
			mainWindow,
			[=]()
			{
				GetCurrentController()->DialogService()->ShowMessageBox(
					mainWindow,
					errorMessage,
					L"ERROR from GacUI Core",
					INativeDialogService::DisplayOK,
					INativeDialogService::DefaultFirst,
					INativeDialogService::IconError
				);
				if (renderer)
				{
					renderer->ForceExitByFatelError();
				}
			});
	}

	void OnLocalError(const WString& errorMessage, bool fatal) override
	{
		if (fatal)
		{
			OnReadError(errorMessage);
		}
	}

	void OnDisconnected() override
	{
		Base::OnDisconnected();
		if (asyncRendererChannel)
		{
			asyncRendererChannel->Initialize(nullptr);
		}
		if (renderer && !triggeredFatalError)
		{
			renderer->ForceExitByFatelError();
		}
	}
};

class GuiMainAsyncRendererInvoker : public Object, public virtual IGuiRemoteProtocolAsyncRendererInvoker
{
public:
	void InvokeInMainThread(const Func<void()>& proc) override
	{
		GetApplication()->InvokeInMainThread(nullptr, proc);
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
		windows::WindowsAutomationServiceRenderer automationService(renderer);
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/RemotingTest_Rendering_Win32"), 8888);
		GetCurrentController()->WindowService()->Run(mainWindow);
		GetCurrentController()->AutomationService()->Stop();
		GetNativeServiceSubstitution()->Unsubstitute(&automationService);
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
	int result = SetupRawWindowsDirect2DRenderer();
	networkClient->GetConnection()->Stop();
	renderer = nullptr;
	asyncChannel = nullptr;
	channelClient.SetAsyncRendererChannel(nullptr);
	channelClient.SetRenderer(nullptr);

	return result;
}

int StartNamedPipeClient()
{
	return StartClient(Ptr(new inter_process::NamedPipeClient(WString::Unmanaged(GacUIRemoteProtocolNamedPipeName))));
}

int StartHttpClient()
{
	return StartClient(Ptr(new inter_process::HttpClient(WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl), GacUIRemoteProtocolHttpPort)));
}

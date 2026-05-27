#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../../../Import/VlppOS.Windows.h"

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
	GuiRemoteRendererSingle* renderer = nullptr;
	GuiRemoteProtocolAsyncJsonChannelRenderer* asyncRendererChannel = nullptr;

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
	
	void OnError(const WString& errorMessage) override
	{
		GetCurrentController()->DialogService()->ShowMessageBox(
			GetCurrentController()->WindowService()->GetMainWindow(),
			errorMessage,
			L"ERROR from GacUI Core",
			INativeDialogService::DisplayOK,
			INativeDialogService::DefaultFirst,
			INativeDialogService::IconError
			);
	}

	void OnDisconnected() override
	{
		Base::OnDisconnected();
		if (asyncRendererChannel)
		{
			asyncRendererChannel->Initialize(nullptr);
		}
		if (renderer)
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
	GetCurrentController()->WindowService()->Run(mainWindow);
	asyncChannel->SetInvokeInMainThread(nullptr);
	renderer->UnregisterMainWindow();
}

int StartClient(Ptr<inter_process::INetworkProtocolClient> networkClient)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	RemotingTestChannelClient channelClient(networkClient, jsonParser);
	GuiRemoteProtocolAsyncJsonChannelRenderer asyncRendererChannel(channelClient.GetProtocolChannel());
	GuiRemoteRendererSingle remoteRenderer;
	GuiRemoteProtocolRendererChannel rendererChannel(&channelClient, &asyncRendererChannel, &remoteRenderer);
	channelClient.SetRenderer(&remoteRenderer);
	channelClient.SetAsyncRendererChannel(&asyncRendererChannel);
	channelClient.WaitForServer();

	asyncChannel = &asyncRendererChannel;
	renderer = &remoteRenderer;
	int result = SetupRawWindowsDirect2DRenderer();
	renderer = nullptr;
	asyncChannel = nullptr;
	channelClient.SetAsyncRendererChannel(nullptr);
	channelClient.SetRenderer(nullptr);
	networkClient->GetConnection()->Stop();

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

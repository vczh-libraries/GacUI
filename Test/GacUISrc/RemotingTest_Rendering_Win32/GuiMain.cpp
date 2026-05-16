#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../../../Import/VlppOS.Windows.h"

using namespace vl;
using namespace vl::presentation;
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
	renderer->RegisterMainWindow(mainWindow);
	GetCurrentController()->WindowService()->Run(mainWindow);
	renderer->UnregisterMainWindow();
}

int StartClient(Ptr<inter_process::INetworkProtocolClient> networkClient)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	GuiRemoteProtocolChannelClient channelClient(networkClient, jsonParser);
	channelClient.WaitForServer();

	GuiRemoteRendererSingle remoteRenderer;
	GuiRemoteProtocolRendererChannel rendererChannel(&channelClient, channelClient.GetProtocolChannel(), &remoteRenderer);

	renderer = &remoteRenderer;
	int result = SetupRawWindowsDirect2DRenderer();
	renderer = nullptr;

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

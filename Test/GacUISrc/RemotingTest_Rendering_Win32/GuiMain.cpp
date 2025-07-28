#include "../../../Source/GacUI.h"
#include "RendererChannel.h"
#include "../RemotingTest_Core/Shared/NamedPipeShared.h"
#include "../RemotingTest_Core/Shared/HttpClient.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

RendererChannel* rendererChannel = nullptr;
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
	rendererChannel->RegisterMainWindow(mainWindow);
	renderer->RegisterMainWindow(mainWindow);
	GetCurrentController()->WindowService()->Run(mainWindow);
	renderer->UnregisterMainWindow();
	rendererChannel->UnregisterMainWindow();
}

template<typename TClient>
int StartClient(TClient& client)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	GuiRemoteRendererSingle remoteRenderer;
	GuiRemoteJsonChannelFromProtocol channelReceiver(&remoteRenderer);
	GuiRemoteJsonChannelStringDeserializer channelJsonDeserializer(&channelReceiver, jsonParser);
	RendererChannel clientRendererChannel(&remoteRenderer, &client, &channelJsonDeserializer);

	rendererChannel = &clientRendererChannel;
	renderer = &remoteRenderer;
	int result = SetupRawWindowsDirect2DRenderer();
	client.Stop();
	clientRendererChannel.WaitForDisconnected();
	renderer = nullptr;
	rendererChannel = nullptr;

	return result;
}

int StartNamedPipeClient()
{
	NamedPipeClient namedPipeClient;
	namedPipeClient.WaitForServer();
	return StartClient(namedPipeClient);
}

int StartHttpClient()
{
	HttpClient httpClient;
	httpClient.WaitForClient();
	return StartClient(httpClient);
}
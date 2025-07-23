#include "../../../Source/GacUI.h"
#include "RendererChannel.h"
#include "../RemotingTest_Core/Shared/HttpClient.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

extern RendererChannel* rendererChannel;
extern GuiRemoteRendererSingle* renderer;

int StartHttpClient()
{
	HttpClient httpClient;
	httpClient.WaitForClient();

	int result = 0;
	{
		auto jsonParser = Ptr(new glr::json::Parser);
		GuiRemoteRendererSingle remoteRenderer;
		GuiRemoteJsonChannelFromProtocol channelReceiver(&remoteRenderer);
		GuiRemoteJsonChannelStringDeserializer channelJsonDeserializer(&channelReceiver, jsonParser);
		RendererChannel namedPipeRendererChannel(&remoteRenderer, &httpClient, &channelJsonDeserializer);

		rendererChannel = &namedPipeRendererChannel;
		renderer = &remoteRenderer;
		result = SetupRawWindowsDirect2DRenderer();
		httpClient.Stop();
		namedPipeRendererChannel.WaitForDisconnected();
		renderer = nullptr;
		rendererChannel = nullptr;
	}
	return result;
}
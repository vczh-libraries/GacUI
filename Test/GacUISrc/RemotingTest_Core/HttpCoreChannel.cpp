#include "../../../Source/GacUI.h"
#include "CoreChannel.h"
#include "Shared/HttpServer.h"

extern CoreChannel* coreChannel;

int StartHttpServer()
{
	HttpServer httpServer;

	Console::WriteLine(L"> HTTP server created, waiting on: " + WString::Unmanaged(HttpServerUrl));
	{
		CoreChannel namedPipeCoreChannel(&httpServer);

		auto jsonParser = Ptr(new glr::json::Parser);
		GuiRemoteJsonChannelStringSerializer channelJsonSerializer(&namedPipeCoreChannel, jsonParser);

		GuiRemoteProtocolAsyncJsonChannelSerializer asyncChannelSender;
		asyncChannelSender.Start(
			&channelJsonSerializer,
			[&namedPipeCoreChannel](GuiRemoteProtocolAsyncJsonChannelSerializer* channel)
			{
				GuiRemoteProtocolFromJsonChannel channelSender(channel);
				GuiRemoteProtocolFilter filteredProtocol(&channelSender);
				GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);
				coreChannel = &namedPipeCoreChannel;
				SetupRemoteNativeController(&diffConverterProtocol);
				coreChannel = nullptr;
			},
			[&namedPipeCoreChannel](
				GuiRemoteProtocolAsyncJsonChannelSerializer::TChannelThreadProc channelThreadProc,
				GuiRemoteProtocolAsyncJsonChannelSerializer::TUIThreadProc uiThreadProc
				)
			{
				Thread::CreateAndStart(channelThreadProc);
				Thread::CreateAndStart(uiThreadProc);
			});

		Console::WriteLine(L"> Waiting for a renderer ...");
		httpServer.WaitForClient();
		namedPipeCoreChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
		asyncChannelSender.WaitForStopped();
		httpServer.StopHttpServer();
		namedPipeCoreChannel.WaitForDisconnected();
	}
	return 0;
}
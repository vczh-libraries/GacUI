#include "../../../Source/GacUI.h"
#include "CoreChannel.h"
#include "Shared/NamedPipeShared.h"

extern CoreChannel* coreChannel;

int StartNamedPipeServer()
{
	NamedPipeServer namedPipeServer;

	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(NamedPipeId));
	{
		CoreChannel namedPipeCoreChannel(&namedPipeServer);

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
		namedPipeServer.WaitForClient();
		namedPipeCoreChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
		asyncChannelSender.WaitForStopped();
		namedPipeServer.Stop();
		namedPipeCoreChannel.WaitForDisconnected();
	}
	return 0;
}
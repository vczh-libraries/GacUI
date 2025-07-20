#include "../../../Source/GacUI.h"
#include "CoreChannel.h"

CoreChannel* coreChannel = nullptr;

void WriteErrorToRendererChannel(const WString& message)
{
	coreChannel->WriteErrorThreadUnsafe(message);
}

template<typename T>
void RunInNewThread(T&& threadProc, CoreChannel* channel)
{
	Thread::CreateAndStart([=]()
	{
		try
		{
			threadProc();
		}
		catch (const Exception& e)
		{
			channel->WriteErrorThreadUnsafe(e.Message());
			return;
		}
		catch (const Error& e)
		{
			channel->WriteErrorThreadUnsafe(WString::Unmanaged(e.Description()));
			return;
		}
	});
}

int StartNamedPipeServer()
{
	HANDLE hPipe = NamedPipeShared::ServerCreatePipe();
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(NamedPipeId));
	{
		CoreChannel namedPipeCoreChannel(hPipe);

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
		NamedPipeShared::ServerWaitForClient(hPipe);
		namedPipeCoreChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
		asyncChannelSender.WaitForStopped();
		CloseHandle(hPipe);
		namedPipeCoreChannel.WaitForDisconnected();
	}
	return 0;
}
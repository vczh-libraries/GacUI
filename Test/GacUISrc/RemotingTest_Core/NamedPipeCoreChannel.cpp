#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "NamedPipeShared.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::repeatfiltering;
using namespace vl::presentation::remoteprotocol::channeling;

class NamedPipeCoreChannel
	: public NamedPipeShared
	, public virtual IGuiRemoteProtocolChannel<WString>
{
protected:
	IGuiRemoteProtocolChannelReceiver<WString>*		receiver = nullptr;
	bool											connected = false;
	List<WString>									pendingMessages;
	vint											pendingMessageCount = 0;

	void OnReadStringThreadUnsafe(Ptr<List<WString>> strs) override
	{
		for (auto str : *strs.Obj())
		{
			Console::WriteLine(L"Received: " + str);
			receiver->OnReceive(str);
		}
	}

	void SendPendingMessages()
	{
		SendStringArray(pendingMessageCount, pendingMessages);
		pendingMessageCount = 0;
	}

public:

	NamedPipeCoreChannel(HANDLE _hPipe)
		: NamedPipeShared(_hPipe)
	{
	}

	~NamedPipeCoreChannel()
	{
	}

	void RendererConnectedThreadUnsafe(GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel)
	{
		Console::WriteLine(L"> Renderer connected");
		BeginReadingLoopUnsafe();
		asyncChannel->ExecuteInChannelThread([this]()
		{
			connected = true;
		});
	}

	void WriteErrorThreadUnsafe(const WString& error)
	{
		Console::WriteLine(L"Error: " + error);
		SendSingleString(L"!" + error);
	}

	void Initialize(IGuiRemoteProtocolChannelReceiver<WString>* _receiver) override
	{
		receiver = _receiver;
	}

	IGuiRemoteProtocolChannelReceiver<WString>* GetReceiver() override
	{
		return receiver;
	}

	void Write(const WString& package) override
	{
		Console::WriteLine(L"Sent: " + package);
		if (pendingMessageCount < pendingMessages.Count() - 1)
		{
			pendingMessages[pendingMessageCount] = package;
		}
		else
		{
			pendingMessages.Add(package);
		}
		pendingMessageCount++;
	}

	WString GetExecutablePath() override
	{
		return WString::Unmanaged(NamedPipeId);
	}

	void Submit(bool& disconnected) override
	{
		if (connected)
		{
			if (pendingMessageCount > 0)
			{
				Console::WriteLine(L"Submit");
				SendPendingMessages();
			}
		}
		else
		{
			if (pendingMessageCount > 0)
			{
				Console::WriteLine(L"Submit (unconnected)");
				pendingMessageCount = 0;
			}
			disconnected = true;
		}
	}

	void ProcessRemoteEvents() override
	{
	}
};

template<typename T>
void RunInNewThread(T&& threadProc, NamedPipeCoreChannel* channel)
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
			throw;
		}
		catch (const Error& e)
		{
			channel->WriteErrorThreadUnsafe(WString::Unmanaged(e.Description()));
			throw;
		}
	});
}

int StartNamedPipeServer()
{
	HANDLE hPipe = NamedPipeCoreChannel::ServerCreatePipe();
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(NamedPipeId));
	{
		NamedPipeCoreChannel namedPipeServerChannel(hPipe);

		auto jsonParser = Ptr(new glr::json::Parser);
		GuiRemoteJsonChannelStringSerializer channelJsonSerializer(&namedPipeServerChannel, jsonParser);

		GuiRemoteProtocolAsyncJsonChannelSerializer asyncChannelSender;
		asyncChannelSender.Start(
			&channelJsonSerializer,
			[](GuiRemoteProtocolAsyncJsonChannelSerializer* channel)
			{
				GuiRemoteProtocolFromJsonChannel channelSender(channel);
				GuiRemoteProtocolFilter filteredProtocol(&channelSender);
				GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);
				SetupRemoteNativeController(&diffConverterProtocol);
			},
			[&namedPipeServerChannel](
				GuiRemoteProtocolAsyncJsonChannelSerializer::TChannelThreadProc channelThreadProc,
				GuiRemoteProtocolAsyncJsonChannelSerializer::TUIThreadProc uiThreadProc
				)
			{
				RunInNewThread(channelThreadProc, &namedPipeServerChannel);
				RunInNewThread(uiThreadProc, &namedPipeServerChannel);
			});

		Console::WriteLine(L"> Waiting for a renderer ...");
		NamedPipeCoreChannel::ServerWaitForClient(hPipe);
		namedPipeServerChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
		asyncChannelSender.WaitForStopped();
	}
	CloseHandle(hPipe);
	return 0;
}
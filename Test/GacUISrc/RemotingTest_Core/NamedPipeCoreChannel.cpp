#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "NamedPipeShared.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::repeatfiltering;
using namespace vl::presentation::remoteprotocol::channeling;

class NamedPipeCoreChannel;
NamedPipeCoreChannel* coreChannel = nullptr;

class NamedPipeCoreChannel
	: public NamedPipeShared
	, public virtual IGuiRemoteProtocolChannel<WString>
{
protected:
	IGuiRemoteProtocolChannelReceiver<WString>*		receiver = nullptr;
	bool											connected = false;
	EventObject										eventDisconnected;

	List<WString>									pendingMessages;
	vint											pendingMessageCount = 0;

	void OnReadStringThreadUnsafe(Ptr<List<WString>> strs) override
	{
		static WString filteredStrings[] = {
			WString::Unmanaged(LR"JSON({"semantic":"Event","name":"IOMouseMoving")JSON"),
			WString::Unmanaged(LR"JSON({"semantic":"Event","name":"IOMouseEntered")JSON"),
			WString::Unmanaged(LR"JSON({"semantic":"Event","name":"IOMouseLeaved")JSON"),
			WString::Unmanaged(LR"JSON({"semantic":"Event","name":"WindowBoundsUpdated")JSON")
		};
		for (auto str : *strs.Obj())
		{
			for (auto&& filtered : filteredStrings)
			{
				if (str.Length() > filtered.Length() && str.Left(filtered.Length()) == filtered)
				{
					goto ON_RECEIVE;
				}
			}
#ifdef _DEBUG
			Console::WriteLine(L"Received: " + str);
#endif
		ON_RECEIVE:
			receiver->OnReceive(str);
		}
	}

	void OnReadStoppedThreadUnsafe() override
	{
		eventDisconnected.Signal();
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
		eventDisconnected.CreateManualUnsignal(false);
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

	void WaitForDisconnected()
	{
		eventDisconnected.Wait();
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
#ifdef _DEBUG
		Console::WriteLine(L"Sent: " + package);
#endif
		if (pendingMessageCount < pendingMessages.Count())
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
#ifdef _DEBUG
				Console::WriteLine(L"Submit");
#endif
				SendPendingMessages();
			}
		}
		else
		{
			if (pendingMessageCount > 0)
			{
#ifdef _DEBUG
				Console::WriteLine(L"Submit (unconnected)");
#endif
				pendingMessageCount = 0;
			}
			disconnected = true;
		}
	}

	void ProcessRemoteEvents() override
	{
	}
};

void WriteErrorToRendererChannel(const WString& message)
{
	coreChannel->WriteErrorThreadUnsafe(message);
}

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
	HANDLE hPipe = NamedPipeCoreChannel::ServerCreatePipe();
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(NamedPipeId));
	{
		NamedPipeCoreChannel namedPipeCoreChannel(hPipe);

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
		NamedPipeCoreChannel::ServerWaitForClient(hPipe);
		namedPipeCoreChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
		asyncChannelSender.WaitForStopped();
		CloseHandle(hPipe);
		namedPipeCoreChannel.WaitForDisconnected();
	}
	return 0;
}
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

	void OnReadStringThreadUnsafe(const WString& str) override
	{
		Console::Write(L"Received: ");
		Console::WriteLine(str);
		receiver->OnReceive(str);
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
			Console::WriteLine(L"> Sending pending messages ...");
			connected = true;
			SendPendingMessages();
		});
	}

	void WriteErrorThreadUnsafe(const WString& error)
	{
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
		Console::Write(L"Sent: ");
		Console::WriteLine(package);
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
			Console::WriteLine(L"Submit");
			SendPendingMessages();
		}
		else
		{
			Console::WriteLine(L"Submit (pending)");
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
	HANDLE hPipe = CreateNamedPipe(
		NamedPipeId,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
		1,
		65536,
		65536,
		6000,
		NULL);
	CHECK_ERROR(hPipe != INVALID_HANDLE_VALUE, L"CreateNamedPipe failed.");
	Console::Write(L"> Named pipe created, waiting on: ");
	Console::WriteLine(NamedPipeId);
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
				SetupRemoteNativeController(&filteredProtocol);
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
		{
			OVERLAPPED overlapped;
			ZeroMemory(&overlapped, sizeof(overlapped));
			overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			CHECK_ERROR(overlapped.hEvent != NULL, L"ConnectNamedPipe failed on CreateEvent.");

			BOOL result = ConnectNamedPipe(hPipe, &overlapped);
			CHECK_ERROR(result == FALSE, L"ConnectNamedPipe failed.");
			DWORD error = GetLastError();
			switch (error)
			{
			case ERROR_IO_PENDING:
				WaitForSingleObject(overlapped.hEvent, INFINITE);
				break;
			default:
				CHECK_ERROR(error == ERROR_PIPE_CONNECTED, L"ConnectNamedPipe failed on unexpected GetLastError.");
			}

			CloseHandle(overlapped.hEvent);
			namedPipeServerChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
		}
		asyncChannelSender.WaitForStopped();
	}
	CloseHandle(hPipe);
	return 0;
}
#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include <Windows.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;

const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeServerChannel
	: public Object
	, public IGuiRemoteProtocolChannel<WString>
{
protected:
	IGuiRemoteProtocolChannelReceiver<WString>*		receiver = nullptr;
	HANDLE											hPipe = INVALID_HANDLE_VALUE;
	bool											connected = false;
	List<WString>									pendingMessages;

	void OnReceive(const WString& package)
	{
		Console::Write(L"Received: ");
		Console::WriteLine(package);
		receiver->OnReceive(package);
	}

public:

	NamedPipeServerChannel(HANDLE _hPipe)
		: hPipe(_hPipe)
	{
	}

	~NamedPipeServerChannel()
	{
	}

	void AcceptRenderer()
	{
		Console::WriteLine(L"> Wait for renderer...");
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
		if (connected)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
		else
		{
			pendingMessages.Add(package);
		}
	}

	WString GetExecutablePath() override
	{
		return WString::Unmanaged(NamedPipeId);
	}

	void Submit(bool& disconnected) override
	{
		Console::Write(L"Submit");
	}

	void ProcessRemoteEvents() override
	{
	}
};

template<typename T>
void RunInNewThread(T&& threadProc)
{
	Thread::CreateAndStart([threadProc]()
	{
		try
		{
			threadProc();
		}
		catch (const Exception& e)
		{
			(void)e;
			throw;
		}
		catch (const Error& e)
		{
			(void)e;
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
		NamedPipeServerChannel namedPipeServerChannel(hPipe);

		auto jsonParser = Ptr(new glr::json::Parser);
		channeling::GuiRemoteJsonChannelStringSerializer channelJsonSerializer(&namedPipeServerChannel, jsonParser);

		channeling::GuiRemoteProtocolAsyncJsonChannelSerializer asyncChannelSender;
		asyncChannelSender.Start(
			&channelJsonSerializer,
			[](channeling::GuiRemoteProtocolAsyncJsonChannelSerializer* channel)
			{
				channeling::GuiRemoteProtocolFromJsonChannel channelSender(channel);
				repeatfiltering::GuiRemoteProtocolFilter filteredProtocol(&channelSender);
				GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);
				SetupRemoteNativeController(&filteredProtocol);
			},
			[](
				channeling::GuiRemoteProtocolAsyncJsonChannelSerializer::TChannelThreadProc channelThreadProc,
				channeling::GuiRemoteProtocolAsyncJsonChannelSerializer::TUIThreadProc uiThreadProc
				)
			{
				RunInNewThread(channelThreadProc);
				RunInNewThread(uiThreadProc);
			});

		namedPipeServerChannel.AcceptRenderer();
		asyncChannelSender.WaitForStopped();
	}
	CloseHandle(hPipe);
	return 0;
}
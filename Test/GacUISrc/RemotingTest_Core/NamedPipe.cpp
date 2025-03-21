#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include <Windows.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::repeatfiltering;
using namespace vl::presentation::remoteprotocol::channeling;

const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeCoreChannel
	: public Object
	, public virtual IGuiRemoteProtocolChannel<WString>
{
protected:
	IGuiRemoteProtocolChannelReceiver<WString>*		receiver = nullptr;
	HANDLE											hPipe = INVALID_HANDLE_VALUE;
	bool											connected = false;
	List<WString>									pendingMessages;

	void OnReceiveThreadUnsafe(const WString& package)
	{
		Console::Write(L"Received: ");
		Console::WriteLine(package);
		receiver->OnReceive(package);
	}

public:

	NamedPipeCoreChannel(HANDLE _hPipe)
		: hPipe(_hPipe)
	{
	}

	~NamedPipeCoreChannel()
	{
	}

	void RendererConnectedThreadUnsafe(GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel)
	{
		Console::WriteLine(L"> Renderer connected");
		asyncChannel->ExecuteInChannelThread([]()
		{
			Console::WriteLine(L"> Sending pending nessages ...");
			// Set connected and process pendingMessages
		});
	}

	void WriteErrorThreadUnsafe(const WString& error)
	{
		CHECK_FAIL(L"Not Implemented!");
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
		Console::WriteLine(L"Submit");
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
			CHECK_ERROR(result == 0, L"ConnectNamedPipe failed.");
			if (GetLastError() == ERROR_PIPE_CONNECTED)
			{
				CloseHandle(overlapped.hEvent);
				namedPipeServerChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
			}
			else
			{
				struct ConnectNamedPipeContext
				{
					HANDLE hWait;
					HANDLE hEvent;
					NamedPipeCoreChannel* namedPipeChannel;
					GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel;
				};
				auto context = new ConnectNamedPipeContext;
				context->hWait = INVALID_HANDLE_VALUE;
				context->hEvent = overlapped.hEvent;
				context->namedPipeChannel = &namedPipeServerChannel;
				context->asyncChannel = &asyncChannelSender;
				RegisterWaitForSingleObject(
					&context->hWait,
					overlapped.hEvent,
					[](PVOID lpParameter, BOOLEAN TimerOrWaitFired)
					{
						auto context = (ConnectNamedPipeContext*)lpParameter;
						UnregisterWait(context->hWait);
						CloseHandle(context->hEvent);
						context->namedPipeChannel->RendererConnectedThreadUnsafe(context->asyncChannel);
					},
					context,
					INFINITE,
					WT_EXECUTEONLYONCE);
			}
		}
		asyncChannelSender.WaitForStopped();
	}
	CloseHandle(hPipe);
	return 0;
}
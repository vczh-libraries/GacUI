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
	vint											pendingMessageCount = 0;
	stream::MemoryStream							bufferStream;
	OVERLAPPED										overlappedWriteFile;
	HANDLE											hWriteFileOverlappedEvent = INVALID_HANDLE_VALUE;

	void OnReceiveThreadUnsafe(const WString& package)
	{
		Console::Write(L"Received: ");
		Console::WriteLine(package);
		receiver->OnReceive(package);
	}

	vint32_t WriteInt32ToStream(vint32_t number)
	{
		return (vint32_t)bufferStream.Write(&number, sizeof(number));
	}

	vint32_t WriteStringToStream(const WString& str)
	{
		vint32_t bytes = 0;
		{
			vint32_t count = (vint32_t)str.Length();
			bytes += (vint32_t)bufferStream.Write(&count, sizeof(count));
		}
		bytes += (vint32_t)bufferStream.Write((void*)str.Buffer(), sizeof(wchar_t) * str.Length());
		return bytes;
	}

	void BeginSendStream()
	{
		vint32_t bytes = 0;
		bufferStream.SeekFromBegin(0);
		bufferStream.Write(&bytes, sizeof(bytes));
	}

	void EndSendStream(vint32_t bytes)
	{
		bufferStream.SeekFromBegin(0);
		WriteInt32ToStream(bytes);

		WaitForSingleObject(hWriteFileOverlappedEvent, INFINITE);
		ResetEvent(hWriteFileOverlappedEvent);
		ZeroMemory(&overlappedWriteFile, sizeof(overlappedWriteFile));
		overlappedWriteFile.hEvent = hWriteFileOverlappedEvent;
		WriteFile(hPipe, bufferStream.GetInternalBuffer(), (DWORD)(bytes + sizeof(bytes)), NULL, &overlappedWriteFile);
	}

	void SendPendingMessages()
	{
		vint32_t bytes = 0;
		BeginSendStream();
		bytes += WriteInt32ToStream((vint32_t)pendingMessageCount);
		for (vint i = 0; i < pendingMessageCount; i++)
		{
			bytes += WriteStringToStream(pendingMessages[i]);
		}
		EndSendStream(bytes);
		pendingMessageCount = 0;
	}

public:

	NamedPipeCoreChannel(HANDLE _hPipe)
		: hPipe(_hPipe)
	{
		hWriteFileOverlappedEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		CHECK_ERROR(hWriteFileOverlappedEvent != NULL, L"NamedPipeCoreChannel initialization failed on CreateEvent.");
	}

	~NamedPipeCoreChannel()
	{
		CloseHandle(hWriteFileOverlappedEvent);
	}

	void RendererConnectedThreadUnsafe(GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel)
	{
		Console::WriteLine(L"> Renderer connected");
		asyncChannel->ExecuteInChannelThread([this]()
		{
			Console::WriteLine(L"> Sending pending nessages ...");
			connected = true;
			SendPendingMessages();
		});
	}

	void WriteErrorThreadUnsafe(const WString& error)
	{
		BeginSendStream();
		EndSendStream(WriteStringToStream(L"!" + error));
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
			case ERROR_PIPE_CONNECTED:
				break;
			default:
				CHECK_FAIL(L"ConnectNamedPipe failed on unexpected GetLastError.");
			}

			CloseHandle(overlapped.hEvent);
			namedPipeServerChannel.RendererConnectedThreadUnsafe(&asyncChannelSender);
			/*
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
						delete context;
					},
					context,
					INFINITE,
					WT_EXECUTEONLYONCE);
			}
			*/
		}
		asyncChannelSender.WaitForStopped();
	}
	CloseHandle(hPipe);
	return 0;
}
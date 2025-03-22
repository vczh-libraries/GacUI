#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../RemotingTest_Core/NamedPipeShared.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

class NamedPipeRendererChannel
	: public NamedPipeShared
	, protected virtual IGuiRemoteProtocolChannelReceiver<WString>
{
protected:
	IGuiRemoteProtocolChannel<WString>*				channel = nullptr;
	EventObject										eventDisconnected;

	void OnReadStringThreadUnsafe(const WString& str) override
	{
		return;
		GetCurrentController()->AsyncService()->InvokeInMainThread(
			GetCurrentController()->WindowService()->GetMainWindow(),
			[this, str]()
			{
				channel->Write(str);
			});
	}

	void OnReceive(const WString& package) override
	{
		SendSingleString(package);
	}

public:

	NamedPipeRendererChannel(HANDLE _hPipe, IGuiRemoteProtocolChannel<WString>* _channel)
		: NamedPipeShared(_hPipe)
		, channel(_channel)
	{
		eventDisconnected.CreateManualUnsignal(false);
		_channel->Initialize(this);
		// TODO: Call BeginReadingLoopUnsafe after main window created
		BeginReadingLoopUnsafe();
	}

	~NamedPipeRendererChannel()
	{
	}

	void WaitForDisconnected()
	{
		eventDisconnected.Wait();
	}
};

extern void InstallRemoteRenderer(GuiRemoteRendererSingle* _remoteRenderer);

int StartNamedPipeClient()
{
	HANDLE hPipe = CreateFile(
		NamedPipeId,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	CHECK_ERROR(hPipe != INVALID_HANDLE_VALUE, L"CreateFile failed.");
	CHECK_ERROR(GetLastError() == 0, L"Another renderer already connected.");

	DWORD dwPipeMode = PIPE_READMODE_MESSAGE;
	BOOL bSucceeded = SetNamedPipeHandleState(
		hPipe,
		&dwPipeMode,
		NULL,
		NULL);
	CHECK_ERROR(bSucceeded, L"SetNamedPipeHandleState failed.");

	int result = 0;
	{
		auto jsonParser = Ptr(new glr::json::Parser);
		GuiRemoteRendererSingle remoteRenderer;
		GuiRemoteJsonChannelFromProtocol channelReceiver(&remoteRenderer);
		GuiRemoteJsonChannelStringDeserializer channelJsonDeserializer(&channelReceiver, jsonParser);
		NamedPipeRendererChannel namedPipeServerChannel(hPipe, &channelJsonDeserializer);

		InstallRemoteRenderer(&remoteRenderer);
		result = SetupRawWindowsDirect2DRenderer();
		namedPipeServerChannel.WaitForDisconnected();
	}
	CloseHandle(hPipe);
	return result;
}
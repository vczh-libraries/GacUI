#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include <Windows.h>

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;

const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeRendererChannel
	: public Object
	, public IGuiRemoteProtocolChannel<WString>
{
protected:
	IGuiRemoteProtocolChannelReceiver<WString>*		receiver = nullptr;
	HANDLE											hPipe = INVALID_HANDLE_VALUE;
	EventObject										eventDisconnected;

	void OnReceiveThreadUnsafe(const WString& package)
	{
		receiver->OnReceive(package);
	}

public:

	NamedPipeRendererChannel(HANDLE _hPipe)
		: hPipe(_hPipe)
	{
		eventDisconnected.CreateManualUnsignal(false);
	}

	~NamedPipeRendererChannel()
	{
	}

	void WaitForDisconnected()
	{
		eventDisconnected.Wait();
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
		CHECK_FAIL(L"Not Implemented!");
	}

	WString GetExecutablePath() override
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	void Submit(bool& disconnected) override
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	void ProcessRemoteEvents() override
	{
		CHECK_FAIL(L"This function should not be called!");
	}
};

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
		NamedPipeRendererChannel namedPipeServerChannel(hPipe);
		namedPipeServerChannel.WaitForDisconnected(); 
		result = SetupRawWindowsDirect2DRenderer();
	}
	CloseHandle(hPipe);
	return result;
}
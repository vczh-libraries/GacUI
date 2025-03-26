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

	void OnReadStringThreadUnsafe(Ptr<List<WString>> strs) override
	{
		GetCurrentController()->AsyncService()->InvokeInMainThread(
			GetCurrentController()->WindowService()->GetMainWindow(),
			[this, strs]()
			{
				for (auto str : *strs.Obj())
				{
					channel->Write(str);
				}
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
	}

	~NamedPipeRendererChannel()
	{
	}

	void RegisterMainWindow(INativeWindow* _window)
	{
		BeginReadingLoopUnsafe();
	}

	void UnregisterMainWindow()
	{
	}

	void WaitForDisconnected()
	{
		eventDisconnected.Wait();
	}
};

NamedPipeRendererChannel* channel = nullptr;
GuiRemoteRendererSingle* renderer = nullptr;

void GuiMain()
{
	auto mainWindow = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
	mainWindow->SetTitle(L"Connecting ...");
	{
		auto size = mainWindow->Convert(Size(320, 240));
		auto screen = GetCurrentController()->ScreenService()->GetScreen((vint)0);
		auto client = screen->GetClientBounds();
		auto x = client.Left() + (client.Width() - size.x) / 2;
		auto y = client.Top() + (client.Height() - size.y) / 2;
		mainWindow->SetBounds({ {x,y},size });
	}
	channel->RegisterMainWindow(mainWindow);
	renderer->RegisterMainWindow(mainWindow);
	GetCurrentController()->WindowService()->Run(mainWindow);
	renderer->UnregisterMainWindow();
	channel->UnregisterMainWindow();
}

int StartNamedPipeClient()
{
	HANDLE hPipe = NamedPipeRendererChannel::ClientCreatePipe();
	NamedPipeRendererChannel::ClientWaitForServer(hPipe);
	int result = 0;
	{
		auto jsonParser = Ptr(new glr::json::Parser);
		GuiRemoteRendererSingle remoteRenderer;
		GuiRemoteJsonChannelFromProtocol channelReceiver(&remoteRenderer);
		GuiRemoteJsonChannelStringDeserializer channelJsonDeserializer(&channelReceiver, jsonParser);
		NamedPipeRendererChannel namedPipeServerChannel(hPipe, &channelJsonDeserializer);

		channel = &namedPipeServerChannel;
		renderer = &remoteRenderer;
		result = SetupRawWindowsDirect2DRenderer();
		namedPipeServerChannel.WaitForDisconnected();
		renderer = nullptr;
		channel = nullptr;
	}
	CloseHandle(hPipe);
	return result;
}
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../RemotingTest_Core/Shared/NamedPipeShared.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

class RendererChannel
	: public NamedPipeShared
	, protected virtual IGuiRemoteProtocolChannelReceiver<WString>
{
protected:
	GuiRemoteRendererSingle*						renderer = nullptr;
	IGuiRemoteProtocolChannel<WString>*				channel = nullptr;
	EventObject										eventDisconnected;

	void											OnReadStringThreadUnsafe(Ptr<List<WString>> strs) override;
	void											OnReadStoppedThreadUnsafe() override;
	void											OnReceive(const WString& package) override;

public:

	RendererChannel(GuiRemoteRendererSingle* _renderer, HANDLE _hPipe, IGuiRemoteProtocolChannel<WString>* _channel);
	~RendererChannel();

	void											RegisterMainWindow(INativeWindow* _window);
	void											UnregisterMainWindow();
	void											WaitForDisconnected();
};
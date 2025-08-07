#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../RemotingTest_Core/Shared/ProtocolCallback.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

class RendererChannel
	: protected virtual INetworkProtocolCallback
	, protected virtual IGuiRemoteProtocolChannelReceiver<WString>
{
protected:
	INetworkProtocol*								networkProtocol = nullptr;
	GuiRemoteRendererSingle*						renderer = nullptr;
	IGuiRemoteProtocolChannel<WString>*				channel = nullptr;
	EventObject										eventDisconnected;
	bool											displayedError = false;

	void											OnReadStringThreadUnsafe(Ptr<List<WString>> strs) override;
	void											OnReadStoppedThreadUnsafe() override;

protected:
	SpinLock										cacheLock;
	bool											isCaching = false;
	List<WString>									cachedPackages;

	void											OnReceive(const WString& package) override;

public:

	RendererChannel(GuiRemoteRendererSingle* _renderer, INetworkProtocol* _networkProtocol, IGuiRemoteProtocolChannel<WString>* _channel);
	~RendererChannel();

	void											RegisterMainWindow(INativeWindow* _window);
	void											UnregisterMainWindow();
	void											WaitForDisconnected();

	void											BeforeWrite(const ChannelPackageInfo& info);
	void											BeforeOnReceive(const ChannelPackageInfo& info);
};
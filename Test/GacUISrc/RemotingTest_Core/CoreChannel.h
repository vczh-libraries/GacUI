#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "Shared/NamedPipeShared.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::repeatfiltering;
using namespace vl::presentation::remoteprotocol::channeling;

class CoreChannel
	: public NamedPipeShared
	, protected virtual INetworkProtocolCallback
	, public virtual IGuiRemoteProtocolChannel<vl::WString>
{
protected:
	IGuiRemoteProtocolChannelReceiver<vl::WString>*		receiver = nullptr;
	bool												connected = false;
	vl::EventObject										eventDisconnected;

	vl::collections::List<vl::WString>					pendingMessages;
	vl::vint											pendingMessageCount = 0;

	void												OnReadStringThreadUnsafe(vl::Ptr<vl::collections::List<vl::WString>> strs) override;
	void												OnReadStoppedThreadUnsafe() override;
	void												SendPendingMessages();

public:

	CoreChannel(HANDLE _hPipe);
	~CoreChannel();

	void												RendererConnectedThreadUnsafe(GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel);
	void												WaitForDisconnected();
	void												WriteErrorThreadUnsafe(const vl::WString& error);

	void												Initialize(IGuiRemoteProtocolChannelReceiver<vl::WString>* _receiver) override;
	IGuiRemoteProtocolChannelReceiver<vl::WString>*		GetReceiver() override;
	void												Write(const vl::WString& package) override;
	vl::WString											GetExecutablePath() override;
	void												Submit(bool& disconnected) override;
	void												ProcessRemoteEvents() override;
};
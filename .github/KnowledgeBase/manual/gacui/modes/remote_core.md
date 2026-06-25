# Remote Protocol Core Application

A remote protocol core application is a normal GacUI application that calls SetupRemoteNativeController. It becomes headless and is forced into hosted mode. The core does not draw to an OS native window; it sends JSON remote protocol packages through a vl::inter_process channel.

The standard C++ path is small:
- Start a GuiRemoteProtocolNetworkChannelServer\<TServerBase\> over an INetworkProtocolServer implementation such as NamedPipeServer or HttpServer.
- Connect the core to that server with GuiRemoteProtocolLocalChannelClient.
- Wrap the core channel with GuiRemoteProtocolJsonChannelRenderer_Async and GuiRemoteProtocolCoreChannel.
- Pass the protocol, usually after GuiRemoteProtocolFilter and GuiRemoteProtocolDomDiffConverter, to SetupRemoteNativeController.

The core client is expected to be registered as GacUIRemoteProtocolCoreClientId. A renderer created by GuiRemoteProtocolChannelClient advertises GacUIRemoteProtocolChannelName. GuiRemoteProtocolCoreChannel learns the renderer client id from the renderer's ControllerConnect event, so user code does not need to route individual remote protocol messages.

A compact named pipe setup looks like this:
```c++
using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remoteprotocol::repeatfiltering;

class NamedPipeRemoteCoreServer
    : public GuiRemoteProtocolNetworkChannelServer<inter_process::NamedPipeServer>
{
    using Base = GuiRemoteProtocolNetworkChannelServer<inter_process::NamedPipeServer>;

    EventObject rendererConnected;

public:
    NamedPipeRemoteCoreServer(Ptr<glr::json::Parser> parser, const WString& pipeName)
        : Base(parser, pipeName)
    {
        rendererConnected.CreateManualUnsignal(false);
    }

    void WaitForRenderer()
    {
        rendererConnected.Wait();
    }

protected:
    inter_process::WaitForClientResult OnClientConnected(
        vint clientId,
        const IJsonChannelClient::ChannelNameList& availableChannels,
        IJsonChannelClient* localClient) override
    {
        if (localClient)
        {
            return inter_process::WaitForClientResult::Accept;
        }

        if (availableChannels.Contains(WString::Unmanaged(GacUIRemoteProtocolChannelName)))
        {
            rendererConnected.Signal();
            return inter_process::WaitForClientResult::Accept;
        }
        return inter_process::WaitForClientResult::Reject;
    }
};

void StartNamedPipeRemoteCore()
{
    auto jsonParser = Ptr(new glr::json::Parser);
    NamedPipeRemoteCoreServer server(jsonParser, L"GacUIRemoteProtocolNamedPipe");
    server.Start();

    auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
    auto coreClientId = server.ConnectLocalClient(coreClient);
    CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"Failed to register the core client.");

    server.WaitForRenderer();

    GuiRemoteProtocolJsonChannelRenderer_Async asyncChannel(coreClient->GetProtocolChannel());
    GuiRemoteProtocolCoreChannel coreProtocol(
        coreClient.Obj(),
        &asyncChannel,
        L"MyRemoteCore.exe",
        asyncChannel.GetRemoteEventProcessor());

    GuiRemoteProtocolFilter filteredProtocol(&coreProtocol);
    GuiRemoteProtocolDomDiffConverter diffProtocol(&filteredProtocol);
    SetupRemoteNativeController(&diffProtocol);

    server.Stop();
}
```

GuiMain still creates and runs GacUI windows. If the UI throws while the remote server is alive, send the error through IJsonLocalChannelServer::BroadcastError so the renderer stops instead of waiting for more packages. A production server can also reject clients that do not offer GacUIRemoteProtocolChannelName, disconnect previous renderers, or call GuiRemoteProtocolCoreChannel::DetachRenderer when replacing a renderer.

See [RemotingTest_Core](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) for the complete named pipe and HTTP implementation.


# Remote Protocol Client Application

GuiRemoteRendererSingle is the standard C++ renderer for one remote GacUI core. It implements IGuiRemoteProtocol, renders to one INativeWindow, and sends user input, window, screen, measuring, and automation events back to the core.

The renderer side does not implement a string channel anymore. It only chooses a vl::inter_process transport client, creates a GuiRemoteProtocolChannelClient, and bridges that JSON channel to GuiRemoteRendererSingle.

A compact named pipe renderer looks like this:
```c++
using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

GuiRemoteRendererSingle* remoteRenderer = nullptr;
GuiRemoteProtocolAsyncJsonChannelRenderer* asyncChannel = nullptr;

class GuiMainInvoker : public Object, public virtual IGuiRemoteProtocolAsyncRendererInvoker
{
public:
    void InvokeInMainThread(const Func<void()>& proc) override
    {
        GetApplication()->InvokeInMainThread(nullptr, proc);
    }
};

void GuiMain()
{
    auto mainWindow = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
    mainWindow->SetTitle(L"Connecting ...");

    auto invoker = Ptr(new GuiMainInvoker);
    remoteRenderer->RegisterMainWindow(mainWindow);
    asyncChannel->SetInvokeInMainThread(invoker);
    asyncChannel->ProcessPendingMessages();

    GetCurrentController()->WindowService()->Run(mainWindow);

    asyncChannel->SetInvokeInMainThread(nullptr);
    remoteRenderer->UnregisterMainWindow();
}

int StartNamedPipeRemoteRenderer()
{
    auto jsonParser = Ptr(new glr::json::Parser);
    auto networkClient = Ptr(new inter_process::named_pipe::NamedPipeClient(L"GacUIRemoteProtocolNamedPipe"));

    GuiRemoteProtocolChannelClient channelClient(networkClient, jsonParser);
    GuiRemoteProtocolAsyncJsonChannelRenderer asyncRendererChannel(channelClient.GetProtocolChannel());
    GuiRemoteRendererSingle renderer(true);
    GuiRemoteProtocolRendererChannel rendererChannel(
        &asyncRendererChannel,
        &renderer);

    channelClient.WaitForServer();

    remoteRenderer = &renderer;
    asyncChannel = &asyncRendererChannel;
    auto result = SetupRawWindowsDirect2DRenderer();
    asyncChannel = nullptr;
    remoteRenderer = nullptr;
    return result;
}
```

GuiRemoteProtocolAsyncJsonChannelRenderer queues messages from the channel and runs them on the UI thread through IGuiRemoteProtocolAsyncRendererInvoker. GuiRemoteProtocolRendererChannel converts JSON channel packages to calls on GuiRemoteRendererSingle, and converts renderer events back to JSON packages for the core client id.

The portable **/MiniHttp** path creates a default loopback TCP client with **vl::inter_process::async_tcp_socket::CreateDefaultAsyncSocketClient(port)**, wraps it in **vl::inter_process::async_tcp_socket::SocketHttpClient(socketClient, L"localhost", urlPrefix)**, and passes that object to **GuiRemoteProtocolChannelClient**. The renderer application shown by the linked test remains Win32 even though the VlppOS transport itself is available on Windows, Linux and macOS.

Override `GuiRemoteProtocolChannelClient::OnReadError` when a Core-authored `!Error` should show fatal UI. Treat `OnLocalError(..., true)` as an independently complete, prompt-free disconnected transition and do not wait for `OnDisconnected`; use `OnDisconnected` as the idempotent fallback when it is delivered. See [RemotingTest_Rendering_Win32](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Rendering_Win32) for the complete named-pipe (**/Pipe**), Windows HTTP.sys/WinHTTP (**/Http**) and portable Mini HTTP (**/MiniHttp**) implementations.

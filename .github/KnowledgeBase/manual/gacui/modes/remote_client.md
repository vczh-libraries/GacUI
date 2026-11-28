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

class GuiMainInvoker : public IGuiRemoteProtocolAsyncRendererInvoker
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

    GuiMainInvoker invoker;
    remoteRenderer->RegisterMainWindow(mainWindow, L"localhost");
    asyncChannel->SetInvokeInMainThread(&invoker);

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

For application-defined drawing, use [<Raw/>](../.././gacui/components/elements/home.md). `GuiRemoteRendererSingle` creates the active platform's native raw element and exposes `GetRawElements()`, `RawElementInstalled`, `RawElementUninstalled` and `RawElementChanged` so renderer code can attach its drawing callbacks. The protocol carries application ID and data strings; each renderer implements the drawing locally.

Override GuiRemoteProtocolChannelClient::OnReadError, OnLocalError, or OnDisconnected when the renderer should show a fatal error dialog or call GuiRemoteRendererSingle::ForceExitByFatelError. See [RemotingTest_Rendering_Win32](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Rendering_Win32) for the complete named-pipe (**/Pipe**), Windows HTTP.sys/WinHTTP (**/Http**) and portable Mini HTTP (**/MiniHttp**) implementations.

## Advertised service capabilities

`ControllerGlobalConfig::injectDateTime` advertises local timezone conversion; `ControllerGlobalConfig::injectLocale` advertises locale information; `ControllerGlobalConfig::injectFileSystem` advertises filesystem access. These capabilities are independent and require the matching [Core configuration](../.././gacui/modes/remote_core.md) flag for remote use.

The native renderer's `GuiRemoteRendererSingle::GlobalConfig` initializes these three capabilities to `true`. The demo applications explicitly disable them unless the respective `/remoteDT`, `/remoteLocale` or `/remoteFS` option is supplied. A default protocol `ControllerGlobalConfig` record alone does not enable them.

When a capability is unavailable, Core DateTime and Locale use predecessor implementations. An enabled remote filesystem exposes an empty root. A native renderer can install its own underlying services with `vl::InjectDateTimeImpl`, `vl::InjectLocaleImpl` and `vl::filesystem::InjectFileSystemImpl`; this replacement is separate from capability advertisement.

## Sharing the renderer clipboard

Clipboard sharing is enabled by default. The renderer advertises `ControllerGlobalConfig::injectClipboard` when it can provide the service. Core sends `InjectionRequestClipboard` once per enabled connection. The renderer publishes its current clipboard and later OS changes with `InjectionUpdateClipboardFromRenderer`. It acknowledges each `InjectionUpdateClipboardFromCore` write with the result for that request.

GacJS uses the clipboard available to its browser page. Use the `remoteClipboard` checkbox or URL option. An omitted option, a bare option and `remoteClipboard=true` enable it. `remoteClipboard=false` disables it after refresh. The page needs a secure context and browser clipboard access. It asks before sharing with a different network destination. A denied read leaves the last known contents intact. On browsers without clipboard-change events, focus and permitted input gestures refresh the snapshot.

Copying text in another renderer-side application updates the remote editor’s Paste command. Styled selections use a GacUI document representation where the clipboard supports it. A standalone image uses PNG. The Wasm worker uses the same page-side adapter.


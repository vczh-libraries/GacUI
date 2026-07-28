# Hosted Mode and Remote Protocol

Using **SetupHostedWindowsGDIRenderer** or **SetupHostedWindowsDirect2DRenderer** instead of **SetupWindowsGDIRenderer** or **SetupWindowsDirect2DRenderer** runs a GacUI application in hosted mode. A hosted mode GacUI application will start only one OS native window, other windows are rendered inside it virtually. System dialogs will be replaced by predefined GacUI implemented dialogs by default, so that anything will be strictly inside the OS native window.

Using **SetupRemoteNativeController** runs a GacUI application with remote protocol (forced in hosted mode), which becomes headless, instead of render anything on the screen, it sends out rendering commands to a remote client. This part will be covered in [Remote Protocol Core Application](../.././gacui/modes/remote_core.md).

If you want a remote protocol renderer that still work with other components of GacUI, **GuiRemoteRendererSingle** implements everything that the remote protocol needs and starts a window, displaying anything from the core GacUI application, with user interactions covered. This part will be covered in [Remote Protocol Client Application](../.././gacui/modes/remote_client.md).

The [GacJS](https://github.com/vczh-libraries/GacJS) also implements a GacUI renderer in a browser, connecting to a GacUI application via HTTP protocol. But the HTTP protocol implementation is not official, it is for demo only, no security guarantee is provided.

GacUI provides a JSON channel layer for the remote protocol, implemented with **vl::inter_process**. A C++ core application usually starts a **GuiRemoteProtocolNetworkChannelServer**, connects itself through **GuiRemoteProtocolLocalChannelClient**, and waits for a renderer connected through **GuiRemoteProtocolChannelClient**. The underlying data-transmission implementation is still selected by the application. VlppOS transport choices include:
- **vl::inter_process::async_tcp_socket::NetworkProtocolServer\<TAsyncSocketServer\>** and **NetworkProtocolClient\<TAsyncSocketClient\>** for direct length-framed loopback TCP on Windows, Linux and macOS.
- **vl::inter_process::async_tcp_socket::SocketHttpServer** and **vl::inter_process::async_tcp_socket::SocketHttpClient** for cross-platform Mini HTTP over loopback TCP. Create their injected sockets with **CreateDefaultAsyncSocketServer** and **CreateDefaultAsyncSocketClient**.
- **vl::inter_process::named_pipe::NamedPipeServer** and **vl::inter_process::named_pipe::NamedPipeClient** on Windows.
- **vl::inter_process::windows_http::HttpServer** and **vl::inter_process::windows_http::HttpClient**, implemented with HTTP.sys and WinHTTP, on Windows.

VlppOS provides the **INetworkProtocol** and **IChannel** interface families only for test-purpose local cross-process communication. Applications may use these public contracts directly, but product code is strongly recommended to provide its own implementation and keep the GacUI JSON channel classes unchanged.

A custom data-transmission implementation can be added by implementing these interfaces from **vl::inter_process**: **INetworkProtocolConnection**, **INetworkProtocolCallback**, **INetworkProtocolClient** and **INetworkProtocolServer**. They are named network protocol interfaces, but the implementation does not have to be a network protocol; stdio, DLL function calls, shared memory, a WebAssembly/JavaScript bridge, sockets or any other asynchronous message transport can fit this layer. This part will be covered in [Remote Protocol Channel Layer](../.././gacui/modes/remote_communication.md).

The complete Windows test projects are [RemotingTest_Core](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) and [RemotingTest_Rendering_Win32](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Rendering_Win32). They expose **/Pipe**, **/Http** and **/MiniHttp** modes for the Windows named-pipe, Windows HTTP and portable Mini HTTP setups.


# Hosted Mode and Remote Protocol

Using **SetupHostedWindowsGDIRenderer** or **SetupHostedWindowsDirect2DRenderer** instead of **SetupWindowsGDIRenderer** or **SetupWindowsDirect2DRenderer** runs a GacUI application in hosted mode. A hosted mode GacUI application will start only one OS native window, other windows are rendered inside it virtually. System dialogs will be replaced by predefined GacUI implemented dialogs by default, so that anything will be strictly inside the OS native window.

Using **SetupRemoteNativeController** runs a GacUI application with remote protocol (forced in hosted mode), which becomes headless, instead of render anything on the screen, it sends out rendering commands to a remote client. This part will be covered in [Remote Protocol Core Application](../.././gacui/modes/remote_core.md).

If you want a remote protocol renderer that still work with other components of GacUI, **GuiRemoteRendererSingle** implements everything that the remote protocol needs and starts a window, displaying anything from the core GacUI application, with user interactions covered. This part will be covered in [Remote Protocol Client Application](../.././gacui/modes/remote_client.md).

The [GacJS](https://github.com/vczh-libraries/GacJS) also implements a GacUI renderer in a browser, connecting to a GacUI application via HTTP protocol. But the HTTP protocol implementation is not official, it is for demo only, no security guarantee is provided.

GacUI provides a JSON channel layer for the remote protocol, implemented with **vl::inter_process**. A C++ core application usually starts a **GuiRemoteProtocolNetworkChannelServer**, connects itself through **GuiRemoteProtocolLocalChannelClient**, and waits for a renderer connected through **GuiRemoteProtocolChannelClient**. The underlying data-transmission implementation is still selected by the application. Current Windows examples use:
- **vl::inter_process::NamedPipeServer** and **vl::inter_process::NamedPipeClient**.
- **vl::inter_process::HttpServer** and **vl::inter_process::HttpClient**.

A different data-transmission implementation should be added by implementing the **vl::inter_process****INetworkProtocolConnection**, **INetworkProtocolCallback**, **INetworkProtocolClient** and **INetworkProtocolServer** interfaces. They are named network protocol interfaces, but the implementation does not have to be a network protocol; stdio, DLL function calls, shared memory, sockets or any other message transport can fit this layer. so the GacUI JSON channel classes can stay unchanged. This part will be covered in [Remote Protocol Channel Layer](../.././gacui/modes/remote_communication.md).

The complete Windows test projects are [RemotingTest_Core](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core) and [RemotingTest_Rendering_Win32](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Rendering_Win32). They show the current named pipe and HTTP setup.


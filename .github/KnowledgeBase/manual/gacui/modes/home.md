# Hosted Mode and Remote Protocol

Using**SetupHostedWindowsGDIRenderer**or**SetupHostedWindowsDirect2DRenderer**instead of**SetupWindowsGDIRenderer**or**SetupWindowsDirect2DRenderer**runs a GacUI application in hosted mode. A hosted mode GacUI application will start only one OS native window, other windows are rendered inside it virtually. System dialogs will be replaced by predefined GacUI implemented dialogs by default, so that anything will be strictly inside the OS native window.

Using**SetupRemoteNativeController**runs a GacUI application with remote protocol (forced in hosted mode), which becomes headless, instead of render anything on the screen, it sends out rendering commands to a remote client. This part will be covered in[Remote Protocol Core Application](../.././gacui/modes/remote_core.md).

If you want a remote protocol renderer that still work with other components of GacUI,**GuiRemoteRendererSingle**implements everything that the remote protocol needs and starts a window, displaying anything from the core GacUI application, with user interactions covered. This part will be covered in[Remote Protocol Client Application](../.././gacui/modes/remote_client.md).

The[GacJS](https://github.com/vczh-libraries/GacJS)also implements a GacUI renderer in a browser, connecting to a GacUI application via HTTP protocol. But the HTTP protocol implementation is not official, it is for demo only, no security guarantee is provided.

GacUI does not provide any protocol implementation, you need to connect the GacUI application with a remote client by yourself. And you are free to go with and not limited to anything below:
- Network protocols such as TCP, HTTP, HTTPS, NamedPipe, etc. For HTTP and NamedPipd, the**GacUI**repo has a test client as a demo.
- Non-networking protocols such as Foreign Function Interface (FFI) or even Dynamic-Linked Library (DLL).But GacUI still offers some wrappers that simplifies everything to string sending and receiving. This part will be covered in[Implementing a Communication Protocol](../.././gacui/modes/remote_communication.md).

In[this folder](https://github.com/vczh-libraries/Release/tree/master/Import/Metadata)there are definitions of the remote protocol in JSON format and in its original format. The original format can be parsed using**vl::presentation::remoteprotocol::Parser**in**GacUICompiler.h**. You can then write any code generation tool to generate code in any programming language you want.


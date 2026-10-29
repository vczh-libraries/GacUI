This is a continuation of the previous work:
- TODO_Injection.md

1. Previous work also fixes a bug when saved private document format doesn not contain styles, but there is one thing to improve:
The full style copy is done in `DocumentModel::CopyDocument()`, but I think we should move the work into `DocumentModel::CopyDocument(TextPos, TextPos, deepCopy)`.
Currently `DocumentModel::CopyDocument(TextPos, TextPos, deepCopy)` only copies used styles, but I would like when `deepCopy` is true it copies all styles just like how `DocumentModel::CopyDocument()` is doing.
So that the simple `CopyDocument` could just delegate the work to the complex one.

2. Using `CppTest` and switch to the ribbon document tab, after selecting a piece of text you can change its style, e.g., "Header 1".
In the style dropdown, if you select any item in the ribbon directly, it works fine.
But when you open the dropdown, or when the window does not have enough space and you can only use the dropdown, select a style item in the dropdown does not respond.
Figure it out and fix the issue.

3. Support `RemotingTest_Win32_Renderer /corehost:localhost`, so that it could connects to other computer.
Fix `test.sh` in `wGac` and `iGac` to support it, but skip verification as you are on Windows.

First you need to fix VlppOS. There are `AsyncSocketClient` in `AsyncSocket.(Windows|macOS|Linux).h` with the same layout.
The current constructor connects a socket server in localhost.
You are going to add another constructor with an extra `WString host`.
The host could be IPv4 address, IPv6 address or a name that needs to look up via DNS (e.g. `getaddrinfo` but you must choose the best way per OS).
You may or may not need to parse the IPv4/6 address by yourself, use OS provided ways when possible.
Therefore `SocketHttpClientApi` could actually connects to other computer.
When such API change is ready, you can then fix all test apps when using `/corehost` with `/MiniHttp`.

You still need to make it work for `/Http` and `/Pipe`.
`windows_http::HttpClient` should already support this feature.
`named_pipe::NamedPipeClient` needs to add another constructor, with an extra `WString host`. And then you can pass "." or "host" to `ClientCreatePipe to construct the actual pipe url.

Both added `WString host` should be the first argument.

`/corehost` should be optional, when it is not used, it means `/corehost:localhost`.

During verification, you may need to pay attention if `/Http` and `/MiniHttp` accepts connections other than "localhost". You should verify if it accept "127.0.0.1" and the machine name, by not performing any constraint on the host name.
And then you can start core and native renderer, see if using `/corehost:127.0.0.1` and `/corehost:machine name` work or not.

4. Introduce this new parameter in `Project.md`, and `AGENTS.md` in `wGac` and `iGac`, as well as their `README.md` if `test.sh` is already explained there.

5. `GacJS` it out of scope.

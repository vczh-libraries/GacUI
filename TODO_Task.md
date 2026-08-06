Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

- Move `RequesterPhrase` to header.
- `RemotingRequesterSession::TaskQueueThread|BroadcastingLocalClient|RequesterLocalClient` could be moved to parallel level types.
- `RemoteViewModelChannelServer` inherits from `RemotingRequesterSession` protectedly, to keep more functions in cpp files:
  - Delete `GetSession` method.
    - `RemotingTest_Core` and `CppTest_Rvm` only needs `Start`, `Stop` and `RequestService`.
    - `Start` and `Stop` are virtual functions, they could be overrided in `RemoteViewModelChannelServer`. Therefore no need to call `Start` and `Stop` for eachclass separately.
    - `RequestService` is an extra function, `RemotingTest_Core` could cast from `RemotingChannelServer` to `RemoteViewModelChannelServer` to use it.
  - `RequestService` will be moved to `RemoteViewModelChannelServer` so nothing from `RemotingRequesterSession` needs to expose for users.
    - Delete `ViewModelServiceName` constant.
    - A type name `WString` argument will be added to `RequestService`.
    - Both `RemotingTest_Core` and `CppTest_Rvm` will call `RequestService` with a string literal directly.
  - `FinalizeRpcOnTaskQueue` moved into `RemotingRequesterSession` as trivial method.
- Rename:
  - `RemotingRequesterSession` -> `RpcServerHelpers`
  - `BroadcastingLocalClient` -> `RpcBroadcastingLocalClient`
  - `RequesterLocalClient` -> `RpcServiceAccessLocalClient`

To verify, follow `DebugRemoteProtocolWithNativeRenderer.md` to cover `CppTest_Rvm`, and `RemotingTest_(Core|Renderer_Win32)`, with only `/Http` and `/RVMT` only.
No need to run unit test and other test apps.

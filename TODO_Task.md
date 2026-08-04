Perform the following refactor on the design in `Test/RemotingHelpers` and test apps.

1) Move new files
- `Test/RemotingHelpers/ViewModelHostClient/ViewModelHostClient.*` to `Test/Rvmt/ViewModel/ViewModelHostClient.*`.
- `RemoteViewModelTestRuntime.h` to `Test/Rvmt/ViewModel/ViewModelHostServer.h`, and there will be a new `ViewModelHostServer.cpp`.
- `RemoteViewModelTestShared.*` to `Test/Rvmt/ViewModel/ViewModelShared.*`.
All these files should be added to `Source_RemotingHelpers` folder and solution explorer folders properly updated, other project will no longer reference these files directly.

2) Move source file content
- In `ViewModelHostClient.h` there are some `inline constexpr`, move them to `ViewModelShared.h`.
- From `ViewModelHostClient.*` move `RemotingRequesterSession` to `ViewModelHostServer.*`.
- Delete `RemotingJsonDispatcherClient` and `RemotingDispatcherFactory`:
  - These two constructions are originally created so that they defer the `InitializeRpc` function to the implementation.
  - Now all files are moved in `Test/RemotingHelpers` so such twisted decoupling will no longer needed.
  - The `InitializeRpc` is in `ViewModelShared.cpp`, rewrite to a ordinaly function, receiving enough arguments so the initialization could perform.
  - Call the actual `InitializeRpc` directly, it needs to be externed in `ViewModelShared.h` of course.
  - Therefore `CreateDispatcherFactory` will be no longer needed. In where the original `RemotingDispatcherFactory` is needed, `RpcJsonDispatcherClientForTaskQueue` could be created directly instead.

3) Inline `RemotingRequesterSession::Impl`
- `RemotingRequesterSession::Impl` is no longer needed.
- All members could just be moved in `RemotingRequesterSession` directly.
- Short methods of `RemotingRequesterSession::Impl` could just be inlined.

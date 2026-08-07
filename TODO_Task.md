Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

1) Release `Workflow` to `GacUI` as `RpcJsonDispatcherClient::SetRpcObjects` was made public.

2) Delete `RemoteViewModelJsonDispatcherClient`.
- `InitializeRpc` would become a static function, receiving `rpc_controller::channeling::RpcJsonDispatcherClient` and `clientId`.
- A file `RemoteViewMOdeLTestInitialize.(h|cpp)` will be created in `Test\GacUISrc\Generated_RemoteViewModelTest` and added to `Generated_RemoteViewModelTest`.
  - Note that `RemoteViewModelTestRpc.h` could be either x64 or x86 version, it will be resolved correctly because test apps already set import folder per platform.
- `CreateViewModelReadyMessage` and `IsViewModelReadyMessage` could just be moved to `ViewModelShared.h` as inline functions.
- Delete `ViewModelShared.cpp`, note that `RemoteViewModelJsonDispatcherClient` is also no longer needed.

3) Remove actual dependency from `Test/RemotingHelpers/Rvmt` to `Generated_RemoteViewModelTest`.
- The goal in this step is to let each test app calls `InitializeRpc` directly.
- `RemotingTest_RvmHost`:
  - `channelClient->Connect();` is called in `Main.cpp`.
  - `Connect()` calls `InitializeRpc`, but now it should not.
  - `InitializeRpc` will be called right after calling `Connect()`.
- `CppTest_Rvm` and `RemotingTest_Core`:
  - Currently `RpcServerHelpers::RequestService` calls `RpcServiceAccessLocalClient::Connect` calls `InitializeRpc`. The issue is that, although it is not the case in test apps, but `RpcServerHelpers::RequestService` should be able to call multiple times, as there is no limitation that `RemotingTest_RvmHost` could only expose one service (although in fact it is). As a demo app, it should show how to architect them correctly. So we should assume the design should allows `RequestService` to be called multiple times for multiple services, even for the same type name. The underlying architecture already make sure that each type name only has one instance so `RpcServerHelpers` doesn't need to worry about this.
  - Both `GuiMain.cpp` calls `RequestService`.
  - Add `RemoteViewModelChannelServer::Connect` to call `RpcServerHelpers::Connect` to call `requesterClient->Connect` which should no longer calls `InitializeRpc` inside it.
    - Pay attention to `RpcServerHelpers::Connect`, although `InitializeRpc` from `requesterClient->Connect` is currently called in the middle, but many code after it looks like need to be also part of `Connect`. Carefully figure out which part of the code should only be called once and put them in `RpcServerHelpers::Connect`.
  - Now both `GuiMain.cpp` should calls:
    - `Connect`
    - `InitializeRpc`
    - `RequestService`

4) Now every files in `Test/RemotingHelpers/Rvmt` is clean:
- They no longer depends on `Generated_RemoteViewModelTest` syntatically and semantically, which means they are independent of any actual Workflow RPC generated code.
- `Release/CodegenConfig.xml` should include `Test/RemotingHelpers` now, they are generating `Test.RemotingHelpers(.Windows)?.(h|cpp)`:
  - Fix `../Tools/Tools/Build.ps1 UpdateRelease`, after these files are copied to the `Release` folder, delete them immediately. Test files are not included in release.
    - To verify it, run `Build.ps1 UpdateRelease`, and see if they are deleted. But you should revert all changes made to `Release`, do not actually update the `Release` repo.
  - Fix `wGac/Import.sh` and `iGac/Import.sh`, unfortunately you are not able to test them, but the change should be very simple:
    - Delete the whole `Import-Test` folder if exists, and recreate one.
    - After copying all code, `Test.RemotingHelpers(.Windows)?.(h|cpp)` should be moved from `Import` to `Import-Test`.

Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

- Terminating `RemotingTest_RvmHost` process manually, should crash `CppTest_Rvm`, and create a fatal error in `RemotingTest_Core`, no recovery needed.
  - The current behavior is the UI hangs when `RemotingTest_RvmHost` is forced terminated.
  - This should generates a disconnection signal in `CppTest_Rvm` or `RemotingTest_Core`, where is a good place to trigger.
  - A task to throw an exception could be injected using `InvokeInMainThread`, to make it thrown in UI thread, to make fatal error generating much easier. In `CppTest_Rvm` the exception doesn't need to catch, it just crashes the app as expected.
- Remove the dependency to `TaskQueue`:
  - Do not use `RpcJsonDispatcherClientForTaskQueue`, use `RpcJsonDispatcherClient` instead, as `RemoteViewModelJsonDispatcherClient`'s base class.
  - For `CppTest_Rvm` and `RemotingTest_Core`:
    - DO not use `TaskQueue`, delete `TaskQueueThread`.
    - The `ScheduleTask` function will be override in `GuiMain.cpp` files, twice.
    - `GetCurrentController()->AsyncService()->InvokeInMainThread` could be modeled as a task queue, just inherit from `RpcJsonDispatcherClient` and use this function instead, therefore no `TaskQueue` will be needed.
  - For `RemotingTest_RvmHost`:
    - The `ScheduleTask` function will be override in `Main.cpp` to use `TaskQueue`.
    - Unfortunately the `RpcJsonDispatcherClientForTaskQueue` class cannot be used here.
  - Since new sub classes of `RemoteViewModelJsonDispatcherClient` is created in each test app, they need to be passed to `RemoteViewModelChannelServer` and `ViewModelHostClient`, instead of being created in these two classes internally.
- No `ViewModelHostClient::Impl` is needed, just merge the class into `ViewModelHostClient` directly.
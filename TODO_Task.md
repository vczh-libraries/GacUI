Perform the following refactor on the design in `Test/RemotingHelpers` and test apps.

1) Interface re-organization

I notice that `CreateCallbacks` functions in test apps (meaning `CppTest_Rvm` and `RemotingTest_*` in this task) is creating incorrect patterns of the code.
They are just wired code simulating interface and inheritance.
`RemotingChannelServerCallbacks` should actually be a list of virtual function in `RemotingChannelServer`, and test apps calling `CreateCallbacks` should instead inherit from this class to implement these functions.

When implementing these virtual functions, do not introduce too many unnecessary layers of redirection like the current only.
Currently the server calls `callbacks`, which redirects to `RemoteViewModelRequesterSession` by `CreateCallbacks`, which redirects to `RemotingRequesterSession`, but the only differences between `RemotingTest_Core` and `CppTest_Rvm` is just about wanting to accept renderer clients or not. I would say just:
- Just inline everything to `RemotingChannelServer`, removing unnecessary virtual functions during translating `RemotingChannelServerCallbacks`
- `acceptRenderer` as a constructor argument is already enough.

Example of reason to not make certain virtual functions:
- `canAcceptLocalClient` should be removed. As `ConnectLocalClient` is called by the process creating the server, if a local client cannot be accepted, just don't call the `ConnectLocalClient` instead of creating a barrier. This is unnecessary dynamic check that can be done by static analysis (e.g. type checking, or review the code more carefully).
- `canAdmitRenderer` is just monitoring a state. It is acceptable to have a dedicated function, but since this is not supposed to be a behavior which can be modified, making it virtual does not make sense.
It might ends up no or just a few virtual fucntions survive, which is acceptable.

After doing this, `Start*Server` in `CppTest_Rvm` and `RemotingTest_Core` no longer need to pass constructor arguments `StartServer<>`, as there will be no objedts to create before starting the server. The only exception might be the `parser`, it is acceptable to create them in each `Start*Server` as this is just single line, and pass `parser` via argument to `StartServer<>`.

2) MISC

`RemotingCoreChannel` should rename. The original class `GuiRemoteProtocolCoreChannel` actually says, this is a channel for `GuiRemoteProtocol`, and it is for the "core" side. Naming its sub class `RemotingCoreChannel` is just making people confusing. Since the only purpose of making this class is to add a `getTransportRendererClientId` guard:
- Turn `getTransportRendererClientId` to be a virtual function in the base class `IsCorrectRendererClientId`, returning true by default.
- `RemotingTest_Core` is going to inherit this class and name it after `SwitchableRenderersCoreChannel`.
- Delete `RemotingCoreChannel`.

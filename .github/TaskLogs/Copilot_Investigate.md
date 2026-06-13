# Investigate Repro

## Task 4: Release VlppOS and Workflow to GacUI

### Changes

- Released updated VlppOS artifacts into `Import`, including the named-pipe shutdown and channel-client cleanup fixes.
- Released updated Workflow artifacts into `Import` for the refactored RPC/channel server surface.
- Updated GacUI remote protocol channel aliases so `NetworkProtocolChannelServer` is parameterized by a concrete protocol server base.
- Refactored `RemotingTest_Core` channel servers so HTTP and named-pipe transports share the same channel-server base through the new template form.
- Updated local-client handling to use the `OnClientConnected(..., localClient)` argument, replacing the previous explicit core-client connection tracking.
- Updated the renderer test client shutdown path so process exit marks the channel disconnected before deciding whether to stop the network connection.

### Validation

- GacUI Debug|x64 build: passed with 0 errors.
- GacUI Debug|x64 UnitTest: passed, 84/84 files and 1686/1686 cases.
- Remote protocol test procedure:
  - HTTP: `RemotingTest_Core.exe /Http /RPT` with browser flow `File -> self.Close() (InvokeInMainThread) -> OK` rendered `IGacUIRenderer exited due to receiving RequestControllerConnectionStopped.`
  - Named pipe: native remote protocol flow exited both core and renderer after `File -> self.Close() (InvokeInMainThread) -> OK`, without showing the previous `ERROR from GacUI Core` named-pipe dialog.
- GacJS:
  - `yarn build`: passed.
  - `yarn test`: passed.
  - Browser verification against `http://localhost:8896/index.html`: passed with the expected rendered shutdown message.

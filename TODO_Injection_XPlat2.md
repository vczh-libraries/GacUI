This is a continuation of previous works:
- TODO_Injection3.md

When you are on Linux, you are working on `wGac` repo.
When you are on macOS, you are working on `iGac` repo, and you need to skip all WASM related content.
According to previous works, make sure new features work on `wGac` or `iGac`:
- Make sure `./import.sh` and `./syncProj.sh` properly updates source files.
- Start `RemotingTest_Core` with some or all `/remoteDT`, `/remoteLocale`, `/remoteFS`, `/corehost`, make sure `GacJS` renderer offered implementation is working.

When you are on Linux, make WASM works:
- WASM version of GacUI core will by default reject remote implementation (including date time, locale and file system), this could be done by correctly configuring remote protocol on each app's main function, instead of hardcoding in GacUI itself.
- Compile WASM apps and verify them with GacJS's all WASM pages. Since WASM and the renderer is running in the same browser environment, this makes remote implementation completely not needed, therefore in WASM test pages no need to test if renderer enables or disables remote implementation, but you need to update the code to report this via `ControllerConnected` arguments.

commit and push all local changes once finishing.

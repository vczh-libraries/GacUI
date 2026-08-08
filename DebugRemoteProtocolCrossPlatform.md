# Ensure GacUI Working Properly in Linux/macOS

- If you are on Linux, the repo you are working on are `GacUI`, `GacJS` and `wGac`.
- If you are on macOS, the repo you are working on are `GacUI`, `GacJS` and `iGac`.
- `wGac` and `iGac` has very similar structure:
  - When a `%ABC` folder is mentioned below, it means `wGac/WGacABC` or `iGac/MacABC`.
  - WWhen a repo name is not mentioned, it means `wGac` or `iGac`.

## Goal

- The current document is the source of truth, when unalignment happens, align to this document.
- Maintain documents and scripts in `wGac` and `iGac`.
- Makesure test apps build and work as expected.
- `DebugRemoteProtocolWithRenderer.md` and `DebugRemoteProtocolWithGacJS.md` documents details of verification.

## Helper Bash Scripts

- `import.sh` copy released source file from `GacUI`, but `GacUI` might not be released yet, always run `CodePack` first if you are not sure.
  - `GacUI/Source` is `CodePack`-ed to `GacUI/Release` and then copied to `Import`.
  - `GacUI/Test/RemotingHelpers` is `CodePack`-ed to `GacUI/Release` and then copied to `Import-Test`.
- `syncProj.sh` is supposed to maintain the structure described in `## Ensure Project Availability`.
  - Unalignment usually happens here because `GacUI` is changing progressively, `wGac` and `iGac` will then catch up after `GacUI` finishes some features.
  - `GacUI`, `wGac` and `iGac` might needs some manual fix, especially `syncProj.sh`.
- `build.sh` builds the whole repo.
- `test.sh` runs each app.

## Ensure Project Availability

In `GacUI` there are GacUI XML Resources to build C++ code:
- `FullControlTest`
- `RemoteProtocolTest`
- `RemoteViewModelTest`
XML resources and assets will be copied from `GacUI/Resources/App/<APP-NAME>` to `Apps/<APP-NAME>/Resources`, but C++ files are generated from them locally.

In `GacUI` there are important test apps:
- `CppTest`, a test app with with `FullControlTest` running, no network protocol is involved.
  - `test.sh --app:fct (--hosted)?`
  - This project will be ported to `%FullControlTest`.
- `CppTest_Rvm`, a test app with actual UI, connecting to a view model implementation hosted in `RemotingTest_RvmHost`.
  - `test.sh --app:rvmt`
  - This project will be ported to `%CppTestRvm`.
- `RemotingTest_Renderer_Win32`, a test app as a native renderer in remote protocol, it connects to `RemotingTest_Core`.
  - `test.sh --app:renderer`
  - This project will be ported to `wGac/RemotingTest_Renderer_Wayland` or `iGac/RemotingTest_Renderer_macOS`.
- `RemotingTest_Core`, a test ap as the core app in remote protocol.
  - It runs from `GacUI/Test/Linux/RemotingTest_Core`.
- `RemotingTest_RvmHost`, a CLI app hosts the implementation of `IViewModel` with `RemoteViewModelTest` is used.
  - Requres `CppTest_Rvm` or `RemotingTest_Core /RVMT`.
  - It runs from `GacUI/Test/Linux/RemotingTest_RvmHost`.

Only `/MiniHttp` is available as a network protocol.
Additionally, `test.sh --app:simple` runs a hello world GacUI app, it could be used as a smoke test hosting native renderers.

## Maintaining Test App Mirrors

Some test apps are just copied from `GacUI` with just a few local files.
It means core files for test apps are shared between `GacUI`, `wGac` and `iGac`.
To edit them, make the change in `GacUI` and run `import.sh` or `syncProj.sh` to update local files.
It includes files from:
- `GacUI/Test/RemotingHelpers`, which is `CodePack`-ed and `import.sh`-ed to `Import-Test`.
  - It is also part of test apps, but it has reusable code to build future test apps.
  - DO NOT bring any actual test app specific knowledge in these files. The only exception will be URL names using in network protocol.
- `GuiMain.cpp` from ported test apps.
  - Note that `GuiMain.cpp` in `GacUI` is using pre-`CodePack`-ed files, and in here is using `CodePack`-ed files.
  - To handle include file path differences, use the following macros:
    - `#if defined VCZH_MSVC` for code working in `GacUI`.
    - `#if defined VCZH_GCC` for code working in both `wGac` and `iGac`.
    - `#if defined VCZH_GCC && !defined VCZH_APPLE` for code working in only `wGac`.
    - `#if defined VCZH_GCC && defined VCZH_APPLE` for code working in only `iGac`.

`%Shared` is a project used by all test apps, it references `Import`, `Import-Test` with additional sharable local source files (mostly in `%`).

## Verification

When executing this instruction files, it usually means `GacUI` itself alone on Windows has been properly verified.
Try to limit changes in `GacUI` to only include making release or fixing cross-platform building issues:
- Unless there are fundemental issues to fix.
- You need to figure out why the code is just work in `GacUI` on Windows. Having correct answers to this question help you figure out root cause efficiently and precisely.

Follow `DebugRemoteProtocolWithRenderer.md` and `DebugRemoteProtocolWithGacJS.md` to make sure all test apps are behaving expectedly.
Sometimes it might only need native renderer or GacJS to be verified, this will be explicitly instructed additionally. Otherwise both should be tested.
Make sure to commit and push local changes across all affected repos.

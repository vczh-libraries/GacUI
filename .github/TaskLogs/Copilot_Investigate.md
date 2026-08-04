# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

A small refactor:
- Move `Source/RemotingHelpers` to `Test/RemotingHelpers`
- Fix `Release/CodegenConfig.xml` about path references, and codepack-ed code becomes `RemotingHelpers*` instead of the original `GacUI.RemotingHelpers*`.
- Scan markdown files, fix path references.
- Build test apps, try `RemotingTest_RvmHost` in a very light-weight manual tests, since there are only file renames so we can assume nothing would be break, if this small demo works then we assume everything is fine.
- commit and push.

# UPDATES

# TEST [CONFIRMED]

- Confirm all tracked references that formerly targeted `Source/RemotingHelpers` target `Test/RemotingHelpers` after the move, including project metadata, Linux build metadata, release CodePack configuration, C++ includes, and Markdown documentation.
- Build the GacUISrc test solution in the default Debug|x64 configuration with the repository build wrapper. Success requires zero build errors.
- Run a lightweight RVM smoke test by starting `CppTest_Rvm` and `RemotingTest_RvmHost` with the same transport. Success requires the requester and host to connect without a crash; after observing the connection, terminate the demo processes.
- Inspect `Release/CodegenConfig.xml` to confirm the remoting helper code pairs are named `RemotingHelpers` and `RemotingHelpers.Windows` and source their files from `Test/RemotingHelpers`.

The reference scan passed outside this document's verbatim problem statement. CodePack completed successfully and generated the renamed ordinary and IncludeOnly pairs. The Debug|x64 GacUISrc solution build succeeded with zero warnings and zero errors. In the `/Http` smoke test, `RemotingTest_RvmHost` printed that it declared `rvmt::IViewModel`, and `CppTest_Rvm` returned HTTP 200 from its automation endpoint with the `Remote View Model Test` control tree and the RPC-backed `Hello, !` label. No Microsoft Visual C++ Runtime Library dialog appeared. The requester accepted `!Exit` with `Queued`; the host then exited with its expected requester-disconnection result.

# PROPOSALS

- No.1 Relocate and retarget the test-only remoting helper layer [CONFIRMED]

## No.1 Relocate and retarget the test-only remoting helper layer

Move the helper source tree without changing C++ contents, retarget all build and include paths, update release CodePack categories and output pair names, and update Markdown references so the helper layer is consistently described as test-only under `Test`.

### CODE CHANGE

- Move `Source/RemotingHelpers` to `Test/RemotingHelpers`.
- Update project, Linux build, C++ include, release CodePack, and Markdown references.
- Keep the shared-items project name unchanged because it describes the shared inventory rather than the physical source location.

### CONFIRMED

All helper source files are now under `Test/RemotingHelpers`, and every build consumer resolves them from that location. Relative includes inside the moved headers continue to resolve production GacUI dependencies from `Source`. `Release/CodegenConfig.xml` includes the test helper folder, categorizes the neutral and Windows helpers from their new paths, and emits `RemotingHelpers*`; a successful CodePack run regenerated both ordinary and IncludeOnly outputs under those names. The full test solution build and the paired RVM requester/host smoke test confirm the refactor preserves the demo workflow.

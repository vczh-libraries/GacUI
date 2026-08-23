# Ensure GacUI Working Properly on Windows

## Goal

- Makesure test apps build and work as expected.
- `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md` have details of verification.

## Verification

Try to limit changes in `GacUI` only, unless there are fundemental issues to fix in upstream repos.
Follow `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md` to make sure all test apps are behaving expectedly.
Without explicitly instructed, the complete matrix of all combinations of test apps, test UI loaded with remote protocol, and available network protocol choices, should be verified.
Make sure to commit and push local changes across all affected repos.

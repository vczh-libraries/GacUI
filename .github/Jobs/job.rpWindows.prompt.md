# Ensure GacUI Working Properly on Windows

## Goal

- Makesure test apps build and work as expected.
- `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md` have details of verification.
- You must fix any issue you see during verification.

## Maintain a Test Matrix Card

You need to maintain test card in:
- `<repo-root>/.github/TaskLogs/TestMatrix_NativeRenderer.md` for `DebugRemoteProtocolWithNativeRenderer.md`
- `<repo-root>/.github/TaskLogs/TestMatrix_GacJS.md` for `DebugRemoteProtocolWithGacJS.md`

If these files already exist, delete them, and recreate them following the `### Test Matrix Card` section in each document.
During verification, test matrix cards should be updated in time:
- When a test item starts, succeeds, fails, fixed, the file need to be updated immediately and saved, but no need to commit immediately, a commit is only required after all test items are done.
- I will read the file to check the progress of your verification.
Without explicitly instructed, you are required to complete every available test item in the matrix:
- If some test items are excluded explicitly, you still need to keep complete test items in the test matrix card, but leave blanks for results of excluded items.

Sometimes the test item fails because of lacking mature steps, instead of defect in the code.
In this case we don't need to mark it with `(fixed)`, instead we can write the experience down in:
- `<GacUI-repo>/.github/Learning/Learning_RpJob.md`
Therefore reading this file is also important to avoid repeating mistakes and to improve the testing process.
In this files there are 4 sections:
- `## Shared`
- `## Windows`
- `## Linux (wGac repo specific)`
- `## macOS (iGac repo specific)`
If an issue is not tight to a specific platform, put the experience in the `## Shared` section.

## Verification

Try to limit changes in `GacUI` only, unless there are fundemental issues to fix in upstream repos.
Follow `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md` to make sure all test apps are behaving expectedly.
Without explicitly instructed, the complete matrix of all combinations of test apps, test UI loaded with remote protocol, and available network protocol choices, should be verified.

## Finishing

Make sure to commit and push local changes across all affected repos.
Conflict should be resolved by rebasing.
`job.rpWindows.prompt.md` and `job.rpXPlat.prompt.md` could be running in different computers at the same time,
so when you find the issue you just fixed, which is also resolved in remote commits,
you need to undo your own, and verify the fix in remote commits again.

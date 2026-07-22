# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

You are going to investigate a bug which could repro by `RemotingTest_Renderer_Win32`:
  - With `/FCT` and any network protocol, first clicking into `Controls` tab, followed by `Document Editor (Ribbon)` or `Document Editor (Toolstrip)` tabs, you will find some icons are missing.
    - The layout is wrong as those icon does not exist or has size 0x0 which probably means the renderer either received but did not process those images or did not receive at all.
    - Selecting text in document box would change toolbar button enability causing it to refresh, affected buttons become normal, unaffected buttons remain. I didn't confirm if this observation is strict or not.
    - `GacJS` is just fine, which probably means the issue is not in the core side.
- Unit Test snapshots also look good.
The interesting part is that, this bug cannot repro in a slower PC.
find the root cause and fix the bug.

# UPDATES

## UPDATE

By saying slower I mean the machine not the running performance as Release configuration in this machine also repros. Maybe speed is not the key factor, but just want you to know it doesn't repro everywhere, which concerns me.

# TEST [CONFIRMED]

Add a focused remote graphics-host regression that creates a non-stretched `GuiImageFrameElement` before the first full rendering frame. Its fake image payload declares a `16x24` frame, and the mock renderer returns that image metadata from `ElementMeasurings::createdImages` without returning any font-height measurements. After the first frame completes, assert that the core-side image renderer has adopted the `16x24` minimum size. This isolates the response shape that is environment-dependent in the full application: image metadata arriving in a frame that has no `fontHeights` payload.

Before the fix, the focused test must fail with a retained `0x0` minimum size even though the protocol returned valid image metadata. After the fix, it must pass without changing or re-enabling the image element. Build the complete Debug x64 solution and run the full `UnitTest` project through the repository wrappers; success requires all test files/cases to pass and no memory-leak dump.

Also reproduce the reported FullControlTest sequence with `RemotingTest_Core /FCT` and `RemotingTest_Rendering_Win32` over a network protocol. Inspect renderer DOM/image-frame state after Controls > Document Editor (Ribbon) and Controls > Document Editor (Toolstrip), and confirm affected icon bounds are positive after the fix without relying on a selection/enabled-state refresh. Check both Debug and Release behavior where practical so the result does not depend on execution speed, and treat the other PC only as an environment contrast rather than a timing proof.

The focused regression was confirmed on 2026-07-21 against the unfixed Debug x64 implementation. The complete solution built through `copilotBuild.ps1` with 0 warnings and 0 errors. The full `UnitTest` run passed every earlier file it reached and then stopped in `TestRemote_GraphicsHost_Startup.cpp` at `Update image min size when only image metadata is returned`: after frame 0 returned a valid `16x24` `createdImages` entry without `fontHeights`, frame 1 observed `GuiImageFrameElementRenderer::GetMinSize() == Size(0, 0)` instead of `Size(16, 24)`.

Source history confirms the response-shape distinction. The `renderersAskingForCache` refresh loop was originally introduced for font-height responses in February 2024. Image metadata handling was added later, but only updates `GuiRemoteGraphicsImage`; it does not run the waiting-renderer refresh loop. Therefore image renderers are refreshed accidentally only when the same frame also contains a `fontHeights` list. The confirmed test supplies no label or font measurement, so it deterministically exposes the missing image-cache handoff that ordinary snapshots can mask.

# PROPOSALS

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

- No.1 [CONFIRMED] REFRESH WAITING RENDERERS AFTER ALL CACHE RESPONSES

## No.1 [CONFIRMED] REFRESH WAITING RENDERERS AFTER ALL CACHE RESPONSES

In `GuiRemoteGraphicsRenderTarget::StopRenderingOnNativeWindow`, keep response ingestion ordered by cache dependency: first add all returned font heights to `fontHeights`, then apply all returned image metadata to `GuiRemoteGraphicsImage`, and only then iterate `renderersAskingForCache`. Move the existing generic waiting-renderer loop out of the `if (measuring.fontHeights)` block and place it after both response-cache blocks.

Each waiting renderer already owns the correct type-specific lookup through `TryFetchMinSizeFromCache()`: solid-label renderers query `fontHeights`, while image-frame renderers query their `GuiRemoteGraphicsImage` metadata. Running the loop after a response containing either cache makes each renderer consume whichever cache became available in that frame, without coupling image progress to an unrelated font response. Renderers whose dependencies are still unavailable remain in `renderersAskingForCache`; renderers that resolve are removed, and a changed minimum size on an element rendered in the last batch requests the existing hosted-layout refresh.

This changes no protocol schema, renderer-side image creation, Direct2D behavior, image ownership, or document-editor templates. It fixes the owning state transition: response caches become visible first, then all consumers waiting on those caches are retried. The focused `16x24` image-only regression must pass without an element property change, and the existing label-measurement paths must remain covered by the full unit-test suite.

### CODE CHANGE

- Move the `renderersAskingForCache` retry/removal loop in `Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp` from inside the `measuring.fontHeights` branch to immediately after `measuring.createdImages` is processed. Run it when either `fontHeights` or `createdImages` is present, so ordinary frames without new cache data do not retry outstanding measurements. Preserve the existing minimum-size comparison and `minSizeChanged` refresh behavior.
- Keep the focused first-full-frame image regression in `Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Startup.cpp`; it requires a `16x24` image minimum size to become available when `createdImages` is the only cache response.

### CONFIRMED

The root cause is the cache-consumer handoff in `GuiRemoteGraphicsRenderTarget::StopRenderingOnNativeWindow`. The response first updated `GuiRemoteGraphicsImage`, but the generic `renderersAskingForCache` retry loop was guarded by `measuring.fontHeights`. An image-only response therefore left `GuiImageFrameElementRenderer` at `0x0` until an unrelated property change caused another element update. Whether a full application frame also carried font measurements depended on the active renderer/cache mix, which explains why machine-to-machine behavior differed without requiring machine speed to be causal. GacJS and existing snapshots could avoid the image-only response shape and therefore did not disprove the core-side omission.

The focused regression failed before the fix with `Size(0, 0)` after valid `16x24` image metadata, and passes after the fix without changing the image element. The final Debug x64 solution build completed with 0 warnings and 0 errors. The complete final `UnitTest` run passed 85/85 files and 1698/1698 cases with no `.memoryleaks` or `.unfinished` marker. Relevant Ribbon, Toolstrip, and single-image snapshots now converge one frame earlier because image minimum sizes are consumed in the frame that returns their metadata; unrelated file-dialog snapshot drift was discarded.

The reported `/Http /FCT` navigation was also exercised against the live Win32 renderer. In Debug, Document Editor (Ribbon) exposed 26 active image frames and Document Editor (Toolstrip) exposed 16; every image had positive bounds and positive valid area immediately after navigation. In Release, the same normal wrapper launch produced the same 26 and 16 active images with zero invalid bounds or valid areas. The complete Release x64 solution also built with 0 warnings and 0 errors. This confirms the fix in both configurations on the machine that reproduces the original bug and does not rely on a text-selection/enabled-state refresh.

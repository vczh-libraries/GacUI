# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

You are going to investigate a bug that could repro by `RemotingTest_Renderer_Win32`.
- Toolbar vertical splitter is missing.
  - Repros with `GacJS`.
  - Cannot repro without remote protocol.
  - Cannot repro in unit test.

Find the root cause and fix it. 

# UPDATES

# TEST [CONFIRMED]

Reproduce the visual defect at the remote-renderer executable boundary using the toolbar vertical splitter in FullControlTest, because both the native remote renderer and GacJS exhibit the issue while the local renderer and the unit-test remote renderer do not. Capture enough renderer state or pixels to prove that the splitter is absent before the fix and present after the fix, while adjacent toolbar content remains laid out correctly.

Build `Test/GacUISrc/GacUISrc.sln` and run the existing `UnitTest` project using the repository wrappers. The full unit-test suite must pass after the source change even though its renderer does not reproduce the defect. Exercise `RemotingTest_Core` with `RemotingTest_Rendering_Win32` after the fix and confirm the toolbar splitter is rendered. Where practical, repeat the same remote-protocol path with GacJS or inspect the protocol data responsible for the shared failure. Compare with the non-remote application to verify that the fix preserves its correctly rendered splitter.

Success requires an evidence-backed root cause in the remote rendering path, a before/after confirmation in `RemotingTest_Rendering_Win32`, a successful solution build with no new warnings or errors, and all unit tests passing without failures or memory leaks.

The baseline was confirmed on 2026-07-21 with a fresh Debug x64 build. The mandated `copilotBuild.ps1` wrapper completed with 0 warnings and 0 errors. A `RemotingTest_Core /MiniHttp /FCT` and `RemotingTest_Rendering_Win32 /MiniHttp` pair was navigated through the core automation service to Control > Document Editor (Toolstrip). The renderer automation catalog contained ten `SinkSplitter` elements and the active DOM contained four vertical splitters, proving that element creation and property serialization succeeded. All four active nodes had zero-width rendering bounds and zero-width valid areas: element 2768 used `[92,141,92,163]`, 2776 used `[146,141,146,163]`, 2787 used `[224,141,224,163]`, and 2792 used `[254,141,254,163]`. `GuiRemoteRendererSingle::RenderDom` skips every child whose valid area has a non-positive width or height, so none of these splitters reaches the native Direct2D renderer. GacJS likewise applies the zero-width DOM bounds to an `overflow:hidden` element, explaining the identical browser failure.

The non-remote composition path renders an owned element before pushing the composition itself as a clipper, so `Gui3DSplitterElementRenderer` is called even for the intentional zero-width vertical splitter bounds. Its native renderer draws two lines centered around that coordinate. This difference confirms why local rendering works and why the unit-test protocol recorder, which records rendering commands without executing the real remote DOM renderer, does not reproduce the missing pixels.

# PROPOSALS

- No.1 [CONFIRMED] TRANSMIT THE SPLITTER'S ACTUAL TWO-PIXEL FOOTPRINT

## No.1 [CONFIRMED] TRANSMIT THE SPLITTER'S ACTUAL TWO-PIXEL FOOTPRINT

Override rendering only for the remote-protocol `Gui3DSplitterElementRenderer`. Native `Gui3DSplitterElementRenderer` implementations draw two one-pixel lines centered on the supplied bounds. When a vertical splitter receives a width below two pixels, calculate the same first-line coordinate used by the native renderer and transmit a two-pixel-wide rectangle beginning at that coordinate. Apply the symmetric transformation to a horizontal splitter whose height is below two pixels. Delegate all other behavior to the existing generic remote element renderer.

This makes the remote DOM bounds describe the splitter's real graphical footprint instead of the zero-width or zero-height layout anchor. The DOM builder then produces a positive valid area, `GuiRemoteRendererSingle` no longer discards the node, and GacJS no longer clips the two-pixel child inside a zero-sized `overflow:hidden` element. Because only the transmitted rendering rectangle changes, control-template measurement, toolbar spacing, hit testing, local rendering, element descriptions, and the protocol schema remain unchanged. Splitters already receiving at least two pixels in the perpendicular direction retain their exact bounds.

Add focused coverage to the existing remote graphics-host unit tests. Create a vertical `Gui3DSplitterElement` with zero-width composition bounds and assert that the protocol still records its element description but transmits two-pixel rendering bounds at the same line coordinates as native rendering. This tests the reported root cause at the layer available to the unit-test protocol recorder even though that recorder cannot reproduce the final missing pixels visually. The production normalization handles horizontal splitters symmetrically.

### CODE CHANGE

- Override `Gui3DSplitterElementRenderer::Render` in `Source/PlatformProviders/Remote/GuiRemoteGraphics_BasicElements.h` and `.cpp`. Expand only a sub-two-pixel perpendicular dimension to the two-pixel footprint that preserves the native renderer's original line coordinates, then call the generic remote-protocol rendering implementation.
- Extend `Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_HitTest.cpp` with a vertical zero-width splitter case whose expected rendering log contains `{99,100:2,20}` while its cached layout bounds remain `{100,100:0,20}`.
- Regenerate the affected Ribbon and Toolstrip remote-protocol snapshots so their active splitter bounds and valid areas record the new two-pixel footprint. Exclude unrelated nondeterministic file-dialog snapshot rewrites.

### CONFIRMED

The proposal was implemented and confirmed on 2026-07-21. `Gui3DSplitterElementRenderer::Render` now changes only the transmitted perpendicular dimension when it is below two pixels: vertical splitters receive a two-pixel width centered on the original anchor and horizontal splitters receive the symmetric two-pixel height. Layout bounds, hit testing, element descriptors, and the protocol schema are unchanged.

The focused regression proves the distinction that caused the bug. Its `GuiBoundsComposition` remains zero-width at `{100,100:0,20}`, while the remote rendering protocol records `Render(1, {99,100:2,20}, {0,0:640,480})`. The Debug x64 solution built through `copilotBuild.ps1` with 0 warnings and 0 errors. The full `UnitTest` run completed with `Passed test files: 85/85` and `Passed test cases: 1696/1696`; the wrapper appended no memory-leak dump.

A fresh `RemotingTest_Core /MiniHttp /FCT` and `RemotingTest_Rendering_Win32 /MiniHttp` pair was navigated to Control > Document Editor (Toolstrip) after the fix. The renderer catalog again contained ten `SinkSplitter` elements and the active DOM contained the same four toolbar splitters. Their bounds and valid areas were now positive: element 2768 used `[91,141,93,163]`, 2776 used `[145,141,147,163]`, 2787 used `[223,141,225,163]`, and 2792 used `[253,141,255,163]`. Every active splitter was `2x22`, so `GuiRemoteRendererSingle::RenderDom` no longer skips it. These same positive outer DOM bounds also prevent GacJS's `overflow:hidden` splitter container from clipping the two-pixel child. The exact two-pixel expansion, with all surrounding layout coordinates preserved, confirms the proposal fixes the shared remote-protocol defect rather than a native-renderer-only symptom.

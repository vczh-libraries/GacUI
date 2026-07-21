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

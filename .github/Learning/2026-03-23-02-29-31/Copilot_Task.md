# !!!TASK!!!

# PROBLEM DESCRIPTION
I would like to add a new message in remote protocol to tell that all layout works has been done and no future rendering request will be sent unless new changes happens. This is only a hint, in most of the case nothing needs to be done upon receiving this message. But it works for GacJS unit tests as playwright waiting on a fixed time is not a good strategy, because changing to other computer might break test cases.

I would name it `RendererIdle`.

My proposal is that:
- In `IGuiGraphicsRenderTarget` add a new `HostedRenderingIdle` method, `GuiGraphicsRenderTarget` should have an empty implementation for it.
- `GuiRemoteGraphicsRenderTarget` can use it to send the message.
- In `GuiHostedController::GlobalTimer` I believe the second `return;` in line 692 is a good place to trigger it. But you need to do it in this way:
  - I hope this function not being kept calling. Only once is allowed between two actual rendering.

After adding a new message to the remote protocol, some places will break because the handling is missing. You should keep all handling empty unless in `UnitTestRemoteProtocolFeatures`. `ProcessRemoteEvents` and `LogRenderingResult` combined detect if a frame is making no change / being blocked and when to execute the next frame. A frame will be executed only when the layout triggered by the last frame finished. Multiple rounds of rendering requests might happen during it. So it is a good place to make sure, if I have received exactly one `RendererIdle`? This check should apply before executing every frame from the second one. The first one is tricky, I would like you to make sure, instead of exactly one, I would like at most one `RendererIdle` is issued.

You need to change UnitTest.vcxproj.user to ensure every test files will be executed.
You don't need to change any existing test cases, as `UnitTestRemoteProtocolFeatures` is going to be called in every GacUI test cases. You are going to call CHECK_ERROR instead of TEST_ASSERT just like how ProcessRemoteEvents does.

# UPDATES

# INSIGHTS AND REASONING

## Context / Evidence

- Hosted rendering is driven by `vl::presentation::GuiHostedController::GlobalTimer()`.
  - It returns early when nothing needs refreshing and the previous frame didn’t update any window (`if (!wmManager->needRefresh && !windowsUpdatedInLastFrame) { return; }` in `Source\PlatformProviders\Hosted\GuiHostedController.cpp`). This is the proposed trigger point.
  - Actual rendering happens inside the `StartHostedRendering()` / `StopHostedRendering()` loop.
- Remote protocol message definitions are code-generated from `Source\PlatformProviders\Remote\Protocol\Protocol_*.txt` into `...\Protocol\Generated\GuiRemoteProtocolSchema.h` (see `KB_GacUI_Design_RemoteProtocolCoreArchitecture.md`).
- The unit test framework executes per-frame callbacks only after rendering has settled:
  - `UnitTestRemoteProtocolFeatures::LogRenderingResult()` commits a frame only when a previously-seen rendering frame is followed by a cycle where no new rendering occurs.
  - `UnitTestRemoteProtocol::ProcessRemoteEvents()` executes the next `OnNextIdleFrame` callback only when `LogRenderingResult()` returns `true` (see `Source\UnitTestUtilities\GuiUnitTestProtocol.h`).

This means the unit test framework already has a well-defined “renderer is now idle” moment, and it is appropriate to validate `RendererIdle` there.

## Proposed Design

### 1) Add a new remote-protocol message: `RendererIdle`

- Add a **core → renderer** message `RendererIdle` to the renderer protocol definition (expected location: `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer.txt`).
  - Message definition should be the minimal no-payload form: `message RendererIdle {}`.
- **Drop annotation**: keep it **not dropped**.
  - Explicitly: it must **NOT** be annotated with `[@DropRepeat]` or `[@DropConsecutive]`, so tests can observe duplicates if they happen.
- Codegen prerequisite: after editing `Protocol_Renderer.txt`, regenerate protocol headers via the existing codegen flow (project `Metadata_UpdateProtocol`, per `Project.md`) **before** building anything that depends on the generated protocol code.

### 2) Add a render-target hook: `HostedRenderingIdle()`

- Extend `vl::presentation::elements::IGuiGraphicsRenderTarget` with a new **pure-virtual** method:
  - `HostedRenderingIdle()` (name per request; semantics: “hosted mode determined there will be no more rendering requests until the next change”).
- Provide an empty default implementation in `vl::presentation::elements::GuiGraphicsRenderTarget` (so non-remote targets stay no-op by default).

Rationale:
- The “idle” concept is a hosted-mode scheduling decision, but the effect (emitting `RendererIdle`) is protocol-specific, so the hook keeps the controller generic and makes remote/non-remote targets behave correctly with no behavioral change.

### 3) Trigger `HostedRenderingIdle()` from `GuiHostedController::GlobalTimer` (once per idle period)

- At the early-return branch:
  - `if (!wmManager->needRefresh && !windowsUpdatedInLastFrame) { /* trigger */ return; }`
  - Keep the existing `renderTarget->IsInHostedRendering()` early-return taking precedence (do not emit idle while hosted rendering is in progress).
  - Null-check `renderTarget` before calling `HostedRenderingIdle()` to avoid startup/initialization edge crashes.
  - Call `renderTarget->HostedRenderingIdle()` right before returning.
- Add a small piece of state to ensure **the hook is invoked at most once between two actual renderings**:
  - Track a boolean like `idleNotifiedSinceLastRendering`.
  - Reset it when a rendering cycle is about to happen (e.g., when taking the `NEED_REFRESH:` path / entering the rendering loop).
  - Only call `HostedRenderingIdle()` when the early-return branch is hit *and* `idleNotifiedSinceLastRendering == false`, then set it to `true`.

Rationale:
- `GlobalTimer()` is called continuously in remote mode (and often in hosted mode generally), so the early return would otherwise spam `RendererIdle`.
- The flag-based gating satisfies the “only once allowed between two actual rendering” constraint.

### 4) Remote render target emits the protocol hint

- `vl::presentation::elements::GuiRemoteGraphicsRenderTarget` overrides `HostedRenderingIdle()`:
  - Implementation: enqueue `RequestRendererIdle()` through existing `GuiRemoteMessages` batching (no synchronous waiting).
  - No behavior on disconnected state beyond existing safe-guards (i.e., same as other renderer messages: they become no-ops / are naturally ignored when not connected).

### 5) Keep all other receiver-side handlers empty (except unit tests)

Adding a new message expands `GACUI_REMOTEPROTOCOL_MESSAGES(...)` usages across multiple implementers; to keep impact minimal:
- For all protocol implementers other than the unit test protocol, add an **empty** handler for `RequestRendererIdle`.
  - Example targets include the “real” renderer implementations and any JSON/channel adapters that explicitly switch over message enums.
- The only non-empty behavior should be inside `UnitTestRemoteProtocolFeatures`.

### 6) Unit test framework: validate `RendererIdle` sequencing

Goal:
- Starting from the **second executed idle frame callback**, ensure **exactly one** `RendererIdle` was received since the previous callback execution.
- For the **first** callback execution, allow **at most one** `RendererIdle`.

Design:
- In `UnitTestRemoteProtocolFeatures`, add a counter `rendererIdleCount` (use `vint`) and increment it in `Impl_RendererIdle()`.
- In `UnitTestRemoteProtocol::ProcessRemoteEvents()`, after `LogRenderingResult()` returns `true` and before executing the callback (right before calling `func()`):
  - First callback (`nextEventIndex == 0`): `CHECK_ERROR(rendererIdleCount <= 1, ...)`.
  - Later callbacks (`nextEventIndex >= 1`): `CHECK_ERROR(rendererIdleCount == 1, ...)`.
  - Then reset `rendererIdleCount = 0`.
- Ensure `CHECK_ERROR` messages follow the existing `ERROR_MESSAGE_PREFIX` convention so failures are actionable.

Rationale:
- This placement leverages the existing “frame is settled” boundary: `LogRenderingResult()` returning `true` is exactly when the framework is about to run the next `OnNextIdleFrame` callback.
- Using `CHECK_ERROR` matches existing error signaling in `ProcessRemoteEvents()` and `LogRenderingResult()`.

### 7) Update `UnitTest.vcxproj.user` to run all tests

- `Test\GacUISrc\UnitTest\UnitTest.vcxproj.user` currently uses `/F:` in `LocalDebuggerCommandArguments` (e.g., `/F:TestApplication_Dialog_File.cpp`), which filters which tests run.
- Update it so the default debugger arguments do **not** filter to a single file, ensuring “every test file will be executed” when running the UnitTest project.
- Update `LocalDebuggerCommandArgumentsHistory` accordingly so the default profile remains “run everything”.

# AFFECTED PROJECTS

- Run CLI|UnitTest project Metadata_UpdateProtocol (trigger: `Source\PlatformProviders\Remote\Protocol\Protocol_*.txt` changes).
- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# !!!FINISHED!!!

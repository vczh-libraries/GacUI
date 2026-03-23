# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS

- Run CLI|UnitTest project Metadata_UpdateProtocol (trigger: `Source\PlatformProviders\Remote\Protocol\Protocol_*.txt` changes).
- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Add `RendererIdle` to the remote protocol schema

### What to change

Update `REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer.txt` to include a new **core → renderer** message with no payload:

```
message RendererIdle                                {}
```

Suggested placement: after `RendererEndRendering` (or any stable location in the renderer protocol file).

### Why

This provides an explicit “rendering has gone idle” hint so renderer-side automation (notably Playwright-driven tests) can wait for a deterministic signal instead of fixed delays.

Notes / constraints:
- Do **not** annotate this message with `[@DropRepeat]` / `[@DropConsecutive]` so tests can detect duplicates if they ever happen.

## STEP 2: Regenerate generated protocol headers / schema

### What to change

Follow the repo CLI-running guideline (`REPO-ROOT\.github\Guidelines\Running-CLI.md`) and run the protocol codegen project via the provided PowerShell wrapper scripts (do **not** run executables directly):

```powershell
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable Metadata_UpdateProtocol
```

Run this whenever any `Source\PlatformProviders\Remote\Protocol\Protocol_*.txt` changes.

### Why

`GACUI_REMOTEPROTOCOL_MESSAGES(...)` and all request/response types are generated from the `Protocol_*.txt` files; without regeneration, the new message won’t exist in the compiled schema and downstream implementations will not compile.

## STEP 3: Add `HostedRenderingIdle()` hook to render targets

### What to change

In `REPO-ROOT\Source\GraphicsElement\GuiGraphicsElementInterfaces.h`, extend `vl::presentation::elements::IGuiGraphicsRenderTarget` with a new method:

```cpp
class IGuiGraphicsRenderTarget : public Interface
{
public:
  // ...existing methods...

  /// <summary>
  /// Notify the render target that hosted rendering has gone idle, i.e. the hosted controller
  /// determined that no more rendering requests will be issued until something changes.
  /// </summary>
  virtual void HostedRenderingIdle() = 0;
};
```

Provide a default empty implementation in `vl::presentation::elements::GuiGraphicsRenderTarget` consistent with existing method organization (declare in the header, implement in the `.cpp`):

In `REPO-ROOT\Source\GraphicsElement\GuiGraphicsElementInterfaces.h`:

```cpp
class GuiGraphicsRenderTarget : public Object, public IGuiGraphicsRenderTarget
{
  // ...existing members...
public:
  // ...existing overrides...

  void HostedRenderingIdle() override;
};
```

In `REPO-ROOT\Source\GraphicsElement\GuiGraphicsElementInterfaces.cpp`:

```cpp
void GuiGraphicsRenderTarget::HostedRenderingIdle()
{
}
```

### Why

The “idle” moment is determined by `GuiHostedController`, but the action taken is render-target-specific (remote protocol wants to emit a protocol message; native render targets don’t). A hook avoids hard-coding remote logic into the hosted controller.

## STEP 4: Trigger `HostedRenderingIdle()` from `GuiHostedController::GlobalTimer` once per idle period

### What to change

Add a small piece of state to `REPO-ROOT\Source\PlatformProviders\Hosted\GuiHostedController.h`:

```cpp
protected:
  // ...existing fields...
  bool idleNotifiedSinceLastRendering = true; // avoid emitting before the first rendering
```

Then update `REPO-ROOT\Source\PlatformProviders\Hosted\GuiHostedController.cpp` inside `GuiHostedController::GlobalTimer()` at the existing early-return block (currently line ~690).

Current code:
```cpp
if (!wmManager->needRefresh && !windowsUpdatedInLastFrame)
{
  return;
}
```

Planned replacement (note: no redundant `if (renderTarget)` here because `renderTarget` has already been dereferenced earlier in this function):

```cpp
if (!wmManager->needRefresh && !windowsUpdatedInLastFrame)
{
  if (!idleNotifiedSinceLastRendering)
  {
    idleNotifiedSinceLastRendering = true;
    renderTarget->HostedRenderingIdle();
  }
  return;
}
```

In the existing `NEED_REFRESH:` block (the label and the two existing reset lines already exist), add only the new reset line:

```cpp
NEED_REFRESH:
wmManager->needRefresh = false;
windowsUpdatedInLastFrame = false;
idleNotifiedSinceLastRendering = false;
```

Important ordering constraints:
- Keep the existing `if (renderTarget->IsInHostedRendering()) { return; }` return (line ~673) **before** any idle notification; do not emit idle while hosted rendering is in progress.
- The `idleNotifiedSinceLastRendering` flag guarantees the hook is invoked **at most once** between two actual renderings, even though `GlobalTimer()` keeps being called.

### Why

The “renderer is idle” moment corresponds to the hosted controller deciding to do nothing (`needRefresh == false` and no windows were updated last frame). Without gating, the idle hook would fire on every timer tick and spam `RendererIdle`.

The `= true` initialization is to satisfy the unit-test requirement that **before the first idle-frame callback**, there is **at most one** `RendererIdle` message observed.

## STEP 5: Remote render target emits the `RendererIdle` protocol message

### What to change

Update `REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics.h` (`vl::presentation::elements::GuiRemoteGraphicsRenderTarget`) to override the new hook:

```cpp
class GuiRemoteGraphicsRenderTarget : public GuiGraphicsRenderTarget
{
  // ...existing code...
public:
  void HostedRenderingIdle() override;
};
```

Implement it in `REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics.cpp`:

```cpp
void GuiRemoteGraphicsRenderTarget::HostedRenderingIdle()
{
  remote->remoteMessages.RequestRendererIdle();
}
```

### Why

Remote protocol needs to forward the hosted-controller’s idle decision to the renderer side. Keeping it in the render target preserves separation of concerns and makes the default behavior a no-op for non-remote render targets.

## STEP 6: Keep all non-test handlers empty (ignore `RendererIdle` everywhere else)

### What to change

Adding a new `GACUI_REMOTEPROTOCOL_MESSAGES` entry will require all message implementers to compile with a new `RequestRendererIdle` virtual.

For renderer-side implementations, ensure the new handler does nothing. For example in `REPO-ROOT\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp` add:

```cpp
void GuiRemoteRendererSingle::RequestRendererIdle()
{
  // Hint only; intentionally ignored.
}
```

Most `IGuiRemoteProtocol` implementers use `GACUI_REMOTEPROTOCOL_MESSAGES(...)` macro expansion and will pick up the new message with an empty handler automatically (e.g. `GuiRemoteProtocol_Filter`, `GuiRemoteProtocol_FilterVerifier`, `GuiRemoteProtocol_Channel_Json`, `GuiRemoteProtocol_Shared`, `GuiRemoteEvents`, `UnitTestRemoteProtocol`).

Only manual/non-macro implementers should add an empty `RequestRendererIdle` implementation. The known one is `GuiRemoteRendererSingle` (in `REPO-ROOT\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`). `GuiRemoteProtocolDomDiffConverter` only overrides frame-related rendering messages and does **not** need a `RendererIdle` override.

### Why

`RendererIdle` is a hint; most runtime code should not change behavior because of it. Keeping handlers empty minimizes regressions and limits behavior changes strictly to unit-test validation.

## STEP 7: Unit test framework validates `RendererIdle` sequencing

### What to change

In the rendering protocol test module (to keep all `Impl_Renderer*` handlers together), add the counter and handler alongside existing rendering handlers:

In `REPO-ROOT\Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h` (in `UnitTestRemoteProtocol_Rendering`):

```cpp
protected:
  vint rendererIdleCount = 0;

  void Impl_RendererIdle();
```

In `REPO-ROOT\Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`:

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_RendererIdle()
{
  rendererIdleCount++;
}
```

Then in `UnitTestRemoteProtocol::ProcessRemoteEvents()` (in `REPO-ROOT\Source\UnitTestUtilities\GuiUnitTestProtocol.h`), insert the validation/reset inside `if (LogRenderingResult())`:
- After existing frame-related `CHECK_ERROR` assertions and `lastFrameIndex` / frame-name updates
- Immediately before `frameExecuting = true;` and the callback invocation

Validate the count using `CHECK_ERROR`:

```cpp
if (LogRenderingResult())
{
  // ...existing code...

  if (nextEventIndex == 0)
  {
    CHECK_ERROR(rendererIdleCount <= 1, ERROR_MESSAGE_PREFIX L"Expected at most one RendererIdle before the first idle frame callback.");
  }
  else
  {
    CHECK_ERROR(rendererIdleCount == 1, ERROR_MESSAGE_PREFIX L"Expected exactly one RendererIdle between two idle frame callbacks.");
  }
  rendererIdleCount = 0;

  // Note: do not reset rendererIdleCount when LogRenderingResult() is false;
  // it must accumulate across non-boundary ticks to detect duplicate/out-of-order RendererIdle.

  frameExecuting = true;
  func();
  frameExecuting = false;
  nextEventIndex++;
}
```

### Why

`LogRenderingResult()` already defines the “frame settled / next idle frame can run” boundary; validating here ensures:
- `RendererIdle` is emitted once per settled frame period (no spam)
- The first callback is tolerant (`<= 1`) per the requested special-case

Using `CHECK_ERROR` matches existing error signaling in this code path.

## STEP 8: Update UnitTest debugger defaults to run all tests

### What to change

In `REPO-ROOT\Test\GacUISrc\UnitTest\UnitTest.vcxproj.user`, remove the default per-file filter for `Debug|x64`:

Current:
```xml
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <LocalDebuggerCommandArguments>/F:TestApplication_Dialog_File.cpp</LocalDebuggerCommandArguments>
  <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  <LocalDebuggerCommandArgumentsHistory>...</LocalDebuggerCommandArgumentsHistory>
</PropertyGroup>
```

Planned (clear the active args, preserve history exactly as-is):
```xml
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <LocalDebuggerCommandArguments>
  </LocalDebuggerCommandArguments>
  <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  <LocalDebuggerCommandArgumentsHistory>...keep existing entries, do not overwrite...</LocalDebuggerCommandArgumentsHistory>
</PropertyGroup>
```

Notes / constraints:
- Clear only `LocalDebuggerCommandArguments` to remove `/F:` filtering.
- Preserve existing `LocalDebuggerCommandArgumentsHistory` entries exactly (append only if needed, do not rewrite or duplicate).
- Since `UnitTest.vcxproj.user` is typically user-local, treat this as a shared-policy change only if the repo intentionally tracks/enforces this file.

### Why

The repository expects the UnitTest project to execute the full suite by default; leaving `/F:` in the active debugger args silently runs only a subset and can hide regressions.

# TEST PLAN

1. Run the `Metadata_UpdateProtocol` CLI project after editing `Protocol_Renderer.txt` (via wrapper script):

   ```powershell
   & REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
   cd REPO-ROOT\Test\GacUISrc
   & REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable Metadata_UpdateProtocol
   ```

   Confirm generated headers include `RendererIdle` in `GACUI_REMOTEPROTOCOL_MESSAGES`.
2. Build `REPO-ROOT\Test\GacUISrc\GacUISrc.sln` with `Debug|x64`.
3. Run the `UnitTest` project (`Debug|x64`) and ensure:
   - No `CHECK_FAIL("Not Implemented!")` occurs from the new message.
   - All existing GacUI unit tests complete without time-based flakes.
   - Failures (if any) point to the new `CHECK_ERROR` messages with clear `ERROR_MESSAGE_PREFIX` text.

# !!!FINISHED!!!

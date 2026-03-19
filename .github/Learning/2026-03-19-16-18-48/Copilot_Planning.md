# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS
- Build the solution in folder REPO-ROOT\Test\GacUISrc, Debug|x64.
- Run CLI project Metadata_UpdateProtocol, Debug|x64.
- Build the solution in folder REPO-ROOT\Test\GacUISrc, Debug|x64.
- Always Run UnitTest project UnitTest, Debug|x64.

# EXECUTION PLAN

## STEP 1: Update protocol definitions (remove 9 messages, add batched updates)

### What to change
1) Delete 8 messages in `REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_BasicElements.txt`:

Remove exactly these lines (currently around the end of the file):
```txt
message RendererUpdateElement_SolidBorder           { request: ElementDesc_SolidBorder; }
message RendererUpdateElement_SinkBorder            { request: ElementDesc_SinkBorder; }
message RendererUpdateElement_SinkSplitter          { request: ElementDesc_SinkSplitter; }
message RendererUpdateElement_SolidBackground       { request: ElementDesc_SolidBackground; }
message RendererUpdateElement_GradientBackground    { request: ElementDesc_GradientBackground; }
message RendererUpdateElement_InnerShadow           { request: ElementDesc_InnerShadow; }
message RendererUpdateElement_Polygon               { request: ElementDesc_Polygon; }
message RendererUpdateElement_SolidLabel            { request: ElementDesc_SolidLabel; }
```

2) Delete 1 message in `REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_ImageFrame.txt`:
```txt
message RendererUpdateElement_ImageFrame            { request: ElementDesc_ImageFrame; }
```

3) Add a new union and a new field to `REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer.txt`.

Add this union (place it before `struct ElementBeginRendering` so the struct can reference it):
```txt
union OrdinaryElementDescVariant
{
    ElementDesc_SolidBorder,
    ElementDesc_SinkBorder,
    ElementDesc_SinkSplitter,
    ElementDesc_SolidBackground,
    ElementDesc_GradientBackground,
    ElementDesc_InnerShadow,
    ElementDesc_Polygon,
    ElementDesc_SolidLabel,
    ElementDesc_ImageFrame,
}
```

Extend `struct ElementBeginRendering` from:
```txt
struct ElementBeginRendering
{
    var frameId : int;
}
```

to:
```txt
struct ElementBeginRendering
{
    var frameId : int;
    var updatedElements : OrdinaryElementDescVariant[];
}
```

### Why
This removes 9 per-element update protocol messages and replaces them with a single batched list delivered in `RendererBeginRendering`, ensuring updates are applied before any `RendererRenderElement` calls in the same frame and reducing protocol dispatch overhead.

## STEP 2: Regenerate remote protocol code

### What to do
Run the CLI project `Metadata_UpdateProtocol` (per `REPO-ROOT\.github\Project.md`) using the repository wrapper scripts (e.g. `copilotExecute.ps1`).

### Why
The protocol schema change must regenerate `REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Generated\*` so the deleted message handlers disappear and the new `OrdinaryElementDescVariant` + `updatedElements` field become available in C++.

## STEP 3: Core side: collect ordinary element updates into ElementBeginRendering.updatedElements

### What to change
Files:
- `REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.h`
- `REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.cpp`
- `REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics.cpp`
- `REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics.h`

1) Change the contract of `vl::presentation::elements_remoteprotocol::IGuiRemoteProtocolElementRender::SendUpdateElementMessages` to append into a caller-owned list.

Current (in `GuiRemoteGraphics_BasicElements.h`):
```cpp
virtual void SendUpdateElementMessages(bool fullContent) = 0;
```

Change to (exact type may be `remoteprotocol::OrdinaryElementDescVariant` after regeneration):
```cpp
virtual void SendUpdateElementMessages(bool fullContent, collections::List<remoteprotocol::OrdinaryElementDescVariant>& updatedElements) = 0;
```

Update all overrides accordingly.

2) Update each renderer implementation in `GuiRemoteGraphics_BasicElements.cpp`:
Replace calls like:
```cpp
remoteRenderTarget->GetRemoteMessages().RequestRendererUpdateElement_SolidBorder(arguments);
```

with appending to the list:
```cpp
updatedElements.Add(remoteprotocol::OrdinaryElementDescVariant(arguments));
```

Do this for:
- `GuiSolidBorderElementRenderer`
- `Gui3DBorderElementRenderer`
- `Gui3DSplitterElementRenderer`
- `GuiSolidBackgroundElementRenderer`
- `GuiGradientBackgroundElementRenderer`
- `GuiInnerShadowElementRenderer`
- `GuiPolygonElementRenderer`
- `GuiSolidLabelElementRenderer`
- `GuiImageFrameElementRenderer`

`GuiFocusRectangleElementRenderer` and `GuiRawElementRenderer` keep their "nothing to update" behavior, but their `SendUpdateElementMessages` signatures still need to be updated to match the new interface.

3) Move the reconnect full-sync logic from `GuiRemoteGraphicsRenderTarget::OnControllerConnect()` into the next `StartRenderingOnNativeWindow()` using a flag.

Add a boolean flag in `GuiRemoteGraphicsRenderTarget` (in `GuiRemoteGraphics.h`), e.g.:
```cpp
bool needFullElementUpdateOnNextFrame = false;
```

In `OnControllerConnect()` (in `GuiRemoteGraphics.cpp`), remove the loop that currently does:
```cpp
for (auto renderer : renderers.Values())
{
    renderer->SendUpdateElementMessages(true);
    ...
}
```

Replace it with:
```cpp
needFullElementUpdateOnNextFrame = true;
```

In `StartRenderingOnNativeWindow()` (in `GuiRemoteGraphics.cpp`), keep existing boilerplate (especially `EnsureRequestedRenderersCreated()`) before sending begin-rendering; build `ElementBeginRendering arguments;` first, and always allocate the list:
```cpp
remoteprotocol::ElementBeginRendering arguments;
arguments.frameId = ++usedFrameIds;
arguments.updatedElements = Ptr(new List<remoteprotocol::OrdinaryElementDescVariant>);
```

Note: `updatedElements` is intentionally always allocated (possibly empty) to reduce future ambiguity; receiver-side null checks remain compatible.
Then fill `updatedElements` using one of two paths:
- If `needFullElementUpdateOnNextFrame` is true:
  - loop all `renderers.Values()` and call `renderer->SendUpdateElementMessages(true, *arguments.updatedElements.Obj());`
  - do the same `NeedUpdateMinSizeFromCache()` bookkeeping as the old `OnControllerConnect` loop
  - `renderer->ResetUpdated();`
  - reset the flag to false
- Else:
  - keep the current incremental behavior (only `renderer->IsUpdated()`), but call the new overload:
    `renderer->SendUpdateElementMessages(false, *arguments.updatedElements.Obj());`
  - keep existing bookkeeping in this path (e.g. `renderer->ResetUpdated()` and any cache/min-size related tracking such as `renderersAskingForCache` / `NeedUpdateMinSizeFromCache()`).

Finally send:
```cpp
remote->remoteMessages.RequestRendererBeginRendering(arguments);
```

### Why
- The deleted `RequestRendererUpdateElement_*` requests no longer exist after regeneration.
- The batching must happen inside `RendererBeginRendering` for correct ordering.
- The reconnect full-sync must still happen exactly once after reconnect, but now via the batched list.

## STEP 4: Remote renderer side: apply updatedElements in RequestRendererBeginRendering (keep per-element handlers)

### What to change
Files:
- `REPO-ROOT\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`
- `REPO-ROOT\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.h`

1) Keep all existing per-element apply functions unchanged (definitions already exist):
- `GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBorder(...)` in `GuiRemoteRendererSingle_Rendering_Elements.cpp`
- ...
- `GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(...)` in `GuiRemoteRendererSingle_Rendering_Image.cpp`

2) After regeneration, these will no longer be declared via `GACUI_REMOTEPROTOCOL_MESSAGES`, so add explicit declarations under `private:` in `GuiRemoteRendererSingle` (in `GuiRemoteRendererSingle.h`) as regular member functions (NOT overrides), e.g.:
```cpp
void RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments);
void RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments);
void RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments);
void RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments);
void RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments);
void RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments);
void RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments);
void RequestRendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments);
void RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments);
```

3) Implement batched dispatch in `GuiRemoteRendererSingle::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering&)` (currently empty in `GuiRemoteRendererSingle_Rendering.cpp`).

Preferred dispatch: `Variant::Apply`.
Example structure:
```cpp
void GuiRemoteRendererSingle::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
{
    if (arguments.updatedElements)
    {
        for (auto&& desc : *arguments.updatedElements.Obj())
        {
            desc.Apply(Overloading(
                [&](const remoteprotocol::ElementDesc_SolidBorder& d) { RequestRendererUpdateElement_SolidBorder(d); },
                [&](const remoteprotocol::ElementDesc_SinkBorder& d) { RequestRendererUpdateElement_SinkBorder(d); },
                [&](const remoteprotocol::ElementDesc_SinkSplitter& d) { RequestRendererUpdateElement_SinkSplitter(d); },
                [&](const remoteprotocol::ElementDesc_SolidBackground& d) { RequestRendererUpdateElement_SolidBackground(d); },
                [&](const remoteprotocol::ElementDesc_GradientBackground& d) { RequestRendererUpdateElement_GradientBackground(d); },
                [&](const remoteprotocol::ElementDesc_InnerShadow& d) { RequestRendererUpdateElement_InnerShadow(d); },
                [&](const remoteprotocol::ElementDesc_Polygon& d) { RequestRendererUpdateElement_Polygon(d); },
                [&](const remoteprotocol::ElementDesc_SolidLabel& d) { RequestRendererUpdateElement_SolidLabel(d); },
                [&](const remoteprotocol::ElementDesc_ImageFrame& d) { RequestRendererUpdateElement_ImageFrame(d); }
            ));
        }
    }
}
```

Document paragraph updates are intentionally excluded and remain handled by their existing separate protocol/message path.

### Why
- Protocol handlers for the 9 deleted messages disappear after regeneration.
- The code must still reuse the existing per-element apply logic, but now driven by `RendererBeginRendering.updatedElements`.

## STEP 5: Unit test remote protocol: apply updatedElements in Impl_RendererBeginRendering (keep handlers unchanged)

### What to change
Files:
- `REPO-ROOT\Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestRemote_Shared_RenderingProtocol.h` (rendering event log helper)

1) In `UnitTestRemoteProtocol_Rendering::Impl_RendererBeginRendering` (currently only records the frame), apply the same batched update logic.

Keep existing `Impl_RendererUpdateElement_*` functions unchanged, but call them from begin-rendering:
```cpp
void UnitTestRemoteProtocol_Rendering::Impl_RendererBeginRendering(const ElementBeginRendering& arguments)
{
    receivedDomDiffMessage = false;
    receivedElementMessage = false;
    lastRenderingCommandsOpening = true;

    auto frame = Ptr(new UnitTestLoggedFrame);
    frame->frameId = arguments.frameId;
    loggedFrames.Add(frame);

    if (arguments.updatedElements && arguments.updatedElements.Obj()->Count() > 0)
    {
        renderingDomBuilder.RequestRendererBeginRendering();
        receivedElementMessage = true;

        for (auto&& desc : *arguments.updatedElements.Obj())
        {
            desc.Apply(Overloading(
                [&](const ElementDesc_SolidBorder& d) { Impl_RendererUpdateElement_SolidBorder(d); },
                [&](const ElementDesc_SinkBorder& d) { Impl_RendererUpdateElement_SinkBorder(d); },
                [&](const ElementDesc_SinkSplitter& d) { Impl_RendererUpdateElement_SinkSplitter(d); },
                [&](const ElementDesc_SolidBackground& d) { Impl_RendererUpdateElement_SolidBackground(d); },
                [&](const ElementDesc_GradientBackground& d) { Impl_RendererUpdateElement_GradientBackground(d); },
                [&](const ElementDesc_InnerShadow& d) { Impl_RendererUpdateElement_InnerShadow(d); },
                [&](const ElementDesc_Polygon& d) { Impl_RendererUpdateElement_Polygon(d); },
                [&](const ElementDesc_SolidLabel& d) { Impl_RendererUpdateElement_SolidLabel(d); },
                [&](const ElementDesc_ImageFrame& d) { Impl_RendererUpdateElement_ImageFrame(d); }
            ));
        }
    }
}
```

This preserves existing semantics where the per-element handlers update `lastElementDescs` and any measuring behavior.

2) Update `TestRemote_Shared_RenderingProtocol.h` so tests that previously observed separate `RequestRendererUpdateElement_*` calls still see the same `Updated(...)` logs.

Approach:
- After regeneration, remove `override` from the 9 `RequestRendererUpdateElement_*` methods (keep them as regular helper members).
- Extend `RequestRendererBeginRendering(const ElementBeginRendering&)` to iterate `arguments.updatedElements` and call those existing helper methods to append the exact same `Updated(...)` log strings (avoid duplicating formatting inline).

### Why
Unit tests and the test protocol harness currently rely on update messages to:
- populate `lastElementDescs` (for size calculations / validation), and
- produce deterministic event logs.
After protocol regeneration removes the 9 update messages, those behaviors must be triggered by `updatedElements` during begin-rendering.

# TEST PLAN

1) Run `Metadata_UpdateProtocol` (via repository wrapper scripts, e.g. `copilotExecute.ps1`) and confirm:
- `GuiRemoteProtocolSchema.h/.cpp` no longer contains handlers for `RendererUpdateElement_*` (except `RendererUpdateElement_DocumentParagraph`).
- `remoteprotocol::OrdinaryElementDescVariant` exists.
- `remoteprotocol::ElementBeginRendering` contains `updatedElements`.

2) Build `REPO-ROOT\Test\GacUISrc\GacUISrc.sln` with Debug|x64.

3) Run UnitTest (Debug|x64) and specifically watch for:
- Rendering protocol tests expecting `Updated(...)` events still pass.
- No regressions in document paragraph-related tests (paragraph update path is unchanged).

4) Manual sanity (debug-only, if needed):
- Verify that `GuiRemoteRendererSingle::RequestRendererBeginRendering` applies all `updatedElements` before any `RequestRendererRenderElement` processing in the same frame.
- Verify reconnect flow sends exactly one full ordinary element sync on the first frame after reconnect (then returns to incremental updates).

# !!!FINISHED!!!

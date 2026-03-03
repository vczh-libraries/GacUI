# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS
- Build the solution in folder REPO-ROOT\Test\GacUISrc, Debug|x64.
- Run CLI project Metadata_UpdateProtocol, Debug|x64.
- Build the solution in folder REPO-ROOT\Test\GacUISrc, Debug|x64.
- Always Run UnitTest project UnitTest, Debug|x64.

# EXECUTION PLAN

## STEP 1: Update protocol definitions (remove 9 messages, add batched updates)

### 1.1 Delete 8 messages in REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_BasicElements.txt

Remove these message declarations:
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

### 1.2 Delete 1 message in REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_ImageFrame.txt

Remove this message declaration:
```txt
message RendererUpdateElement_ImageFrame            { request: ElementDesc_ImageFrame; }
```

### 1.3 Add OrdinaryElementDescVariant and ElementBeginRendering.updatedElements in REPO-ROOT\Source\PlatformProviders\Remote\Protocol\Protocol_Renderer.txt

Add this union (place it before struct ElementBeginRendering):
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

Update struct ElementBeginRendering to include updatedElements:

Replace:
```txt
struct ElementBeginRendering
{
    var frameId : int;
}
```

With:
```txt
struct ElementBeginRendering
{
    var frameId : int;
    var updatedElements : OrdinaryElementDescVariant[];
}
```

## STEP 2: Regenerate remote protocol code

Run the CLI project Metadata_UpdateProtocol using repository wrapper scripts (exact invocation):
```powershell
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable Metadata_UpdateProtocol -Configuration Debug -Platform x64
```

Then ensure the generated protocol code compiles before continuing with code fixes.

## STEP 3: Core side: collect ordinary element updates into ElementBeginRendering.updatedElements

Files:
- REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.h
- REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.cpp
- REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics.cpp
- REPO-ROOT\Source\PlatformProviders\Remote\GuiRemoteGraphics.h

### 3.1 Change IGuiRemoteProtocolElementRender::SendUpdateElementMessages signature

In GuiRemoteGraphics_BasicElements.h, replace:
```cpp
virtual void SendUpdateElementMessages(bool fullContent) = 0;
```

With:
```cpp
virtual void SendUpdateElementMessages(bool fullContent, collections::List<remoteprotocol::OrdinaryElementDescVariant>& updatedElements) = 0;
```

Update all overrides accordingly.

### 3.2 Replace RequestRendererUpdateElement_* calls with list append in GuiRemoteGraphics_BasicElements.cpp

For each of the following renderers:
- GuiSolidBorderElementRenderer
- Gui3DBorderElementRenderer
- Gui3DSplitterElementRenderer
- GuiSolidBackgroundElementRenderer
- GuiGradientBackgroundElementRenderer
- GuiInnerShadowElementRenderer
- GuiPolygonElementRenderer
- GuiSolidLabelElementRenderer
- GuiImageFrameElementRenderer

Replace code like:
```cpp
remoteRenderTarget->GetRemoteMessages().RequestRendererUpdateElement_SolidBorder(arguments);
```

With:
```cpp
updatedElements.Add(remoteprotocol::OrdinaryElementDescVariant(arguments));
```

IMPORTANT: For GuiSolidLabelElementRenderer and GuiImageFrameElementRenderer, keep all existing fullContent-dependent computation intact; only replace the final protocol emission with the list append.

For GuiFocusRectangleElementRenderer and GuiRawElementRenderer: keep their “nothing to update” behavior, but update the method signature to match the interface.

### 3.3 Move reconnect full-sync logic from OnControllerConnect to next StartRenderingOnNativeWindow via a flag

In GuiRemoteGraphics.h (GuiRemoteGraphicsRenderTarget), add:
```cpp
bool needFullElementUpdateOnNextFrame = false;
```

In GuiRemoteGraphics.cpp, in OnControllerConnect(), ONLY replace the existing loop that sends full ordinary-element updates with:
```cpp
needFullElementUpdateOnNextFrame = true;
```
Keep all other reconnect work in OnControllerConnect unchanged (renderer creation, paragraph sync, and any other reconnect-specific initialization).

In GuiRemoteGraphics.cpp, in StartRenderingOnNativeWindow(), build ElementBeginRendering arguments first and always allocate updatedElements (so receiver-side null checks are defensive-only):
```cpp
remoteprotocol::ElementBeginRendering arguments;
arguments.frameId = ++usedFrameIds;
arguments.updatedElements = Ptr(new List<remoteprotocol::OrdinaryElementDescVariant>);
```

Then fill updatedElements using one of these paths:
- If needFullElementUpdateOnNextFrame is true:
  - loop all renderers.Values() and call:
    - renderer->SendUpdateElementMessages(true, *arguments.updatedElements.Obj());
  - keep the same NeedUpdateMinSizeFromCache() bookkeeping as the old OnControllerConnect loop, BUT ensure it is duplicate-safe:
    ```cpp
    if (renderer->NeedUpdateMinSizeFromCache())
    {
        if (!renderersAskingForCache.Contains(renderer))
        {
            renderersAskingForCache.Add(renderer);
        }
    }
    ```
  - renderer->ResetUpdated();
  - set needFullElementUpdateOnNextFrame = false;
- Else:
  - keep the existing incremental behavior (only renderer->IsUpdated()), but call:
    - renderer->SendUpdateElementMessages(false, *arguments.updatedElements.Obj());
  - keep existing bookkeeping in this path (including ResetUpdated and any cache/min-size tracking).

Finally send begin-rendering:
```cpp
remote->remoteMessages.RequestRendererBeginRendering(arguments);
```

## STEP 4: Remote renderer side: apply updatedElements in RequestRendererBeginRendering (keep per-element handlers)

Files:
- REPO-ROOT\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp
- REPO-ROOT\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.h

### 4.1 Keep existing per-element apply functions unchanged

Keep the existing method definitions (they become internal helpers after regeneration):
- GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBorder(...) in GuiRemoteRendererSingle_Rendering_Elements.cpp
- ...
- GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(...) in GuiRemoteRendererSingle_Rendering_Image.cpp

### 4.2 Declare the 9 helper methods explicitly in GuiRemoteRendererSingle.h (as regular members, NOT overrides)

Add under private: in GuiRemoteRendererSingle:
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

### 4.3 Implement batched dispatch in GuiRemoteRendererSingle::RequestRendererBeginRendering

In GuiRemoteRendererSingle_Rendering.cpp:
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

## STEP 5: Unit test remote protocol: apply updatedElements in Impl_RendererBeginRendering (keep handlers unchanged)

Files:
- REPO-ROOT\Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp
- REPO-ROOT\Test\GacUISrc\UnitTest\TestRemote_Shared_RenderingProtocol.h

### 5.1 Apply updatedElements in UnitTestRemoteProtocol_Rendering::Impl_RendererBeginRendering

In GuiUnitTestProtocol_Rendering.cpp (IMPORTANT: preserve log ordering as described in STEP 5.2):
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
        // Apply element updates first so event logs remain backward-compatible:
        // Updated(...), Updated(...), ..., Begin()
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

        renderingDomBuilder.RequestRendererBeginRendering();
        receivedElementMessage = true;
    }
}
```

### 5.2 Update TestRemote_Shared_RenderingProtocol.h to preserve Updated(...) logs AND ordering

After regeneration:
- Remove override from the 9 RequestRendererUpdateElement_* methods (keep them as regular helper members).
- Extend RequestRendererBeginRendering(const ElementBeginRendering&) to iterate arguments.updatedElements and call those existing helper methods so the same Updated(...) log strings are produced.
- IMPORTANT: Preserve the existing event-log ordering in remote protocol tests:
  - Before this change, logs were: `Updated(...), Updated(...), ..., Begin()`.
  - After batching, a naive implementation logs: `Begin(), Updated(...), ...`.
  - Make the implementation log/apply Updated(...) entries BEFORE the Begin() entry inside RequestRendererBeginRendering so existing snapshots/assertions remain stable.

## STEP 6: Verification checklist

- Protocol regeneration succeeded (Metadata_UpdateProtocol).
- Build succeeded (Test\GacUISrc, Debug|x64).
- UnitTest passed (Debug|x64).
- Rendering protocol tests: Updated(...) log strings are preserved AND ordering remains backward-compatible: `Updated(...), ..., Begin()`.
- Reconnect path: only the full ordinary-element sync was deferred (other OnControllerConnect work remains intact).

# FIXING ATTEMPTS

# !!!FINISHED!!!

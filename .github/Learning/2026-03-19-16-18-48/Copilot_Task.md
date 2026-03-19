# !!!TASK!!!

# PROBLEM DESCRIPTION
In ONLY Protocol_Renderer_BasicElements.txt and Protocol_Renderer_ImageFrame.txt,
there are in total 9 `RendererUpdateElement_*` messages, delete them.

Instead, add the following to `Protocol_Renderer.txt`
- union OrdinaryElementDescVariant, which is similar to `UnitTest_ElementDescVariant` but without `ElementDesc_DocumentParagraphFull`.
- `ElementBeginRendering` adds a new field `var updatedElements : OrdinaryElementDescVariant[];`.

After running `Metadata_UpdateProtocol` there will be many errors. Here is a helpful categorization:

In `GuiRemoteGraphics_BasicElements.cpp`, since 9 methods are missing, so the `SendUpdateElementMessages` function needs to be changed, now new element desc will be added to a list in the argument.
There are two calls to `SendUpdateElementMessages`.
The first one is in `GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow`, it is easy, the list is in `remoteprotocol::ElementBeginRendering arguments;`.
The second one is in `GuiRemoteGraphicsRenderTarget::OnControllerConnect`. Notice the first bool argument is different.
You need to have a flag in `GuiRemoteGraphicsRenderTarget`. So that in `StartRenderingOnNativeWindow`, when the flag is on, it runs the `foreach renderer` moved from `OnControllerConnect` instead of the one in it, and the flag is reset.

There are also some handler functions for deleted messages.
Keep all of them unchanged.
But since those abstract methods are removed, you need to fix the class declaration accordingly.
And in the handler function for `RendererBeginRendering`, call them. Prefer `Variant::Apply` over any other type of dispatching.
Notice that document paragraph is not in this list. Document paragraph is handled in a separate way.
Affected classes here would be `GuiRemoteRendererSingle`, `UnitTestRemoteProtocol_Rendering` and some others.

# UPDATES

# INSIGHTS AND REASONING

## Summary
The renderer side currently pushes element property changes using 9 separate `RendererUpdateElement_*` messages (8 basic elements + 1 image frame). This change removes those protocol messages and instead batches “ordinary element” updates into `RendererBeginRendering` via a new `updatedElements` array of a variant type.

The intent is to (1) reduce message count / dispatch overhead, and (2) guarantee that all property updates relevant to a frame are delivered before any `RendererRenderElement` calls for that frame.

## Protocol design changes

### Files involved
- `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_BasicElements.txt`: remove the 8 `message RendererUpdateElement_* { request: ElementDesc_*; }` declarations.
- `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_ImageFrame.txt`: remove `message RendererUpdateElement_ImageFrame { request: ElementDesc_ImageFrame; }`.
- `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer.txt`: extend the core renderer protocol with:
  - `union OrdinaryElementDescVariant`: same alternatives as `Protocol_UnitTest.txt`’s `UnitTest_ElementDescVariant` **except** it excludes `ElementDesc_DocumentParagraphFull`.
    - This union therefore includes the “ordinary element desc” structs: `ElementDesc_SolidBorder`, `ElementDesc_SinkBorder`, `ElementDesc_SinkSplitter`, `ElementDesc_SolidBackground`, `ElementDesc_GradientBackground`, `ElementDesc_InnerShadow`, `ElementDesc_Polygon`, `ElementDesc_SolidLabel`, `ElementDesc_ImageFrame`.
    - Identity is carried inside each `ElementDesc_*` (`var id : int;`) and must be used by the receiver to locate the target element when applying batched updates.
  - Add `var updatedElements : OrdinaryElementDescVariant[];` to `struct ElementBeginRendering`.
    - `updatedElements` is always present; an empty array means “no ordinary element updates this frame”.

### Explicit non-goals
- Document paragraph updates are *not* moved into this list.
  - `Protocol_Renderer_Document.txt` keeps `message RendererUpdateElement_DocumentParagraph { request: ElementDesc_DocumentParagraph; response: ...; }` and its associated paragraph query messages.
  - This matches the requirement that document paragraphs continue to be handled separately.

## Code-level impact (high-level)
Running `Metadata_UpdateProtocol` regenerates the remote protocol schema (e.g. `Source\PlatformProviders\Remote\Protocol\Generated\...`) so the 9 request methods and related abstract interface members disappear. Code that previously called `RequestRendererUpdateElement_*` directly must be refactored to populate `ElementBeginRendering.updatedElements` instead.

### Core side: collecting element updates

#### Where updates are currently sent
In `Source\PlatformProviders\Remote\GuiRemoteGraphics.cpp`:
- `GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()` iterates `renderers` and calls `renderer->SendUpdateElementMessages(false);` (only updated ones), then sends `RendererBeginRendering`.
- `GuiRemoteGraphicsRenderTarget::OnControllerConnect()` iterates *all* renderers and calls `renderer->SendUpdateElementMessages(true);` (full content push after connect).

In `Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.h`, `IGuiRemoteProtocolElementRender::SendUpdateElementMessages(bool fullContent)` is currently a pure virtual used by all element renderers.

#### New mechanism
The renderers should no longer “send update messages”. Instead, they should *append* their current `ElementDesc_*` into a list that will become `ElementBeginRendering.updatedElements`.

Design-wise, this implies:
- Change the `SendUpdateElementMessages` contract so it can add to a provided collection (owned by the caller), rather than calling `remoteMessages.RequestRendererUpdateElement_*`.
  - Concrete option: `SendUpdateElementMessages(bool fullContent, collections::List<remoteprotocol::OrdinaryElementDescVariant>& updatedElements)`.
  - The required call-site convenience comes from the fact that in `StartRenderingOnNativeWindow` the list lives inside `remoteprotocol::ElementBeginRendering arguments;`.

#### Two call sites + reconnect flag
The two places that need to populate the list are exactly the two existing call sites:
- `StartRenderingOnNativeWindow`: build `ElementBeginRendering arguments;`, fill `arguments.updatedElements` from the updated renderers, then send `RequestRendererBeginRendering(arguments)`.
- `OnControllerConnect`: keep its current responsibility of resending full state after a reconnect, but move the “foreach renderer send full update” logic to run in the *next* `StartRenderingOnNativeWindow`.
  - Add a boolean flag on `GuiRemoteGraphicsRenderTarget` to indicate “need full element sync on next begin-rendering”.
  - `OnControllerConnect` sets the flag (ordinary elements only).
  - `StartRenderingOnNativeWindow` checks the flag:
    - when on: run the “foreach renderer” loop that previously lived in `OnControllerConnect` (the `fullContent == true` behavior), but populating `arguments.updatedElements` instead of sending messages.
      - This is the *only* collection path taken for that frame (not in addition to the incremental path), to preserve “exactly one full sync after reconnect”.
    - reset the flag.
    - when off: run the existing “only updated renderers” behavior (the `fullContent == false` behavior).
    - In both branches, preserve existing renderer lifecycle bookkeeping (e.g. `NeedUpdateMinSizeFromCache()` and `ResetUpdated()` calls) so size caches and incremental-update tracking remain correct.

This preserves the original semantics where reconnect triggers a full element refresh, while conforming to the new batching protocol.

### Remote side: applying batched updates

#### Keep per-element handler functions
Classes like `GuiRemoteRendererSingle` currently contain per-element update handlers like:
- `RequestRendererUpdateElement_SolidBorder(const ElementDesc_SolidBorder&)` (see `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Elements.cpp`).

Even after protocol regeneration removes the corresponding protocol message definitions, these functions remain useful as internal “apply desc” helpers, so they should be kept unchanged as requested.

#### Fix class declarations after regeneration
Because the 9 protocol messages are deleted, the regenerated abstract interface(s) that `GuiRemoteRendererSingle`, `UnitTestRemoteProtocol_Rendering`, etc. implement will no longer require those methods. The design change is to update class inheritance / `override` lists accordingly, while retaining the method bodies as regular member functions.

#### Apply `updatedElements` in `RendererBeginRendering`
`GuiRemoteRendererSingle::RequestRendererBeginRendering(const ElementBeginRendering&)` is currently empty (see `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`). With the new protocol field, this handler becomes the single place that applies all ordinary element updates, and it must do so *before* any `RendererRenderElement` processing for the frame.

Dispatching should be done via `Variant::Apply` (per repo guidance in the request), mapping each alternative in `OrdinaryElementDescVariant` to the corresponding existing apply function:
- e.g. `ElementDesc_SolidBorder` -> `RequestRendererUpdateElement_SolidBorder(desc)`
- ...
- `ElementDesc_ImageFrame` -> `RequestRendererUpdateElement_ImageFrame(desc)`

Document paragraph is intentionally excluded from this list; its update is still handled through the separate document-paragraph messages and code paths.

## Verification / generated code
Because this is a protocol schema change:
- Run `Metadata_UpdateProtocol` after editing `Source\PlatformProviders\Remote\Protocol\*.txt`.
- Build-fix all implementers after regeneration:
  - remove stale `override` declarations for the deleted `RendererUpdateElement_*` request handlers (while keeping their bodies as internal helper methods).
  - ensure `RequestRendererBeginRendering` applies `updatedElements` via `Variant::Apply`.
- Ensure all affected translation units compile with the regenerated protocol schema.
- Verification checkpoints:
  - Within a frame, all entries in `updatedElements` are applied before any `RendererRenderElement` processing.
  - After reconnect, exactly one full-content ordinary element sync occurs on the next rendering frame (then returns to incremental updates).
  - Paragraph update path remains separate and unchanged.
- Always run unit tests for these changes (noting that `GuiRemoteRendererSingle` itself is not covered, but the build will validate its compilation).

# AFFECTED PROJECTS
- Build the solution in folder REPO-ROOT\Test\GacUISrc, Debug|x64.
- Run CLI project Metadata_UpdateProtocol, Debug|x64.
- Build the solution in folder REPO-ROOT\Test\GacUISrc, Debug|x64.
- Always Run UnitTest project UnitTest, Debug|x64.

# !!!FINISHED!!!


# !!!SCRUM!!!

# DESIGN REQUEST

Your main work is to implement functions in [GuiRemoteRendererSingle_Rendering_Document.cpp](Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Document.cpp) , as well as other files if they are affected, especially [GuiRemoteRendererSingle.h](Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h) 

This is the client implementation of the `IGuiGraphicsParagraph` interface. `IGuiGraphicsParagraph` is not an `IGuiGraphicsElement`, but the remote protocol treat it is, so you need to create your own `IGuiGraphicsParagraph` but put it in an `IGuiGraphicsElement`. There is an example in [GuiUnitTestSnapshotViewerApp.cpp](Source/UnitTestUtilities/SnapshotViewer/Application/GuiUnitTestSnapshotViewerApp.cpp) called `GuiGraphicsParagraphWrapperElement`.

But the behaves differently, as `GuiGraphicsParagraphWrapperElement` loads a rich text document and it never changes, while the implementation you are working on keep updating the rich text document in `RequestRendererUpdateElement_DocumentParagraph`. It should works similarily with the unit test mock implementation in [GuiUnitTestProtocol_Rendering_Document.cpp](Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering_Document.cpp) .

When an GacUI application is running with remote protocol, the actual implementation is the core application, the client `GuiRemoteRendererSingle` only renders and send user interactions to core. For reference, the `IGuiGraphicsParagraph` in core is in [GuiRemoteGraphics_Document.h](Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h) 

You are currently not offered enough tools for actually running the application to test, so your work is to make it compiles, but you need to research deeply and think carefully to ensure it actually works.

I suggest there should be only one task, in this task you should:
- Create the element class for the underlying IGuiGraphicsParagraph, it could be created from `GetCurrentController`, and you should use it.
- Handle element creation and deletion in [GuiRemoteRendererSingle_Rendering.cpp](Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering.cpp) 
- Update the document using the run diffs in RequestRendererUpdateElement_DocumentParagraph. The first call will bring a non-empty text property so that you can create the underlying paragraph object. All following call with the same element id will have an empty text as the text can't be changed.`IGuiGraphicsParagraph` will be immediately ready after you update the text and character attributes, so that you can use it to construct a reply
- Fill all remain functions, they are almost directly mapping to members in `IGuiGraphicsParagraph`.

# UPDATES

# TASKS

- [x] TASK No.1: Remote Document Paragraph Rendering (Client)

## TASK No.1: Remote Document Paragraph Rendering (Client)

Implement the client-side `IGuiGraphicsParagraph` integration in `GuiRemoteRendererSingle` so that the remote protocol can treat a paragraph as an element, while still behaving correctly under incremental updates (run diffs) from the core application. The result should compile and match the expected runtime behavior (even without end-to-end execution available in this environment).

### what to be done

- Fix document hygiene for this task: remove encoding/mojibake artifacts so the content is readable and searchable.
- Define acceptance criteria tied to repo tooling:
  - Must compile at least `Test/GacUISrc/GacUISrc.sln`.
  - Include `Test/GacUISrc/UnitTest/UnitTest.vcxproj` as a compile sanity check for protocol handler completeness.
- Implement a concrete wrapper design that matches the remote protocol model:
  - Create a client-side `IGuiGraphicsElement` wrapper stored in `availableElements` by element id, so all paragraph protocol handlers can resolve state from id only.
  - The wrapper owns the underlying `IGuiGraphicsParagraph` and paragraph-specific state (cached runs, inline-object bounds, caret state as needed).
  - The wrapper implements `IGuiGraphicsParagraphCallback` (at least `OnRenderInlineObject`) and is passed to paragraph creation, otherwise inline objects cannot be measured/rendered.
- Make thread-affinity explicit:
  - Paragraph creation/update and any render-target interaction must run on the UI thread.
  - If protocol handling can occur off-thread, marshal via `GetCurrentController()->AsyncService()->InvokeInMainThread(...)`.
- Specify the exact paragraph creation API and ordering:
  - Use `GetGuiGraphicsResourceManager()->GetLayoutProvider()->CreateParagraph(text, renderTarget, callback)`.
  - Clarify what happens if updates arrive before a render target exists (defer creation vs `CHECK_ERROR`) and keep the behavior consistent across all handlers.
- Define render-target lifecycle and registration rules:
  - `IGuiGraphicsParagraph` is render-target-bound; on render target changes, invalidate and recreate the paragraph (and re-register as needed).
  - Register/unregister paragraphs on the render target, and use `id == -1` as the single source of truth for "not registered / not available" (avoid parallel boolean flags).
- Expand "apply run diffs" into a concrete, mock-aligned flow in `RequestRendererUpdateElement_DocumentParagraph`:
  - Mirror behavior in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering_Document.cpp` (do not re-invent semantics) for both text runs and inline-object runs.
  - Apply known constraints/learnings: capture inline-object background element id before inserting runs; compare `IGuiGraphicsParagraph::TextStyle` flags against `(TextStyle)0`; `DiffRuns` must not drop old ranges (use `CHECK_ERROR`).
  - Decide (and document) robustness for protocol violations (e.g. later updates containing non-empty `text`: reinitialize vs `CHECK_ERROR` vs ignore) and keep it consistent with the mock/protocol expectations.
- Replace "fill remaining functions" with an explicit checklist of handlers to implement, mapping directly to `IGuiGraphicsParagraph` APIs:
  - `RequestRendererUpdateElement_DocumentParagraph`
  - `RequestDocumentParagraph_GetCaret`
  - `RequestDocumentParagraph_GetCaretBounds`
  - `RequestDocumentParagraph_GetInlineObjectFromPoint`
  - `RequestDocumentParagraph_GetNearestCaretFromTextPos`
  - `RequestDocumentParagraph_IsValidCaret`
  - `RequestDocumentParagraph_OpenCaret`
  - `RequestDocumentParagraph_CloseCaret`
  - Plus any other paragraph queries required by the protocol.
- Make response requirements explicit:
  - Include paragraph size (`GetSize()`) and inline-object bounds (callback id -> `Rect`) so caret/hit-testing/nearest-caret queries can succeed.
  - Clarify whether an explicit layout/render step (e.g. `Render()`) is required before answering measurement-related requests.
- Reassert codebase conventions:
  - Use `vint`, `WString`, `Ptr<>`, and `vl::collections::*` (avoid `std::*`); use tabs; keep header edits comment-free; extract helpers to avoid duplicated conversion/mapping blocks.

### rationale

- The remote protocol treats `IGuiGraphicsParagraph` as if it were an `IGuiGraphicsElement`, but the actual API separation requires a wrapper element on the client to fit into the renderer's element management and rendering pipeline.
- `GuiGraphicsParagraphWrapperElement` demonstrates the basic pattern, but it is static; the remote client needs a dynamic, incremental-update version to support `RequestRendererUpdateElement_DocumentParagraph` and match core-side behavior.
- Handling creation/deletion in the central rendering file keeps ownership and lifetime rules consistent with other remote-rendered objects, reducing the chance of leaks, dangling references, or inconsistent ids.
- Implementing the remaining functions as direct mappings lowers risk and keeps behavior aligned with the underlying `IGuiGraphicsParagraph` contract, while the unit test mock provides a ground truth for expected message-level behavior.

# Impact to the Knowledge Base

## GacUI

- Consider adding a short knowledge-base note under the Remote Protocol / client renderer documentation to explain why paragraphs are wrapped as elements on the client, render-target binding/registration rules (`id == -1` state), and how incremental run diffs (text + inline-object runs) are applied to match `GuiUnitTestProtocol_Rendering_Document.cpp`.

# !!!FINISHED!!!

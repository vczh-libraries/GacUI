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

- [ ] TASK No.1: Remote Document Paragraph Rendering (Client)

## TASK No.1: Remote Document Paragraph Rendering (Client)

Implement the client-side `IGuiGraphicsParagraph` integration in `GuiRemoteRendererSingle` so that the remote protocol can treat a paragraph as an element, while still behaving correctly under incremental updates (run diffs) from the core application. The result should compile and match the expected runtime behavior (even without end-to-end execution available in this environment).

### what to be done

- Create a client-side element type that internally owns/hosts an `IGuiGraphicsParagraph` instance, so it can be stored and managed in the existing remote element table keyed by element id.
- Construct the underlying paragraph implementation from `GetCurrentController()` using the platform text services available on the client.
- Wire up element creation and deletion in `Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering.cpp` so the remote protocol’s “paragraph-as-element” lifecycle is handled consistently with other remote elements.
- Implement `RequestRendererUpdateElement_DocumentParagraph` to apply the incoming document/run diffs:
  - The first update call for an element id contains a non-empty text and should initialize the underlying paragraph/document model.
  - Subsequent updates for the same element id have an empty text; only apply run/attribute diffs and any other incremental changes supported by the protocol.
  - Ensure the paragraph becomes ready immediately after updates so replies can query measurements/layout and return expected values.
- Fill remaining paragraph-related request/response handlers in `Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Document.cpp`, mapping as directly as possible to `IGuiGraphicsParagraph` members (and matching the mock behavior in `Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering_Document.cpp`).

### rationale

- The remote protocol treats `IGuiGraphicsParagraph` as if it were an `IGuiGraphicsElement`, but the actual API separation requires a wrapper element on the client to fit into the renderer’s element management and rendering pipeline.
- `GuiGraphicsParagraphWrapperElement` demonstrates the basic pattern, but it is static; the remote client needs a dynamic, incremental-update version to support `RequestRendererUpdateElement_DocumentParagraph` and match core-side behavior.
- Handling creation/deletion in the central rendering file keeps ownership and lifetime rules consistent with other remote-rendered objects, reducing the chance of leaks, dangling references, or inconsistent ids.
- Implementing the remaining functions as direct mappings lowers risk and keeps behavior aligned with the underlying `IGuiGraphicsParagraph` contract, while the unit test mock provides a ground truth for expected message-level behavior.

# Impact to the Knowledge Base

## GacUI

- Consider adding a short knowledge-base note under the Remote Protocol / client renderer documentation to explain why paragraphs are wrapped as elements on the client, and how incremental run diffs are applied (static wrapper vs. remote-updated wrapper).

# !!!FINISHED!!!

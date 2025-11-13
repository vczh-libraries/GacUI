# !!!PLANNING!!!
# UPDATES
## UPDATE
You can remove `registered` since `id == -1` should just good enough to know.
Examine how other elements call `messages.RequestRendererDestroyed`, since it is a list, submitting each one in a list hurts performance. Is there a better way?
There is no need to have `textDirty` as the text cannot be changed (did you realize?)
In SetInlineObject I believe the AddInlineObjectRun test should also bring before `renderer->SetRenderTarget`, what do you think?
## UPDATE
Since only GuiRemoteGraphicsParagraph calls QueueRendererCreated, you can just call it `RegisterParagraph` and `UnregisterParagraph`, aka move the map from resource manager to render target. It simplifies, and solve the problem when an element id does not have a renderer (because it is a paragraph).
And since EnsureParagraphRegistered is in the constructor, you don't need that function, just expand it in the constructor. And the id will be available to all methods.
You will also need to make `EnsureRemoteParagraphSynced` returns bool, so that when it fails, no further steps need to perform in those methods needing it but only fallback.
## UPDATE
`DocumentParagraph_RenderInlineObject` has been upgraded to `DocumentParagraph_RenderInlineObjects` so the `GuiRemoteGraphicsResourceManager::OnDocumentParagraph_RenderInlineObject` already changed. And in the current phase you can just leave it unimplemented, and remove `GuiRemoteGraphicsParagraph::RenderInlineObject`, with `inlineStatesByCallback` and `UpdateInlineState` which are used in `RenderInlineObject`
## UPDATE

	auto& storedProp = inlineObjectRuns[range];
	storedProp.backgroundElementId = backgroundElementId;

this is not correct as storedProp will be const&. You need to call remoteRenderer->GetID() right away, just leave renderer->renderTarget after AddInlineObjectRun

# IMPROVEMENT PLAN
## STEP 1: Centralize Paragraph Registration in Render Target
Teach `GuiRemoteGraphicsRenderTarget` to track document paragraphs directly. Introduce a dictionary from id to `GuiRemoteGraphicsParagraph*`, replace the old `QueueRendererCreated` / `QueueRendererDestroyed` helpers with `RegisterParagraph` / `UnregisterParagraph`, and expose a lookup so inline-object callbacks can forward to the paragraph.

```cpp
// Source/PlatformProviders/Remote/GuiRemoteGraphics.h
class GuiRemoteGraphicsParagraph;
collections::Dictionary<vint, GuiRemoteGraphicsParagraph*>	paragraphs;
collections::List<remoteprotocol::RendererCreation>			pendingParagraphCreations;

GuiRemoteGraphicsParagraph*			GetParagraph(vint id);
void								RegisterParagraph(GuiRemoteGraphicsParagraph* paragraph);
void								UnregisterParagraph(vint id);

// Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp
void GuiRemoteGraphicsRenderTarget::RegisterParagraph(GuiRemoteGraphicsParagraph* paragraph)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiRemoteGraphicsRenderTarget::RegisterParagraph(GuiRemoteGraphicsParagraph*)#"
	auto id = paragraph->GetParagraphId();
	CHECK_ERROR(!paragraphs.Keys().Contains(id), ERROR_MESSAGE_PREFIX L"Duplicated paragraph id.");
	paragraphs.Add(id, paragraph);
	pendingParagraphCreations.Add({ id,remoteprotocol::RendererType::DocumentParagraph });
#undef ERROR_MESSAGE_PREFIX
}

void GuiRemoteGraphicsRenderTarget::UnregisterParagraph(vint id)
{
	vint index = paragraphs.Keys().IndexOf(id);
	if (index != -1)
	{
		paragraphs.Remove(id);
		for (vint i = pendingParagraphCreations.Count() - 1; i >= 0; i--)
		{
			if (pendingParagraphCreations[i].id == id)
			{
				pendingParagraphCreations.RemoveAt(i);
				return;
			}
		}
		if (!destroyedRenderers.Contains(id))
		{
			destroyedRenderers.Add(id);
		}
	}
}

GuiRemoteGraphicsParagraph* GuiRemoteGraphicsRenderTarget::GetParagraph(vint id)
{
	vint index = paragraphs.Keys().IndexOf(id);
	return index == -1 ? nullptr : paragraphs.Values()[index];
}
```

Leave the upgraded `GuiRemoteGraphicsResourceManager::OnDocumentParagraph_RenderInlineObjects` stub untouched for now (it can continue to fail fast) so the infrastructure is ready when we wire the new batched callback pipeline.

*Why*: Having the render target own paragraph ids keeps lifecycle consistent with other remote renderers, avoids stale ids during remote message batching, and positions us for the upcoming `DocumentParagraph_RenderInlineObjects` batching flow once its client-side handling is implemented.

## STEP 2: Enrich GuiRemoteGraphicsParagraph state and helpers
Augment the paragraph class (header) with the task-mandated members, staging maps for run diffs, inline-object caches, and helper declarations for synchronization, text-position conversions, and id exposure back to the render target.

```cpp
// Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h
DocumentTextRunPropertyMap                  textRuns;
DocumentInlineObjectRunPropertyMap          inlineObjectRuns;
DocumentRunPropertyMap                      stagedRuns;      // corresponds to "runs" in the task brief
DocumentRunPropertyMap                      committedRuns;
bool                                         wrapLine = false;
vint                                         maxWidth = -1;
Alignment                                   paragraphAlignment = Alignment::Left;
Size                                        cachedSize = Size(0, 0);
bool                                         needUpdate = true;
vint                                         id = -1;
vuint64_t                                    lastRenderedBatchId = 0;

public:
vint                                         GetParagraphId() const;

protected:
vint                                         NativeTextPosToRemoteTextPos(vint textPos);
vint                                         RemoteTextPosToNativeTextPos(vint textPos);
bool                                         TryBuildCaretRange(vint start, vint length, CaretRange& range);
bool                                         EnsureRemoteParagraphSynced();
void                                         MarkParagraphDirty(bool invalidateSize);
```

*Why*: These members allow us to batch formatting changes, compute diffs with `DiffRuns`, cache responses, and stage remote paragraph descriptors efficiently. Keeping the helper declarations tight ensures the remaining methods share consistent synchronization and conversion logic.

## STEP 3: Implement lifecycle, synchronization, and helpers
Update the render target batching to flush `pendingParagraphCreations`, expand the paragraph constructor/destructor so ids are allocated and registered immediately, and return a `bool` from `EnsureRemoteParagraphSynced` so callers can bail out early on disconnects.

```cpp
// Source/PlatformProviders/Remote/GuiRemoteGraphics.h
collections::List<remoteprotocol::RendererCreation> pendingParagraphCreations;

// Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp
void GuiRemoteGraphicsRenderTarget::StartRenderingOnNativeWindow()
{
	// ...
	if (createdRenderers.Count() > 0 || pendingParagraphCreations.Count() > 0)
	{
		auto ids = Ptr(new List<remoteprotocol::RendererCreation>);
		for (auto id : createdRenderers)
		{
			ids->Add({ id,renderers[id]->GetRendererType() });
		}
		for (auto&& creation : pendingParagraphCreations)
		{
			ids->Add(creation);
		}
		createdRenderers.Clear();
		pendingParagraphCreations.Clear();
		remote->remoteMessages.RequestRendererCreated(ids);
	}
}
```

In `GuiRemoteGraphics_Document.cpp` add `#include "GuiRemoteGraphics_BasicElements.h"` for background renderers, flesh out the constructor/destructor, and implement the helper routines declared above. The helpers should register the paragraph id with the render target, build `ElementDesc_DocumentParagraph`, send `RendererUpdateElement_DocumentParagraph`, and maintain cached state.

```cpp
// include
#include "GuiRemoteGraphics_BasicElements.h"

GuiRemoteGraphicsParagraph::GuiRemoteGraphicsParagraph(...)
	: id(renderTarget->AllocateNewElementId())
{
	renderTarget->RegisterParagraph(this);
	needUpdate = true;
}

GuiRemoteGraphicsParagraph::~GuiRemoteGraphicsParagraph()
{
	if (id != -1)
	{
		renderTarget->UnregisterParagraph(id);
		id = -1;
	}
}

vint GuiRemoteGraphicsParagraph::GetParagraphId() const
{
	return id;
}

bool GuiRemoteGraphicsParagraph::EnsureRemoteParagraphSynced()
{
	if (!needUpdate)
	{
		return id != -1;
	}

	if (id == -1)
	{
		return false;
	}

	stagedRuns.Clear();
	MergeRuns(textRuns, inlineObjectRuns, stagedRuns);

	remoteprotocol::ElementDesc_DocumentParagraph desc;
	if (committedRuns.Count() == 0) desc.text = text;
	desc.wrapLine = wrapLine;
	desc.maxWidth = maxWidth;
	desc.alignment = remoteprotocol::ElementHorizontalAlignment(paragraphAlignment);
	DiffRuns(committedRuns, stagedRuns, desc);

	auto& messages = renderTarget->GetRemoteMessages();
	vint requestId = messages.RequestRendererUpdateElement_DocumentParagraph(id, desc);
	bool disconnected = false;
	messages.Submit(disconnected);
	if (disconnected)
	{
		return false;
	}

	cachedSize = messages.RetrieveRendererUpdateElement_DocumentParagraph(requestId);
	committedRuns = stagedRuns;
	needUpdate = false;
	return true;
}

bool GuiRemoteGraphicsParagraph::TryBuildCaretRange(vint start, vint length, CaretRange& range)
{
	if (length <= 0 || start < 0 || start + length > text.Length()) return false;
	range.caretBegin = NativeTextPosToRemoteTextPos(start);
	range.caretEnd = NativeTextPosToRemoteTextPos(start + length);
	return true;
}

void GuiRemoteGraphicsParagraph::MarkParagraphDirty(bool invalidateSize)
{
	needUpdate = true;
	if (invalidateSize)
	{
		cachedSize = Size(0, 0);
	}
}

vint GuiRemoteGraphicsParagraph::NativeTextPosToRemoteTextPos(vint textPos)
{
	return textPos;
}

vint GuiRemoteGraphicsParagraph::RemoteTextPosToNativeTextPos(vint textPos)
{
	return textPos;
}
```

*Why*: Registering paragraphs at construction keeps ids stable for every call site, and the boolean `EnsureRemoteParagraphSynced` lets higher layers terminate work gracefully if the remote controller disconnects.

## STEP 4: Handle property and run mutations
Replace each `CHECK_FAIL` in the property and run mutator methods with real logic that touches the staging maps, updates inline-object caches, and flags the paragraph as dirty.

```cpp
bool GuiRemoteGraphicsParagraph::SetFont(vint start, vint length, const WString& value)
{
	CaretRange range;
	if (!TryBuildCaretRange(start, length, range)) return false;

	DocumentTextRunPropertyOverrides overrides;
	overrides.fontFamily = value;
	AddTextRun(textRuns, range, overrides);
	MarkParagraphDirty(true);
	return true;
}

bool GuiRemoteGraphicsParagraph::SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)
{
	CaretRange range;
	if (!TryBuildCaretRange(start, length, range)) return false;

	elements_remoteprotocol::IGuiRemoteProtocolElementRender* remoteRenderer = nullptr;
	vint backgroundElementId = -1;
	auto renderer = properties.backgroundImage ? properties.backgroundImage->GetRenderer() : nullptr;
	if (renderer)
	{
		remoteRenderer = dynamic_cast<elements_remoteprotocol::IGuiRemoteProtocolElementRender*>(renderer);
		if (remoteRenderer)
		{
			backgroundElementId = remoteRenderer->GetID();
		}
	}

	remoteprotocol::DocumentInlineObjectRunProperty remoteProp;
	remoteProp.size = properties.size;
	remoteProp.baseline = properties.baseline;
	remoteProp.breakCondition = properties.breakCondition;
	remoteProp.callbackId = properties.callbackId;
	remoteProp.backgroundElementId = backgroundElementId;

	if (!AddInlineObjectRun(inlineObjectRuns, range, remoteProp)) return false;

	if (renderer)
	{
		renderer->SetRenderTarget(renderTarget);
	}

	MarkParagraphDirty(true);
	return true;
}

bool GuiRemoteGraphicsParagraph::ResetInlineObject(vint start, vint length)
{
	CaretRange range;
	if (!TryBuildCaretRange(start, length, range)) return false;
	if (!ResetInlineObjectRun(inlineObjectRuns, range)) return false;
	MarkParagraphDirty(true);
	return true;
}
```

*Why*: These implementations apply the run helpers from Tasks 6-7, reuse the new batching support, and only set up background renderers after the inline object run is accepted, avoiding redundant work while still forcing the next sync. By capturing the background element id before inserting the run we avoid mutating the dictionary entry (which is exposed as a const reference) and still ensure the renderer is bound to the current target. Inline-object bookkeeping now stays within the protocol run dictionaries, matching the simplified state model.

## STEP 5: Implement querying, caret control, and rendering
Turn the remaining `CHECK_FAIL`s into real remote protocol calls. Each query first attempts to synchronize via the new boolean `EnsureRemoteParagraphSynced`; if it reports failure the method returns cached or conservative fallbacks immediately. Otherwise we invoke the appropriate `GuiRemoteMessages` request, handle disconnections gracefully, and translate caret positions through the conversion helpers. Rendering delegates to the remote renderer exactly like other remote elements.

```cpp
Size GuiRemoteGraphicsParagraph::GetSize()
{
	if (!EnsureRemoteParagraphSynced())
	{
		return cachedSize;
	}
	return cachedSize;
}

vint GuiRemoteGraphicsParagraph::GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)
{
	if (!EnsureRemoteParagraphSynced())
	{
		return comparingCaret;
	}
	auto& messages = renderTarget->GetRemoteMessages();
	remoteprotocol::GetCaretRequest request;
	request.caret = NativeTextPosToRemoteTextPos(comparingCaret);
	request.relativePosition = position;
	vint requestId = messages.RequestDocumentParagraph_GetCaret(id, request);
	bool disconnected = false;
	messages.Submit(disconnected);
	if (disconnected) return comparingCaret;
	auto response = messages.RetrieveDocumentParagraph_GetCaret(requestId);
	preferFrontSide = response.preferFrontSide;
	return RemoteTextPosToNativeTextPos(response.newCaret);
}

bool GuiRemoteGraphicsParagraph::OpenCaret(vint caret, Color color, bool frontSide)
{
	if (!EnsureRemoteParagraphSynced())
	{
		return false;
	}
	remoteprotocol::OpenCaretRequest request;
	request.caret = NativeTextPosToRemoteTextPos(caret);
	request.caretColor = color;
	request.frontSide = frontSide;
	auto& messages = renderTarget->GetRemoteMessages();
	messages.RequestDocumentParagraph_OpenCaret(id, request);
	bool disconnected = false;
	messages.Submit(disconnected);
	return !disconnected;
}

void GuiRemoteGraphicsParagraph::Render(Rect bounds)
{
	if (id == -1 || !EnsureRemoteParagraphSynced()) return;
	remoteprotocol::ElementRendering rendering;
	rendering.id = id;
	rendering.bounds = bounds;
	rendering.areaClippedByParent = renderTarget->GetClipperValidArea();
	auto& messages = renderTarget->GetRemoteMessages();
	messages.RequestRendererRenderElement(rendering);
	lastRenderedBatchId = renderTarget->renderingBatchId;
}
```

*Why*: These methods are the core behaviors consumed by the document renderer and downstream controls. Synchronizing lazily avoids redundant round-trips, while the disconnection checks keep the UI resilient if the remote controller drops.

# TEST PLAN
- Extend `TestRemote_DocumentRunManagement` to cover paragraph registration/unregistration on the render target, inline-object run diffs with background elements, and the new bool fallback path of `EnsureRemoteParagraphSynced`; run via `& .github/TaskLogs/copilotBuild.ps1 -Test TestRemote_DocumentRunManagement`.
- Execute the broader remote renderer suite (`copilotExecute.ps1` with the remote graphics scenario) to validate message sequencing across caret navigation, destruction batching, and rendering flows.
- Perform a manual sanity check by opening a sample document in the remote viewer to confirm text styling, inline images, and caret behavior replicate the desktop renderer even after a remote disconnect/reconnect cycle.

# !!!FINISHED!!!

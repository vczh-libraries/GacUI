# !!!EXECUTION!!!
# FIXING ATTEMPTS

## Fixing attempt No.1

### Analysis of Build Errors

The build failed with 93 errors, all in `GuiRemoteGraphics_Document.cpp` across three projects (Lib_GacUI, Lib_GacUI_Metaonly, Lib_GacUI_Reflection). The errors fall into several categories:

1. **Missing include for `GuiRemoteMessages`**: The file uses `GuiRemoteMessages` class but doesn't include `GuiRemoteEvents.h` where it's defined, causing "use of undefined type" errors.

2. **Inaccessible base class conversion (line 693)**: `GuiRemoteGraphicsResourceManager` inherits from `IGuiGraphicsLayoutProvider` with `protected` access, but the code tries to convert to it with `public` access.

3. **Deleted copy assignment operator (line 656)**: Attempting to use copy assignment on a `Dictionary` which has it explicitly deleted.

### Root Cause

The original implementation plan included all the necessary code, but:
- Missing `#include "GuiRemoteEvents.h"` in `GuiRemoteGraphics_Document.cpp`
- The `GetProvider()` method returns `resourceManager` cast to `IGuiGraphicsLayoutProvider*`, but the inheritance is `protected`, not `public`
- Line 656 tries to assign dictionaries with `committedRuns = stagedRuns` but Dictionary's copy assignment is deleted - need to use move semantics

### Fix Strategy

1. Add `#include "GuiRemoteEvents.h"` to make `GuiRemoteMessages` fully defined
2. Change `GuiRemoteGraphicsResourceManager`'s inheritance to use `public` for `IGuiGraphicsLayoutProvider`
3. Fix line 656 to use move semantics: `committedRuns = std::move(stagedRuns)` or create a new dictionary and swap

### Why This Should Work

- Including the header will provide the full definition of `GuiRemoteMessages`, resolving all "undefined type" errors
- Making the inheritance public will allow the conversion in `GetProvider()`
- Using move semantics or proper dictionary operations will resolve the assignment issue

**Result:** Errors reduced from 93 to 21 ✓

---

## Fixing attempt No.2

### Analysis of Remaining Build Errors

Down from 93 to 21 errors! All remaining errors are about wrong number of arguments to `GuiRemoteMessages` methods. The code is passing paragraph `id` as first argument, but the functions only take the request parameter.

Examples:
- `messages.RequestRendererUpdateElement_DocumentParagraph(id, desc)` - function takes 1 arg, not 2
- `messages.RequestDocumentParagraph_GetCaret(id, request)` - function takes 1 arg, not 2
- Same pattern for all 7 errors

### Root Cause

Inspecting the protocol definition files and comparing with BasicElements:
- `ElementDesc_SolidBorder` in Protocol_Renderer_BasicElements.txt HAS `var id : int;` field
- `ElementDesc_DocumentParagraph` in Protocol_Renderer_Document.txt is MISSING the `id` field!

The protocol definition is incomplete. All `ElementDesc_*` structures need an `id` field so the remote side knows which element to update.

### Fix Strategy

Add `var id : int;` as the first field in `ElementDesc_DocumentParagraph` in the protocol file `Protocol_Renderer_Document.txt`, then regenerate the C++ schema file. After that, the code will compile because `desc.id = id` will be valid and the function calls won't need the separate id parameter.

### Why This Should Work

- All other element descriptors have `id` fields
- The C++ implementation already tries to set `desc.id = id` in the planned code (from IMPROVEMENT PLAN)
- Once the protocol is updated and regenerated, the generated C++ struct will have the `id` field
- The function signatures will remain taking just the request parameter, but the request will contain the id

**Result:** Build succeeded with 0 errors ✓

---

## Build Success Summary

All compilation errors have been resolved through 2 fixing attempts:
- **Attempt 1**: Fixed missing includes, inheritance visibility, and move semantics
- **Attempt 2**: Corrected protocol schema to include `id` field and updated function calls

**Final Result:** Build succeeded - 0 Error(s)

# UPDATES
# IMPROVEMENT PLAN
## STEP 1: Centralize Paragraph Registration in Render Target [DONE]
- `Source/PlatformProviders/Remote/GuiRemoteGraphics.h` (forward declaration)
```cpp
namespace vl::presentation::elements
{
	class GuiRemoteGraphicsParagraph;
```
- `Source/PlatformProviders/Remote/GuiRemoteGraphics.h` (render target members and APIs)
```cpp
		class GuiRemoteGraphicsRenderTarget : public GuiGraphicsRenderTarget
		{
			using RendererMap = collections::Dictionary<vint, elements_remoteprotocol::IGuiRemoteProtocolElementRender*>;
			using RendererSet = collections::SortedList<elements_remoteprotocol::IGuiRemoteProtocolElementRender*>;
			using FontHeightMap = collections::Dictionary<Tuple<WString, vint>, vint>;
			using HitTestResult = INativeWindowListener::HitTestResult;
			using SystemCursorType = INativeCursor::SystemCursorType;
		protected:
			GuiRemoteController*				remote;
			GuiHostedController*				hostedController;
			NativeSize							canvasSize;
			vint								usedFrameIds = 0;
			vint								usedElementIds = 0;
			vint								usedCompositionIds = 0;
			RendererMap							renderers;
			collections::SortedList<vint>		createdRenderers;
			collections::SortedList<vint>		destroyedRenderers;
			RendererSet							renderersAskingForCache;
			Nullable<Rect>						clipperValidArea;
			collections::List<HitTestResult>	hitTestResults;
			collections::List<SystemCursorType>	cursors;
			collections::Dictionary<vint, GuiRemoteGraphicsParagraph*>	paragraphs;
			collections::List<remoteprotocol::RendererCreation>			pendingParagraphCreations;

			HitTestResult						GetHitTestResultFromGenerator(reflection::DescriptableObject* generator);
			Nullable<SystemCursorType>			GetCursorFromGenerator(reflection::DescriptableObject* generator);

			void								StartRenderingOnNativeWindow() override;
			RenderTargetFailure					StopRenderingOnNativeWindow() override;
```
- `Source/PlatformProviders/Remote/GuiRemoteGraphics.h` (render target public methods)
```cpp
			GuiRemoteMessages&					GetRemoteMessages();
			vint								AllocateNewElementId();
			void								RegisterRenderer(elements_remoteprotocol::IGuiRemoteProtocolElementRender* renderer);
			void								UnregisterRenderer(elements_remoteprotocol::IGuiRemoteProtocolElementRender* renderer);
			Rect								GetClipperValidArea();
			GuiRemoteGraphicsParagraph*			GetParagraph(vint id);
			void								RegisterParagraph(GuiRemoteGraphicsParagraph* paragraph);
			void								UnregisterParagraph(vint id);
```
- `Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp` (`StartRenderingOnNativeWindow`)
```cpp
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
```
- `Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp` (paragraph registry helpers)
```cpp
Rect GuiRemoteGraphicsRenderTarget::GetClipperValidArea()
{
	return clipperValidArea.Value();
}

GuiRemoteGraphicsParagraph* GuiRemoteGraphicsRenderTarget::GetParagraph(vint id)
{
	vint index = paragraphs.Keys().IndexOf(id);
	return index == -1 ? nullptr : paragraphs.Values()[index];
}

void GuiRemoteGraphicsRenderTarget::RegisterParagraph(GuiRemoteGraphicsParagraph* paragraph)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiRemoteGraphicsRenderTarget::RegisterParagraph(GuiRemoteGraphicsParagraph*)#"
	auto paragraphId = paragraph->GetParagraphId();
	CHECK_ERROR(!paragraphs.Keys().Contains(paragraphId), ERROR_MESSAGE_PREFIX L"Duplicated paragraph id.");
	paragraphs.Add(paragraphId, paragraph);
	pendingParagraphCreations.Add({ paragraphId,remoteprotocol::RendererType::DocumentParagraph });
#undef ERROR_MESSAGE_PREFIX
}

void GuiRemoteGraphicsRenderTarget::UnregisterParagraph(vint id)
{
	vint index = paragraphs.Keys().IndexOf(id);
	if (index == -1)
	{
		return;
	}

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
```
## STEP 2: Enrich GuiRemoteGraphicsParagraph state and helpers [DONE]
- `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h`
```cpp
	protected:
		WString									text;
		GuiRemoteController*					remote = nullptr;
		GuiRemoteGraphicsResourceManager*		resourceManager = nullptr;
		GuiRemoteGraphicsRenderTarget*			renderTarget = nullptr;
		IGuiGraphicsParagraphCallback*			callback = nullptr;

		DocumentTextRunPropertyMap					textRuns;
		DocumentInlineObjectRunPropertyMap			inlineObjectRuns;
		DocumentRunPropertyMap						stagedRuns;
		DocumentRunPropertyMap						committedRuns;
		collections::Dictionary<CaretRange, InlineObjectProperties>	inlineObjectProperties;
		bool										wrapLine = false;
		vint										maxWidth = -1;
		Alignment									paragraphAlignment = Alignment::Left;
		Size										cachedSize = Size(0, 0);
		bool										needUpdate = true;
		vint										id = -1;
		vuint64_t									lastRenderedBatchId = 0;

	public:
		GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback);
		~GuiRemoteGraphicsParagraph();

		vint											GetParagraphId() const;

	protected:
		vint											NativeTextPosToRemoteTextPos(vint textPos);
		vint											RemoteTextPosToNativeTextPos(vint textPos);
		bool											TryBuildCaretRange(vint start, vint length, CaretRange& range);
		bool											EnsureRemoteParagraphSynced();
		void											MarkParagraphDirty(bool invalidateSize);
```
## STEP 3: Implement lifecycle, synchronization, and helpers [DONE]
- `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp` (include list)
```cpp
#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"
#include "GuiRemoteGraphics_BasicElements.h"
```
- `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp` (constructor, destructor, helpers)
```cpp
	GuiRemoteGraphicsParagraph::GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback)
		: text(_text)
		, remote(_remote)
		, resourceManager(_resourceManager)
		, renderTarget(_renderTarget)
		, callback(_callback)
	{
		if (renderTarget)
		{
			id = renderTarget->AllocateNewElementId();
			renderTarget->RegisterParagraph(this);
		}
	}

	GuiRemoteGraphicsParagraph::~GuiRemoteGraphicsParagraph()
	{
		if (renderTarget && id != -1)
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

		if (id == -1 || !renderTarget)
		{
			return false;
		}

		stagedRuns.Clear();
		MergeRuns(textRuns, inlineObjectRuns, stagedRuns);

		remoteprotocol::ElementDesc_DocumentParagraph desc;
		if (committedRuns.Count() == 0)
		{
			desc.text = text;
		}

		switch (paragraphAlignment)
		{
		case Alignment::Right:
			desc.alignment = remoteprotocol::ElementHorizontalAlignment::Right;
			break;
		case Alignment::Center:
			desc.alignment = remoteprotocol::ElementHorizontalAlignment::Center;
			break;
		default:
			desc.alignment = remoteprotocol::ElementHorizontalAlignment::Left;
			break;
		}

		desc.wrapLine = wrapLine;
		desc.maxWidth = maxWidth;
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
		if (length <= 0 || start < 0 || start + length > text.Length())
		{
			return false;
		}
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
## STEP 4: Handle property and run mutations [DONE]
- `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp`
```cpp
	IGuiGraphicsLayoutProvider* GuiRemoteGraphicsParagraph::GetProvider()
	{
		return resourceManager;
	}

	IGuiGraphicsRenderTarget* GuiRemoteGraphicsParagraph::GetRenderTarget()
	{
		return renderTarget;
	}

	bool GuiRemoteGraphicsParagraph::GetWrapLine()
	{
		return wrapLine;
	}

	void GuiRemoteGraphicsParagraph::SetWrapLine(bool value)
	{
		if (wrapLine != value)
		{
			wrapLine = value;
			MarkParagraphDirty(true);
		}
	}

	vint GuiRemoteGraphicsParagraph::GetMaxWidth()
	{
		return maxWidth;
	}

	void GuiRemoteGraphicsParagraph::SetMaxWidth(vint value)
	{
		if (maxWidth != value)
		{
			maxWidth = value;
			MarkParagraphDirty(true);
		}
	}

	Alignment GuiRemoteGraphicsParagraph::GetParagraphAlignment()
	{
		return paragraphAlignment;
	}

	void GuiRemoteGraphicsParagraph::SetParagraphAlignment(Alignment value)
	{
		if (paragraphAlignment != value)
		{
			paragraphAlignment = value;
			MarkParagraphDirty(true);
		}
	}

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

	bool GuiRemoteGraphicsParagraph::SetSize(vint start, vint length, vint value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.size = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetStyle(vint start, vint length, TextStyle value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.textStyle = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetColor(vint start, vint length, Color value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.textColor = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetBackgroundColor(vint start, vint length, Color value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.backgroundColor = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		vint backgroundElementId = -1;
		IGuiGraphicsRenderer* renderer = properties.backgroundImage ? properties.backgroundImage->GetRenderer() : nullptr;
		if (renderer)
		{
			renderer->SetRenderTarget(renderTarget);
			if (auto remoteRenderer = dynamic_cast<elements_remoteprotocol::IGuiRemoteProtocolElementRender*>(renderer))
			{
				backgroundElementId = remoteRenderer->GetID();
			}
		}

		remoteprotocol::DocumentInlineObjectRunProperty remoteProp;
		remoteProp.size = properties.size;
		remoteProp.baseline = properties.baseline;
		remoteProp.breakCondition = properties.breakCondition;
		remoteProp.backgroundElementId = backgroundElementId;
		remoteProp.callbackId = properties.callbackId;

		if (!AddInlineObjectRun(inlineObjectRuns, range, remoteProp))
		{
			return false;
		}

		inlineObjectProperties.Remove(range);
		inlineObjectProperties.Add(range, properties);
		MarkParagraphDirty(true);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::ResetInlineObject(vint start, vint length)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;
		if (!ResetInlineObjectRun(inlineObjectRuns, range)) return false;

		vint index = inlineObjectProperties.Keys().IndexOf(range);
		if (index != -1)
		{
			auto stored = inlineObjectProperties.Values()[index];
			if (stored.backgroundImage)
			{
				if (auto renderer = stored.backgroundImage->GetRenderer())
				{
					renderer->SetRenderTarget(nullptr);
				}
			}
			inlineObjectProperties.Remove(range);
		}

		MarkParagraphDirty(true);
		return true;
	}
```
## STEP 5: Implement querying, caret control, and rendering [DONE]
- `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp`
```cpp
	Size GuiRemoteGraphicsParagraph::GetSize()
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return cachedSize;
		}
		return cachedSize;
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
		messages.RequestDocumentParagraph_OpenCaret(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		return !disconnected;
	}

	bool GuiRemoteGraphicsParagraph::CloseCaret()
	{
		if (id == -1 || !renderTarget)
		{
			return false;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestDocumentParagraph_CloseCaret();
		bool disconnected = false;
		messages.Submit(disconnected);
		return !disconnected;
	}

	void GuiRemoteGraphicsParagraph::Render(Rect bounds)
	{
		if (!renderTarget)
		{
			return;
		}

		if (!EnsureRemoteParagraphSynced())
		{
			return;
		}

		remoteprotocol::ElementRendering rendering;
		rendering.id = id;
		rendering.bounds = bounds;
		rendering.areaClippedByParent = renderTarget->GetClipperValidArea();

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestRendererRenderElement(rendering);
		lastRenderedBatchId = renderTarget->renderingBatchId;
	}

	vint GuiRemoteGraphicsParagraph::GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			preferFrontSide = false;
			return comparingCaret;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		remoteprotocol::GetCaretRequest request;
		request.caret = NativeTextPosToRemoteTextPos(comparingCaret);
		request.relativePosition = position;

		vint requestId = messages.RequestDocumentParagraph_GetCaret(id, request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			preferFrontSide = false;
			return comparingCaret;
		}

		auto response = messages.RetrieveDocumentParagraph_GetCaret(requestId);
		preferFrontSide = response.preferFrontSide;
		return RemoteTextPosToNativeTextPos(response.newCaret);
	}

	Rect GuiRemoteGraphicsParagraph::GetCaretBounds(vint caret, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return {};
		}

		remoteprotocol::GetCaretBoundsRequest request;
		request.caret = NativeTextPosToRemoteTextPos(caret);
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(id, request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return {};
		}

		return messages.RetrieveDocumentParagraph_GetCaretBounds(requestId);
	}

	vint GuiRemoteGraphicsParagraph::GetCaretFromPoint(Point point)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return 0;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		vint bestCaret = 0;
		vint bestDistance = -1;

		for (vint caret = 0; caret <= text.Length(); caret++)
		{
			remoteprotocol::GetCaretBoundsRequest request;
			request.caret = NativeTextPosToRemoteTextPos(caret);
			request.frontSide = true;

			bool disconnected = false;
			vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(id, request);
			messages.Submit(disconnected);
			if (disconnected)
			{
				return bestCaret;
			}

			Rect bounds = messages.RetrieveDocumentParagraph_GetCaretBounds(requestId);
			if (bounds.x1 <= point.x && point.x < bounds.x2 && bounds.y1 <= point.y && point.y < bounds.y2)
			{
				return caret;
			}

			vint horizontalDistance = 0;
			if (point.x < bounds.x1) horizontalDistance = bounds.x1 - point.x;
			else if (point.x > bounds.x2) horizontalDistance = point.x - bounds.x2;

			vint verticalDistance = 0;
			if (point.y < bounds.y1) verticalDistance = bounds.y1 - point.y;
			else if (point.y > bounds.y2) verticalDistance = point.y - bounds.y2;

			vint distance = horizontalDistance + verticalDistance;
			if (bestDistance == -1 || distance < bestDistance)
			{
				bestDistance = distance;
				bestCaret = caret;
			}
		}

		return bestCaret;
	}

	Nullable<IGuiGraphicsParagraph::InlineObjectProperties> GuiRemoteGraphicsParagraph::GetInlineObjectFromPoint(Point point, vint& start, vint& length)
	{
		start = 0;
		length = 0;

		if (!EnsureRemoteParagraphSynced())
		{
			return {};
		}

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetInlineObjectFromPoint(id, point);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return {};
		}

		auto response = messages.RetrieveDocumentParagraph_GetInlineObjectFromPoint(requestId);
		if (!response)
		{
			return {};
		}

		CaretRange range;
		range.caretBegin = response.Value().caretBegin;
		range.caretEnd = response.Value().caretEnd;

		vint index = inlineObjectProperties.Keys().IndexOf(range);
		if (index == -1)
		{
			return {};
		}

		start = RemoteTextPosToNativeTextPos(range.caretBegin);
		length = RemoteTextPosToNativeTextPos(range.caretEnd) - start;
		return inlineObjectProperties.Values()[index];
	}

	vint GuiRemoteGraphicsParagraph::GetNearestCaretFromTextPos(vint textPos, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return textPos;
		}

		remoteprotocol::GetCaretBoundsRequest request;
		request.caret = NativeTextPosToRemoteTextPos(textPos);
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetNearestCaretFromTextPos(id, request);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return textPos;
		}

		auto response = messages.RetrieveDocumentParagraph_GetNearestCaretFromTextPos(requestId);
		return RemoteTextPosToNativeTextPos(response);
	}

	bool GuiRemoteGraphicsParagraph::IsValidCaret(vint caret)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return false;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_IsValidCaret(id, NativeTextPosToRemoteTextPos(caret));
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return false;
		}

		return messages.RetrieveDocumentParagraph_IsValidCaret(requestId);
	}

	bool GuiRemoteGraphicsParagraph::IsValidTextPos(vint textPos)
	{
		return 0 <= textPos && textPos <= text.Length();
	}
```
# TEST PLAN
- Extend `TestRemote_DocumentRunManagement` to cover registration lifecycle, inline-object synchronization, and `EnsureRemoteParagraphSynced` failure paths; run via `& .github/TaskLogs/copilotBuild.ps1 -Test TestRemote_DocumentRunManagement`.
- Execute the remote graphics scenario with `copilotExecute.ps1` to validate message batching, caret operations, and render submissions end-to-end.
- Manually verify a sample document through the remote viewer to ensure formatting, inline objects, and caret navigation remain functional after remote disconnect/reconnect.

# !!!FINISHED!!!

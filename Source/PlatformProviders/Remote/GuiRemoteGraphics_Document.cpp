#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"
#include "GuiRemoteGraphics_BasicElements.h"
#include "GuiRemoteEvents.h"

namespace vl::presentation::elements
{
	using namespace collections;

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

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

		renderTarget->EnsureRequestedRenderersCreated();

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
		desc.id = id;
		DiffRuns(committedRuns, stagedRuns, desc);

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestRendererUpdateElement_DocumentParagraph(desc);
		bool disconnected = false;
		messages.Submit(disconnected);
		if (disconnected)
		{
			return false;
		}

		{
			auto response = messages.RetrieveRendererUpdateElement_DocumentParagraph(requestId);
			cachedSize = response.documentSize;
			cachedInlineObjectBounds = response.inlineObjectBounds;
		}
		committedRuns = std::move(stagedRuns);
		needUpdate = false;

		if (needUpdateCaretBoundsCache)
		{
			needUpdateCaretBoundsCache = false;
			ResetCaretBoundsCache();
		}
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

	void GuiRemoteGraphicsParagraph::ResetCaretBoundsCache()
	{
		caretBoundsFrontSide.Resize(text.Length() + 1);
		caretBoundsBackSide.Resize(text.Length() + 1);
		size_t size = (size_t)(sizeof(Rect) * (text.Length() + 1));
		memset(&caretBoundsFrontSide[0], 0, size);
		memset(&caretBoundsBackSide[0], 0, size);
	}

	void GuiRemoteGraphicsParagraph::MarkParagraphDirty(bool invalidateSize, bool invalidateCaretBoundsCache)
	{
		needUpdate = true;
		if (invalidateSize)
		{
			cachedSize = Size(0, 0);
		}
		if (invalidateCaretBoundsCache)
		{
			needUpdateCaretBoundsCache = true;
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
			MarkParagraphDirty(true, true);
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
			MarkParagraphDirty(true, true);
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
			MarkParagraphDirty(true, true);
		}
	}

	bool GuiRemoteGraphicsParagraph::SetFont(vint start, vint length, const WString& value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.fontFamily = value;
		bool updated = AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true, updated);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetSize(vint start, vint length, vint value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.size = value;
		bool updated = AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true, updated);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetStyle(vint start, vint length, TextStyle value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.textStyle = value;
		bool updated = AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true, updated);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetColor(vint start, vint length, Color value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.textColor = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true, false);
		return true;
	}

	bool GuiRemoteGraphicsParagraph::SetBackgroundColor(vint start, vint length, Color value)
	{
		CaretRange range;
		if (!TryBuildCaretRange(start, length, range)) return false;

		DocumentTextRunPropertyOverrides overrides;
		overrides.backgroundColor = value;
		AddTextRun(textRuns, range, overrides);
		MarkParagraphDirty(true, false);
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
		MarkParagraphDirty(true, true);
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

		MarkParagraphDirty(true, true);
		return true;
	}

	Size GuiRemoteGraphicsParagraph::GetSize()
	{
		EnsureRemoteParagraphSynced();
		return cachedSize;
	}

	bool GuiRemoteGraphicsParagraph::EnableCaret(vint caret, Color color, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return false;
		}

		remoteprotocol::OpenCaretRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(caret);
		request.caretColor = color;
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestDocumentParagraph_OpenCaret(request);
		bool disconnected = false;
		messages.Submit(disconnected);
		return !disconnected;
	}

	void GuiRemoteGraphicsParagraph::DisableCaret()
	{
		if (id == -1 || !renderTarget)
		{
			return;
		}

		auto& messages = renderTarget->GetRemoteMessages();
		messages.RequestDocumentParagraph_CloseCaret(id);
		bool disconnected = false;
		messages.Submit(disconnected);
	}

	bool GuiRemoteGraphicsParagraph::BlinkCaret()
	{
		return false;
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

		if (callback && cachedInlineObjectBounds)
		{
			for (auto [callbackId, location] : *cachedInlineObjectBounds.Obj())
			{
				auto newSize = callback->OnRenderInlineObject(callbackId, location);
				if (newSize != location.GetSize())
				{
					MarkParagraphDirty(false, true);
					for (auto&& inlineObjectRun : inlineObjectRuns.Values())
					{
						if (inlineObjectRun.callbackId == callbackId)
						{
							auto& editable = const_cast<remoteprotocol::DocumentInlineObjectRunProperty&>(inlineObjectRun);
							editable.size = newSize;
							break;
						}
					}
				}
			}
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
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(comparingCaret);
		request.relativePosition = position;

		vint requestId = messages.RequestDocumentParagraph_GetCaret(request);
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

		Rect& cached = frontSide ? caretBoundsFrontSide[caret] : caretBoundsBackSide[caret];
		if (cached == Rect{})
		{
			remoteprotocol::GetCaretBoundsRequest request;
			request.id = id;
			request.caret = NativeTextPosToRemoteTextPos(caret);
			request.frontSide = frontSide;

			auto& messages = renderTarget->GetRemoteMessages();
			vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(request);
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected)
			{
				return {};
			}

			cached = messages.RetrieveDocumentParagraph_GetCaretBounds(requestId);
		}

		return cached;
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
			request.id = id;
			request.caret = NativeTextPosToRemoteTextPos(caret);
			request.frontSide = true;

			bool disconnected = false;
			vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(request);
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

		remoteprotocol::GetInlineObjectFromPointRequest request;
		request.id = id;
		request.point = point;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetInlineObjectFromPoint(request);
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
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(textPos);
		request.frontSide = frontSide;

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_GetNearestCaretFromTextPos(request);
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

		remoteprotocol::IsValidCaretRequest request;
		request.id = id;
		request.caret = NativeTextPosToRemoteTextPos(caret);

		auto& messages = renderTarget->GetRemoteMessages();
		vint requestId = messages.RequestDocumentParagraph_IsValidCaret(request);
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
}

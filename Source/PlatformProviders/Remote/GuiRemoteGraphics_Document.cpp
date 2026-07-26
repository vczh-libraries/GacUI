#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"
#include "GuiRemoteGraphics_BasicElements.h"
#include "GuiRemoteController.h"

namespace vl::presentation::elements
{
	using namespace collections;

/***********************************************************************
GuiRemoteCaretCache
***********************************************************************/

	GuiRemoteCaretCache::GuiRemoteCaretCache(const WString& text, remoteprotocol::CharacterEncoding encoding)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiRemoteCaretCache::GuiRemoteCaretCache(const WString&, remoteprotocol::CharacterEncoding)#"
		CHECK_ERROR(
			encoding == remoteprotocol::CharacterEncoding::UTF8 ||
			encoding == remoteprotocol::CharacterEncoding::UTF16 ||
			encoding == remoteprotocol::CharacterEncoding::UTF32,
			ERROR_MESSAGE_PREFIX L"Unknown character encoding.");

		List<vint> nativeMapping;
		List<vint> remoteMapping;
		vint nativePos = 0;
		vint remotePos = 0;
		while (nativePos < text.Length())
		{
			char32_t scalar = 0;
			auto nativeLength = vl::encoding::UtfConversion<wchar_t>::To32(
				text.Buffer() + nativePos,
				text.Length() - nativePos,
				scalar);

			vint remoteLength = 1;
			if (nativeLength == -1)
			{
				nativeLength = 1;
			}
			else
			{
				switch (encoding)
				{
				case remoteprotocol::CharacterEncoding::UTF8:
					{
						char8_t buffer[vl::encoding::UtfConversion<char8_t>::BufferLength];
						remoteLength = vl::encoding::UtfConversion<char8_t>::From32(scalar, buffer);
					}
					break;
				case remoteprotocol::CharacterEncoding::UTF16:
					{
						char16_t buffer[vl::encoding::UtfConversion<char16_t>::BufferLength];
						remoteLength = vl::encoding::UtfConversion<char16_t>::From32(scalar, buffer);
					}
					break;
				case remoteprotocol::CharacterEncoding::UTF32:
					remoteLength = 1;
					break;
				}
				if (remoteLength == -1)
				{
					remoteLength = 1;
				}
			}

			for (vint i = 0; i < nativeLength; i++)
			{
				nativeMapping.Add(remotePos);
			}
			for (vint i = 0; i < remoteLength; i++)
			{
				remoteMapping.Add(nativePos);
			}
			nativePos += nativeLength;
			remotePos += remoteLength;
		}
		nativeMapping.Add(remotePos);
		remoteMapping.Add(nativePos);
		CopyFrom(nativeToRemote, nativeMapping);
		CopyFrom(remoteToNative, remoteMapping);
#undef ERROR_MESSAGE_PREFIX
	}

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
		if (!remoteParagraphCreated)
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
		for (vint i = 0; i < desc.runsDiff->Count(); i++)
		{
			auto&& run = (*desc.runsDiff.Obj())[i];
			run.caretBegin = NativeTextPosToRemoteTextPos(run.caretBegin);
			run.caretEnd = NativeTextPosToRemoteTextPos(run.caretEnd);
		}

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
			if (desc.removedInlineObjects)
			{
				for (auto callbackId : *desc.removedInlineObjects.Obj())
				{
					cachedInlineObjectBounds.Remove(callbackId);
				}
			}
		}
		committedRuns = std::move(stagedRuns);
		remoteParagraphCreated = true;
		needUpdate = false;

		if (needUpdateCaretBoundsCache)
		{
			needUpdateCaretBoundsCache = false;
			cachedCaretBounds = {};
		}
		return true;
	}

	void GuiRemoteGraphicsParagraph::ResetRemoteParagraphSyncState()
	{
		committedRuns.Clear();
		cachedInlineObjectBounds.Clear();
		cachedCaretBounds = {};
		remoteParagraphCreated = false;
		needUpdate = true;
		needUpdateCaretBoundsCache = true;
	}

	bool GuiRemoteGraphicsParagraph::TryBuildCaretRange(vint start, vint length, CaretRange& range)
	{
		if (length <= 0 || start < 0 || start + length > text.Length())
		{
			return false;
		}
		range.caretBegin = start;
		range.caretEnd = start + length;
		return true;
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

	void GuiRemoteGraphicsParagraph::SetInlineObjectBounds(vint callbackId, const Rect& bounds)
	{
		cachedInlineObjectBounds.Set(callbackId, bounds);
	}

	GuiRemoteCaretCache* GuiRemoteGraphicsParagraph::GetCaretCache()
	{
		auto encoding = remote->GetGlobalConfig().documentCaretFromEncoding;
#if defined VCZH_WCHAR_UTF16
		constexpr auto nativeEncoding = remoteprotocol::CharacterEncoding::UTF16;
#elif defined VCZH_WCHAR_UTF32
		constexpr auto nativeEncoding = remoteprotocol::CharacterEncoding::UTF32;
#endif
		if (encoding == nativeEncoding)
		{
			return nullptr;
		}

		auto index = caretCaches.Keys().IndexOf(encoding);
		if (index == -1)
		{
			auto cache = Ptr(new GuiRemoteCaretCache(text, encoding));
			caretCaches.Add(encoding, cache);
			return cache.Obj();
		}
		return caretCaches.Values()[index].Obj();
	}

	vint GuiRemoteGraphicsParagraph::NativeTextPosToRemoteTextPos(vint textPos)
	{
		auto cache = GetCaretCache();
		return cache ? cache->nativeToRemote[textPos] : textPos;
	}

	vint GuiRemoteGraphicsParagraph::RemoteTextPosToNativeTextPos(vint textPos)
	{
		auto cache = GetCaretCache();
		return cache ? cache->remoteToNative[textPos] : textPos;
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
			if (wrapLine)
			{
				MarkParagraphDirty(true, true);
			}
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
		remoteProp.backgroundColor = properties.backgroundColor;
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

		if (callback)
		{
			for (auto [callbackId, location] : cachedInlineObjectBounds)
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

	bool GuiRemoteGraphicsParagraph::GetCaretBoundsInternal(vint caret, bool frontSide, Rect& bounds)
	{
		if (!cachedCaretBounds.frontSideBounds || !cachedCaretBounds.backSideBounds)
		{
			remoteprotocol::GetCaretBoundsRequest request;
			request.id = id;

			auto& messages = renderTarget->GetRemoteMessages();
			vint requestId = messages.RequestDocumentParagraph_GetCaretBounds(request);
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected)
			{
				return false;
			}

			cachedCaretBounds = messages.RetrieveDocumentParagraph_GetCaretBounds(requestId);
		}

		auto remoteCaret = NativeTextPosToRemoteTextPos(caret);
		if (frontSide)
		{
			bounds = cachedCaretBounds.frontSideBounds->Get(remoteCaret);
		}
		else
		{
			bounds = cachedCaretBounds.backSideBounds->Get(remoteCaret);
		}
		return true;
	}

	Rect GuiRemoteGraphicsParagraph::GetCaretBounds(vint caret, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return {};
		}

		Rect bounds;
		GetCaretBoundsInternal(caret, frontSide, bounds);
		return bounds;
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
			Rect bounds;
			if (!GetCaretBoundsInternal(caret, true, bounds))
			{
				return bestCaret;
			}

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
		range.caretBegin = RemoteTextPosToNativeTextPos(response.Value().caretBegin);
		range.caretEnd = RemoteTextPosToNativeTextPos(response.Value().caretEnd);

		vint index = inlineObjectProperties.Keys().IndexOf(range);
		if (index == -1)
		{
			return {};
		}

		start = range.caretBegin;
		length = range.caretEnd - start;
		return inlineObjectProperties.Values()[index];
	}

	vint GuiRemoteGraphicsParagraph::GetNearestCaretFromTextPos(vint textPos, bool frontSide)
	{
		if (!EnsureRemoteParagraphSynced())
		{
			return textPos;
		}

		remoteprotocol::GetNearestCaretFromTextPosRequest request;
		request.id = id;
		request.textPos = NativeTextPosToRemoteTextPos(textPos);
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

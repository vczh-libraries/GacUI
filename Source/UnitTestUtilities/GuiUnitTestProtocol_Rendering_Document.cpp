#include "GuiUnitTestProtocol_Rendering.h"
#include "../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl::presentation::unittest
{
	using namespace collections;
	using namespace remoteprotocol;

/***********************************************************************
Helper Functions for Document Paragraph
***********************************************************************/

	constexpr vint DefaultFontSize = 12;
	constexpr vint DefaultLineHeight = 16;

	void ClearCaretLayouts(DocumentParagraphState& state)
	{
		state.caretLayouts.Clear();
		state.caretLayoutKeys.Clear();
	}

	void AddCaretLayout(DocumentParagraphState& state, vint caret, const DocumentParagraphCharLayout& layout)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::AddCaretLayout(DocumentParagraphState&, vint, const DocumentParagraphCharLayout&)#"
		if (state.caretLayoutKeys.Count() > 0)
		{
			CHECK_ERROR(state.caretLayoutKeys[state.caretLayoutKeys.Count() - 1] < caret, ERROR_MESSAGE_PREFIX L"Duplicate or out-of-order caret.");
		}
		state.caretLayoutKeys.Add(caret);
		state.caretLayouts.Add(caret, layout);
#undef ERROR_MESSAGE_PREFIX
	}

	vint FindLayoutCaretLE(const DocumentParagraphState& state, vint textPos)
	{
		if (state.caretLayoutKeys.Count() == 0) return -1;
		vint index = -1;
		auto comparer = [&](const vint& key, const vint& search) -> std::strong_ordering
		{
			return key <=> search;
		};
		auto hit = BinarySearchLambda(&state.caretLayoutKeys[0], state.caretLayoutKeys.Count(), textPos, index, comparer);
		if (hit != -1) return state.caretLayoutKeys[hit];
		if (index < 0) return -1;
		if (state.caretLayoutKeys[index] < textPos) return state.caretLayoutKeys[index];
		if (index == 0) return -1;
		return state.caretLayoutKeys[index - 1];
	}

	vint FindLayoutCaretGT(const DocumentParagraphState& state, vint textPos)
	{
		if (state.caretLayoutKeys.Count() == 0) return -1;
		vint index = -1;
		auto comparer = [&](const vint& key, const vint& search) -> std::strong_ordering
		{
			return key <=> search;
		};
		auto hit = BinarySearchLambda(&state.caretLayoutKeys[0], state.caretLayoutKeys.Count(), textPos, index, comparer);
		if (hit != -1)
		{
			if (hit + 1 < state.caretLayoutKeys.Count()) return state.caretLayoutKeys[hit + 1];
			return -1;
		}
		if (index < 0) return state.caretLayoutKeys[0];
		if (state.caretLayoutKeys[index] > textPos) return state.caretLayoutKeys[index];
		if (index + 1 < state.caretLayoutKeys.Count()) return state.caretLayoutKeys[index + 1];
		return -1;
	}

	vint FindFirstCaretKeyAtOrAfter(const DocumentParagraphState& state, vint textPos)
	{
		if (state.caretLayoutKeys.Count() == 0) return -1;
		vint index = -1;
		auto comparer = [&](const vint& key, const vint& search) -> std::strong_ordering
		{
			return key <=> search;
		};
		auto hit = BinarySearchLambda(&state.caretLayoutKeys[0], state.caretLayoutKeys.Count(), textPos, index, comparer);
		if (hit != -1) return hit;
		if (index < 0) return 0;
		if (state.caretLayoutKeys[index] < textPos)
		{
			if (index + 1 < state.caretLayoutKeys.Count()) return index + 1;
			return -1;
		}
		return index;
	}

	bool TryGetLayoutAtCaret(const DocumentParagraphState& state, vint caret, DocumentParagraphCharLayout& layout)
	{
		if (!state.caretLayoutKeys.Contains(caret)) return false;
		layout = state.caretLayouts[caret];
		return true;
	}

	bool TryGetInlineObjectRangeContaining(const DocumentParagraphState& state, vint caret, elements::CaretRange& range)
	{
		for (auto&& [inlineRange, _] : state.inlineObjectRuns)
		{
			if (caret > inlineRange.caretBegin && caret < inlineRange.caretEnd)
			{
				range = inlineRange;
				return true;
			}
		}
		return false;
	}

	bool TryGetInlineObjectRangeAtBegin(const DocumentParagraphState& state, vint caret, elements::CaretRange& range)
	{
		for (auto&& [inlineRange, _] : state.inlineObjectRuns)
		{
			if (caret == inlineRange.caretBegin)
			{
				range = inlineRange;
				return true;
			}
		}
		return false;
	}

	bool TryGetInlineObjectRangeAtEnd(const DocumentParagraphState& state, vint caret, elements::CaretRange& range)
	{
		for (auto&& [inlineRange, _] : state.inlineObjectRuns)
		{
			if (caret == inlineRange.caretEnd)
			{
				range = inlineRange;
				return true;
			}
		}
		return false;
	}

	bool IsValidCaretPosition(const DocumentParagraphState& state, vint caret)
	{
		if (caret < 0 || caret > state.text.Length()) return false;
		for (auto&& [range, _] : state.inlineObjectRuns)
		{
			if (caret > range.caretBegin && caret < range.caretEnd)
			{
				return false;
			}
		}
		return true;
	}

	vint GetFontSizeForPosition(
		const DocumentParagraphState& state,
		vint pos,
		Nullable<Pair<vint, DocumentInlineObjectRunProperty>>& inlineProp)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::GetFontSizeForPosition(...)#"
		inlineProp.Reset();

		for (auto [range, prop] : state.mergedRuns)
		{
			if (pos >= range.caretBegin && pos < range.caretEnd)
			{
				if (auto textProp = prop.TryGet<DocumentTextRunProperty>())
				{
					return textProp->fontProperties.size;
				}
				if (auto objProp = prop.TryGet<DocumentInlineObjectRunProperty>())
				{
					inlineProp = { range.caretEnd - range.caretBegin,*objProp };
					return 0;
				}
			}
		}
		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Every character is expected to have a font.");
#undef ERROR_MESSAGE_PREFIX
	}

	double GetCharacterWidth(wchar_t c, vint fontSize)
	{
		return (c < 128 ? 0.6 : 1.0) * fontSize;
	}

	void CalculateParagraphLayout(DocumentParagraphState& state)
	{
		ClearCaretLayouts(state);
		state.lines.Clear();
		state.cachedSize = Size(0, DefaultLineHeight);
		state.cachedInlineObjectBounds.Clear();

		const WString& text = state.text;

		if (text.Length() == 0)
		{
			// Empty paragraph has default size
			DocumentParagraphLineInfo line;
			line.startPos = 0;
			line.endPos = 0;
			line.y = 0;
			line.height = DefaultLineHeight; // Default: 12 (font) + 4
			line.baseline = DefaultFontSize;
			line.width = 0;
			state.lines.Add(line);
			return;
		}

		// First pass: calculate per-character metrics
		struct TempCharInfo
		{
			vint caret;
			double x;
			double width;
			vint height;
			vint length;
			Nullable<DocumentInlineObjectRunProperty> inlineObjectProp;
		};
		List<TempCharInfo> tempChars;
		List<Pair<vint, vint>> lineRanges; // [start, end) for each line

		double currentX = 0;
		vint currentLineStart = 0;

		for (vint i = 0; i < text.Length(); i++)
		{
			wchar_t c = text[i];
			TempCharInfo info = { i, currentX, 0, 0, 1, {} };

			// Handle \r - zero width, no line break
			if (c == L'\r')
			{
				tempChars.Add(info);
				continue;
			}

			// Get character properties
			Nullable<Pair<vint, DocumentInlineObjectRunProperty>> inlinePair;
			vint fontSize = GetFontSizeForPosition(state, i, inlinePair);

			if (inlinePair)
			{
				auto& prop = inlinePair.Value().value;
				info.width = (double)prop.size.x;
				info.height = prop.size.y;
				info.length = inlinePair.Value().key;
				info.inlineObjectProp = inlinePair.Value().value;
			}
			else
			{
				if (fontSize <= 0) fontSize = DefaultFontSize;
				info.width = GetCharacterWidth(c, fontSize);
				info.height = fontSize;
			}

			// Handle \n - always break line
			if (c == L'\n')
			{
				info.width = 0;
				tempChars.Add(info);
				lineRanges.Add({ currentLineStart, i + 1 });
				currentLineStart = i + 1;
				currentX = 0;
				continue;
			}

			// Check word wrap
			if (state.wrapLine && state.maxWidth > 0 && currentX > 0)
			{
				if (currentX + info.width > state.maxWidth)
				{
					lineRanges.Add({ currentLineStart, i });
					currentLineStart = i;
					currentX = 0;
				}
			}

			info.x = currentX;
			tempChars.Add(info);
			currentX += info.width;

			if(inlinePair)
			{
				i += inlinePair.Value().key - 1;
			}
		}

		// Add final line
		if (currentLineStart <= text.Length())
		{
			lineRanges.Add({ currentLineStart, text.Length() });
		}

		// Handle empty case
		if (lineRanges.Count() == 0)
		{
			lineRanges.Add({ 0, 0 });
		}

		// Second pass: calculate line heights using baseline alignment
		vint currentY = 0;
		for (auto [lineStart, lineEnd] : lineRanges)
		{
			vint maxAboveBaseline = 0;
			vint maxBelowBaseline = 0;

			for (auto&& info : tempChars)
			{
				if (info.caret < lineStart || info.caret >= lineEnd) continue;
				if (info.inlineObjectProp)
				{
					auto&& prop = info.inlineObjectProp.Value();
					vint baseline = prop.baseline;
					if (baseline == -1)
						baseline = info.height;
					vint above = baseline;
					vint below = info.height - baseline;
					if (above < 0) above = 0;
					if (below < 0) below = 0;
					if (maxAboveBaseline < above)
						maxAboveBaseline = above;
					if (maxBelowBaseline < below)
						maxBelowBaseline = below;
				}
				else
				{
					if (maxAboveBaseline < info.height)
						maxAboveBaseline = info.height;
				}
			}

			DocumentParagraphLineInfo line;
			line.startPos = lineStart;
			line.endPos = lineEnd;
			line.y = currentY;
			line.height = maxAboveBaseline + maxBelowBaseline + 4;
			line.baseline = maxAboveBaseline;

			// Calculate line width
			double lineWidth = 0;
			for (auto&& info : tempChars)
			{
				if (info.caret < lineStart || info.caret >= lineEnd) continue;
				double endX = info.x + info.width;
				if (endX > lineWidth) lineWidth = endX;
			}
			line.width = (vint)lineWidth;

			// Fill inline object bounds

			for (auto&& info : tempChars)
			{
				if (info.caret < lineStart || info.caret >= lineEnd) continue;
				if (info.inlineObjectProp)
				{
					auto&& prop = info.inlineObjectProp.Value();
					if (prop.callbackId != -1)
					{
						vint baseline = prop.baseline;
						if (baseline == -1)
							baseline = info.height;
						vint y = line.y + 2 + line.baseline - baseline;
						state.cachedInlineObjectBounds.Add(prop.callbackId, Rect(Point((vint)info.x, y), prop.size));
					}
				}
			}

			state.lines.Add(line);
			currentY += line.height;
		}

		// Third pass: create final character layouts with line indices
		vint lineIdx = 0;
		for (auto&& info : tempChars)
		{
			while (lineIdx < state.lines.Count() - 1 && info.caret >= state.lines[lineIdx].endPos)
			{
				lineIdx++;
			}
			DocumentParagraphCharLayout cl;
			cl.x = info.x;
			cl.width = info.width;
			cl.lineIndex = lineIdx;
			cl.height = info.height;
			cl.length = info.length;
			cl.baseline = 0;
			cl.isInlineObject = (bool)info.inlineObjectProp;
			AddCaretLayout(state, info.caret, cl);
		}

		// Calculate total size
		vint maxWidth = 0;
		for (auto&& line : state.lines)
		{
			if (line.width > maxWidth) maxWidth = line.width;
		}
		state.cachedSize = Size(maxWidth, currentY > 0 ? currentY : DefaultLineHeight);
	}

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - Document)
***********************************************************************/

	void UnitTestRemoteProtocol_Rendering::Impl_RendererUpdateElement_DocumentParagraph(vint id, const ElementDesc_DocumentParagraph& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_RendererUpdateElement_DocumentParagraph(vint, const ElementDesc_DocumentParagraph&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Paragraph not created.");
		auto state = paragraphStates.Values()[index];

		// Apply text if provided (distinguish null vs empty string)
		if (arguments.text)
		{
			state->text = arguments.text.Value();
			// Text changed - clear run maps since positions may be invalid
			state->textRuns.Clear();
			state->inlineObjectRuns.Clear();
		}

		// Always update these
		state->wrapLine = arguments.wrapLine;
		state->maxWidth = arguments.maxWidth;
		state->alignment = arguments.alignment;

		// Process removed inline objects first
		if (arguments.removedInlineObjects)
		{
			for (auto callbackId : *arguments.removedInlineObjects.Obj())
			{
				// Find the range for this inline object and reset it
				for (auto [range, prop] : state->inlineObjectRuns)
				{
					if (prop.callbackId == callbackId)
					{
						elements::ResetInlineObjectRun(state->inlineObjectRuns, range);
						break;
					}
				}
			}
		}

		// Apply runsDiff using helper functions
		if (arguments.runsDiff)
		{
			for (auto run : *arguments.runsDiff.Obj())
			{
				elements::CaretRange range{ run.caretBegin, run.caretEnd };

				if (auto textProp = run.props.TryGet<DocumentTextRunProperty>())
				{
					elements::DocumentTextRunPropertyOverrides overrides;
					overrides.textColor = textProp->textColor;
					overrides.backgroundColor = textProp->backgroundColor;
					overrides.fontFamily = textProp->fontProperties.fontFamily;
					overrides.size = textProp->fontProperties.size;
					// Convert bool flags back to TextStyle
					elements::IGuiGraphicsParagraph::TextStyle style = (elements::IGuiGraphicsParagraph::TextStyle)0;
					if (textProp->fontProperties.bold) style = (elements::IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)elements::IGuiGraphicsParagraph::TextStyle::Bold);
					if (textProp->fontProperties.italic) style = (elements::IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)elements::IGuiGraphicsParagraph::TextStyle::Italic);
					if (textProp->fontProperties.underline) style = (elements::IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)elements::IGuiGraphicsParagraph::TextStyle::Underline);
					if (textProp->fontProperties.strikeline) style = (elements::IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)elements::IGuiGraphicsParagraph::TextStyle::Strikeline);
					overrides.textStyle = style;
					elements::AddTextRun(state->textRuns, range, overrides);
				}
				else if (auto inlineProp = run.props.TryGet<DocumentInlineObjectRunProperty>())
				{
					bool result = elements::AddInlineObjectRun(state->inlineObjectRuns, range, *inlineProp);
					TEST_ASSERT(result);
				}
			}
		}

		// Merge runs to create final result
		state->mergedRuns.Clear();
		elements::MergeRuns(state->textRuns, state->inlineObjectRuns, state->mergedRuns);

		// Recalculate layout
		CalculateParagraphLayout(*state.Obj());

		{
			index = loggedTrace.createdElements->Keys().IndexOf(arguments.id);
			CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");

			auto rendererType = loggedTrace.createdElements->Values()[index];
			CHECK_ERROR(rendererType == RendererType::DocumentParagraph, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");

			index = lastElementDescs.Keys().IndexOf(arguments.id);
			ElementDesc_DocumentParagraphFull element;
			if (index != -1)
			{
				auto paragraphRef = lastElementDescs.Values()[index].TryGet<ElementDesc_DocumentParagraphFull>();
				CHECK_ERROR(paragraphRef, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");
				element = *paragraphRef;
			}
			element.paragraph = arguments;
			element.paragraph.text = state->text;
			element.paragraph.createdInlineObjects = {};
			element.paragraph.removedInlineObjects = {};
			element.paragraph.runsDiff = Ptr(new List<remoteprotocol::DocumentRun>);
			for (auto [range, props] : state->mergedRuns)
			{
				remoteprotocol::DocumentRun run;
				run.caretBegin = range.caretBegin;
				run.caretEnd = range.caretEnd;
				run.props = props;
				element.paragraph.runsDiff->Add(run);
			}
			lastElementDescs.Set(arguments.id, element);
		}

		// Send response with calculated size and inline object bounds
		UpdateElement_DocumentParagraphResponse response;
		response.documentSize = state->cachedSize;
		GetEvents()->RespondRendererUpdateElement_DocumentParagraph(id, response);

		// Store inlineObjectBounds
		if (state->cachedInlineObjectBounds.Count() > 0)
		{
			if (!measuringForNextRendering.inlineObjectBounds)
			{
				measuringForNextRendering.inlineObjectBounds = Ptr(new List<ElementMeasuring_InlineObjectBounds>);
			}
			for (vint i = 0; i < state->cachedInlineObjectBounds.Count(); i++)
			{
				ElementMeasuring_InlineObjectBounds bounds;
				bounds.elementId = arguments.id;
				bounds.callbackId = state->cachedInlineObjectBounds.Keys()[i];
				bounds.bounds = state->cachedInlineObjectBounds.Values()[i];
				measuringForNextRendering.inlineObjectBounds.Obj()->Add(bounds);
			}
		}
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaretBounds(vint, const GetCaretBoundsRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
		auto state = paragraphStates.Values()[index];

		GetCaretBoundsResponse response;
		response.frontSideBounds = Ptr(new List<Rect>);
		response.backSideBounds = Ptr(new List<Rect>);

		// Handle empty text
		if (state->text.Length() == 0)
		{
			Rect bounds = { 0,0,0,DefaultLineHeight };
			response.frontSideBounds->Add(bounds);
			response.backSideBounds->Add(bounds);
		}
		else
		{
			for (vint caret = 0; caret <= state->text.Length(); caret++)
			{
				DocumentParagraphCharLayout layout;
				if (TryGetLayoutAtCaret(*state.Obj(), caret, layout))
				{
					vint x1 = (vint)layout.x;
					vint y1 = state->lines[layout.lineIndex].y;
					vint x2 = (vint)(layout.x + layout.width);
					vint y2 = y1 + layout.height;
					response.backSideBounds->Add(Rect(x1, y1, x1, y2));
					response.frontSideBounds->Add(Rect(x2, y1, x2, y2));
				}
				else
				{
					vint anchorCaret = FindLayoutCaretLE(*state.Obj(), caret);
					if (anchorCaret == -1) anchorCaret = FindLayoutCaretGT(*state.Obj(), caret);
					if (anchorCaret == -1)
					{
						Rect bounds = { 0,0,0,DefaultLineHeight };
						response.frontSideBounds->Add(bounds);
						response.backSideBounds->Add(bounds);
					}
					else
					{
						auto anchorLayout = state->caretLayouts[anchorCaret];
						vint lineIndex = anchorLayout.lineIndex;
						vint y1 = state->lines[lineIndex].y;
						vint y2 = y1 + state->lines[lineIndex].height;
						vint x = (vint)(anchorLayout.x + anchorLayout.width);
						if (caret <= anchorCaret)
						{
							x = (vint)anchorLayout.x;
						}
						response.backSideBounds->Add(Rect(x, y1, x, y2));
						response.frontSideBounds->Add(Rect(x, y1, x, y2));
					}
				}
			}
		}
		GetEvents()->RespondDocumentParagraph_GetCaretBounds(id, response);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaret(vint id, const GetCaretRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaret(vint, const GetCaretRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
		auto state = paragraphStates.Values()[index];

		vint caret = arguments.caret;
		auto relPos = arguments.relativePosition;
		GetCaretResponse response;
		response.preferFrontSide = true;

		vint textLen = state->text.Length();

		// Clamp caret
		if (caret < 0) caret = 0;
		if (caret > textLen) caret = textLen;

		// Find current line
		vint lineIdx = 0;
		for (vint i = 0; i < state->lines.Count(); i++)
		{
			if (caret >= state->lines[i].startPos && caret <= state->lines[i].endPos)
			{
				lineIdx = i;
				break;
			}
		}

		using CRP = elements::IGuiGraphicsParagraph::CaretRelativePosition;

		switch (relPos)
		{
		case CRP::CaretFirst:
			response.newCaret = 0;
			break;
		case CRP::CaretLast:
			response.newCaret = textLen;
			break;
		case CRP::CaretLineFirst:
			response.newCaret = state->lines[lineIdx].startPos;
			if (!IsValidCaretPosition(*state.Obj(), response.newCaret))
			{
				auto candidate = FindLayoutCaretGT(*state.Obj(), response.newCaret);
				if (candidate != -1 && candidate < state->lines[lineIdx].endPos)
				{
					response.newCaret = candidate;
				}
			}
			break;
		case CRP::CaretLineLast:
			response.newCaret = state->lines[lineIdx].endPos;
			if (response.newCaret > state->lines[lineIdx].startPos && response.newCaret > 0)
			{
				// Don't include CR/LF at end of line
				while (response.newCaret > state->lines[lineIdx].startPos && response.newCaret > 0)
				{
					auto ch = state->text[response.newCaret - 1];
					if (ch == L'\r' || ch == L'\n')
					{
						response.newCaret--;
					}
					else
					{
						break;
					}
				}
			}
			if (!IsValidCaretPosition(*state.Obj(), response.newCaret))
			{
				auto candidate = FindLayoutCaretLE(*state.Obj(), response.newCaret);
				if (candidate != -1 && candidate >= state->lines[lineIdx].startPos)
				{
					response.newCaret = candidate;
				}
			}
			break;
		case CRP::CaretMoveLeft:
			if (caret > 0)
			{
				elements::CaretRange inlineRange;
				if (TryGetInlineObjectRangeAtEnd(*state.Obj(), caret, inlineRange))
				{
					response.newCaret = inlineRange.caretBegin;
				}
				else if (TryGetInlineObjectRangeContaining(*state.Obj(), caret - 1, inlineRange))
				{
					response.newCaret = inlineRange.caretBegin;
				}
				else
				{
					response.newCaret = caret - 1;
				}
			}
			else
			{
				response.newCaret = 0;
			}
			break;
		case CRP::CaretMoveRight:
			if (caret < textLen)
			{
				elements::CaretRange inlineRange;
				if (TryGetInlineObjectRangeAtBegin(*state.Obj(), caret, inlineRange))
				{
					response.newCaret = inlineRange.caretEnd;
				}
				else if (TryGetInlineObjectRangeContaining(*state.Obj(), caret + 1, inlineRange))
				{
					response.newCaret = inlineRange.caretEnd;
				}
				else
				{
					response.newCaret = caret + 1;
				}
			}
			else
			{
				response.newCaret = textLen;
			}
			break;
		case CRP::CaretMoveUp:
			if (lineIdx > 0)
			{
				// Calculate x offset in current line
				vint xOffset = 0;
				if (caret > 0)
				{
					DocumentParagraphCharLayout prevLayout;
					auto layoutCaret = FindLayoutCaretLE(*state.Obj(), caret - 1);
					if (layoutCaret != -1 && TryGetLayoutAtCaret(*state.Obj(), layoutCaret, prevLayout))
					{
						xOffset = (vint)(prevLayout.x + prevLayout.width);
					}
				}
				// Find corresponding position in previous line
				auto& prevLine = state->lines[lineIdx - 1];
				response.newCaret = prevLine.startPos;
				auto keyIndex = FindFirstCaretKeyAtOrAfter(*state.Obj(), prevLine.startPos);
				if (keyIndex != -1)
				{
					for (vint i = keyIndex; i < state->caretLayoutKeys.Count(); i++)
					{
						auto caretKey = state->caretLayoutKeys[i];
						if (caretKey >= prevLine.endPos) break;
						auto& ch = state->caretLayouts[caretKey];
						if (ch.x + ch.width / 2 > xOffset)
							break;
						response.newCaret = caretKey + ch.length;
					}
				}
			}
			else
			{
				response.newCaret = caret;
			}
			break;
		case CRP::CaretMoveDown:
			if (lineIdx < state->lines.Count() - 1)
			{
				// Calculate x offset in current line
				vint xOffset = 0;
				if (caret > 0)
				{
					DocumentParagraphCharLayout prevLayout;
					auto layoutCaret = FindLayoutCaretLE(*state.Obj(), caret - 1);
					if (layoutCaret != -1 && TryGetLayoutAtCaret(*state.Obj(), layoutCaret, prevLayout))
					{
						xOffset = (vint)(prevLayout.x + prevLayout.width);
					}
				}
				// Find corresponding position in next line
				auto& nextLine = state->lines[lineIdx + 1];
				response.newCaret = nextLine.startPos;
				auto keyIndex = FindFirstCaretKeyAtOrAfter(*state.Obj(), nextLine.startPos);
				if (keyIndex != -1)
				{
					for (vint i = keyIndex; i < state->caretLayoutKeys.Count(); i++)
					{
						auto caretKey = state->caretLayoutKeys[i];
						if (caretKey >= nextLine.endPos) break;
						auto& ch = state->caretLayouts[caretKey];
						if (ch.x + ch.width / 2 > xOffset)
							break;
						response.newCaret = caretKey + ch.length;
					}
				}
			}
			else
			{
				response.newCaret = caret;
			}
			break;
		default:
			response.newCaret = caret;
			break;
		}
		while (!IsValidCaretPosition(*state.Obj(), response.newCaret))
		{
			if (response.newCaret < caret)
			{
				auto candidate = FindLayoutCaretLE(*state.Obj(), response.newCaret);
				if (candidate == -1) break;
				if (candidate == response.newCaret) break;
				response.newCaret = candidate;
			}
			else if (response.newCaret > caret)
			{
				auto candidate = FindLayoutCaretGT(*state.Obj(), response.newCaret);
				if (candidate == -1) break;
				if (candidate == response.newCaret) break;
				response.newCaret = candidate;
			}
			else
			{
				break;
			}
		}
		if (!IsValidCaretPosition(*state.Obj(), response.newCaret))
		{
			response.newCaret = caret;
		}

		GetEvents()->RespondDocumentParagraph_GetCaret(id, response);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetNearestCaretFromTextPosRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint, const GetCaretBoundsRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
		auto state = paragraphStates.Values()[index];

		vint textPos = arguments.textPos;
		vint textLen = state->text.Length();

		// Clamp to valid range
		if (textPos < 0) textPos = 0;
		if (textPos > textLen) textPos = textLen;

		if (IsValidCaretPosition(*state.Obj(), textPos))
		{
			GetEvents()->RespondDocumentParagraph_GetNearestCaretFromTextPos(id, textPos);
			return;
		}

		vint caret = -1;
		if (arguments.frontSide && textPos == textLen)
		{
			caret = textLen;
		}
		else if (arguments.frontSide)
		{
			caret = FindLayoutCaretLE(*state.Obj(), textPos);
			if (caret == -1)
			{
				caret = FindLayoutCaretGT(*state.Obj(), textPos);
			}
		}
		else
		{
			caret = FindLayoutCaretGT(*state.Obj(), textPos);
			if (caret == -1)
			{
				caret = FindLayoutCaretLE(*state.Obj(), textPos);
			}
		}
		if (caret == -1) caret = 0;
		GetEvents()->RespondDocumentParagraph_GetNearestCaretFromTextPos(id, caret);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetInlineObjectFromPoint(vint id, const GetInlineObjectFromPointRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetInlineObjectFromPoint(vint, const GetInlineObjectFromPointRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
		auto state = paragraphStates.Values()[index];

		Point pt = arguments.point;
		Nullable<remoteprotocol::DocumentRun> result;

		// Find the line containing the Y coordinate
		vint lineIdx = -1;
		for (vint i = 0; i < state->lines.Count(); i++)
		{
			if (pt.y >= state->lines[i].y && pt.y < state->lines[i].y + state->lines[i].height)
			{
				lineIdx = i;
				break;
			}
		}

		if (lineIdx >= 0)
		{
			auto& line = state->lines[lineIdx];

			// Apply alignment offset
			vint alignmentOffset = 0;
			if (state->alignment == ElementHorizontalAlignment::Center)
			{
				alignmentOffset = (state->cachedSize.x - line.width) / 2;
			}
			else if (state->alignment == ElementHorizontalAlignment::Right)
			{
				alignmentOffset = state->cachedSize.x - line.width;
			}
			vint relativeX = pt.x - alignmentOffset;

			// Check each character in the line
			auto keyIndex = FindFirstCaretKeyAtOrAfter(*state.Obj(), line.startPos);
			if (keyIndex != -1)
			{
				for (vint i = keyIndex; i < state->caretLayoutKeys.Count(); i++)
				{
					auto caretKey = state->caretLayoutKeys[i];
					if (caretKey >= line.endPos) break;
					auto& ch = state->caretLayouts[caretKey];
					if (ch.isInlineObject && relativeX >= ch.x && relativeX < ch.x + ch.width)
					{
						// Found an inline object - look up its properties
						for (auto [range, prop] : state->mergedRuns)
						{
							if (caretKey >= range.caretBegin && caretKey < range.caretEnd)
							{
								if (auto inlineProp = prop.TryGet<DocumentInlineObjectRunProperty>())
								{
									remoteprotocol::DocumentRun run;
									run.caretBegin = range.caretBegin;
									run.caretEnd = range.caretEnd;
									run.props = *inlineProp;
									result = run;
									break;
								}
							}
						}
						break;
					}
				}
			}
		}

		GetEvents()->RespondDocumentParagraph_GetInlineObjectFromPoint(id, result);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_IsValidCaret(vint id, const IsValidCaretRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_IsValidCaret(vint, const IsValidCaretRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		if (index == -1)
		{
			GetEvents()->RespondDocumentParagraph_IsValidCaret(id, false);
			return;
		}
		auto state = paragraphStates.Values()[index];

		vint caret = arguments.caret;

		GetEvents()->RespondDocumentParagraph_IsValidCaret(id, IsValidCaretPosition(*state.Obj(), caret));
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_OpenCaret(const OpenCaretRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_OpenCaret(const OpenCaretRequest&)#"
		vint index = loggedTrace.createdElements->Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");

		auto rendererType = loggedTrace.createdElements->Values()[index];
		CHECK_ERROR(rendererType == RendererType::DocumentParagraph, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");

		index = lastElementDescs.Keys().IndexOf(arguments.id);
		ElementDesc_DocumentParagraphFull element;
		if (index != -1)
		{
			auto paragraphRef = lastElementDescs.Values()[index].TryGet<ElementDesc_DocumentParagraphFull>();
			CHECK_ERROR(paragraphRef, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");
			element = *paragraphRef;
		}
		element.caret = arguments;
		lastElementDescs.Set(arguments.id, element);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_CloseCaret(const vint& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_CloseCaret(const vint&)#"
		vint index = loggedTrace.createdElements->Keys().IndexOf(arguments);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");

		auto rendererType = loggedTrace.createdElements->Values()[index];
		CHECK_ERROR(rendererType == RendererType::DocumentParagraph, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");

		index = lastElementDescs.Keys().IndexOf(arguments);
		ElementDesc_DocumentParagraphFull element;
		if (index != -1)
		{
			auto paragraphRef = lastElementDescs.Values()[index].TryGet<ElementDesc_DocumentParagraphFull>();
			CHECK_ERROR(paragraphRef, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");
			element = *paragraphRef;
		}
		element.caret.Reset();
		lastElementDescs.Set(arguments, element);
#undef ERROR_MESSAGE_PREFIX
	}
}

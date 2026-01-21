#include "GuiUnitTestProtocol_Rendering.h"
#include "../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl::presentation::unittest
{
	using namespace collections;
	using namespace remoteprotocol;

/***********************************************************************
Helper Functions for Document Paragraph
***********************************************************************/

	vint GetFontSizeForPosition(
		const DocumentParagraphState& state,
		vint pos,
		bool& isInlineObject,
		Nullable<DocumentInlineObjectRunProperty>& inlineProp)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::GetFontSizeForPosition(...)#"
		isInlineObject = false;
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
					isInlineObject = true;
					inlineProp = *objProp;
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
		state.characterLayouts.Clear();
		state.lines.Clear();

		const WString& text = state.text;

		if (text.Length() == 0)
		{
			// Empty paragraph has default size
			DocumentParagraphLineInfo line;
			line.startPos = 0;
			line.endPos = 0;
			line.y = 0;
			line.height = 16; // Default: 12 (font) + 4
			line.baseline = 12;
			line.width = 0;
			state.lines.Add(line);
			state.cachedSize = Size(0, 16);
			return;
		}

		// First pass: calculate per-character metrics
		struct TempCharInfo
		{
			double x;
			double width;
			vint height;
			vint baseline;
			bool isInlineObject;
		};
		List<TempCharInfo> tempChars;
		List<Pair<vint, vint>> lineRanges; // [start, end) for each line

		double currentX = 0;
		vint currentLineStart = 0;

		for (vint i = 0; i < text.Length(); i++)
		{
			wchar_t c = text[i];
			TempCharInfo info = { currentX, 0, 0, 0, false };

			// Handle \r - zero width, no line break
			if (c == L'\r')
			{
				tempChars.Add(info);
				continue;
			}

			// Get character properties
			bool isInline = false;
			Nullable<DocumentInlineObjectRunProperty> inlineProp;
			vint fontSize = GetFontSizeForPosition(state, i, isInline, inlineProp);

			if (isInline && inlineProp)
			{
				auto& prop = inlineProp.Value();
				info.width = (double)prop.size.x;
				info.height = prop.size.y;
				info.baseline = (prop.baseline == -1) ? prop.size.y : prop.baseline;
				info.isInlineObject = true;
			}
			else
			{
				if (fontSize <= 0) fontSize = 12;
				info.width = GetCharacterWidth(c, fontSize);
				info.height = fontSize + 4;
				info.baseline = fontSize;
				info.isInlineObject = false;
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

			for (vint i = lineStart; i < lineEnd && i < tempChars.Count(); i++)
			{
				auto& info = tempChars[i];
				if (info.width > 0)
				{
					if (info.baseline > maxAboveBaseline)
						maxAboveBaseline = info.baseline;
					vint belowBaseline = info.height - info.baseline;
					if (belowBaseline > maxBelowBaseline)
						maxBelowBaseline = belowBaseline;
				}
			}

			// Default line height if empty
			if (maxAboveBaseline == 0 && maxBelowBaseline == 0)
			{
				maxAboveBaseline = 12;
				maxBelowBaseline = 4;
			}

			DocumentParagraphLineInfo line;
			line.startPos = lineStart;
			line.endPos = lineEnd;
			line.y = currentY;
			line.height = maxAboveBaseline + maxBelowBaseline;
			line.baseline = maxAboveBaseline;

			// Calculate line width
			double lineWidth = 0;
			for (vint i = lineStart; i < lineEnd && i < tempChars.Count(); i++)
			{
				double endX = tempChars[i].x + tempChars[i].width;
				if (endX > lineWidth) lineWidth = endX;
			}
			line.width = (vint)lineWidth;

			state.lines.Add(line);
			currentY += line.height;
		}

		// Third pass: create final character layouts with line indices
		vint lineIdx = 0;
		for (vint i = 0; i < tempChars.Count(); i++)
		{
			while (lineIdx < state.lines.Count() - 1 && i >= state.lines[lineIdx].endPos)
			{
				lineIdx++;
			}
			DocumentParagraphCharLayout cl;
			cl.x = tempChars[i].x;
			cl.width = tempChars[i].width;
			cl.lineIndex = lineIdx;
			cl.height = tempChars[i].height;
			cl.baseline = tempChars[i].baseline;
			cl.isInlineObject = tempChars[i].isInlineObject;
			state.characterLayouts.Add(cl);
		}

		// Calculate total size
		vint maxWidth = 0;
		for (auto&& line : state.lines)
		{
			if (line.width > maxWidth) maxWidth = line.width;
		}
		state.cachedSize = Size(maxWidth, currentY > 0 ? currentY : 16);
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
					elements::AddInlineObjectRun(state->inlineObjectRuns, range, *inlineProp);
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
		{
			auto bounds = Ptr(new Dictionary<vint, Rect>);
			for (auto [range, prop] : state->inlineObjectRuns)
			{
				if (prop.callbackId == -1) continue;

				bool hasBounds = false;
				vint x1 = 0, y1 = 0, x2 = 0, y2 = 0;
				for (vint i = range.caretBegin; i < range.caretEnd && i < state->characterLayouts.Count(); i++)
				{
					auto& ch = state->characterLayouts[i];
					if (!ch.isInlineObject) continue;

					auto& line = state->lines[ch.lineIndex];
					vint alignmentOffset = 0;
					if (state->alignment == ElementHorizontalAlignment::Center)
					{
						alignmentOffset = (state->cachedSize.x - line.width) / 2;
					}
					else if (state->alignment == ElementHorizontalAlignment::Right)
					{
						alignmentOffset = state->cachedSize.x - line.width;
					}

					vint cx1 = (vint)ch.x + alignmentOffset;
					vint cy1 = line.y + (line.baseline - ch.baseline);
					vint cx2 = cx1 + (vint)ch.width;
					vint cy2 = cy1 + ch.height;

					if (!hasBounds)
					{
						x1 = cx1; y1 = cy1; x2 = cx2; y2 = cy2;
						hasBounds = true;
					}
					else
					{
						if (x1 > cx1) x1 = cx1;
						if (y1 > cy1) y1 = cy1;
						if (x2 < cx2) x2 = cx2;
						if (y2 < cy2) y2 = cy2;
					}
				}

				if (hasBounds)
				{
					bounds->Set(prop.callbackId, Rect(Point(x1, y1), Size(x2 - x1, y2 - y1)));
				}
			}

			if (bounds->Count() > 0)
			{
				response.inlineObjectBounds = bounds;
			}
		}

		GetEvents()->RespondRendererUpdateElement_DocumentParagraph(id, response);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaretBounds(vint, const GetCaretBoundsRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
		auto state = paragraphStates.Values()[index];

		vint caret = arguments.caret;

		// Handle empty text
		if (state->text.Length() == 0 || state->lines.Count() == 0)
		{
			auto& line = state->lines[0];
			GetEvents()->RespondDocumentParagraph_GetCaretBounds(id, Rect(Point(0, line.y), Size(0, line.height)));
			return;
		}

		// Clamp caret to valid range
		if (caret < 0) caret = 0;
		if (caret > state->text.Length()) caret = state->text.Length();

		// Find which line the caret is on
		vint lineIdx = 0;
		for (vint i = 0; i < state->lines.Count(); i++)
		{
			if (caret >= state->lines[i].startPos && caret <= state->lines[i].endPos)
			{
				lineIdx = i;
				break;
			}
			if (i == state->lines.Count() - 1)
			{
				lineIdx = i;
			}
		}

		auto& line = state->lines[lineIdx];

		// Calculate x position
		vint x = 0;
		if (caret > 0 && caret <= state->characterLayouts.Count())
		{
			// Caret is at the end of the previous character
			auto& prevChar = state->characterLayouts[caret - 1];
			x = (vint)(prevChar.x + prevChar.width);
		}
		else if (caret < state->characterLayouts.Count())
		{
			// Caret is at the start of this character
			x = (vint)state->characterLayouts[caret].x;
		}

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

		Rect bounds(Point(x + alignmentOffset, line.y), Size(0, line.height));
		GetEvents()->RespondDocumentParagraph_GetCaretBounds(id, bounds);
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
			break;
		case CRP::CaretMoveLeft:
			response.newCaret = caret > 0 ? caret - 1 : 0;
			break;
		case CRP::CaretMoveRight:
			response.newCaret = caret < textLen ? caret + 1 : textLen;
			break;
		case CRP::CaretMoveUp:
			if (lineIdx > 0)
			{
				// Calculate x offset in current line
				vint xOffset = 0;
				if (caret > 0 && caret <= state->characterLayouts.Count())
				{
					auto& prevChar = state->characterLayouts[caret - 1];
					xOffset = (vint)(prevChar.x + prevChar.width);
				}
				// Find corresponding position in previous line
				auto& prevLine = state->lines[lineIdx - 1];
				response.newCaret = prevLine.startPos;
				for (vint i = prevLine.startPos; i < prevLine.endPos && i < state->characterLayouts.Count(); i++)
				{
					auto& ch = state->characterLayouts[i];
					if (ch.x + ch.width / 2 > xOffset)
						break;
					response.newCaret = i + 1;
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
				if (caret > 0 && caret <= state->characterLayouts.Count())
				{
					auto& prevChar = state->characterLayouts[caret - 1];
					xOffset = (vint)(prevChar.x + prevChar.width);
				}
				// Find corresponding position in next line
				auto& nextLine = state->lines[lineIdx + 1];
				response.newCaret = nextLine.startPos;
				for (vint i = nextLine.startPos; i < nextLine.endPos && i < state->characterLayouts.Count(); i++)
				{
					auto& ch = state->characterLayouts[i];
					if (ch.x + ch.width / 2 > xOffset)
						break;
					response.newCaret = i + 1;
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

		GetEvents()->RespondDocumentParagraph_GetCaret(id, response);
#undef ERROR_MESSAGE_PREFIX
	}

	void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetCaretBoundsRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint, const GetCaretBoundsRequest&)#"
		vint index = paragraphStates.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
		auto state = paragraphStates.Values()[index];

		vint textPos = arguments.caret;
		vint textLen = state->text.Length();

		// Clamp to valid range
		if (textPos < 0) textPos = 0;
		if (textPos > textLen) textPos = textLen;

		// For simple implementation, text position equals caret position
		GetEvents()->RespondDocumentParagraph_GetNearestCaretFromTextPos(id, textPos);
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
			for (vint i = line.startPos; i < line.endPos && i < state->characterLayouts.Count(); i++)
			{
				auto& ch = state->characterLayouts[i];
				if (ch.isInlineObject && relativeX >= ch.x && relativeX < ch.x + ch.width)
				{
					// Found an inline object - look up its properties
					for (auto [range, prop] : state->mergedRuns)
					{
						if (i >= range.caretBegin && i < range.caretEnd)
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

		// Check range: valid positions are 0 to text.Length() inclusive
		if (caret < 0 || caret > state->text.Length())
		{
			GetEvents()->RespondDocumentParagraph_IsValidCaret(id, false);
			return;
		}

		// Check if position is inside an inline object (not at the beginning)
		// Inline objects occupy a range [caretBegin, caretEnd)
		// Position at caretBegin is valid (cursor can be placed before the object)
		// Positions inside (caretBegin < pos < caretEnd) are invalid
		for (auto&& [range, _] : state->inlineObjectRuns)
		{
			if (caret > range.caretBegin && caret < range.caretEnd)
			{
				GetEvents()->RespondDocumentParagraph_IsValidCaret(id, false);
				return;
			}
		}

		GetEvents()->RespondDocumentParagraph_IsValidCaret(id, true);
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

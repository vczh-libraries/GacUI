#include "TuiTextLayout.h"
#include "TuiGraphics.h"

namespace vl::presentation::elements
{
	using namespace console;
	using namespace collections;

	char32_t TuiReadScalar(const WString& text, vint start, vint& length)
	{
		length = 1;
		auto code = (char32_t)text[start];
#ifdef VCZH_WCHAR_UTF16
		if (code >= 0xD800 && code <= 0xDBFF && start + 1 < text.Length())
		{
			auto next = (char32_t)text[start + 1];
			if (next >= 0xDC00 && next <= 0xDFFF)
			{
				length = 2;
				return 0x10000 + ((code - 0xD800) << 10) + next - 0xDC00;
			}
		}
#endif
		if (code == U'\r' && start + 1 < text.Length() && text[start + 1] == L'\n') length = 2;
		return code;
	}

	TuiTextStyle TuiGetTextStyle(IGuiGraphicsParagraph::TextStyle style)
	{
		using P = IGuiGraphicsParagraph;
		return {
			(style & P::Bold) != (P::TextStyle)0,
			(style & P::Italic) != (P::TextStyle)0,
			(style & P::Underline) != (P::TextStyle)0,
			(style & P::Strikeline) != (P::TextStyle)0
		};
	}

	WString TuiEllipsizeText(const WString& text, vint width)
	{
		WString result;
		for (vint start = 0; start < text.Length();)
		{
			vint end = start;
			vint visibleEnd = start;
			vint lineWidth = 0;
			vint newlineLength = 0;
			while (end < text.Length())
			{
				vint length;
				auto code = TuiReadScalar(text, end, length);
				if (code == U'\r' || code == U'\n')
				{
					newlineLength = length;
					break;
				}
				lineWidth += code == U'\t' ? 4 - lineWidth % 4 : TUI::MeasureChar(code);
				end += length;
				if (lineWidth <= width - 1) visibleEnd = end;
			}
			if (lineWidth <= width) result += text.Sub(start, end - start);
			else if (width > 0) result += text.Sub(start, visibleEnd - start) + L"\u2026";
			result += text.Sub(end, newlineLength);
			start = end + newlineLength;
		}
		return result;
	}

/***********************************************************************
TuiGraphicsParagraph
***********************************************************************/

	TuiGraphicsParagraph::TuiGraphicsParagraph(const WString& value, IGuiGraphicsLayoutProvider* owner, TuiGraphicsRenderTarget* target, IGuiGraphicsParagraphCallback* listener)
		: provider(owner)
		, renderTarget(target)
		, callback(listener)
		, text(value)
		, styles(value.Length())
		, colors(value.Length())
		, backgrounds(value.Length())
	{
		for (vint i = 0; i < text.Length(); i++)
		{
			styles[i] = (TextStyle)0;
			colors[i] = Color(255, 255, 255);
			backgrounds[i] = Color(0, 0, 0, 0);
		}
	}

	bool TuiGraphicsParagraph::ValidRange(vint start, vint length)
	{
		return start >= 0 && length >= 0 && start <= text.Length() && length <= text.Length() - start;
	}

	void TuiGraphicsParagraph::EnsureLayout()
	{
		if (!dirty) return;
		dirty = false;
		cells.Clear();
		lines.Clear();
		caretToCell.Clear();
		size = {};

		List<TuiTextCell> tokens;
		for (vint start = 0; start < text.Length();)
		{
			TuiTextCell cell;
			cell.start = start;
			cell.code = TuiReadScalar(text, start, cell.length);
			auto inlineIndex = inlineObjects.Keys().IndexOf(start);
			if (inlineIndex != -1)
			{
				auto entry = inlineObjects.Values()[inlineIndex];
				cell.length = entry.key;
				cell.inlineObject = entry.value;
			}
			tokens.Add(cell);
			start += cell.length;
		}
		auto isNewline = [](const TuiTextCell& cell)
		{
			return !cell.inlineObject && (cell.code == U'\r' || cell.code == U'\n');
		};
		auto cellWidth = [](const TuiTextCell& cell, vint x)
		{
			return cell.inlineObject
				? std::max((vint)0, cell.inlineObject.Value().size.x)
				: cell.code == U'\t' ? 4 - x % 4 : TUI::MeasureChar(cell.code);
		};
		auto cellBaseline = [](const TuiTextCell& cell)
		{
			if (!cell.inlineObject) return (vint)1;
			auto properties = cell.inlineObject.Value();
			auto height = std::max((vint)1, properties.size.y);
			return properties.baseline < 0 ? height : std::min(height, std::max((vint)0, properties.baseline));
		};

		TuiTextLine line;
		vint x = 0;
		vint y = 0;
		vint ascent = 1;
		vint descent = 0;
		auto finishLine = [&](vint end, vint next)
		{
			line.lastCell = cells.Count();
			line.end = end;
			line.bounds = Rect(Point(0, y), Size(x, ascent + descent));
			auto offset = maxWidth < 0 ? 0 : std::max((vint)0, maxWidth - x);
			if (alignment == Alignment::Center) offset /= 2;
			else if (alignment != Alignment::Right) offset = 0;
			line.bounds.x1 += offset;
			line.bounds.x2 += offset;
			for (vint i = line.firstCell; i < line.lastCell; i++)
			{
				auto& cell = cells[i];
				cell.bounds.x1 += offset;
				cell.bounds.x2 += offset;
				auto shift = ascent - cellBaseline(cell);
				cell.bounds.y1 += shift;
				cell.bounds.y2 += shift;
			}
			lines.Add(line);
			size.x = std::max(size.x, x);
			y += ascent + descent;
			line = {};
			line.firstCell = cells.Count();
			line.start = next;
			x = 0;
			ascent = 1;
			descent = 0;
		};
		for (vint first = 0; first < tokens.Count();)
		{
			vint last = first + 1;
			while (last < tokens.Count() && !isNewline(tokens[last - 1]) && !isNewline(tokens[last]))
			{
				auto previous = tokens[last - 1].inlineObject;
				auto next = tokens[last].inlineObject;
				if (!(previous && previous.Value().breakCondition == StickToNextRun)
					&& !(next && next.Value().breakCondition == StickToPreviousRun)) break;
				last++;
			}
			// Keep an inline object with its adjacent text when the group fits a row.
			// Oversized groups still wrap at scalar boundaries without splitting the object.
			if (wrapLine && maxWidth >= 0 && x > 0 && last > first + 1)
			{
				vint widthAtOrigin = 0;
				vint widthAtCurrent = x;
				for (vint i = first; i < last; i++)
				{
					widthAtOrigin += cellWidth(tokens[i], widthAtOrigin);
					widthAtCurrent += cellWidth(tokens[i], widthAtCurrent);
				}
				if (widthAtOrigin <= std::max((vint)1, maxWidth) && widthAtCurrent > maxWidth)
				{
					finishLine(tokens[first].start, tokens[first].start);
				}
			}
			for (vint i = first; i < last; i++)
			{
				auto cell = tokens[i];
				auto newline = isNewline(cell);
				auto width = newline ? 0 : cellWidth(cell, x);
				auto height = cell.inlineObject ? std::max((vint)1, cell.inlineObject.Value().size.y) : 1;
				if (wrapLine && maxWidth >= 0 && x > 0 && x + width > std::max((vint)1, maxWidth))
				{
					finishLine(cell.start, cell.start);
					width = cellWidth(cell, x);
				}
				cell.line = lines.Count();
				cell.bounds = Rect(Point(x, y), Size(width, height));
				caretToCell.Add(cell.start, cells.Count());
				cells.Add(cell);
				auto baseline = cellBaseline(cell);
				ascent = std::max(ascent, baseline);
				descent = std::max(descent, height - baseline);
				x += width;
				if (newline) finishLine(cell.start, cell.start + cell.length);
			}
			first = last;
		}
		finishLine(text.Length(), text.Length());
		size.y = y;
	}

	bool TuiGraphicsParagraph::SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)
	{
		if (length <= 0 || !ValidRange(start, length) || !IsValidCaret(start) || !IsValidCaret(start + length)) return false;
		for (auto&& [begin, entry] : inlineObjects)
		{
			if (begin == start && entry.key == length)
			{
				if (entry.value.callbackId != properties.callbackId || entry.value.backgroundImage != properties.backgroundImage) return false;
				inlineObjects.Set(start, {length, properties});
				dirty = true;
				return true;
			}
			if (begin < start + length && begin + entry.key > start) return false;
		}
		inlineObjects.Add(start, {length, properties});
		dirty = true;
		return true;
	}

	bool TuiGraphicsParagraph::ResetInlineObject(vint start, vint length)
	{
		if (!ValidRange(start, length)) return false;
		List<vint> removed;
		for (auto&& [begin, entry] : inlineObjects)
		{
			if (begin < start + length && begin + entry.key > start) removed.Add(begin);
		}
		for (auto begin : removed) inlineObjects.Remove(begin);
		dirty = true;
		return true;
	}

	void TuiGraphicsParagraph::Render(Rect bounds)
	{
		EnsureLayout();
		List<TuiTextCell> snapshot;
		CopyFrom(snapshot, cells);
		for (auto cell : snapshot)
		{
			if (TUI::IsStopRequested()) return;
			auto location = cell.bounds;
			location.x1 += bounds.x1;
			location.x2 += bounds.x1;
			location.y1 += bounds.y1;
			location.y2 += bounds.y1;
			if (cell.inlineObject)
			{
				auto properties = cell.inlineObject.Value();
				renderTarget->Fill(location, properties.backgroundColor);
				if (properties.backgroundImage)
				{
					auto renderer = properties.backgroundImage->GetRenderer();
					renderer->SetRenderTarget(renderTarget);
					renderer->Render(location);
				}
				if (callback && properties.callbackId != -1)
				{
					auto newSize = callback->OnRenderInlineObject(properties.callbackId, cell.bounds);
					if (newSize != properties.size && inlineObjects.Keys().Contains(cell.start))
					{
						auto entry = inlineObjects[cell.start];
						entry.value.size = newSize;
						inlineObjects.Set(cell.start, entry);
						dirty = true;
					}
				}
			}
			else if (cell.code == U'\t')
			{
				for (vint x = location.x1; x < location.x2; x++)
				{
					renderTarget->Print(Point(x, location.y1), U' ', colors[cell.start], backgrounds[cell.start], TuiGetTextStyle(styles[cell.start]));
				}
			}
			else
			{
				renderTarget->Print(location.LeftTop(), cell.code, colors[cell.start], backgrounds[cell.start], TuiGetTextStyle(styles[cell.start]));
			}
		}
		if (caretPosition != -1 && caretVisible)
		{
			auto caret = GetCaretBounds(caretPosition, caretFrontSide);
			renderTarget->Caret(Point(bounds.x1 + caret.x1, bounds.y1 + caret.y1), caretColor);
		}
	}

	vint TuiGraphicsParagraph::FindLine(vint caret, bool frontSide)
	{
		EnsureLayout();
		for (vint i = 0; i < lines.Count(); i++)
		{
			auto line = lines[i];
			if (caret < line.end || (caret == line.end && (frontSide || i + 1 == lines.Count() || lines[i + 1].start != caret))) return i;
		}
		return lines.Count() - 1;
	}

	Rect TuiGraphicsParagraph::GetCaretBounds(vint caret, bool frontSide)
	{
		if (!IsValidCaret(caret)) return {};
		auto line = lines[FindLine(caret, frontSide)];
		vint x = line.bounds.x2;
		vint y = line.bounds.y1;
		vint height = line.bounds.Height();
		auto index = caretToCell.Keys().IndexOf(caret);
		if (caret < line.end && index != -1)
		{
			auto bounds = cells[caretToCell.Values()[index]].bounds;
			x = bounds.x1;
			y = bounds.y1;
			height = bounds.Height();
		}
		return Rect(Point(x, y), Size(0, height));
	}

	vint TuiGraphicsParagraph::GetCaretFromPoint(Point point)
	{
		EnsureLayout();
		auto line = lines[lines.Count() - 1];
		for (auto candidate : lines)
		{
			if (point.y < candidate.bounds.y2)
			{
				line = candidate;
				break;
			}
		}
		if (point.x <= line.bounds.x1) return line.start;
		for (vint i = line.firstCell; i < line.lastCell; i++)
		{
			auto cell = cells[i];
			if (point.x * 2 < cell.bounds.x1 + cell.bounds.x2) return cell.start;
			if (point.x < cell.bounds.x2) return std::min(line.end, cell.start + cell.length);
		}
		return line.end;
	}

	Nullable<IGuiGraphicsParagraph::InlineObjectProperties> TuiGraphicsParagraph::GetInlineObjectFromPoint(Point point, vint& start, vint& length)
	{
		EnsureLayout();
		for (auto cell : cells)
		{
			if (cell.inlineObject && cell.bounds.Contains(point))
			{
				start = cell.start;
				length = cell.length;
				return cell.inlineObject;
			}
		}
		start = -1;
		length = 0;
		return {};
	}

	vint TuiGraphicsParagraph::GetNearestCaretFromTextPos(vint textPos, bool frontSide)
	{
		EnsureLayout();
		vint previous = 0;
		for (auto caret : caretToCell.Keys())
		{
			if (caret == textPos) return caret;
			if (caret > textPos) return frontSide ? previous : caret;
			previous = caret;
		}
		return frontSide && textPos < text.Length() ? previous : text.Length();
	}

	vint TuiGraphicsParagraph::GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)
	{
		if (!IsValidCaret(comparingCaret)) return -1;
		auto lineIndex = FindLine(comparingCaret, preferFrontSide);
		switch (position)
		{
		case CaretFirst: return 0;
		case CaretLast: return text.Length();
		case CaretLineFirst:
			preferFrontSide = false;
			return lines[lineIndex].start;
		case CaretLineLast:
			preferFrontSide = true;
			return lines[lineIndex].end;
		case CaretMoveLeft:
			preferFrontSide = false;
			return GetNearestCaretFromTextPos(comparingCaret - 1, true);
		case CaretMoveRight:
			preferFrontSide = true;
			return GetNearestCaretFromTextPos(comparingCaret + 1, false);
		case CaretMoveUp:
		case CaretMoveDown:
			{
				auto bounds = GetCaretBounds(comparingCaret, preferFrontSide);
				auto targetLine = lineIndex + (position == CaretMoveUp ? -1 : 1);
				if (targetLine < 0 || targetLine >= lines.Count()) return comparingCaret;
				preferFrontSide = true;
				return GetCaretFromPoint(Point(bounds.x1, lines[targetLine].bounds.y1));
			}
		default: return comparingCaret;
		}
	}

	IGuiGraphicsLayoutProvider* TuiGraphicsParagraph::GetProvider()
	{
		return provider;
	}

	IGuiGraphicsRenderTarget* TuiGraphicsParagraph::GetRenderTarget()
	{
		return renderTarget;
	}

	bool TuiGraphicsParagraph::GetWrapLine()
	{
		return wrapLine;
	}

	void TuiGraphicsParagraph::SetWrapLine(bool value)
	{
		if (wrapLine != value) { wrapLine = value; dirty = true; }
	}

	vint TuiGraphicsParagraph::GetMaxWidth()
	{
		return maxWidth;
	}

	void TuiGraphicsParagraph::SetMaxWidth(vint value)
	{
		if (maxWidth != value) { maxWidth = value; dirty = true; }
	}

	Alignment TuiGraphicsParagraph::GetParagraphAlignment()
	{
		return alignment;
	}

	void TuiGraphicsParagraph::SetParagraphAlignment(Alignment value)
	{
		if (alignment != value) { alignment = value; dirty = true; }
	}

	bool TuiGraphicsParagraph::SetFont(vint start, vint length, const WString& value)
	{
		return ValidRange(start, length);
	}

	bool TuiGraphicsParagraph::SetSize(vint start, vint length, vint value)
	{
		return ValidRange(start, length);
	}

	bool TuiGraphicsParagraph::SetStyle(vint start, vint length, TextStyle value)
	{
		if (!ValidRange(start, length)) return false;
		for (vint i = start; i < start + length; i++) styles[i] = value;
		return true;
	}

	bool TuiGraphicsParagraph::SetColor(vint start, vint length, Color value)
	{
		if (!ValidRange(start, length)) return false;
		for (vint i = start; i < start + length; i++) colors[i] = value;
		return true;
	}

	bool TuiGraphicsParagraph::SetBackgroundColor(vint start, vint length, Color value)
	{
		if (!ValidRange(start, length)) return false;
		for (vint i = start; i < start + length; i++) backgrounds[i] = value;
		return true;
	}

	Size TuiGraphicsParagraph::GetSize()
	{
		EnsureLayout();
		return size;
	}

	bool TuiGraphicsParagraph::EnableCaret(vint caret, Color color, bool frontSide)
	{
		if (!IsValidCaret(caret)) return false;
		caretPosition = caret;
		caretColor = color;
		caretFrontSide = frontSide;
		caretVisible = true;
		return true;
	}

	void TuiGraphicsParagraph::DisableCaret()
	{
		caretPosition = -1;
	}

	bool TuiGraphicsParagraph::BlinkCaret()
	{
		if (caretPosition == -1) return false;
		caretVisible = !caretVisible;
		return true;
	}

	bool TuiGraphicsParagraph::IsValidCaret(vint caret)
	{
		EnsureLayout();
		return caret == text.Length() || (caret >= 0 && caretToCell.Keys().Contains(caret));
	}

	bool TuiGraphicsParagraph::IsValidTextPos(vint textPos)
	{
		return textPos >= 0 && textPos < text.Length();
	}

/***********************************************************************
TuiGraphicsLayoutProvider
***********************************************************************/

	Ptr<IGuiGraphicsParagraph> TuiGraphicsLayoutProvider::CreateParagraph(const WString& text, IGuiGraphicsRenderTarget* renderTarget, IGuiGraphicsParagraphCallback* callback)
	{
		return Ptr(new TuiGraphicsParagraph(text, this, static_cast<TuiGraphicsRenderTarget*>(renderTarget), callback));
	}
}

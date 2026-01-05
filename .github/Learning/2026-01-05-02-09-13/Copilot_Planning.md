# !!!PLANNING!!!
# UPDATES

## UPDATE

I found I made a mistake. In all document/paragraph awared requests, I forgot to add the element id which confused you. Now either arguments itself or arguments.id is the element id. Please review your code in the planning document and make necessary changes.

# IMPROVEMENT PLAN

## STEP 1: Add Storage Structures for Document Paragraph State

Add the necessary data structures to track document paragraph layout and state in the header file `GuiUnitTestProtocol_Rendering.h`.

**Changes to make:**

Add these structures before the `UnitTestRemoteProtocol_Rendering` class definition:

```cpp
struct DocumentParagraphCharLayout
{
	double x;               // X position of character start
	double width;           // Character width
	vint lineIndex;         // Which line this character belongs to
	vint height;            // Character height (font size or inline object height)
	vint baseline;          // Distance from top to baseline (for alignment)
	bool isInlineObject;    // True if this position is part of an inline object
};

struct DocumentParagraphLineInfo
{
	vint startPos;          // Text position of line start (inclusive)
	vint endPos;            // Text position of line end (exclusive)
	vint y;                 // Y coordinate of line top
	vint height;            // Line height (computed from baseline alignment)
	vint baseline;          // Line baseline position from line top
	vint width;             // Line width (for alignment calculation)
};

struct DocumentParagraphState
{
	WString text;
	bool wrapLine = false;
	vint maxWidth = -1;
	remoteprotocol::ElementHorizontalAlignment alignment = remoteprotocol::ElementHorizontalAlignment::Left;
	
	elements::DocumentTextRunPropertyMap textRuns;
	elements::DocumentInlineObjectRunPropertyMap inlineObjectRuns;
	elements::DocumentRunPropertyMap mergedRuns;
	
	collections::List<DocumentParagraphCharLayout> characterLayouts;
	collections::List<DocumentParagraphLineInfo> lines;
	Size cachedSize;
	
	bool caretOpen = false;
	vint caretElementId = -1;
	vint caretPos = 0;
	Color caretColor;
	bool caretFrontSide = true;
};
```

Add a storage map as a protected member in `UnitTestRemoteProtocol_Rendering`:

```cpp
collections::Dictionary<vint, Ptr<DocumentParagraphState>> paragraphStates;
```

**Rationale:** The task requires storing per-character layout information since each character may have a different font size. The `DocumentParagraphState` holds all necessary information for layout calculations, caret navigation, and inline object handling.

## STEP 2: Update `Impl_RendererCreated` and `Impl_RendererDestroyed` for Paragraph Lifecycle

Modify these functions to create/destroy `DocumentParagraphState` when paragraphs are created/destroyed.

**Changes in `GuiUnitTestProtocol_Rendering.cpp`:**

In `Impl_RendererCreated`, after tracking the element type, add paragraph state creation:

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_RendererCreated(const Ptr<List<RendererCreation>>& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererCreated(const Ptr<List<RendererCreation>>&)#"
	if (arguments)
	{
		for (auto creation : *arguments.Obj())
		{
			CHECK_ERROR(!loggedTrace.createdElements->Keys().Contains(creation.id), ERROR_MESSAGE_PREFIX L"Renderer with the specified id has been created or used.");
			loggedTrace.createdElements->Add(creation.id, creation.type);
			
			// Create paragraph state for DocumentParagraph elements
			if (creation.type == RendererType::DocumentParagraph)
			{
				auto state = Ptr(new DocumentParagraphState());
				paragraphStates.Add(creation.id, state);
			}
		}
	}
#undef ERROR_MESSAGE_PREFIX
}
```

In `Impl_RendererDestroyed`, remove paragraph state:

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_RendererDestroyed(const Ptr<List<vint>>& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererDestroyed(const Ptr<List<vint>>&)#"
	if (arguments)
	{
		for (auto id : *arguments.Obj())
		{
			CHECK_ERROR(loggedTrace.createdElements->Keys().Contains(id), ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");
			CHECK_ERROR(!removedElementIds.Contains(id), ERROR_MESSAGE_PREFIX L"Renderer with the specified id has been destroyed.");
			removedElementIds.Add(id);
			lastElementDescs.Remove(id);
			
			// Remove paragraph state if this was a DocumentParagraph
			paragraphStates.Remove(id);
		}
	}
#undef ERROR_MESSAGE_PREFIX
}
```

**Rationale:** Following the update notes, paragraphs are created empty via `Impl_RendererCreated` and incrementally updated via `RendererUpdateElement_DocumentParagraph`. We must create an empty state when the paragraph is first created.

## STEP 3: Implement Helper Functions for Layout Calculation

Add helper functions in `GuiUnitTestProtocol_Rendering.cpp` for calculating character widths and getting run properties.

**Add a helper function to get font size for a position:**

```cpp
namespace
{
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
}
```

**Rationale:** Following the existing pattern from `CalculateSolidLabelSizeIfNecessary` (line 394), character width is calculated as 0.6×fontSize for ASCII and 1.0×fontSize for non-ASCII. Each character may have a different font, so we need per-character lookups.

## STEP 4: Implement `CalculateParagraphLayout` Function

Add a function to calculate the complete paragraph layout with line breaking and baseline alignment.

**Add this function in `GuiUnitTestProtocol_Rendering.cpp`:**

```cpp
namespace
{
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
				info.width = prop.size.x;
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
		for (auto line : state.lines)
		{
			if (line.width > maxWidth) maxWidth = line.width;
		}
		state.cachedSize = Size(maxWidth, currentY > 0 ? currentY : 16);
	}
}
```

**Rationale:** This implements the layout algorithm as described in the task:
- Line breaks only on `\n` when wrapLine=false (ignoring `\r` as zero-width)
- Additional character-level word wrap when wrapLine=true at maxWidth
- Per-character layout storage (required because each character may have different font)
- Baseline alignment for inline objects following the specified semantics

## STEP 5: Implement `Impl_RendererUpdateElement_DocumentParagraph`

Implement the main update handler that processes incremental changes and recalculates layout.

**Replace the stub implementation:**

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_RendererUpdateElement_DocumentParagraph(vint id, const ElementDesc_DocumentParagraph& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::Impl_RendererUpdateElement_DocumentParagraph#"
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
		for (auto callbackId : *arguments.removedInlineObjects)
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
		for (auto run : *arguments.runsDiff)
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
				IGuiGraphicsParagraph::TextStyle style = (IGuiGraphicsParagraph::TextStyle)0;
				if (textProp->fontProperties.bold) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Bold);
				if (textProp->fontProperties.italic) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Italic);
				if (textProp->fontProperties.underline) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Underline);
				if (textProp->fontProperties.strikeline) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Strikeline);
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
	CalculateParagraphLayout(*state);
	
	// Store in lastElementDescs (following existing pattern)
	lastElementDescs.Set(arguments.id, arguments);
	
	// Send response with calculated size
	GetEvents()->RespondRendererUpdateElement_DocumentParagraph(id, state->cachedSize);
#undef ERROR_MESSAGE_PREFIX
}
```

**Rationale:** This follows the update notes: use `AddTextRun`, `AddInlineObjectRun`, and `ResetInlineObjectRun` to process diffs, then call `MergeRuns` to create the final run map for layout. The `Nullable<WString> text` field is only applied when not empty.

## STEP 6: Implement Query and Caret Display Functions

With the element ID available directly in the request structures (either `arguments.id` or `arguments` itself for `CloseCaret`), we don't need any separate tracking for the current paragraph. Each function can directly look up the paragraph state using the element ID from the request.

**Key insight from update**: The protocol schema shows:
- `GetCaretRequest`, `GetCaretBoundsRequest`, `GetInlineObjectFromPointRequest`, `IsValidCaretRequest`, `OpenCaretRequest` all have `var id : int;` field
- `DocumentParagraph_CloseCaret` has `request: int;` which is the element ID directly

All query handlers will use `arguments.id` (or `arguments` for `CloseCaret`) to look up the paragraph state from `paragraphStates`.

## STEP 7: Implement `Impl_DocumentParagraph_GetCaretBounds`

Calculate the bounding rectangle for a caret position.

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::Impl_DocumentParagraph_GetCaretBounds#"
	vint index = paragraphStates.Keys().IndexOf(arguments.id);
	CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
	auto state = paragraphStates.Values()[index];
	
	vint caret = arguments.caret;
	
	// Handle empty text
	if (state->text.Length() == 0 || state->lines.Count() == 0)
	{
		auto& line = state->lines.Count() > 0 ? state->lines[0] : state->lines[0];
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
```

**Rationale:** The caret is a zero-width line at a text position. Its Y and height come from the line info, its X comes from the character layout.

## STEP 8: Implement `Impl_DocumentParagraph_GetCaret`

Implement caret navigation for all `CaretRelativePosition` values.

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetCaret(vint id, const GetCaretRequest& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::Impl_DocumentParagraph_GetCaret#"
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
	
	using CRP = IGuiGraphicsParagraph::CaretRelativePosition;
	
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
			// Don't include the \n at end of line
			if (state->text[response.newCaret - 1] == L'\n')
				response.newCaret--;
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
```

**Rationale:** Implements all caret navigation modes. For vertical movement, we calculate the X offset and find the closest position in the target line.

## STEP 9: Implement `Impl_DocumentParagraph_GetNearestCaretFromTextPos`

Convert a text position to the nearest valid caret position.

**Note:** There is no separate `GetCaretFromPoint` protocol message. The `GetCaretFromPoint` functionality is implemented client-side in `GuiRemoteGraphicsParagraph::GetCaretFromPoint` by iterating through all carets and calling `GetCaretBounds` to find the closest one.

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetCaretBoundsRequest& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::Impl_DocumentParagraph_GetNearestCaretFromTextPos#"
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
```

**Rationale:** In this simplified implementation, text positions map directly to caret positions.

## STEP 10: Implement `Impl_DocumentParagraph_GetInlineObjectFromPoint`

Check if a point hits an inline object.

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_GetInlineObjectFromPoint(vint id, const GetInlineObjectFromPointRequest& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::Impl_DocumentParagraph_GetInlineObjectFromPoint#"
	vint index = paragraphStates.Keys().IndexOf(arguments.id);
	CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"No active paragraph.");
	auto state = paragraphStates.Values()[index];
	
	Point pt = arguments.point;
	Nullable<DocumentRun> result;
	
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
							DocumentRun run;
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
```

**Rationale:** Hit test for inline objects specifically, returning the inline object run if found.

## STEP 11: Implement `Impl_DocumentParagraph_IsValidCaret`

**Function signature:** `Impl_DocumentParagraph_IsValidCaret(vint id, const IsValidCaretRequest& arguments)`
- `id`: The response id (used for `RespondDocumentParagraph_IsValidCaret`)
- `arguments.id`: The element ID (to look up the paragraph state)
- `arguments.caret`: The character index to validate as a caret position

**Caret position semantics:**
- Index 0 to (length-1) means the position **before** that character
- Index length means the position at the **end** of the text
- A valid caret is where a cursor in a text box can be placed

**Validation rules:**
1. Out of range (< 0 or > text.Length()) → false
2. Position is inside an inline object (not at the beginning of the inline object) → false
3. Otherwise → true

Note: Surrogate pairs are ignored for simplicity.

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_IsValidCaret(vint id, const IsValidCaretRequest& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::Impl_DocumentParagraph_IsValidCaret#"
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
```

## STEP 12: Implement `Impl_DocumentParagraph_OpenCaret` and `Impl_DocumentParagraph_CloseCaret`

Manage caret display state.

```cpp
void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_OpenCaret(const OpenCaretRequest& arguments)
{
	vint index = paragraphStates.Keys().IndexOf(arguments.id);
	if (index == -1) return;
	
	auto state = paragraphStates.Values()[index];
	state->caretOpen = true;
	state->caretElementId = arguments.id;
	state->caretPos = arguments.caret;
	state->caretColor = arguments.caretColor;
	state->caretFrontSide = arguments.frontSide;
}

void UnitTestRemoteProtocol_Rendering::Impl_DocumentParagraph_CloseCaret(const vint& arguments)
{
	// arguments itself is the element ID for CloseCaret
	vint index = paragraphStates.Keys().IndexOf(arguments);
	if (index == -1) return;
	
	auto state = paragraphStates.Values()[index];
	state->caretOpen = false;
}
```

**Rationale:** These just store/clear the caret state for rendering purposes. Note that `CloseCaret` takes the element ID directly as the argument (not a struct with an `id` field).

## STEP 13: Include Required Headers

Add necessary includes at the top of `GuiUnitTestProtocol_Rendering.cpp`:

```cpp
#include "../PlatformProviders/Remote/GuiRemoteGraphics_Document.h"
```

And in the header file, ensure the forward declaration or include for the document types.

# TEST PLAN

## Existing Test Coverage

The task description states that testing will be covered in subsequent tasks (Task No.15-16) with GacUI test cases. This implementation provides the foundation.

## Verification Approach

1. **Compile-time verification**: Ensure the code compiles without errors after implementation.

2. **Basic functionality tests** (to be added in Task No.15-16):
   - Create a document paragraph element
   - Update with text content
   - Verify size calculation
   - Test caret navigation (left/right/up/down/home/end)
   - Test hit testing (click to place caret)
   - Test inline objects with various baseline values

3. **Edge cases to test**:
   - Empty paragraph
   - Single character
   - Single line with no wrapping
   - Multiple lines with `\n` breaks
   - Word wrapping at `maxWidth`
   - Text alignment (left, center, right)
   - Mixed font sizes in same paragraph
   - Inline objects with `baseline == -1`, `baseline == height`, `baseline < height`, `baseline > height`
   - `\r\n` sequences (should treat `\r` as zero-width)

4. **Protocol response verification**:
   - `RespondRendererUpdateElement_DocumentParagraph` returns correct size
   - `RespondDocumentParagraph_GetCaret` returns correct positions for all navigation modes
   - `RespondDocumentParagraph_GetCaretBounds` returns correct rectangles (used by client-side `GetCaretFromPoint` for hit testing)
   - `RespondDocumentParagraph_IsValidCaret` correctly validates caret positions
   - `RespondDocumentParagraph_GetInlineObjectFromPoint` correctly identifies inline objects
   - `RespondDocumentParagraph_GetNearestCaretFromTextPos` returns correct caret positions

## Notes

- No existing test cases need modification since this is new functionality
- The unit test framework infrastructure is already in place; tests will use `GacUIUnitTest_StartFast_WithResourceAsText` pattern
- Test resources will define document controls in XML format

# !!!FINISHED!!!
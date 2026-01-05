# !!!TASK!!!
# PROBLEM DESCRIPTION

## TASK No.14: Implement document protocol handlers in `GuiUnitTestProtocol_Rendering.cpp`

### description

Implement the document-related protocol handler methods in `UnitTestRemoteProtocol_Rendering` class in `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`.

**Learning from Task 6**: Protocol implementation should start simple and incrementally add complexity. When protocol data structures are complex, verify the actual types used in the schema before writing extensive code. Test basic message flow first before implementing all edge cases.

**Character width calculation**: Follow the same pattern as `GuiSolidLabelElement` in `CalculateSolidLabelSizeIfNecessary` (line 394 of `GuiUnitTestProtocol_Rendering.cpp`). Characters are categorized by their UTF-16 code point:
- Characters with code point < 128: width = 0.6 × font size
- Characters with code point >= 128: width = 1.0 × font size

This simplified approach avoids complex font metrics while providing consistent character width calculations for testing purposes. Do NOT handle surrogate pairs in this task - that will be addressed in Task No.17.

The header file `GuiUnitTestProtocol_Rendering.h` declares the following methods that need implementation in the cpp file. Implement:

**Main rendering handler**:
- `Impl_RendererUpdateElement_DocumentParagraph(vint id, const ElementDesc_DocumentParagraph& arguments)`: 
  - Store paragraph text, wrapLine, maxWidth, alignment
  - Process `runsDiff` array to build internal run representation
  - Calculate paragraph size based on font metrics using the character width formula above
  - Return calculated size

**Caret navigation handlers**:
- `Impl_DocumentParagraph_GetCaret(vint id, const GetCaretRequest& arguments)`:
  - Implement basic caret movement logic for CaretFirst, CaretLast, CaretLineFirst, CaretLineLast
  - For CaretMoveLeft/Right/Up/Down, calculate new position based on text layout
  - Return `GetCaretResponse` with new caret and preferFrontSide

- `Impl_DocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)`:
  - Calculate bounds rectangle for caret at given position
  - Consider frontSide flag for RTL/complex scripts
  - Return Rect with width=0, positioned at caret

- `Impl_DocumentParagraph_GetCaretFromPoint(vint id, const Point& arguments)`:
  - Perform hit testing to find nearest caret to given point
  - Consider line layout and character positions

**Other query handlers**:
- `Impl_DocumentParagraph_GetInlineObjectFromPoint(vint id, const Point& arguments)`:
  - Check if point intersects any inline object
  - Return DocumentRun with inline object info, or null

- `Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetCaretBoundsRequest& arguments)`:
  - Convert text position to nearest valid caret position
  - Consider frontSide preference

- `Impl_DocumentParagraph_IsValidCaret(vint id, const vint& arguments)`:
  - First arg `id` is the response id for `RespondDocumentParagraph_IsValidCaret`
  - Second arg `arguments` is the character index to check
  - Index 0 to (length-1) means position before that character
  - Index length means position at the end
  - Returns false if out of range (< 0 or > text.Length())
  - Returns false if position is inside an inline object (not at the beginning)
  - Returns true otherwise
  - Surrogate pairs are ignored for simplicity

**Caret display handlers**:
- `Impl_DocumentParagraph_OpenCaret(const OpenCaretRequest& arguments)`:
  - Store caret state for rendering (position, color, frontSide)

- `Impl_DocumentParagraph_CloseCaret()`:
  - Clear stored caret state

**Implementation approach**:

Follow the pattern from `CalculateSolidLabelSizeIfNecessary`:
- Use cached font measurements
- Store paragraph state in `lastElementDescs` or similar structure keyed by `id`
- For complex layout calculations, implement simplified version suitable for testing (exact pixel-perfect layout not required, but consistent behavior is)

### what to be done

1. Implement `Impl_RendererUpdateElement_DocumentParagraph` with paragraph state storage and size calculation
2. Implement all caret navigation handlers (`Impl_DocumentParagraph_GetCaret`, `Impl_DocumentParagraph_GetCaretBounds`, `Impl_DocumentParagraph_GetCaretFromPoint`)
3. Implement query handlers (`Impl_DocumentParagraph_GetInlineObjectFromPoint`, `Impl_DocumentParagraph_GetNearestCaretFromTextPos`, `Impl_DocumentParagraph_IsValidCaret`)
4. Implement caret display handlers (`Impl_DocumentParagraph_OpenCaret`, `Impl_DocumentParagraph_CloseCaret`)
5. Follow existing pattern in `GuiUnitTestProtocol_Rendering.cpp` for state management and font metrics
6. Keep implementation simple but consistent for testing purposes

### how to test it

Testing will be covered in subsequent tasks (Task No.15-16) with GacUI test cases. This task provides the foundation for document control testing.

### file locations

Modified file: `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.cpp`
Header file (declarations already present): `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering.h`

### rationale

These handlers are the server-side implementation of the paragraph protocol for the unit test environment. Without these implementations, any test using document controls will fail. The implementations can be simplified compared to production renderers (e.g., Windows/Linux) since they only need to support testing scenarios. However, they must be consistent and functional enough to validate the paragraph protocol and allow document control tests to pass.

# UPDATES

## UPDATE

Please pay attention to the wrapLine and maxWidth control variable. When wrapLine is false, texts are natually splitted by \n (and you will need to ignore the optional leading \r as it has no width). When wrapLine is true, maxWidth has extra control on where to break line

The first priority of the implementation is to provide stable test result on every possible platform, so you don't need to take care about language or direction (ltr or rtl). Break at a character when it is required or when there is no more space.

It is a simple rich text document, each character may have different font. Other information like bold/italic/color/etc can be ignored at layout. It means you need to store layout for every single character instead of just one line.

Finally, `Impl_RendererCreated` may ask for creating new paragraphs, and `Impl_RendererDestroyed` may ask for destroying new paragraphs. When a paragrpah is created, it is empty (have no text and therefore empty or default on everything else). `RendererUpdateElement_DocumentParagraph` will incrementall changing the paragrpah. If any field is nullable, it applies only when it is not empty (important: distinguishing empty value or empty string in Nullable<WString>).

From Task No.1 to No.12 they created and tested a bunch of function helping you incrementally applying changes. But you need to do the layout by yourself. You can also take a look at No 13. and No.14 about when these requests (aka Impl_* function) are sent and what responses are expected from some requests (aka Impl_* function that has an id, you must call the corresponding Respond function when you have a result).

## UPDATE

More hint, when you get a `runDiffs`, `createdInlineObjects` and `removedInlineObjects` from the update message, `AddTextRun`, `AddInlineObjectRun` and `ResetInlineObjectRun` will be useful. After applying all diffs, call `MergeRuns` to create a final result. This is the beginning of layouting. Pay attention to image run and inline object run, they take a portion of a text, and convert these characters to a rectangle with baseline. Font of these characters are not important anymore, the size and baseline of the rectangle are important. When baseline == -1 it means baseline is the height. When baseline is the height, the bottom of the rectangle align with bottom of all characters. When baseline is smaller than height, it moves down. When baseline is greater than height, it moves up. Baseline of all characters are their bottom, they are kept at the same height in one rendering line.

# INSIGHTS AND REASONING

## Overview

Task No.14 requires implementing the document paragraph protocol handlers in `GuiUnitTestProtocol_Rendering.cpp`. These are the server-side (unit test framework) implementations that process messages from `GuiRemoteGraphicsParagraph` (the client-side implementation completed in Task No.13).

## Key Clarifications from Update

The update provides critical implementation guidance:

1. **Line Breaking Rules**:
   - When `wrapLine == false`: Only break lines at `\n` (ignore optional leading `\r` as it has no width)
   - When `wrapLine == true`: Break at `maxWidth` in addition to `\n`
   - Character-level breaking: Break at a character when required or when there is no more space

2. **Per-Character Layout**:
   - Each character may have a different font size (rich text document)
   - Layout information must be stored for every single character, not just per-line
   - Ignore bold/italic/color/etc for layout calculation - only font size matters

3. **Nullable Field Handling (Important!)**:
   - `Nullable<WString> text`: Only apply if not empty (distinguish between null and empty string)
   - When paragraph is first created via `Impl_RendererCreated`, it is empty
   - `RendererUpdateElement_DocumentParagraph` incrementally changes the paragraph

4. **Lifecycle Management**:
   - `Impl_RendererCreated`: May create new paragraphs (empty, with no text and defaults for everything)
   - `Impl_RendererDestroyed`: May destroy paragraphs

5. **Platform Independence**:
   - First priority is stable test results on every platform
   - No need to handle language-specific or RTL/LTR direction concerns

## Current State Analysis

### Existing Code Structure

The header file [GuiUnitTestProtocol_Rendering.h](Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.h#L148-L156) declares the following methods that currently have stub implementations throwing `CHECK_FAIL(L"Not implemented.")`:

1. `Impl_RendererUpdateElement_DocumentParagraph` - Main update handler (has `vint id` for response)
2. `Impl_DocumentParagraph_GetCaret` - Caret navigation (has `vint id` for response)
3. `Impl_DocumentParagraph_GetCaretBounds` - Caret bounds calculation (has `vint id` for response)
4. `Impl_DocumentParagraph_GetInlineObjectFromPoint` - Hit testing (has `vint id` for response)
5. `Impl_DocumentParagraph_GetNearestCaretFromTextPos` - Text position to caret (has `vint id` for response)
6. `Impl_DocumentParagraph_IsValidCaret` - Caret validation (has `vint id` for response)
7. `Impl_DocumentParagraph_OpenCaret` - Open caret for rendering (no response)
8. `Impl_DocumentParagraph_CloseCaret` - Close caret (no response)

### Helper Functions from Task No.1-12

From [GuiRemoteGraphics_Document.h](Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h), the following helper functions and types are available:

```cpp
struct CaretRange { vint caretBegin; vint caretEnd; };

struct DocumentTextRunPropertyOverrides {
    Nullable<Color> textColor;
    Nullable<Color> backgroundColor;
    Nullable<WString> fontFamily;
    Nullable<vint> size;
    Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
};

using DocumentTextRunPropertyMap = Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
using DocumentInlineObjectRunPropertyMap = Dictionary<CaretRange, DocumentInlineObjectRunProperty>;
using DocumentRunPropertyMap = Dictionary<CaretRange, DocumentRunProperty>;

void AddTextRun(DocumentTextRunPropertyMap& map, CaretRange range, const DocumentTextRunPropertyOverrides& propertyOverrides);
bool AddInlineObjectRun(DocumentInlineObjectRunPropertyMap& map, CaretRange range, const DocumentInlineObjectRunProperty& property);
bool ResetInlineObjectRun(DocumentInlineObjectRunPropertyMap& map, CaretRange range);
void MergeRuns(const DocumentTextRunPropertyMap& textRuns, const DocumentInlineObjectRunPropertyMap& inlineObjectRuns, DocumentRunPropertyMap& result);
void DiffRuns(const DocumentRunPropertyMap& oldRuns, const DocumentRunPropertyMap& newRuns, ElementDesc_DocumentParagraph& result);
```

**Update Clarification**: When processing `runsDiff`, `createdInlineObjects`, and `removedInlineObjects` from the update message:
1. Use `AddTextRun` for text run updates
2. Use `AddInlineObjectRun` for inline object run insertions
3. Use `ResetInlineObjectRun` for inline object run removals
4. After applying all diffs, call `MergeRuns` to create the final merged result - this is the starting point for layout calculation

### Inline Object and Image Run Layout - Baseline Semantics

**Critical**: Image runs and inline object runs are special - they take a portion of text and convert those characters to a rectangle with baseline. The font of these characters becomes irrelevant; what matters is:
- **size**: The width and height of the inline object rectangle
- **baseline**: The alignment control for vertical positioning

**Baseline Rules**:
- When `baseline == -1`: baseline equals height (bottom alignment)
- When `baseline == height`: bottom of the rectangle aligns with the bottom of all characters on the same line
- When `baseline < height`: the rectangle moves **down** (hangs below the text baseline)
- When `baseline > height`: the rectangle moves **up** (rises above the text baseline)

**Key principle**: The baseline of all characters is their bottom, and they are kept at the same height in one rendering line. The inline object's baseline determines how it vertically aligns with this common baseline.

### Protocol Data Structures

From [Protocol_Renderer_Document.txt](Source/PlatformProviders/Remote/Protocol/Protocol_Renderer_Document.txt):

```cpp
struct ElementDesc_DocumentParagraph {
    vint id;
    Nullable<WString> text;        // Only sent on first update or when changed
    bool wrapLine;
    vint maxWidth;
    ElementHorizontalAlignment alignment;
    Ptr<List<DocumentRun>> runsDiff;              // Incremental run changes
    Ptr<List<vint>> createdInlineObjects;
    Ptr<List<vint>> removedInlineObjects;
};
```

### Reference Implementation: `CalculateSolidLabelSizeIfNecessary`

From [GuiUnitTestProtocol_Rendering.cpp#L394](Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp#L394):
- Characters with UTF-16 code point < 128: width = 0.6 × font size
- Characters with UTF-16 code point >= 128: width = 1.0 × font size
- Line height = font size + 4

## Updated Design Proposal

### 1. Storage Structure (Per-Character Layout with Baseline Support)

Since each character may have a different font, and inline objects have their own size and baseline, we need comprehensive per-character layout data:

```cpp
struct CharacterLayout {
    double x;                // X position of character start
    double width;            // Character width
    vint lineIndex;          // Which line this character belongs to
    vint height;             // Character height (font size or inline object height)
    vint baseline;           // Distance from top to baseline (for alignment)
    bool isInlineObject;     // True if this position is part of an inline object
};

struct LineInfo {
    vint startPos;           // Text position of line start (inclusive)
    vint endPos;             // Text position of line end (exclusive)
    vint y;                  // Y coordinate of line top
    vint height;             // Line height (computed from baseline alignment)
    vint baseline;           // Line baseline position from line top
    vint width;              // Line width (for alignment calculation)
};

struct DocumentParagraphState {
    WString text;
    bool wrapLine = false;
    vint maxWidth = -1;
    ElementHorizontalAlignment alignment = ElementHorizontalAlignment::Left;
    
    // Run data - use the same map structures as client side
    DocumentTextRunPropertyMap textRuns;
    DocumentInlineObjectRunPropertyMap inlineObjectRuns;
    DocumentRunPropertyMap mergedRuns;  // Result of MergeRuns()
    
    // Per-character layout (computed after layout)
    List<CharacterLayout> characterLayouts;     // Same size as text.Length()
    List<LineInfo> lines;
    Size cachedSize;
    
    // Caret display state
    bool caretOpen = false;
    vint caretPos = 0;
    Color caretColor;
    bool caretFrontSide = true;
};

// Storage in UnitTestRemoteProtocol_Rendering
Dictionary<vint, Ptr<DocumentParagraphState>> paragraphStates;
```

### 2. Run Processing Using Helper Functions

When processing `runsDiff` from `ElementDesc_DocumentParagraph`:

```cpp
void ApplyRunsDiff(DocumentParagraphState& state, const List<DocumentRun>& diffs) {
    for (auto&& diff : diffs) {
        CaretRange range{ diff.caretBegin, diff.caretEnd };
        
        if (diff.props.Is<DocumentTextRunProperty>()) {
            auto& textProp = diff.props.Get<DocumentTextRunProperty>();
            DocumentTextRunPropertyOverrides overrides;
            overrides.textColor = textProp.textColor;
            overrides.backgroundColor = textProp.backgroundColor;
            overrides.fontFamily = textProp.fontProperties.fontFamily;
            overrides.size = textProp.fontProperties.size;
            // Convert textStyle if needed
            AddTextRun(state.textRuns, range, overrides);
        }
        else if (diff.props.Is<DocumentInlineObjectRunProperty>()) {
            auto& inlineProp = diff.props.Get<DocumentInlineObjectRunProperty>();
            AddInlineObjectRun(state.inlineObjectRuns, range, inlineProp);
        }
    }
    
    // After applying all diffs, merge to get final run map
    state.mergedRuns.Clear();
    MergeRuns(state.textRuns, state.inlineObjectRuns, state.mergedRuns);
}
```

For `removedInlineObjects`, use `ResetInlineObjectRun` to remove the inline objects from the map.

### 3. Layout Algorithm with Baseline Handling

The layout must handle inline objects specially:
- Text characters: height = fontSize + 4, baseline = fontSize (bottom of character is the baseline)
- Inline objects: use the `size` and `baseline` from `DocumentInlineObjectRunProperty`

**Baseline alignment within a line**:
1. Find the maximum "baseline-to-top" distance among all elements on the line
2. Find the maximum "bottom-to-baseline" distance among all elements on the line
3. Line height = max-baseline-to-top + max-bottom-to-baseline
4. Position each element so their baselines align at line.y + max-baseline-to-top

```cpp
// For text character at position i with fontSize:
vint charHeight = fontSize + 4;
vint charBaseline = fontSize;  // baseline is at fontSize from top (bottom of char)
// Distance below baseline = charHeight - charBaseline = 4

// For inline object with size and baseline from property:
vint objHeight = inlineProp.size.y;
vint objBaseline = (inlineProp.baseline == -1) ? objHeight : inlineProp.baseline;
// Distance above baseline = objBaseline
// Distance below baseline = objHeight - objBaseline

// When baseline < height: more distance below baseline (hangs down)
// When baseline > height: negative distance below baseline (rises up)
```

### 4. Font Size and Inline Object Lookup

Since each character may have different properties, create helpers to look up run information:

```cpp
// Returns {fontSize, isInlineObject, inlineObjectProperty}
// For text: fontSize > 0, isInlineObject = false
// For inline object: fontSize = 0, isInlineObject = true, property contains size/baseline
std::tuple<vint, bool, Nullable<DocumentInlineObjectRunProperty>> 
GetRunPropertiesForPosition(const DocumentParagraphState& state, vint pos) {
    for (auto&& [range, prop] : state.mergedRuns) {
        if (pos >= range.caretBegin && pos < range.caretEnd) {
            if (prop.Is<DocumentTextRunProperty>()) {
                auto& textProp = prop.Get<DocumentTextRunProperty>();
                return { textProp.fontProperties.size, false, {} };
            }
            if (prop.Is<DocumentInlineObjectRunProperty>()) {
                auto& inlineProp = prop.Get<DocumentInlineObjectRunProperty>();
                return { 0, true, inlineProp };
            }
        }
    }
    return { 12, false, {} }; // Default font size
}
```

### 5. Layout Algorithm (With Baseline Handling)

The layout algorithm must handle inline objects with their baseline semantics:

```cpp
void CalculateParagraphLayout(DocumentParagraphState& state) {
    state.characterLayouts.Clear();
    state.lines.Clear();
    
    const WString& text = state.text;
    
    // First pass: calculate per-character metrics and break into lines
    struct TempCharInfo {
        double x, width;
        vint height, baseline;  // baseline = distance from top to baseline
        bool isInlineObject;
    };
    List<TempCharInfo> tempChars;
    List<Pair<vint, vint>> lineRanges;  // [start, end) for each line
    
    double currentX = 0;
    vint currentLineStart = 0;
    
    for (vint i = 0; i < text.Length(); i++) {
        wchar_t c = text[i];
        TempCharInfo info = { currentX, 0, 0, 0, false };
        
        // Handle \r - zero width, no line break
        if (c == L'\r') {
            tempChars.Add(info);
            continue;
        }
        
        // Get character properties
        auto [fontSize, isInline, inlineProp] = GetRunPropertiesForPosition(state, i);
        
        if (isInline && inlineProp) {
            // Inline object: use its size and baseline
            auto& prop = inlineProp.Value();
            info.width = prop.size.x;
            info.height = prop.size.y;
            info.baseline = (prop.baseline == -1) ? prop.size.y : prop.baseline;
            info.isInlineObject = true;
        } else {
            // Text character
            info.width = (c < 128 ? 0.6 : 1.0) * fontSize;
            info.height = fontSize + 4;
            info.baseline = fontSize;  // baseline at bottom of character
            info.isInlineObject = false;
        }
        
        // Handle \n - always break line
        if (c == L'\n') {
            info.width = 0;
            tempChars.Add(info);
            lineRanges.Add({ currentLineStart, i + 1 });
            currentLineStart = i + 1;
            currentX = 0;
            continue;
        }
        
        // Check word wrap
        if (state.wrapLine && state.maxWidth > 0 && currentX > 0) {
            if (currentX + info.width > state.maxWidth) {
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
    if (currentLineStart <= text.Length()) {
        lineRanges.Add({ currentLineStart, text.Length() });
    }
    
    // Second pass: calculate line heights using baseline alignment
    vint currentY = 0;
    for (auto&& [lineStart, lineEnd] : lineRanges) {
        vint maxAboveBaseline = 0;  // max distance from top to baseline
        vint maxBelowBaseline = 0;  // max distance from baseline to bottom
        
        for (vint i = lineStart; i < lineEnd; i++) {
            auto& info = tempChars[i];
            if (info.width > 0) {  // skip \r and \n
                maxAboveBaseline = std::max(maxAboveBaseline, info.baseline);
                maxBelowBaseline = std::max(maxBelowBaseline, info.height - info.baseline);
            }
        }
        
        // Default line height if empty
        if (maxAboveBaseline == 0 && maxBelowBaseline == 0) {
            maxAboveBaseline = 12;
            maxBelowBaseline = 4;
        }
        
        LineInfo line;
        line.startPos = lineStart;
        line.endPos = lineEnd;
        line.y = currentY;
        line.height = maxAboveBaseline + maxBelowBaseline;
        line.baseline = maxAboveBaseline;
        
        // Calculate line width
        vint lineWidth = 0;
        for (vint i = lineStart; i < lineEnd; i++) {
            lineWidth = (vint)(tempChars[i].x + tempChars[i].width);
        }
        line.width = lineWidth - (lineStart < lineEnd ? (vint)tempChars[lineStart].x : 0);
        
        state.lines.Add(line);
        currentY += line.height;
    }
    
    // Third pass: create final character layouts with line indices
    vint lineIdx = 0;
    for (vint i = 0; i < tempChars.Count(); i++) {
        while (lineIdx < state.lines.Count() - 1 && i >= state.lines[lineIdx].endPos) {
            lineIdx++;
        }
        CharacterLayout cl;
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
    for (auto&& line : state.lines) {
        if (line.width > maxWidth) maxWidth = line.width;
    }
    state.cachedSize = Size(maxWidth, currentY);
}
```

### 6. Paragraph Creation/Destruction in `Impl_RendererCreated`/`Impl_RendererDestroyed`

The existing `Impl_RendererCreated` already tracks element types in `loggedTrace.createdElements`. For paragraphs, we need to also create the state:

```cpp
// In Impl_RendererCreated - add after tracking the element
if (creation.type == RendererType::DocumentParagraph) {
    auto state = Ptr(new DocumentParagraphState());
    // Empty paragraph with defaults
    paragraphStates.Add(creation.id, state);
}

// In Impl_RendererDestroyed
paragraphStates.Remove(id);
```

### 7. `Impl_RendererUpdateElement_DocumentParagraph` Implementation

```cpp
void Impl_RendererUpdateElement_DocumentParagraph(vint id, const ElementDesc_DocumentParagraph& arguments) {
    // Get existing state (created in Impl_RendererCreated)
    vint index = paragraphStates.Keys().IndexOf(arguments.id);
    CHECK_ERROR(index != -1, L"Paragraph not created");
    auto state = paragraphStates.Values()[index];
    
    // Apply text if provided (distinguish null vs empty string)
    if (arguments.text) {
        state->text = arguments.text.Value();
    }
    
    // Always update these
    state->wrapLine = arguments.wrapLine;
    state->maxWidth = arguments.maxWidth;
    state->alignment = arguments.alignment;
    
    // Process inline object lifecycle first
    if (arguments.removedInlineObjects) {
        for (auto callbackId : *arguments.removedInlineObjects) {
            // Find the range for this inline object and reset it
            for (auto&& [range, prop] : state->inlineObjectRuns) {
                if (prop.callbackId == callbackId) {
                    ResetInlineObjectRun(state->inlineObjectRuns, range);
                    break;
                }
            }
        }
    }
    
    // Apply runsDiff using helper functions
    if (arguments.runsDiff) {
        for (auto&& run : *arguments.runsDiff) {
            CaretRange range{ run.caretBegin, run.caretEnd };
            
            if (run.props.Is<DocumentTextRunProperty>()) {
                auto& textProp = run.props.Get<DocumentTextRunProperty>();
                DocumentTextRunPropertyOverrides overrides;
                overrides.textColor = textProp.textColor;
                overrides.backgroundColor = textProp.backgroundColor;
                overrides.fontFamily = textProp.fontProperties.fontFamily;
                overrides.size = textProp.fontProperties.size;
                AddTextRun(state->textRuns, range, overrides);
            }
            else if (run.props.Is<DocumentInlineObjectRunProperty>()) {
                auto& inlineProp = run.props.Get<DocumentInlineObjectRunProperty>();
                AddInlineObjectRun(state->inlineObjectRuns, range, inlineProp);
            }
        }
    }
    
    // Merge runs to create final result
    state->mergedRuns.Clear();
    MergeRuns(state->textRuns, state->inlineObjectRuns, state->mergedRuns);
    
    // Recalculate layout
    CalculateParagraphLayout(*state);
    
    // Store in lastElementDescs (following existing pattern)
    lastElementDescs.Set(arguments.id, arguments);
    
    // Send response
    GetEvents()->RespondRendererUpdateElement_DocumentParagraph(id, state->cachedSize);
}
```

### 8. Response Functions

Looking at the protocol definition, functions with `vint id` parameter need to call corresponding `Respond*` functions:

- `Impl_RendererUpdateElement_DocumentParagraph(vint id, ...)` → `GetEvents()->RespondRendererUpdateElement_DocumentParagraph(id, Size)`
- `Impl_DocumentParagraph_GetCaret(vint id, ...)` → `GetEvents()->RespondDocumentParagraph_GetCaret(id, GetCaretResponse)`
- `Impl_DocumentParagraph_GetCaretBounds(vint id, ...)` → `GetEvents()->RespondDocumentParagraph_GetCaretBounds(id, Rect)`
- `Impl_DocumentParagraph_GetInlineObjectFromPoint(vint id, ...)` → `GetEvents()->RespondDocumentParagraph_GetInlineObjectFromPoint(id, Nullable<DocumentRun>)`
- `Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, ...)` → `GetEvents()->RespondDocumentParagraph_GetNearestCaretFromTextPos(id, vint)`
- `Impl_DocumentParagraph_IsValidCaret(vint id, ...)` → `GetEvents()->RespondDocumentParagraph_IsValidCaret(id, bool)`

## Implementation Order

1. Update `Impl_RendererCreated` and `Impl_RendererDestroyed` to handle paragraph lifecycle
2. Implement helper functions for font size lookup and character width calculation
3. Implement `CalculateParagraphLayout` with line breaking logic
4. Implement `Impl_RendererUpdateElement_DocumentParagraph` with incremental run application
5. Implement `Impl_DocumentParagraph_IsValidCaret` (simplest query)
6. Implement `Impl_DocumentParagraph_GetCaretBounds` (uses per-character layout)
7. Implement `Impl_DocumentParagraph_GetCaret` (caret navigation)
8. Implement `Impl_DocumentParagraph_GetCaretFromPoint` (hit testing)
9. Implement `Impl_DocumentParagraph_GetNearestCaretFromTextPos`
10. Implement `Impl_DocumentParagraph_GetInlineObjectFromPoint`
11. Implement `Impl_DocumentParagraph_OpenCaret` and `Impl_DocumentParagraph_CloseCaret`

## Files to Modify

- [Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.h](Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.h): Add `DocumentParagraphState` struct and storage map
- [Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp](Source/UnitTestUtilities/GuiUnitTestProtocol_Rendering.cpp): Implement all handler methods

## Key Implementation Notes

1. **Nullable<WString> vs empty string**: Check `arguments.text` before accessing `.Value()`. An empty `Nullable` means "don't change", while `Nullable(L"")` means "set to empty string".

2. **Character width formula**: `(c < 128 ? 0.6 : 1.0) * fontSize` - same as `GuiSolidLabelElement`.

3. **Line height**: `fontSize + 4` - same as `GuiSolidLabelElement`.

4. **\r handling**: Treat `\r` as having zero width and do not break line on it. Line break happens only on `\n`.

5. **Platform independence**: The simplified character width formula ensures consistent results across platforms.

6. **Run Management with Helper Functions**:
   - Use `AddTextRun()` for text run diffs
   - Use `AddInlineObjectRun()` for inline object insertions
   - Use `ResetInlineObjectRun()` for inline object removals (from `removedInlineObjects`)
   - Always call `MergeRuns()` after applying all diffs to create the final merged run map for layout

7. **Inline Object Baseline Handling**:
   - Inline objects replace characters with rectangles of specified `size`
   - When `baseline == -1`: baseline equals height (bottom-aligned)
   - When `baseline == height`: rectangle bottom aligns with character baseline
   - When `baseline < height`: rectangle hangs below baseline
   - When `baseline > height`: rectangle rises above baseline
   - All elements on a line align at their baselines

8. **Line Height Calculation with Mixed Content**:
   - Calculate `maxAboveBaseline` (max distance from element top to baseline)
   - Calculate `maxBelowBaseline` (max distance from baseline to element bottom)
   - Line height = `maxAboveBaseline + maxBelowBaseline`
   - Each element positioned so its baseline aligns at `line.y + maxAboveBaseline`

# !!!FINISHED!!!
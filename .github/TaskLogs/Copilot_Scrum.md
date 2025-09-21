# !!!SCRUM!!!

# DESIGN REQUEST

Find out all `GuiDocumentCommonInterface::UserInput_*` methods that are public (ignore what is not)
Design test cases for them
Implement the test case in a dedicated test file `TestDocumentConfig` in the `UnitTest` project, put it in the `Source Files` solution explorer folder

## UPDATE

Remove the first task as I don't want "analyze only" in the list, it is your job to do the analyzing

## UPDATE

THere are 8 functions but you only created 5+1 tasks, I would like to create dedicated tasks for each function, even they are overloaded.

In the first task, you will have to create 8 empty `TEST_CATEGORY`. And in each actual test case implement task, fill each TEST_CATEGORY.

## UPDATE

Carefully look through the source code of the 8 functions in `GuiDocumentCommonInterface.cpp`, find out what they are actually doing, update your best guessing to the document, before we can really design test cases.

You will need to also read the GuiDocumentConfig and GuiDocumentConfigEvaluated class to have a better understanding.

# TASKS

- [x] TASK No.1: Create TestDocumentConfig Test File with Empty TEST_CATEGORY Sections
- [ ] TASK No.2: Implement UserInput_ConvertToPlainText Tests
- [ ] TASK No.3: Implement UserInput_JoinParagraphs(List<WString>&) Tests
- [ ] TASK No.4: Implement UserInput_JoinParagraphs(DocumentModel) Tests
- [ ] TASK No.5: Implement UserInput_JoinLinesInsideParagraph(WString&) Tests
- [ ] TASK No.6: Implement UserInput_JoinLinesInsideParagraph(DocumentParagraphRun) Tests
- [ ] TASK No.7: Implement UserInput_FormatText(List<WString>&) Tests
- [ ] TASK No.8: Implement UserInput_FormatText(WString to List<WString>&) Tests
- [ ] TASK No.9: Implement UserInput_FormatDocument Tests

## TASK No.1: Create TestDocumentConfig Test File with Empty TEST_CATEGORY Sections

## TASK No.1: Create TestDocumentConfig Test File with Empty TEST_CATEGORY Sections

Create a new test file `TestDocumentConfig.cpp` in the UnitTest project following the GacUI test file conventions and structure. This task will set up the complete test file structure with 8 empty TEST_CATEGORY sections, one for each UserInput method that will be tested in subsequent tasks.

### what to be done

- Create `TestDocumentConfig.cpp` file in the `Test\GacUISrc\UnitTest` directory
- Add the file to the `UnitTest.vcxproj` project file
- Add the file to the `UnitTest.vcxproj.filters` file under the `Source Files` solution explorer folder
- Set up the basic test file structure with required includes and namespaces
- Create 8 empty TEST_CATEGORY sections:
  1. `UserInput_ConvertToPlainText`
  2. `UserInput_JoinParagraphs_ListWString`
  3. `UserInput_JoinParagraphs_DocumentModel`
  4. `UserInput_JoinLinesInsideParagraph_WString`
  5. `UserInput_JoinLinesInsideParagraph_DocumentParagraphRun`
  6. `UserInput_FormatText_ListWString`
  7. `UserInput_FormatText_WStringToList`
  8. `UserInput_FormatDocument`
- Include necessary headers for DocumentModel, GuiDocumentConfigEvaluated, and other dependencies
- Add helper function declarations for creating test data that will be used across test categories

### how to test it

- Verify the file compiles without errors after being added to the project
- Ensure the file appears in the correct solution explorer folder
- Run the test to confirm the test framework recognizes all 8 empty test categories
- Verify that the basic test structure is ready for implementation in subsequent tasks

### rationale

Setting up the complete test file structure with all 8 categories ensures that each UserInput method gets dedicated test coverage. Creating empty categories first provides a clear roadmap for subsequent tasks and ensures proper organization. This approach allows each subsequent task to focus solely on implementing tests for one specific method without worrying about file structure or category organization.

## TASK No.2: Implement UserInput_ConvertToPlainText Tests

Implement comprehensive test cases for the `UserInput_ConvertToPlainText` static method that strips all formatting from specified paragraph ranges and resets alignment properties.

### what to be done

- Fill the `UserInput_ConvertToPlainText` TEST_CATEGORY with complete test cases
- Test the method's core functionality: calls `DocumentModel::ConvertToPlainText(begin, end)` to remove all formatting from the specified range
- Test that all paragraph alignments are reset to default (null) within the specified range
- Test edge cases: invalid ranges (beginParagraph > endParagraph returns early), single paragraph, multiple paragraphs
- Test that paragraphs outside the specified range remain unchanged
- Create helper functions to build test DocumentModel instances with various formatting, styles, and alignment settings
- Verify the method calculates proper TextPos range using the last paragraph's run ranges

### how to test it

- Unit tests will verify that formatted DocumentModel paragraphs are converted to plain text correctly
- Tests will confirm that paragraph alignment properties are reset to null after conversion
- Boundary condition tests will ensure the method handles edge cases properly (early return for invalid ranges)
- Tests will verify the method doesn't modify paragraphs outside the specified range
- Tests will verify proper TextPos calculation for the end position using run ranges

### rationale

`UserInput_ConvertToPlainText` is a fundamental text processing function that strips all formatting from documents. It's used internally by `UserInput_FormatDocument` when `pasteAsPlainText` is enabled. Comprehensive testing ensures this core operation works correctly for copy/paste operations and document cleaning functionality.

## TASK No.3: Implement UserInput_JoinParagraphs(List<WString>&) Tests

Implement test cases for the `UserInput_JoinParagraphs(collections::List<WString>& paragraphTexts, bool spaceForFlattenedLineBreak)` overload that concatenates all string paragraphs into a single paragraph.

### what to be done

- Fill the `UserInput_JoinParagraphs_ListWString` TEST_CATEGORY with complete test cases
- Test the core functionality: concatenates all paragraphs in the list into one single paragraph, with optional space insertion between them
- Test the `spaceForFlattenedLineBreak` parameter: when true, inserts a space between each paragraph (except before the first)
- Test edge cases: empty list, single paragraph, multiple paragraphs with various content
- Test with different paragraph content: empty strings, whitespace-only, normal text, special characters
- Verify the list is properly modified: cleared and replaced with the single joined paragraph
- Test that the joining preserves paragraph content exactly (no line processing within paragraphs)

### how to test it

- Unit tests will verify string paragraphs are joined correctly with and without spaces
- Tests will confirm the `spaceForFlattenedLineBreak` parameter behavior (spaces only between paragraphs, not before first)
- Edge case tests will handle empty and single-item scenarios  
- Tests will verify the original list is properly modified to contain exactly one joined result
- Content preservation tests will ensure no unexpected text modifications occur

### rationale

This string-based paragraph joining is fundamental for text processing operations. The method is used by other UserInput functions and needs to work correctly for both single-line mode processing and general text manipulation. Testing the exact space insertion logic is critical for maintaining predictable text formatting behavior.

## TASK No.4: Implement UserInput_JoinParagraphs(DocumentModel) Tests

Implement test cases for the `UserInput_JoinParagraphs(Ptr<DocumentModel> model, bool spaceForFlattenedLineBreak)` overload that merges all paragraphs in a DocumentModel into the first paragraph.

### what to be done

- Fill the `UserInput_JoinParagraphs_DocumentModel` TEST_CATEGORY with complete test cases
- Test the core functionality: merges all paragraph runs from subsequent paragraphs into the first paragraph, with optional space insertion
- Test the `spaceForFlattenedLineBreak` parameter: when true, inserts a space TextRun between merged paragraphs
- Test various DocumentModel structures: single paragraph, multiple paragraphs with different run types (text, formatting, etc.)
- Test that all runs from subsequent paragraphs are copied into the first paragraph using `CopyFrom(firstParagraph->runs, paragraph->runs, true)`
- Verify the document structure after joining: only the first paragraph remains, all others are removed
- Test that the space insertion creates proper DocumentTextRun objects with L" " content

### how to test it

- Unit tests will confirm DocumentModel paragraphs are merged properly with run content preserved
- Tests will verify the `spaceForFlattenedLineBreak` parameter creates proper space TextRuns between merged content
- Structure validation tests will ensure the resulting DocumentModel contains only the first paragraph with all runs merged
- Tests will verify that run copying preserves all formatting and content from source paragraphs
- Tests will confirm proper DocumentTextRun creation for space insertion

### rationale

The DocumentModel overload handles complex document structure merging, preserving all formatting while consolidating paragraphs. This is more sophisticated than simple string joining and is crucial for maintaining rich text formatting during paragraph consolidation operations like single-line mode conversion.

## TASK No.5: Implement UserInput_JoinLinesInsideParagraph(WString&) Tests

Implement test cases for the `UserInput_JoinLinesInsideParagraph(WString& text, bool spaceForFlattenedLineBreak)` overload that flattens line breaks within a single text string.

### what to be done

- Fill the `UserInput_JoinLinesInsideParagraph_WString` TEST_CATEGORY with complete test cases
- Test the core functionality: processes line breaks using FetchLineRecord system to join lines within the text
- Test the `spaceForFlattenedLineBreak` parameter: when true, inserts a space character where line breaks are removed
- Test various line break scenarios: Windows (\r\n), Unix (\n), Mac (\r), and mixed line endings
- Test edge cases: single line text (early return with no processing), empty strings, text with only line breaks
- Test the FetchLineRecord parsing logic: properly identifies line boundaries and handles different line ending formats
- Verify that the text string is modified in place to contain the flattened result
- Test that when no line breaks exist (`!*flr.next` after first line), the method returns early

### how to test it

- Unit tests will verify line breaks are properly flattened within text strings using the FetchLineRecord system
- Line ending format tests will ensure cross-platform compatibility (Windows, Unix, Mac line endings)
- Tests will verify the `spaceForFlattenedLineBreak` parameter functions correctly (space insertion vs. direct joining)
- Edge case tests will handle single-line content (early return), empty strings, and line-break-only content
- Tests will verify proper in-place string modification

### rationale

Line joining within text strings is crucial for text normalization and display formatting. The FetchLineRecord-based implementation needs thorough testing to ensure it handles all line ending formats correctly and maintains cross-platform compatibility. This function is used by higher-level text processing operations.

## TASK No.6: Implement UserInput_JoinLinesInsideParagraph(DocumentParagraphRun) Tests

Implement test cases for the `UserInput_JoinLinesInsideParagraph(Ptr<DocumentParagraphRun> paragraph, bool spaceForFlattenedLineBreak)` overload that recursively processes all text runs within a paragraph container.

### what to be done

- Fill the `UserInput_JoinLinesInsideParagraph_DocumentParagraphRun` TEST_CATEGORY with complete test cases
- Test the recursive container traversal: uses a queue-based approach to process all DocumentContainerRun objects
- Test that DocumentTextRun objects are processed by calling the WString overload of UserInput_JoinLinesInsideParagraph
- Test nested container structures: verify the method properly handles deeply nested DocumentContainerRun hierarchies  
- Test various paragraph structures: simple text runs, mixed content (text + container runs), deeply nested containers
- Test that the traversal queue correctly processes all containers and their child runs
- Verify that line flattening is applied to all text runs found in the container hierarchy
- Test the `spaceForFlattenedLineBreak` parameter propagation to the WString processing function

### how to test it

- Unit tests will confirm paragraph runs are processed recursively using the queue-based container traversal
- Structure traversal tests will verify all text runs are properly found and processed in complex nested hierarchies
- Tests will verify the `spaceForFlattenedLineBreak` parameter is correctly passed to the WString processing function
- Integration tests will ensure proper coordination between container traversal and text run processing
- Tests will verify that DocumentContainerRun and DocumentTextRun types are handled correctly

### rationale

The recursive nature of the DocumentParagraphRun overload requires thorough testing to ensure all nested text runs are processed correctly. This queue-based traversal system is critical for maintaining document structure integrity while performing line joining operations on complex formatted content with arbitrary nesting levels.

## TASK No.7: Implement UserInput_FormatText(List<WString>&) Tests

Implement test cases for the `UserInput_FormatText(collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)` overload that applies paragraph mode formatting to text lists.

### what to be done

- Fill the `UserInput_FormatText_ListWString` TEST_CATEGORY with complete test cases
- Test the paragraph mode logic: when config.paragraphMode != Paragraph, calls UserInput_JoinLinesInsideParagraph on each paragraph
- Test the single-line mode logic: when config.paragraphMode == Singleline, calls UserInput_JoinParagraphs after line joining
- Test all three GuiDocumentParagraphMode values: Paragraph (no processing), Multiline (line joining only), Singleline (line joining + paragraph joining)
- Test the `config.spaceForFlattenedLineBreak` parameter propagation to both line and paragraph joining functions
- Test various paragraph configurations and content scenarios
- Create GuiDocumentConfigEvaluated test configurations for each paragraph mode
- Test that Paragraph mode leaves the list unchanged, while other modes apply appropriate transformations

### how to test it

- Unit tests will verify text formatting according to different paragraph modes using the exact config-driven logic
- Configuration-driven tests will ensure proper handling of GuiDocumentConfigEvaluated settings for each mode
- Integration tests will verify the correct sequencing: line joining first (if needed), then paragraph joining (if needed)
- Tests will confirm the list is properly modified according to the configuration mode
- Parameter propagation tests will verify spaceForFlattenedLineBreak is passed correctly to sub-functions

### rationale

This function implements the core paragraph mode formatting logic used throughout the document system. Testing all three paragraph modes ensures the configuration-driven text processing works correctly for different document types (single-line textboxes, multi-line textboxes, full rich text documents). The sequential processing (lines first, then paragraphs) needs verification.

## TASK No.8: Implement UserInput_FormatText(WString to List<WString>&) Tests

Implement test cases for the `UserInput_FormatText(const WString& text, collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)` overload that parses text into paragraphs and applies formatting.

### what to be done

- Fill the `UserInput_FormatText_WStringToList` TEST_CATEGORY with complete test cases
- Test the double line break paragraph detection: when config.doubleLineBreaksBetweenParagraph is true, uses complex FetchLineRecord logic to detect paragraph boundaries
- Test the simple line-by-line parsing: when config.doubleLineBreaksBetweenParagraph is false, uses StringReader to split by single line breaks
- Test the complex double line break logic: detects empty lines as paragraph separators, handles fragments and ending empty lines
- Test the single-line mode application: calls UserInput_JoinParagraphs after paragraph parsing when config.paragraphMode == Singleline
- Test various input text patterns: single lines, multiple lines, double line breaks, mixed line ending formats
- Test the FetchLineRecord fragment processing: properly handles line grouping and empty line detection
- Test edge cases: empty text, text ending with line breaks, text with only line breaks

### how to test it

- Unit tests will verify text splitting into paragraphs according to the doubleLineBreaksBetweenParagraph configuration
- Line break processing tests will confirm double line break paragraph detection works correctly using FetchLineRecord logic
- Simple parsing tests will verify StringReader-based line splitting for single line break mode
- Integration tests will verify the coordination between paragraph parsing and single-line mode processing
- Fragment processing tests will ensure complex line grouping logic works correctly
- Edge case tests will handle various text boundary conditions

### rationale

The text-to-paragraphs conversion implements sophisticated paragraph detection logic that differs significantly based on configuration. The double line break detection using FetchLineRecord is complex and requires thorough testing. This function is critical for paste operations and text import functionality where paragraph boundaries must be correctly identified.

## TASK No.9: Implement UserInput_FormatDocument Tests

## TASK No.9: Implement UserInput_FormatDocument Tests

Implement test cases for `UserInput_FormatDocument` method that applies comprehensive configuration-driven formatting to entire DocumentModel instances.

### what to be done

- Fill the `UserInput_FormatDocument` TEST_CATEGORY with complete test cases
- Test the null safety check: method returns early if model is null
- Test the pasteAsPlainText processing: when config.pasteAsPlainText is true, calls UserInput_ConvertToPlainText on entire document and handles style copying from baseline
- Test baseline document style handling: copies styles from baselineDocument when provided, clears styles when baseline is null
- Test the empty document check: method returns early if model has no paragraphs after plain text conversion
- Test paragraph mode processing: applies UserInput_JoinLinesInsideParagraph to all paragraphs when config.paragraphMode != Paragraph
- Test single-line mode processing: calls UserInput_JoinParagraphs on the entire model when config.paragraphMode == Singleline
- Test the sequential processing order: plain text conversion → style handling → empty check → line joining → paragraph joining
- Test various GuiDocumentConfigEvaluated configurations and DocumentModel structures
- Test integration with all other UserInput methods called internally

### how to test it

- Unit tests will verify complete document formatting using various configuration settings with the exact processing sequence
- Baseline comparison tests will ensure proper handling of style copying vs clearing based on baseline presence
- Configuration integration tests will verify proper coordination with all called UserInput methods
- Null safety and edge case tests will ensure proper handling of null models and empty documents
- Sequential processing tests will verify the correct order of operations is maintained
- Style management tests will confirm proper baseline document style handling

### rationale

`UserInput_FormatDocument` is the highest-level formatting orchestrator that coordinates all other UserInput methods to apply comprehensive document formatting. It implements the complete document processing pipeline used by paste operations and document loading. Testing this integration point ensures all formatting rules work together correctly and in the proper sequence.

# Impact to the Knowledge Base

## GacUI

### New Content to Add

The knowledge base currently doesn't contain specific guidance about testing document processing methods in the TextEditorPackage. The following should be added:

#### Testing Document Processing APIs

Document processing methods like UserInput_* functions require specific test patterns:

- Use DocumentModel creation helpers for consistent test setup
- Test both string-based and DocumentModel-based overloads for completeness
- Verify configuration-driven behavior with GuiDocumentConfigEvaluated
- Test edge cases like empty content, single items, and boundary conditions
- Ensure cross-platform line ending compatibility in text processing tests

#### Document Model Test Utilities

Testing document processing requires utility functions for:

- Creating test DocumentModel instances with various formatting
- Building GuiDocumentConfigEvaluated test configurations
- Comparing document content and structure after processing
- Generating test text with different line break patterns and content types

### Existing Content Updates

No existing knowledge base content requires updates for this task, as it focuses on adding new test coverage for existing functionality rather than modifying existing APIs or design patterns.

# !!!FINISHED!!!

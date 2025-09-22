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

## UPDATE

The first task has been finished, do not touch it.

All methods for testing now have comments on it, describing its behavior. Read them carefully and redesign test cases.

# TASKS

- [x] TASK No.1: Create TestDocumentConfig Test File with Empty TEST_CATEGORY Sections
- [x] TASK No.2: Implement UserInput_ConvertToPlainText Tests
- [x] TASK No.3: Implement UserInput_JoinParagraphs(List<WString>&) Tests
- [x] TASK No.4: Implement UserInput_JoinParagraphs(DocumentModel) Tests
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

Implement comprehensive test cases for the `UserInput_ConvertToPlainText` static method that converts selected paragraphs to plain text, removing all styles, images and embedded objects.

### what to be done

- Fill the `UserInput_ConvertToPlainText` TEST_CATEGORY with complete test cases
- Test the core functionality: calls `DocumentModel::ConvertToPlainText(begin, end)` to remove all formatting from specified paragraph ranges
- Test paragraph alignment reset: verify all paragraph alignments within the range are reset to null using `model->paragraphs[i]->alignment.Reset()`
- Test edge cases: invalid ranges (beginParagraph > endParagraph causes early return), single paragraph, multiple paragraphs  
- Test that paragraphs outside the specified range remain unchanged
- Test TextPos calculation: verify proper end position calculation using RunRangeMap to get the end position of the last paragraph
- Create helper functions to build test DocumentModel instances with various formatting, styles, alignments, and embedded content

### how to test it

- Unit tests will verify formatted DocumentModel paragraphs are converted to plain text correctly within specified ranges
- Alignment reset tests will confirm all paragraph alignment properties are set to null after conversion
- Boundary condition tests will ensure early return behavior for invalid ranges (beginParagraph > endParagraph)
- Range isolation tests will verify paragraphs outside the specified range remain unchanged  
- TextPos calculation tests will verify proper end position determination using RunRangeMap
- Content preservation tests will ensure text content is preserved while all formatting is removed

### rationale

`UserInput_ConvertToPlainText` is a fundamental text processing function that strips all formatting while preserving text content. The method is used by `UserInput_FormatDocument` when `pasteAsPlainText` is enabled and needs thorough testing to ensure proper plain text conversion, alignment reset, and range handling for copy/paste operations.

## TASK No.3: Implement UserInput_JoinParagraphs(List<WString>&) Tests

Implement test cases for the `UserInput_JoinParagraphs(collections::List<WString>& paragraphTexts, bool spaceForFlattenedLineBreak)` overload that concatenates multiple paragraphs to one paragraph.

### what to be done

- Fill the `UserInput_JoinParagraphs_ListWString` TEST_CATEGORY with complete test cases
- Test the core functionality: concatenates all paragraphs in the list into a single paragraph, replacing the original list
- Test the `spaceForFlattenedLineBreak` parameter: when true, adds a space between paragraphs (but not before the first one)
- Test edge cases: empty list, single paragraph, multiple paragraphs with various content
- Test with different paragraph content: empty strings, whitespace-only, normal text, special characters
- Verify the list modification: original list is cleared and replaced with exactly one concatenated result
- Test the indexed iteration logic that handles space insertion correctly between paragraphs

### how to test it

- Unit tests will verify paragraphs are concatenated correctly with and without space insertion
- Space insertion tests will confirm spaces are added between paragraphs but not before the first paragraph
- Edge case tests will handle empty lists, single items, and various content types
- List modification tests will verify the original list is properly cleared and replaced with the single result
- Content preservation tests will ensure no unexpected text modifications occur during concatenation

### rationale

This string-based paragraph joining is fundamental for text processing operations where simple concatenation is needed. The method is used by higher-level UserInput functions and must handle space insertion correctly according to the `spaceForFlattenedLineBreak` parameter to maintain predictable text formatting behavior for document processing.

## TASK No.4: Implement UserInput_JoinParagraphs(DocumentModel) Tests

Implement test cases for the `UserInput_JoinParagraphs(Ptr<DocumentModel> model, bool spaceForFlattenedLineBreak)` overload that concatenates multiple paragraphs to one paragraph in a DocumentModel.

### what to be done

- Fill the `UserInput_JoinParagraphs_DocumentModel` TEST_CATEGORY with complete test cases
- Test the core functionality: merges all paragraph runs from subsequent paragraphs into the first paragraph, then clears the document and re-adds only the first paragraph
- Test the `spaceForFlattenedLineBreak` parameter: when true, adds a DocumentTextRun with L" " content between merged paragraphs
- Test various DocumentModel structures: single paragraph, multiple paragraphs with different run types (text, formatting, etc.)
- Test run copying: verify all runs from subsequent paragraphs are copied into the first paragraph using `CopyFrom(firstParagraph->runs, paragraph->runs, true)`
- Verify the final document structure: document contains only the first paragraph with all runs merged
- Test DocumentTextRun creation for space insertion: verify proper DocumentTextRun objects are created with L" " content

### how to test it

- Unit tests will confirm DocumentModel paragraphs are merged properly with run content preserved
- Space insertion tests will verify DocumentTextRun creation with L" " content between merged content
- Structure validation tests will ensure the resulting DocumentModel contains only the first paragraph with all runs
- Run copying tests will verify all formatting and content is preserved from source paragraphs during merge
- Integration tests will confirm proper coordination between run copying and space insertion

### rationale

The DocumentModel overload handles complex document structure merging while preserving all formatting. This is more sophisticated than simple string joining as it maintains rich text formatting during paragraph consolidation operations. Testing ensures the run copying preserves all content and the space insertion creates proper DocumentTextRun objects.

## TASK No.5: Implement UserInput_JoinLinesInsideParagraph(WString&) Tests

Implement test cases for the `UserInput_JoinLinesInsideParagraph(WString& text, bool spaceForFlattenedLineBreak)` overload that removes all line breaks within a text string.

### what to be done

- Fill the `UserInput_JoinLinesInsideParagraph_WString` TEST_CATEGORY with complete test cases
- Test the core functionality: uses FetchLineRecord system to detect and remove line breaks within the text
- Test the `spaceForFlattenedLineBreak` parameter: when true, replaces line breaks with spaces; when false, directly joins lines
- Test early return optimization: when no second line exists (`!*flr.next` after first line), function returns early with just the first line
- Test various line break scenarios: Windows (\r\n), Unix (\n), Mac (\r), and mixed line ending formats
- Test edge cases: single line text (early return), empty strings, text with only line breaks
- Verify the text string is modified in place to contain the flattened result
- Test the FetchLineRecord parsing: proper line boundary detection and handling of different line ending formats

### how to test it

- Unit tests will verify line breaks are properly removed using the FetchLineRecord system
- Line ending format tests will ensure cross-platform compatibility (Windows, Unix, Mac line endings)
- Space replacement tests will verify the `spaceForFlattenedLineBreak` parameter functions correctly
- Early return tests will handle single-line content (verify `!*flr.next` condition and early return)
- Edge case tests will handle empty strings and line-break-only content
- In-place modification tests will verify proper string replacement

### rationale

Line break removal within text strings is essential for text normalization. The FetchLineRecord-based implementation with early return optimization needs thorough testing to ensure it handles all line ending formats correctly and maintains cross-platform compatibility. This function supports higher-level text processing operations.

## TASK No.6: Implement UserInput_JoinLinesInsideParagraph(DocumentParagraphRun) Tests

Implement test cases for the `UserInput_JoinLinesInsideParagraph(Ptr<DocumentParagraphRun> paragraph, bool spaceForFlattenedLineBreak)` overload that removes all line breaks within a paragraph by recursively processing all text runs.

### what to be done

- Fill the `UserInput_JoinLinesInsideParagraph_DocumentParagraphRun` TEST_CATEGORY with complete test cases
- Test the recursive container traversal: uses a List-based queue to process all DocumentContainerRun objects starting with the paragraph
- Test DocumentTextRun processing: verify calls to the WString overload of UserInput_JoinLinesInsideParagraph for each text run found
- Test nested container structures: verify proper handling of deeply nested DocumentContainerRun hierarchies
- Test various paragraph structures: simple text runs, mixed content (text + container runs), deeply nested containers
- Test the queue-based traversal: verify all containers are properly added to the queue and processed
- Verify line break removal is applied to all text runs found in the container hierarchy
- Test the `spaceForFlattenedLineBreak` parameter propagation to the WString processing function

### how to test it

- Unit tests will confirm paragraph runs are processed using the List-based queue traversal
- Container traversal tests will verify all DocumentContainerRun objects are properly found and added to the queue
- Text run processing tests will verify DocumentTextRun objects are correctly identified and processed
- Parameter propagation tests will verify `spaceForFlattenedLineBreak` is correctly passed to the WString function
- Nested structure tests will ensure complex document hierarchies are handled correctly
- Integration tests will ensure proper coordination between container traversal and text run processing

### rationale

The recursive nature of DocumentParagraphRun processing requires thorough testing to ensure all nested text runs are found and processed correctly. The List-based queue traversal system is critical for maintaining document structure integrity while performing line removal operations on complex formatted content with arbitrary nesting levels.

The recursive nature of the DocumentParagraphRun overload requires thorough testing to ensure all nested text runs are processed correctly. This queue-based traversal system is critical for maintaining document structure integrity while performing line joining operations on complex formatted content with arbitrary nesting levels.

## TASK No.7: Implement UserInput_FormatText(List<WString>&) Tests

Implement test cases for the `UserInput_FormatText(collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)` overload that applies paragraph mode formatting to text lists based on configuration.

### what to be done

- Fill the `UserInput_FormatText_ListWString` TEST_CATEGORY with complete test cases
- Test the paragraph mode logic: when config.paragraphMode is Paragraph, nothing happens; when Multiline, removes line breaks in each paragraph; when Singleline, removes line breaks and concatenates all paragraphs to one
- Test the sequential processing: line joining is applied first (if paragraphMode != Paragraph), then paragraph joining (if paragraphMode == Singleline)
- Test all three GuiDocumentParagraphMode values: Paragraph (no processing), Multiline (line joining only), Singleline (line joining + paragraph joining)
- Test the `config.spaceForFlattenedLineBreak` parameter propagation to both UserInput_JoinLinesInsideParagraph and UserInput_JoinParagraphs functions
- Test various paragraph configurations and content scenarios with different modes
- Create GuiDocumentConfigEvaluated test configurations for each paragraph mode
- Verify the list is properly modified according to the configuration mode

### how to test it

- Configuration-driven tests will verify proper handling of GuiDocumentConfigEvaluated settings for each paragraph mode
- Sequential processing tests will verify the correct order: line joining first (when needed), then paragraph joining (when needed)
- Mode comparison tests will ensure Paragraph mode leaves content unchanged, Multiline applies line joining, Singleline applies both operations
- Parameter propagation tests will verify spaceForFlattenedLineBreak is passed correctly to sub-functions
- Integration tests will confirm the list is properly modified according to the configuration mode

### rationale

This function implements the core paragraph mode formatting logic used throughout the document system. Testing all three paragraph modes ensures the configuration-driven text processing works correctly for different document types (single-line textboxes, multi-line textboxes, full rich text documents). The sequential processing order (lines first, then paragraphs) needs verification for proper integration.

## TASK No.8: Implement UserInput_FormatText(WString to List<WString>&) Tests

Implement test cases for the `UserInput_FormatText(const WString& text, collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)` overload that parses text into paragraphs and applies formatting.

### what to be done

- Fill the `UserInput_FormatText_WStringToList` TEST_CATEGORY with complete test cases
- Test the paragraph detection logic: when config.doubleLineBreaksBetweenParagraph is true, uses complex FetchLineRecord logic to detect double line breaks as paragraph separators
- Test the simple line-by-line parsing: when config.doubleLineBreaksBetweenParagraph is false, uses StringReader to split by each line break as paragraph separator
- Test the complex double line break logic: detects empty lines as paragraph separators, handles fragment joining and ending empty lines
- Test the single-line mode application: calls UserInput_JoinParagraphs after paragraph parsing when config.paragraphMode == Singleline
- Test various input text patterns: single lines, multiple lines, double line breaks, mixed line ending formats
- Test the FetchLineRecord fragment processing: proper line grouping, empty line detection, and fragment submission
- Test edge cases: empty text, text ending with line breaks, text with only line breaks, remaining empty paragraph handling

### how to test it

- Paragraph detection tests will verify text splitting according to the doubleLineBreaksBetweenParagraph configuration
- Double line break processing tests will confirm complex FetchLineRecord logic works correctly for paragraph boundary detection
- Simple parsing tests will verify StringReader-based line splitting when doubleLineBreaksBetweenParagraph is false
- Fragment processing tests will ensure complex line grouping and empty line detection logic works correctly
- Single-line mode tests will verify UserInput_JoinParagraphs is called when config.paragraphMode == Singleline
- Edge case tests will handle various text boundary conditions and empty content scenarios

### rationale

The text-to-paragraphs conversion implements sophisticated paragraph detection logic that differs significantly based on the doubleLineBreaksBetweenParagraph configuration. The complex FetchLineRecord-based double line break detection requires thorough testing. This function is critical for paste operations and text import functionality where paragraph boundaries must be correctly identified based on line break patterns.

## TASK No.9: Implement UserInput_FormatDocument Tests

Implement test cases for `UserInput_FormatDocument` method that applies comprehensive configuration-driven formatting to entire DocumentModel instances.

### what to be done

- Fill the `UserInput_FormatDocument` TEST_CATEGORY with complete test cases
- Test the null safety check: method returns early if model is null
- Test the pasteAsPlainText processing: when config.pasteAsPlainText is true, calls UserInput_ConvertToPlainText on entire document (0 to paragraphs.Count() - 1) and handles style management
- Test baseline document style handling: when baselineDocument exists, copies styles using CopyFrom; when null, clears all styles
- Test the empty document check: method returns early if model has no paragraphs after plain text conversion
- Test paragraph mode processing: when config.paragraphMode != Paragraph, applies UserInput_JoinLinesInsideParagraph to all paragraphs; when Singleline, also calls UserInput_JoinParagraphs on the entire model
- Test the sequential processing order: plain text conversion → style handling → empty check → line joining → paragraph joining
- Test various GuiDocumentConfigEvaluated configurations and DocumentModel structures
- Test integration with all other UserInput methods called internally

### how to test it

- Null safety tests will verify proper early return behavior for null models
- Plain text conversion tests will verify UserInput_ConvertToPlainText is called with correct range (0, paragraphs.Count() - 1)
- Style management tests will confirm proper baseline style copying vs clearing based on baseline presence
- Empty document tests will verify early return when no paragraphs remain after processing
- Sequential processing tests will verify the exact order of operations is maintained
- Configuration integration tests will verify proper coordination with all called UserInput methods
- Comprehensive integration tests will ensure all formatting rules work together correctly

### rationale

`UserInput_FormatDocument` is the highest-level formatting orchestrator that coordinates all other UserInput methods to apply comprehensive document formatting. It implements the complete document processing pipeline used by paste operations and document loading. Testing this integration point ensures all formatting rules work together correctly and in the proper sequence, especially the style management and sequential processing logic.

# Impact to the Knowledge Base

## GacUI

### New Content to Add

The knowledge base currently doesn't contain specific guidance about testing document processing methods in the TextEditorPackage. The following should be added:

#### Testing Document Processing APIs

Document processing methods like UserInput_* functions require specific test patterns:

- Use DocumentModel creation helpers for consistent test setup with various formatting and content types
- Test both string-based and DocumentModel-based overloads for completeness, ensuring parameter consistency
- Verify configuration-driven behavior with GuiDocumentConfigEvaluated, especially paragraph mode handling
- Test edge cases like empty content, single items, boundary conditions, and early return scenarios
- Ensure cross-platform line ending compatibility in text processing tests (\r\n, \n, \r)
- Verify sequential processing order in complex functions that call multiple UserInput methods

#### Document Model Test Utilities

Testing document processing requires utility functions for:

- Creating test DocumentModel instances with various formatting, nested containers, and text runs
- Building GuiDocumentConfigEvaluated test configurations for different paragraph modes and line break settings
- Comparing document content and structure after processing, including run verification and alignment checks
- Generating test text with different line break patterns, empty lines, and content types for paragraph detection testing

#### FetchLineRecord System Testing

The FetchLineRecord-based line processing system requires specialized testing:

- Test line boundary detection with various line ending formats
- Verify early return optimizations when no additional lines are found
- Test complex fragment processing for double line break paragraph detection
- Ensure proper handling of empty lines and line grouping logic

### Existing Content Updates

No existing knowledge base content requires updates for this task, as it focuses on adding comprehensive test coverage for existing functionality rather than modifying existing APIs or design patterns. The updated test designs now accurately reflect the actual method behaviors as documented in the source code comments.

# !!!FINISHED!!!

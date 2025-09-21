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

# TASKS

- [ ] TASK No.1: Create TestDocumentConfig Test File with Empty TEST_CATEGORY Sections
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

Implement comprehensive test cases for the `UserInput_ConvertToPlainText` static method that removes formatting and converts document ranges to plain text.

### what to be done

- Fill the `UserInput_ConvertToPlainText` TEST_CATEGORY with complete test cases
- Create test cases that verify the method correctly removes formatting from specified paragraph ranges
- Test edge cases like empty ranges, single paragraph, multiple paragraphs
- Test that paragraph alignments are properly reset
- Verify the method handles invalid ranges gracefully (beginParagraph > endParagraph)
- Create helper functions to build test DocumentModel instances with various formatting
- Test both simple text content and complex formatted content scenarios

### how to test it

- Unit tests will verify that formatted DocumentModel paragraphs are converted to plain text correctly
- Tests will confirm that paragraph alignment properties are reset after conversion
- Boundary condition tests will ensure the method handles edge cases properly
- Tests will verify the method doesn't modify paragraphs outside the specified range

### rationale

`UserInput_ConvertToPlainText` is a core text processing function that strips formatting from documents. Comprehensive testing ensures this fundamental operation works correctly across all scenarios, as formatting removal is critical for copy/paste operations and text export functionality.

## TASK No.3: Implement UserInput_JoinParagraphs(List<WString>&) Tests

Implement test cases for the `UserInput_JoinParagraphs(collections::List<WString>& paragraphTexts, bool spaceForFlattenedLineBreak)` overload that joins string paragraphs.

### what to be done

- Fill the `UserInput_JoinParagraphs_ListWString` TEST_CATEGORY with complete test cases
- Test the string list overload that joins string paragraphs with optional space insertion
- Verify proper handling of the `spaceForFlattenedLineBreak` parameter
- Test empty paragraph lists and single paragraph scenarios
- Test multiple paragraphs with various text content
- Test paragraphs with different line ending patterns
- Verify the result is a single combined paragraph in the list

### how to test it

- Unit tests will verify string paragraphs are joined correctly with and without spaces
- Tests will confirm the `spaceForFlattenedLineBreak` parameter behavior
- Edge case tests will handle empty and single-item scenarios
- Tests will verify the original list is properly modified to contain the joined result

### rationale

This specific overload handles string-based paragraph joining which is fundamental for text processing. Testing this overload separately from the DocumentModel version ensures both APIs work correctly and consistently, which is crucial for maintaining data integrity during text manipulation.

## TASK No.4: Implement UserInput_JoinParagraphs(DocumentModel) Tests

Implement test cases for the `UserInput_JoinParagraphs(Ptr<DocumentModel> model, bool spaceForFlattenedLineBreak)` overload that joins DocumentModel paragraphs.

### what to be done

- Fill the `UserInput_JoinParagraphs_DocumentModel` TEST_CATEGORY with complete test cases
- Test the DocumentModel overload that joins paragraph runs with optional space insertion
- Verify proper handling of the `spaceForFlattenedLineBreak` parameter
- Test single and multiple paragraph scenarios
- Test various DocumentModel content types including text runs and formatted content
- Ensure the DocumentModel variant properly merges runs from multiple paragraphs into the first paragraph
- Verify that the document structure remains valid after joining

### how to test it

- Unit tests will confirm DocumentModel paragraphs are merged properly with run content preserved
- Tests will verify the `spaceForFlattenedLineBreak` parameter behavior in the DocumentModel context
- Structure validation tests will ensure the resulting DocumentModel maintains integrity
- Tests will verify that only the first paragraph remains and contains all merged content

### rationale

The DocumentModel overload handles complex document structure merging, which is more sophisticated than simple string joining. Testing this separately ensures proper handling of formatted content, runs, and document structure integrity during paragraph consolidation operations.

## TASK No.5: Implement UserInput_JoinLinesInsideParagraph(WString&) Tests

Implement test cases for the `UserInput_JoinLinesInsideParagraph(WString& text, bool spaceForFlattenedLineBreak)` overload that flattens line breaks within text strings.

### what to be done

- Fill the `UserInput_JoinLinesInsideParagraph_WString` TEST_CATEGORY with complete test cases
- Test the WString overload that joins lines within a text string
- Verify proper line break flattening with and without space insertion
- Test various line break scenarios including Windows (\r\n), Unix (\n), and Mac (\r) line endings
- Test mixed line ending formats within the same string
- Verify the method handles empty strings and single-line content properly
- Test strings with only line breaks and no other content

### how to test it

- Unit tests will verify line breaks are properly flattened within text strings
- Line ending format tests will ensure cross-platform compatibility
- Tests will verify the `spaceForFlattenedLineBreak` parameter functions correctly
- Edge case tests will handle various string content scenarios

### rationale

Line joining within text strings is crucial for text normalization and display formatting. The string-based overload needs thorough testing to ensure it handles all line ending formats correctly and maintains cross-platform compatibility for text processing operations.

## TASK No.6: Implement UserInput_JoinLinesInsideParagraph(DocumentParagraphRun) Tests

Implement test cases for the `UserInput_JoinLinesInsideParagraph(Ptr<DocumentParagraphRun> paragraph, bool spaceForFlattenedLineBreak)` overload that processes paragraph runs recursively.

### what to be done

- Fill the `UserInput_JoinLinesInsideParagraph_DocumentParagraphRun` TEST_CATEGORY with complete test cases
- Test the DocumentParagraphRun overload that processes all text runs within a paragraph container
- Test nested container runs to ensure the recursive processing works correctly
- Verify line break flattening within text runs embedded in the paragraph structure
- Test various paragraph structures including simple and complex nested runs
- Ensure the method properly traverses and processes all text runs in the hierarchy
- Test paragraphs with mixed content types (text runs, container runs, etc.)

### how to test it

- Unit tests will confirm paragraph runs are processed recursively for nested containers
- Structure traversal tests will verify all text runs are properly processed
- Tests will verify the `spaceForFlattenedLineBreak` parameter functions correctly in document context
- Complex structure tests will ensure proper handling of deeply nested run hierarchies

### rationale

The recursive nature of the DocumentParagraphRun overload requires thorough testing to ensure all nested text runs are processed correctly. This is critical for maintaining document structure integrity while performing line joining operations on complex formatted content.

## TASK No.7: Implement UserInput_FormatText(List<WString>&) Tests

Implement test cases for the `UserInput_FormatText(collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)` overload that formats paragraph text lists.

### what to be done

- Fill the `UserInput_FormatText_ListWString` TEST_CATEGORY with complete test cases
- Test the List<WString> overload that formats paragraph text lists according to configuration
- Verify behavior with different `GuiDocumentParagraphMode` values (Paragraph, Multiline, Singleline)
- Test the `spaceForFlattenedLineBreak` configuration parameter
- Create test configurations representing different document formatting scenarios
- Test with multiple paragraphs and various line break patterns within each paragraph
- Verify proper interaction with UserInput_JoinLinesInsideParagraph and UserInput_JoinParagraphs

### how to test it

- Unit tests will verify text formatting according to different paragraph modes
- Configuration-driven tests will ensure proper handling of various document settings
- Integration tests will verify the coordination between different formatting options
- Tests will confirm the list is properly modified according to configuration

### rationale

Text formatting based on configuration is essential for adapting content to different document types and user preferences. Testing this overload separately ensures the string-based formatting logic works correctly for all configuration combinations.

## TASK No.8: Implement UserInput_FormatText(WString to List<WString>&) Tests

Implement test cases for the `UserInput_FormatText(const WString& text, collections::List<WString>& paragraphTexts, const GuiDocumentConfigEvaluated& config)` overload that splits and formats text.

### what to be done

- Fill the `UserInput_FormatText_WStringToList` TEST_CATEGORY with complete test cases
- Test the single WString overload that splits text into paragraphs and formats them
- Verify behavior with the `doubleLineBreaksBetweenParagraph` configuration option
- Test various input text patterns with different line break combinations
- Test complex text with multiple paragraphs and various line break patterns
- Verify proper paragraph detection and splitting logic
- Test interaction with paragraph mode settings after splitting

### how to test it

- Unit tests will verify text splitting into paragraphs according to configuration
- Line break processing tests will confirm double line break paragraph detection works correctly
- Integration tests will verify the coordination between splitting and formatting operations
- Tests will ensure the output list contains properly formatted paragraphs

### rationale

The text-to-paragraphs conversion is a complex operation that requires sophisticated line break analysis. Testing this overload separately ensures the splitting logic works correctly and integrates properly with the formatting engine for different document configurations.

## TASK No.9: Implement UserInput_FormatDocument Tests

## TASK No.9: Implement UserInput_FormatDocument Tests

Implement test cases for `UserInput_FormatDocument` method that applies comprehensive formatting to entire document models.

### what to be done

- Fill the `UserInput_FormatDocument` TEST_CATEGORY with complete test cases
- Test document formatting using baseline document comparison
- Verify the method properly applies GuiDocumentConfigEvaluated settings to document models
- Test scenarios where baseline document is provided vs null baseline
- Test various document structures including complex nested runs and multiple paragraphs
- Verify integration with other UserInput methods called internally
- Test performance considerations with reasonably sized documents
- Ensure proper handling of document model copying and modification

### how to test it

- Unit tests will verify complete document formatting using various configuration settings
- Baseline comparison tests will ensure proper handling of reference documents
- Performance tests will verify the method handles reasonably sized documents efficiently
- Integration tests will confirm proper coordination with other UserInput methods

### rationale

`UserInput_FormatDocument` is the highest-level formatting method that orchestrates the entire document formatting process. Comprehensive testing ensures this central method correctly applies all formatting rules and maintains document consistency, as it's likely used by high-level editor operations like paste and import functions.

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

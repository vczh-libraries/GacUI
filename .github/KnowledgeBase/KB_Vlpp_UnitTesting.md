# Unit Testing Framework

Testing infrastructure with hierarchical test organization and assertion capabilities.

## Basic Test Structure

The unit testing framework in Vlpp provides a hierarchical structure for organizing and executing tests with assertion capabilities. All tests are written in C++ using specific macros that define the test structure.

### Writing Test Code

```C++
using namespace vl;
using namespace vl::unittest;

TEST_FILE
{
	TEST_CASE(L"TOPIC-NAME")
	{
		TEST_ASSERT(EXPRESSION-TO-VERIFY);
	});

	TEST_CATEGORY(L"CATEGORY-NAME")
	{
		TEST_CASE(L"TOPIC-NAME")
		{
			TEST_ASSERT(EXPRESSION-TO-VERIFY);
		});
	});
}
```

If a test case contains only one call to `TEST_ASSERT`, it can be simplified to `TEST_CASE_ASSERT(EXPRESSION-TO-VERIFY);`. The `TOPIC-NAME` becomes the condition.

## Hierarchical Organization

### TEST_FILE

`TEST_FILE` defines the test file scope and serves as the root container for all test cases and categories within a source file. There can be only one `TEST_FILE` per source file.

### TEST_CATEGORY

`TEST_CATEGORY(L"CATEGORY-NAME")` groups related tests under a descriptive category name. Key characteristics:

- Multiple `TEST_CATEGORY` blocks can exist within `TEST_FILE` or nested within other `TEST_CATEGORY` blocks
- Categories can be nested inside other categories for hierarchical organization
- Categories help organize tests logically by functionality or feature area
- Category names should be descriptive and use wide character string literals

### TEST_CASE

`TEST_CASE(L"TOPIC-NAME")` defines individual test implementations. Key characteristics:

- Multiple `TEST_CASE` blocks can exist within `TEST_FILE` or `TEST_CATEGORY`
- Test cases cannot be nested inside other test cases
- Each test case should focus on testing a specific behavior or functionality
- Test case names should clearly describe what is being tested

## Test Assertions

### TEST_ASSERT

`TEST_ASSERT(EXPRESSION-TO-VERIFY)` performs test assertions within test cases. Key characteristics:

- Can only appear inside `TEST_CASE` blocks
- Takes a boolean expression that should evaluate to true for the test to pass
- When the expression evaluates to false, the test fails and reports the failure
- Multiple assertions can be used within a single test case

## Test Execution Integration

### Integration with Main Function

Tests are typically executed through the main function using the unit test runner:

```C++
int main(int argc, char** argv)
{
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	vl::FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
```

### Command Line Options

The unit test framework supports various command line options for controlling test execution:

- `/D`: Disable failure suppression (for debugging)
- `/R`: Enable failure suppression (for release mode)
- `/F:TestFile`: Run only specific test files

### Test Output and Reporting

The framework provides colored console output to distinguish between different types of messages:
- Error messages (test failures)
- Info messages (general information)
- File messages (test file names)
- Category messages (test category names)
- Case messages (test case names)

## Extra Content

### Test Registration

Tests are automatically registered using a global registration system. Each test file is linked into a global list through the `UnitTestLink` structure, which contains:
- The source file name for identification
- A function pointer to the test procedure
- A pointer to the next test file in the linked list

### Error Handling and Exception Safety

The unit testing framework includes comprehensive error handling:

**Assertion Errors**: When `TEST_ASSERT` fails, it throws a `UnitTestAssertError` containing the failure message.

**Configuration Errors**: Invalid test structure (like using `TEST_ASSERT` outside `TEST_CASE`) throws `UnitTestConfigError`.

**Exception Suppression**: The framework can suppress various types of exceptions during test execution:
- C++ exceptions (`Error`, `Exception`, and standard exceptions)
- System exceptions (on Windows using `__try/__except`)
- Unknown exceptions

### Test Context Management

The framework maintains a hierarchical context during test execution:
- **File Context**: Tracks the current test file being executed
- **Category Context**: Tracks nested test categories
- **Case Context**: Tracks individual test cases
- **Failure Tracking**: Each context level tracks whether failures occurred

### Statistics Tracking

The framework maintains comprehensive statistics:
- Total number of test files executed
- Number of test files that passed
- Total number of test cases executed  
- Number of test cases that passed
- Overall pass/fail status

### Debugger Integration

The framework automatically detects whether a debugger is attached:
- When debugging, failures are not suppressed by default (to break into debugger)
- When running normally, failures are suppressed to continue execution
- This behavior can be overridden with command line options

### Memory Leak Detection Integration

The unit testing framework integrates with the memory leak detection system:
- Tests should call `FinalizeGlobalStorage()` before checking for memory leaks
- This ensures proper cleanup of global storage objects before leak detection
- On Windows with MSVC, `_CrtDumpMemoryLeaks()` can be called after finalization

### Cross-Platform Considerations

The framework is designed to work across platforms:
- Console color support varies by platform
- Exception handling mechanisms may differ (Windows has additional __try/__except support)
- File path handling respects platform-specific path separators when filtering tests by file name

### Best Practices

1. **Descriptive Names**: Use clear, descriptive names for test categories and cases
2. **Single Responsibility**: Each test case should test one specific behavior
3. **Assertion Clarity**: Write assertions that clearly indicate what is being tested
4. **Hierarchical Organization**: Use categories to group related tests logically
5. **Error Context**: When tests fail, the hierarchical structure provides clear context about which test failed
6. **Resource Cleanup**: Ensure proper cleanup in tests, especially when testing resource management

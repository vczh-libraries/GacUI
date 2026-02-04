# Using Unit Test Framework

The most simple way to start the unit test framework would be: ```c++ #include <Vlpp.h> using namespace vl; using namespace vl::unittest; #ifdef VCZH_MSVC int wmain(int argc , wchar_t* argv[]) #else int main(int argc, char** argv) #endif { int result = unittest::UnitTest::RunAndDisposeTests(argc, argv); vl::FinalizeGlobalStorage(); #ifdef VCZH_CHECK_MEMORY_LEAKS _CrtDumpMemoryLeaks(); #endif return result; } ```

In this way you could pass **/D**, **/R**, **/C** and **/F** directly to your unit test binary and they will be handled by the unit test framework properly.

You can create multiple cpp files for your test cases, **/F** will automatically react to them. A typical test file would be: ```c++ #include <Vlpp.h> using namespace vl; using namespace vl::unittest; void AnError() { CHECK_FAIL(L"Not Implemented!"); } void AnException() { throw Exception(L"A message"); } TEST_FILE { TEST_CASE(L"This is a test case") { TEST_ASSERT(1 + 1 == 2); }); TEST_CATEGORY(L"This is a test category") { TEST_CASE(L"This is another test case") { TEST_ERROR(AnError()); TEST_EXCEPTION(AnException(), Exception, [](const Exception& e) { TEST_ASSERT(e.Message() == L"A message"); }); }); }); } ``` - **TEST_ASSERT(condition);** accepts an expression and fails if it evaluates to false. - **TEST_ERROR(statement);** accepts a statement and fails if it does not throw an exception is or inheriting from **Error**. Such error are usually triggered by **CHECK_ERROR** and **CHECK_FAIL**. - **TEST_EXCEPTION(statement,exception,handler);** executes the statement, fails if it does not throw an exception is or inheriting from **exception**. The handler will be called on that exception for additional assertion. Please be aware that these are macros, commas may be interepreted unexpectedly.

**TEST_CATEGORY** could be used to group other **TEST_CATEGORY** and **TEST_CASE**. **TEST_ASSERT**, **TEST_ERROR** and **TEST_EXCEPTION** can only be used in **TEST_CASE**. **TEST_CASE_ASSERT(condition);** is a shortcut for a **TEST_CASE** containing only a **TEST_ASSERT**.

If you get **TestA.cpp**, **TestB.cpp** and **TestC.cpp**, using no **/F** will run tests in all three files, using **/F:Test*.cpp** will run tests in all three files, using **/F:TestA.cpp /F:TestB.cpp** will run tests in **TestA.cpp** and **TestB.cpp** only.


# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

## Task 1

This task happens in `VlppOS` repo.

Add a helper function `Nullable<WString> HttpServerApi::GetUtf8Body(PHTTP_REQUEST pRequest)`. Make it static as possible.
- Use implementation from `HttpServer` for reading the complete utf8 body, raise exception as exactly what has been done in `HttpServer`.
Add a helper function `void HttpServerApi::SendResponseUtf8(HANDLE httpRequestQueue, HTTP_REQUEST_ID requestId, WString body)`.
- Use `{ 200, WString::Unmanaged(L"OK"), body, L"application/json; charset=utf8" }` to call the original `SendResponse`.
In `HttpServer` and other places should use the above helper functions

## Task 2

This task happens in `GacUI` repo.
Release `VlppOS` and `VlppParser2 to `GacUI`.
`GacUI` already uses the above functions, ensure it compile.

# UPDATES

# TEST [CONFIRMED]

Use downstream compile coverage after importing dependency releases:

- Copy generated C++ release files from `VlppOS\Release` into `GacUI\Import`, excluding `IncludeOnly`.
- Copy generated C++ release files from `VlppParser2\Release` into `GacUI\Import`, excluding `IncludeOnly`.
- Build `Test\GacUISrc\GacUISrc.sln` in `GacUI`. `Source\PlatformProviders\Windows\WinNativeWindow.cpp` already calls `HttpServerApi::GetUtf8Body` and `HttpServerApi::SendResponseUtf8`, so compile success confirms the imported APIs and call sites match.
- Success criteria: the GacUI solution builds successfully with 0 errors, and the GacUI unit test passes without a memory leak report.

# PROPOSALS

- No.1 Import regenerated dependency releases and align GacUI call site [CONFIRMED]

## No.1 Import regenerated dependency releases and align GacUI call site

Release `VlppOS` and `VlppParser2` into `GacUI\Import`. Since `HttpServerApi::GetUtf8Body` returns `Nullable<WString>`, update the existing GacUI HTTP automation service call site to unbox the returned body before passing it into the main-thread command handler.

### CODE CHANGE

- Copied regenerated `VlppOS` release files into `GacUI\Import`; `Import\VlppOS.Windows.h` and `Import\VlppOS.Windows.cpp` now expose `HttpServerApi::GetUtf8Body` and `HttpServerApi::SendResponseUtf8`.
- Regenerated `VlppParser2\Release` and copied the generated parser release into `GacUI\Import`; `Import\VlppGlrParser.h` reflects the regenerated output.
- Updated `HttpAutomationService` in `Source\PlatformProviders\Windows\WinNativeWindow.cpp` to call `GetUtf8Body(pRequest).Value()`.
- Updated the same HTTP `/IO` handler to call `automationService->RunIOCommand({}, body)` to match the existing `INativeAutomationService` interface.

### CONFIRMED

Built `Test\GacUISrc\GacUISrc.sln` with `copilotBuild.ps1`: build succeeded with 0 errors and one pre-existing `C4297` warning in `RemotingTest_Rendering_Win32\Main.cpp`. Ran `UnitTest` with `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: all 84/84 test files and 1686/1686 test cases passed, and `Execute.log.memoryleaks` was empty.

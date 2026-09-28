# Coding Convention

In general, here is my preference for any languages:
- You are recommended to debug the compiled binary: 
  - Once it crashes.
  - When after one attemp of failed guessing to fix.
  - But respect to `Project.md` first.
- I am a fan of crash early. When something should happen, it should just happen, do not play a game like "what if it is not the case" and silently covers the issue. One example is that, if an object should not be null, then we should just use it, if a nullable object should not be null, we should just cast it. No test is performed in this case, using it will crash if it is null, and we know there is a problem. Fix the actual problem instead of doing "error tolerance".
- I am a fan of **DO NOT REPEAT YOURSELF (DRY)**.
  - DRY focus on not repeating information in source code. For example, compiler always do name mangling, but name mangling is complex. If you implementaion the mangling in two different places, you repeat the information twice. Therefore a function for such thing is always needed.
  - DRY does not focus on not repeating some code. For example, create `json::JsonString` requires filling its field. The way to creat it does not offer any new information. So a three-lines function just to create `json::JsonString` and copy the argument to its field is not needed.
    - But if building an AST requires significantly more lines of code, extracting functions for the work is preferred.
  - DRY requires finding if a feature has already been implemented somewhere else before implementing it. avoiding massive duplication.
    - If the existing implementation is not sharable, refactoring is preferred.
- Unless explicitly instructed, do not invent your own code generator as part of the committed solution. It adds more technical debt in maintenance.
- When it is not explicltly instructed, be conservative on creating new layers of abstractions:
  - It should bring actual benefits, including better separation of ownership and dependencies, allowing needed extensibility.
  - Extraction of common code would also be a good reason, when it doesn't fragmentize the code. I am not a fan of big amounts of small functions.
  - Extractin of functions/classes are welcome, when there are large pieces of duplicated code, or when the same logic is duplicated >= 3 times.
  - Extraction of common interfaces (aka abstract classes in C++) are welcome, when the details of implementation does not really affect how algorithms and tools are built around the concept.
- Unless explicitly instructed:
  - Prefer static analyzing (including ultilizing the type system, or static assertions, etc) over dynamic/runtime assertion.
  - Prefer correctness over stability.
    - Prefer crashing early over recovery.
    - Crashing expose issues immediately, if something can be fixed and made correctly, there is no need to care about what to do when it fails unexpectedly.
    - Expected failures should be part of the signature, when the type system cannot represents expected failures, comments are needed.
  - Handle exceptions only when recovery is practical. DO NOT consume exceptions silently.

## Be Brave Enough to Fix Upstream Code and Make Breaking Change

This section is a high level philosophy of trade-offs during making decision of where to fix the code.
It looks amgibuous, that actually means you are expected to consider the context of each issues you are facing to.

- Always fix the bug at its root cause.
  - If you find any API that doesn't work, fix instead of making a replacement. Even when the API is in an upstream repo, prefer fixing in the upstream repo and releasing it to the current repo.
- DO NOT concern about making breaking change.
  - The project is well covered by unit test, any unexpecting breaking change is highly possibly to be cought.
  - If such breaking change is intented, unit test could help you perform complete refactoring.
  - If an API design does not fit the requirement or contract, just change it.
- Interface design could have been wrong, DO NOT implement twisted logic just to finish the current task while fitting the wrong design.
- But DO NOT leaks information from downstream to upstream repos.
  - Each upstream repo has its own scope, contract, policy, strategy, etc.
  - Each upstream repo releases libraries that is supposed to serve broader purposes.
  - Keep interfaces between repos clean.
  - If downstream repos need to need to do something depending on upstream repos' non-public information:
    - If the issue is the upstream repo itself, fix the upstream repo.
    - Otherwise, expose informations elegantly so that downstream repos could process and react to them properly.

## C++ Thread Safety and Multi-Threading Synchronization

Check out [Coding_MultiThreading.md](./Coding_MultiThreading.md).

## C++ Coding Convention

- Anonymouse namespace `namespace{}` is not welcomed:
  - DO NOT generate such construction.
  - When you edit any existing code and see this, remove that anonymouse namespace and fix the indentation of the content.
  - The reason is that, moust of cpp files will be merged into one single file apon release, all benefits are gone meanwhile the code looks messy.
- Impl classes like `class Something { class Impl; Ptr<Impl> impl; }` is not always welcomed:
  - Hiding information or improve building performance with this pattern is considered incorrect here.
  - The only exception is forcing to hide platform dependend constructions, like `vl::Mutex`.
- Although C++ does not require this but we want to have `extern` on all function forward declarations.
  - In general we don't use `inline` in header files unless such function is performance critical, e.g. very simple comparison operators.
- Rules for C++ header files:
  - Guard them with macros instead of `#pragma once`.
  - In a class/struct/union declaration, member names must be aligned in the same column at least in the same public, protected or private section.
  - Keep the coding style consistent with other header files in the same project.
- Extra Rules for C++ header files in `Source` folder:
  - Do not use `using namespace` statements; the full names of types are always required.
- Rules for cpp files:
  - Use `using namespace` statement if necessary to prevent from repeating namespace everywhere.
  - `vl::stream::` is an exception, always use `stream::` with `using namespace vl;`, DO NOT use `using namespace vl::stream;`.

## Basic C++ Library Leveraging

- This project uses C++ 20, you are recommended to use new C++ 20 features aggressively.
- All code should be cross-platform. In case when an OS feature is needed, a Windows version and a Linux version should be prepared in different files, following the `*.Windows.cpp` and `*.Linux.cpp` naming convention, and keep them as small as possible.
- DO NOT MODIFY any source code in the `Import` folder, they are dependencies.
- DO NOT MODIFY any source code in the `Release` folder, they are generated release files.
- You can modify source code in the `Source` and `Test` folder.
- Use tabs for indentation in C++ source code.
- Use double spaces for indentation for JSON or XML embedded in C++ source code.
- Use `auto` to define variables if it is doable. Use `auto&&` when the type is big or when it is a collection type.
- The project only uses a very minimal subset of the standard library. I have substitutions for most of the STL constructions. Always use mine if possible:
  - Always use `vint` instead of `int`.
  - Always use `L'x'`, `L"x"`, `wchar_t`, `const wchar_t` and `vl::WString`, instead of `std::string` or `std::wstring`.
  - Always use `FilePath` for file path operations.
  - Use my own collection types vl::collections::* instead of std::*
  - Check out `REPO-ROOT/.github/KnowledgeBase/Index.md` for more information on how to choose the correct C++ data types.
- To attach availability semantic to a value:
  - If any number is expected to be valid only when non-negative, you could use `-1` to represent invalid value.
  - If an object is expected to be valid only when non-null, you could use `nullptr` on `T*` or `Ptr<T>` to represent invalid value.
  - Use `Nullable<T>` to represent any invalid value if possible.
    - DO NOT use `Nullable<T*>`, `Nullable<Ptr<T>>` or `Nullable<Nullable<T>>`, this is too confusing.
  - Only when there is no other choice, use an extra `bool` variable.
    - This could happen when "null" semantic is valid.

### Regular Expression

Regular expression utilities are offered by `vl::regex::Regex`, here are important syntax differences from other regular expression implementations:
- "." means the dot character, "/." or "\." (or "\\." in C++ string literal) means any character.
- Both "/" and "\" escape characters, you are recommended to use "/" in C++ string literals.
- Therefore you need "//" for the "/" character and "/\\" or "/\\\\" for the "\" character in C++ string literals.
- Constructing a `Regex` object is expensive. If a regular expression is used multiple times or multiple places, make a variable to reuse it, but it should not be a global variable.

### Creating and Using Parsers

When `VlppParser2` is available to the current project, complex parsers always require to use `VlppParser2`. There are already existing parsers, especially XML and JSON.
- Each parser has a generated `Parser` class, you are always required to use the last piece of namespace with it, e.g. `xml::Parser` and `json::Parser`. `glr::xml::Parser` and `glr::json::Parser` is also equally good.
- Some parsers like XML/JSON has its own parse function `XmlParseDocument`, `XmlParseElement`, `JsonParse`, it has extra preprocessing, they are always required to use instead of using `xml::Parser` directly.
  - Only if such functions cannot be found for a certain parser, the `Parser` class can be used directly.
- Creating a `Parser` class is super expensive, you must do your best to share it across the project:
  - Any `Parser` class is re-entrant, you can run it parallelly in multiple threads.
  - Any unit test project should already have a way to share involved parsers. You are recommended to follow the pattern if you need to use a new parser.
    - The usual pattern would be having a pointer to that parser as a global variable, and a pair of functions for lazy initialization or finalization. And the main function will explicitly call the finalize function to avoid messing up memory leak detection.
  - `GacUI` project has a mechanism to register parsers dynamically.
- Each parser should already provide functions for converting AST back to string, you should not invent it by your own, unless you are making a new parser.
- Each parser should already provide multiple visitors, try to reuse them. To invent your own algorithm, especially recursive algorithm, you should always try to create visitors.

### for Reflectable Types

- Any interface or class `X` should inherit from `vl::reflection::Description<X>`.
  - If such a class (not including interface) should be inheritable in Workflow script, use `AggregatableDescription` instead of `Description`.
  - If a class inherits directly or indirectly from multiple registered classes/interfaces:
    - Either register this class.
    - Or if multiple registered base types are all interfaces, another valid option would be to create a registered interface inheriting all of them, and let the class inherits from this new interface.
    - The reason is that, an object only has one pointer to a piece of reflection metadata. If a class is not registered but it inherits from multiple registered types, only a metadata from one of these base types will be brought along with the actual
object, causing missing of a complete picture.
- No `const` is allowed for methods or reference types.
- Prefer `IValue*` interfaces for container types on interfaces.
- Container types and some other types support range-based for loop. Always prefer range-based for loop over other loops.
  - You can use `indexed(container)` to convert a container of type `T` to `Pair<T, vint>`, to read the correct index.
  - Avoid using an expression that creates temporary objects in `for(... : HERE)` or `for(... : indexed(HERE))`. The current C++ destroys the temporary object too early; therefore this becomes UB.
- Prefer Inversion of Control (IoC) and other design patterns, over trivial virtual functions, over switch-case on types, over if-else on types.
- Prefer static dispatching over dynamic dispatching when possible and reasonable.
- Unless explicitly instructed:
  - You are not allowed to test if `VCZH_DEBUG_NO_REFLECTION` is defined.
  - You are not allowed to test if `VCZH_DEBUG_METAONLY_REFLECTION` is defined.
  - You are not allowed to call any function that does not work with `VCZH_DEBUG_NO_REFLECTION`.
  - Reflection registration is an exception follow the document for recommended patterns.

## Advanced C++ Coding Rules

- DO NOT make helper functions that are only used once, especially if they are only called in one destructor.
- DO NOT make global variables with types that carry constructors or destructors, even when they are implicit.
  - This could mess up the order of initialization, finalization or memory leak detector.
  - One exception is `WString` which is initialized using `WString::Unmanaged`; such constructors and destructors do not do memory management.
  - Another exception is `Pair`, `Nullable`, `Variant` or `Tuple` with valid types here.
  - If pointers are needed, you could only use `T*` and do initialization or finalization explicitly. All such objects should be destroyed in `main`, `wmain`, `WinMain` or `GuiMain`, before memory leak detector runs.
- DO NOT reset any raw/shared pointer member to nullPTR in destructorS.
- Prefer the latest C++ features (up to C++ 20).
- Prefer template variadic arguments, over hard-coded-counting solutions.

### Object Oriented Programming

- I don't have strong preference of which is bettern between inheritance and composition, choose the best one in the context.
- When doing inheritance, the interface design should follow LSP (Liskov Substitution Principle), that is basically:
  - Wherever accepts a type, it means all sub types could be used.
  - Any sub type should not loosen the contract, stricker contract is allowed/encouraged.
- I prefer interface oriented programming.
  - Interface is a general concept, I an not forcing creating abstract classes.
  - Interface means contract. Such contract could be forced by the type system, or described in the class/method level comments on signatures.
- Ownership should be clear.
  - A owns B is usually implemented by `A` inheriting from `B`, `A` having a `Ptr<B>` or `B` field, unless further instructed.
    - Inheriting happens only when it satisfies LSP.
    - Using `B` is preferred, and exposing `B*`, `B&`, or `const B&` is also easy to do.
    - Only use `Ptr<B>` when the relationship is able to rebuild, or the interface requires exposing `Ptr<B>`.
  - Other relationships are usually implemented by A having a `B*` field, with manual lifecycle maintenance.
  - For a group of coupled objects without ownership, it is also acceptable to create one object owning all sub objects, to control their lifecycle in an easy way. 

### Combinator Oriented Programming

- Combinator oriented programming is very common in this project. that is basically:
  - Design an interface that is supposed to build up large/complex strategy or logic recursively, e.g. `vl::collections::LazyList<T>`, `vl::stream::IStream` or `vl::presentation::controls::GuiControl`.
  - Such interface should satisfy LSP.
- A good interface maintains good shape in components forming up a recursive algorithms by cutting in a good place, therefore making it simple, and easy to implement.
- A good interface letting users building recursive algorithms with fluent code.

### RAII (Resource Acquisition Is Initialization)

- RAII is preferred in:
  - Building a value type.
  - Ensure initialization/finalization to happen in pair.
  - Exception handling.
- Especially in exception handling, if a finalization should happen, write it in a destructor, instead of running the same piece of code everywhere in `catch` and before `return`.
  - Such finalization includes but not limited to:
    - Free an object or memory (`Ptr<T>` is a good example).
    - Release a lock (`SPIN_LOCK` and other similar macros are good examples).
- If an exception is surely going to crash the app and nothing will recover from it:
  - It usually means that under the situation, ensuring finalization to execute might bring no benefits.
  - In this case, just call finalization as if there will be no exception, no RAII or try-catch is needed here.
  - Such scenario is very usual in test apps. In shared library it is less likely to happen.

## Keep C++ Code Cross Platform

- All source files must aim for cross platform unless the file name has `.Windows.`, `.Linux.`, `.macOS.` or `.Wasm.`.
  - When macOS or WebAssembly can mostly reuse the Linux implementation, keep the shared code in `*.Linux.*` and guard the few platform differences inside that file.
  - Use `#if defined VCZH_GCC || defined VCZH_WASM` for Linux files shared by native Linux/macOS and WebAssembly. Use explicit `#if` / `#elif` conditions for the platform differences.
  - Use separate `*.macOS.*` or `*.Wasm.*` files only when the implementation cannot substantially share the Linux code.
- Use FilePath to normalize file path, for file path operations and delimiter access.
- If platform specific API could be used, avoid hard-coding a table.
- If not all OS provides enough platform specific API for a requirement:
  - Use platform specific API when the quality and performance would be better for that platform.
    - Except that the calulation is so tiny and simple, then it is fine to do a cross-platform solution instead.
  - Implement the solution for the rest of the platform.

## When Seeing Memory Leaks and Unstable Bugs

- Memory leaks and unstable bugs will always be the highest priority issue.
- Detecting memory leaks:
  - On Windows:
    - By correctly configuring vcxproj files and calling expected tools in the `main` or `WinMain` function, memory leaks would always be detected and logged after a successful unit test run.
    - For non-unit test projects, attaching a debugger would reveal it. 
    - Detect memory leaks with `Debug`, do not detect memory leaks with `Release`.
  - Memory leak detection is not required to be done on Linux and macOS for now.
- Detecting unstable bugs:
  - One of a typical example would be about dangling pointers.
  - When experiencing bugs that can't repro in every test run, do not easily treat it as a non-issue.
  - The standard way in this project would be to repeat the test 25 times, and only treat it as a non-issue only if observing consecutive 25 successful runs.
- Some unit test projects might take a long time to run, therefore it is strongly recommended to use one or multiple `/F:FileName.cpp` to limit the scope to where such issues could happen first, and then use the discovered effective and efficient set of `/F:FileName.cpp` to help testing the fix, instead of always run the whole unit test project.
- Whenever we see such issues:
  - If it is a new issue due to completing the current task, you have to address that as part of the current task.
  - If it is an existing issue, you still have to fix the issue along with the current task. And when the current task requires commit and push, make a separate commit to fix the issue, and push it together. 

## When Seeing Unstable Bugs

- One of a typical example would be about dangling pointers.
- When experiencing bugs that can't repro in every test run, do not easily treat it as non-issue.
- The standard way in this project would be to repeat the test 25 times, and only define non-issue as observing consecutive 25 successful runs.
- If we can't the consecutive 25 successful runs:
  - Such issue would be

## Workflow Script Coding Convention

- Avoid explicit type specification whenever possible:
  - Prefer `var v = e;` whenever `T` can be omitted.
  - Prefer `var v : T = e;` over `var v = e as T;` if `T` cannot avoid.
  - When implicit type conversion works at the place, avoid `cast`, `as` and `infer` expression.
  - Prefer `cast *` over `cast T` when the context accepts `T`.
- Nested `try-catch` and `try-finally` can be merged into one single `try-catch-finally` statement.
- Prefer strong typed collections in Workflow, but when writing C++ reflectable interfaces, use `Ptr<IValue*>`.

## Workflow Script Generation in C++

- When generating Workflow script, avoid building text, you should always build the AST. The AST type for a complete Workflow script module is `WfModule`.


## Working with Web Assembly

- WebAssembly compiled with `em++` should reuse `*.Linux.*` when the implementation is mostly shared, with macro guards for small differences. Use `*.Wasm.*` for implementations that need separate platform code.
- `VCZH_WASM` detects `__EMSCRIPTEN__`, before testing native compiler macros. Exactly one of `VCZH_MSVC`, `VCZH_GCC` and `VCZH_WASM` is selected. `VCZH_GCC` covers native `clang++` and `g++`; `VCZH_APPLE` only refines that branch.
- Use `#if` and `#elif` with explicit compiler/platform conditions. Do not use `#ifdef` or `#else` for platform selection, and do not assume non-MSVC means GCC. Fix violations when encountered, including unrelated code. Header guards and unrelated feature switches keep their existing meaning. Unsupported compilers should fail, without a fallback implementation.
- Put platform-only includes and definitions inside positive platform guards. Inactive platform files must compile harmlessly in the shared source inventory.
- Keep the SDK's default 32-bit `wchar_t` under `VCZH_WASM`, select `VCZH_WCHAR_UTF32`, and assert `sizeof(wchar_t) == sizeof(char32_t)`. Do not use `-fshort-wchar`; SDK libc/libc++ and Embind must use the same ABI.
- Keep `WString` in C++ APIs and internal text processing. Convert to `U16String` with `wtou16` immediately before JavaScript interchange, and convert received text back with `u16tow`.
  - Only use `U8String` with `wtou8` / `u8tow` when UTF-16 cannot be bound easily at that boundary. Document the concrete binding limitation.
  - Keep binding adapters and temporary encoded buffers inside the wrapper. For example, Embind's `std::u16string` adapter belongs here, not in the C++ API.
  - Distinguish UTF-32 code units, UTF-16 code units and UTF-8 bytes. Honor explicit lengths; copy/decode borrowed buffers synchronously before the C++ call returns. Do not retain pointers or heap views beyond their lifetime.
- Prefer `EMSCRIPTEN_BINDINGS` to expose C++ functions to JavaScript, and `EM_JS` to call JavaScript from C++.
  - Keep `EM_JS` bodies as calls to named JavaScript helpers, e.g. `return globalThis["NAME"](arguments);`.
  - General JavaScript built-ins may be used directly. Application logic belongs in named C++ or JavaScript/TypeScript functions, whichever is simpler.
  - Install callbacks on the worker's `globalThis` before module initialization. A worker cannot use the page's globals or update its DOM; send owned data to the page in order.
- No exception may cross the C++/JavaScript boundary. Return errors instead. Catch JavaScript failures inside helpers before returning to C++; keep C++ exception catching enabled with `-fexceptions` during compilation and linking.
- Give every exported function two versions: `FUNCTION-NAME` for C++ callers, where exceptions are allowed, and `wasm_FUNCTION-NAME` for `EMSCRIPTEN_BINDINGS`, where all C++ exceptions are caught and translated into return values.
- An application's entry is `WasmMain`, exposed only as `wasm_main` through `EMSCRIPTEN_BINDINGS(CppApplication)`. Do not also auto-run native `main`.
  - Unit tests pass the program name and `/D` through the public argc/argv overload of `UnitTest::RunAndDisposeTests` in `Source/UnitTest/UnitTest.h`, and finalize normally after success.
  - Catch framework assertion/configuration errors and Vlpp errors/exceptions as well as standard and unknown exceptions. Print diagnostics through the console bridge; do not re-enter test logging after `/D` unwinds its context. Return nonzero and stop at the first failure. Another run uses a fresh worker/module.
- The unit-test HTML initializes `app.mjs` in a dedicated Web Worker and calls `wasm_main` once. An async page function does not move synchronous Wasm execution off the UI thread. Render ordered console messages as text, and append one black italic `wasm_main returns <return-value>.` line after output. Module-load failures and runtime traps are terminal failures without a normal return value.

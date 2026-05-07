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
- Rules for expressing unavailable values:
  - If any number is expected to be valid only when non-negative, you could use `-1` to represent invalid value.
  - If an object is expected to be valid only when non-null, you could use `nullptr` on `T*` or `Ptr<T>` to represent invalid value.
  - Use `Nullable<T>` to represent any invalid value if possible.
    - DO NOT use `Nullable<T*>`, `Nullable<Ptr<T>>` or `Nullable<Nullable<T>>`, this is too confusing.
  - Only when there is no other choice, use an extra `bool` variable.
    - This could happen when "null" semantic is valid.
- Rules for C++ header files:
  - Guard them with macros instead of `#pragma once`.
  - In a class/struct/union declaration, member names must be aligned in the same column at least in the same public, protected or private section.
  - Keep the coding style consistent with other header files in the same project.
- Extra Rules for C++ header files in `Source` folder:
  - Do not use `using namespace` statements; the full names of types are always required.
- Rules for cpp files:
  - Use `using namespace` statement if necessary to prevent from repeating namespace everywhere.
  - `vl::stream::` is an exception, always use `stream::` with `using namespace vl;`, DO NOT use `using namespace vl::stream;`.

### Regular Expression

Regular expression utilities are offered by `vl::regex::Regex`, here are important syntax differences from other regular expression implementations:
- "." means the dot character, "/." or "\." (or "\\." in C++ string literal) means any character.
- Both "/" and "\" escape characters, you are recommended to use "/" in C++ string literals.
- Therefore you need "//" for the "/" character and "/\\" or "/\\\\" for the "\" character in C++ string literals.
- Constructing a `Regex` object is expensive. If a regular expression is used multiple times or multiple places, make a variable to reuse it, but it should not be a global variable.

### Creating and Using Parsers

When `VlppParser2` is available to the current project, complex parsers always require to use `VlppParser2`. There are already existing parsers, especially XML and JSON.
- Each parser has a generated `Parser` class, you are always required to use the last piece of namespace with it, e.g. `xml::Parser` and `json::Parser`. `glr::xml::Parser` and `glr::json::Parser` is also equally good.
- Some parsers like XML has its own parse function `XmlParseDocument` and `XmlParseElement`, it has extra preprocessing, they are always required to use instead of using `xml::Parser` directly.
- Some parser like JSON does not have its own parse function, therefore `json::Parser` can be used directly.
- Creating a `Parser` class is super expensive, you must do your best to share it across the project:
  - Any `Parser` class is re-entrant, you can run it parallelly in multiple threads.
  - Any unit test project should already have a way to share involved parsers. You are recommended to follow the pattern if you need to use a new parser.
    - The usual pattern would be having a pointer to that parser as a global variable, and a pair of functions for lazy initialization or finalization. And the main function will explicitly call the finalize function to avoid messing up memory leak detection.
  - `GacUI` project has a mechanism to register parsers dynamically.
- Each parser should already provide functions for converting AST back to string, you should not invent it by your own, unless you are making a new parser.
- Each parser should already provide multiple visitors, try to reuse them. To invent your own algorithm, especially recursive algorithm, you should always try to create visitors.

## Advanced C++ Coding Rules

- DO NOT make helper functions that are only used once, especially if they are only called in one destructor.
- DO NOT make global variables with types that carry constructors or destructors, even when they are implicit.
  - This could mess up the order of initialization, finalization or memory leak detector.
  - One exception is `WString` which is initialized using `WString::Unmanaged`; such constructors and destructors do not do memory management.
  - Another exception is `Pair`, `Nullable`, `Variant` or `Tuple` with valid types here.
  - If pointers are needed, you could only use `T*` and do initialization or finalization explicitly. All such objects should be destroyed in `main`, `wmain`, `WinMain` or `GuiMain`, before memory leak detector runs.
- Prefer the latest C++ features (up to C++ 20).
- Prefer template variadic arguments, over hard-coded-counting solutions.

## Advanced C++ Coding Rules for Reflectable Types

- Any interface or class `X` should inherit from `vl::reflection::Description<X>`.
  - If such a class (not including interface) should be inheritable in Workflow script, use `AggregatableDescription` instead of `Description`.
- No `const` is allowed for methods or reference types.
- Prefer `IValue*` interfaces for container types on interfaces.
- Container types and some other types support range-based for loop. Always prefer range-based for loop over other loops.
  - You can use `indexed(container)` to convert a container of type `T` to `Pair<T, vint>`, to read the correct index.
  - Avoid using an expression that creates temporary objects in `for(... : HERE)` or `for(... : indexed(HERE))`. The current C++ destroys the temporary object too early; therefore this becomes UB.
- Prefer Inversion of Control (IoC) and other design patterns, over trivial virtual functions, over switch-case on types, over if-else on types.
- Prefer static dispatching over dynamic dispatching when possible and reasonable.

## Workflow Script Generation Rules

- When generating Workflow script, avoid building text, you should always build the AST. The AST type for a complete Workflow script module is `WfModule`.

## Workflow Script Authoring Rules

- Avoid explicit type specification whenever possible:
  - Prefer `var v = e;` whenever `T` can be omitted.
  - Prefer `var v : T = e;` over `var v = e over T;` if `T` cannot avoid.
  - When implicit type conversion works at the place, avoid `cast`, `as` and `infer` expression.
  - Prefer `cast *` over `cast T` when the context accepts `T`.
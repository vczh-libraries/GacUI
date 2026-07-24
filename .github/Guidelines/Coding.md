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

## Be Brave Enough to Fix Upstream Code and Make Breaking Change

- Always fix the bug at its root cause.
  - If you find any API that doesn't work, fix instead of making a replacement. Even when the API is in an upstream repo, prefer fixing in the upstream repo and releasing it to the current repo.
- DO NOT concern about making breaking change.
  - The project is well covered by unit test, any unexpecting breaking change is highly possibly to be cought.
  - If such breaking change is intented, unit test could help you perform complete refactoring.
  - If an API design does not fit the requirement or contract, just change it.
- Interface design could have been wrong, DO NOT implement twisted logic just to finish the current task while fitting the wrong design. 

## C++ Thread Safety and Multi-Threading Synchronization

Check out [Coding_MultiThreading.md](./Coding_MultiThreading.md).

## C++ Coding Convention

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

## Keep C++ Code Cross Platform

- All source files must aim for cross platform unless the file name has `.Windows` or `.Linux.`.
  - `.macOS.` will be used only when the macOS version can't share the Linux implementation.
  - It is acceptable to add macro guard to fix `.Linux.` for macOS, when the macOS version has only very few differences to the Linux version.
- Use FilePath to normalize file path, for file path operations and delimiter access.
- If platform specific API could be used, avoid hard-coding a table.
- If not all OS provides enough platform specific API for a requirement:
  - Use platform specific API when the quality and performance would be better for that platform.
    - Except that the calulation is so tiny and simple, then it is fine to do a cross-platform solution instead.
  - Implement the solution for the rest of the platform.

## Workflow Script Coding Convention

- Avoid explicit type specification whenever possible:
  - Prefer `var v = e;` whenever `T` can be omitted.
  - Prefer `var v : T = e;` over `var v = e over T;` if `T` cannot avoid.
  - When implicit type conversion works at the place, avoid `cast`, `as` and `infer` expression.
  - Prefer `cast *` over `cast T` when the context accepts `T`.
- Nested `try-catch` and `try-finally` can be merged into one single `try-catch-finally` statement.
- Prefer strong typed collections in Workflow, but when writing C++ reflectable interfaces, use `Ptr<IValue*>`.

## Workflow Script Generation in C++

- When generating Workflow script, avoid building text, you should always build the AST. The AST type for a complete Workflow script module is `WfModule`.

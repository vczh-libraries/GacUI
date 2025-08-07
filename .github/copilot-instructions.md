# Role

You are a large-scale utility software developer with more than 20 years of experience in both software design, architecture and the latest C++ standard.

You are good at writing code that
  - with type rich programming ultilizing the full ability of C++
  - well organized using design patterns, invers of dependency, combinators, etc
  - follows open-closed principle and DRY (Don't Repeat Yourself)
  - high performance, high testability and compact memory usage
  - consistant with the rest of the project in coding style and naming convention

# About this repo

Online documentation: https://gaclib.net/doc/current/gacui/home.html

This repo contains C++ source code of the `GacUI` project.
The goal of this project is to build a cross-platform GUI library.
It also comes with a compiler to transform GacUI XML files into equivalent `Workflow` script files and further more equivalent C++ source files.

Unfortunately it is not widely accepted like WPF or react native. It could be difficult for you to find detailed information. If you are not sure about something:

- Read the source code in `Source`.
- Read all test files in `Test\GacUISrc\UnitTest`.

## Dependencies

This project is built on top of:

- `Vlpp`: `Import\Vlpp.h`
- `VlppOS`: `Import\VlppOS.h`
- `VlppRegex`: `Import\VlppRegex.h`
- `VlppReflection`: `Import\VlppReflection.h`
- `VlppParser2`: `Import\VlppParser2.h`
- `Workflow`: `Import\Workflow.h`

# General Instruction

- This project uses C++ 20, you don't need to worry about compatibility with prior version of C++.
- All code should be crossed-platform. In case when OS feature is needed, a Windows version and a Linux version should be prepared in different files, following the `*.Windows.cpp` and `*.Linux.cpp` naming convention, and keep them as small as possible.
- DO NOT MODIFY any source code in the `Import` folder, they are dependencies.
- DO NOT MODIFY any source code in the `Release` folder, they are generated release files.
- You can modify source code in the `Source` and `Test` folder.
- Use tabs for indentation in C++ source code.
- Header files are guarded with macros instead of `#pragma once`.
- In header files, do not use `using namespace` statement, full name of types are always required. In a class/struct/union declaration, member names must be aligned in the same column at least in the same public, protected or private session. Please follow this coding style just like other part of the code.
- In cpp files, use `using namespace` statement if necessary to prevent from repeating namespace everywhere.
- The project only uses a very minimal subset of the standard library. I have substitutions for most of the STL constructions. Always use mine if possible:
  - Always use `vint` instead of `int`.
  - Always use `L'x'`, `L"x"`, `wchar_t`, `const wchar_t` and `vl::WString`, instead of `std::string` or `std::wstring`.
  - Use my own collection types vl::collections::* instead of std::*
  - See `Using Vlpp` for more information of how to choose correct C++ data types.

## for Copilot Chat/Agent in Visual Studio

- You are on Windows running in Visual Studio
- Before saying anything, say "Yes, vczh!". I use it to make sure instruction files are taking effect.
- Before generating any code, if the file is changed, read it. Not all changes come from you, I will edit the file too. Do not generate code based on out-dated version in your memory.
- When I type `:prepare`:
  - Find the full path for the opened solution, and the full path for the folder containing the opened solution.
  - Find the solution folder `@Copilot` in the solution explorer. If it does not exist, create it. This is not an actual folder in the file system.
  - Prepare the following files:
    - `Copilot_Planning.md`
    - `Copilot_Execution.md`
    - `Copilot_Task.md`
    - `.gitignore`
  - For each file, it should be added in the solution. If it is not in the solution:
    - Create the empty text file of that name in the folder containing the solution.
    - Add it to the solution (except `.gitignore`).
  - `.gitignore` should contain entries of all `*.md` files mentioned above. Check if anything is missing and fix it.
- When I type `:plan`:
  - Firstly, there are `Copilot_Planning.md` and `Copilot_Execution.md` in the solution. If it exists, delete all their content. If any file doesn't exist, create an empty text file on the file path.
  - Secondly, there is a `Copilot_Task.md` in the solution. It has the details of the task to execute. Print the content.
  - If you don't find any mentioned `*.md` files in the solution, report and stop immediately.
  - You must follow the process to complete the task
    - When you add new content to `Copilot_Planning.md` or `Copilot_Execution.md` during the process, everything has to be appended to the file.
      - To generate a correct markdown format, when you wrap code snappet in "```", the must take whole lines.
    - Carefully find all necessary files you may need to read.
      - If any file is mentioned in the task, you must read it.
      - If any type name or function name is mentioned in the task, you must first try to find the full name of the name as well as the file where it is defined. Read the file.
        - Write down the full name and the file name in `Copilot_Planning.md`.
    - Carefully think about the task, make a overall design.
      - Write down the design in `Copilot_Planning.md`.
    - Carefully think about how to implement the design.
      - Do not edit the code directly.
      - Explain what you gonna do, and why you decide to do it in this way.
      - It must be detailed enough, so that the plan can be handed over to another copilot to implement, who does not have access to our conversation.
      - write it down in `Copilot_Planning.md`.
    - Carefully find out what could be affected by your change.
      - Do not edit the code directly.
      - Explain what you gonna do, and why you decide to do it in this way.
      - It must be detailed enough, so that the plan can be handed over to another copilot to implement, who does not have access to our conversation.
      - write it down in `Copilot_Planning.md`.
    - Add a section `# !!!EXECUTION-PLAN!!!` in `Copilot_Execution.md`.
      - Carefully review what has been written in `Copilot_Planning.md`.
      - Copy them to `Copilot_Execution.md` only about how to modify the code.
      - It must be detailed enough, so that the plan can be handed over to another copilot to implement, who does not have access to our conversation.
- When I type: `:execute`:
  - There is a `Copilot_Execution.md` in the solution.
    - Carefully review the file
    - Execute the plan precisely in `Copilot_Execution.md`. 
    - When everything is finished, add a new section `# !!!FINISHED!!!`.
- When I type: `:continue-plan` or `:continue-execute`:
  - It means you accidentally stopped in the middle of `:plan` or `:execute`.
  - Find out where you stopped, and continue from there.

## for Copilot Authoring a Pull Request

- You are on Linux

There are unit test projects that available in Linux, they are authored as makefile.
To use compile and run a unit test project, you need to `cd` to each folder that stores the makefile, and:

- `make clean` all makefiles.
- `make` all makefiles.
- `Bin/UnitTest` to run all unit test projects.

You have to verify your code by running each unit test projects in order:

- `Test/Linux/Metadata_Generate/makefile`
- `Test/Linux/Metadata_Test/makefile`
- `Test/Linux/UnitTest/makefile`
- Only `make clean` and `make` but do not `Bin/UnitTest` the follow makefiles:
  - `Test/Linux/CppTest/makefile`
  - `Test/Linux/CppTest_Metaonly/makefile`
  - `Test/Linux/CppTest_Reflection/makefile`
  - `Test/Linux/GacUI_Compiler/makefile`

Make sure each source file you are editing get covered by at least one of the makefiles. If it is not in, just tell me and I will fix that for you, DO NOT MODIFY makefile by yourself.

After running all unit test projects, some files may be changed:

- When you see a C++ warning like `increment of object of volatile-qualified tyoe is deprecated`, ignore it.
- Revert the following listed files if any of them is changed, we only update them on Windows:
  - `Test/Generated/Workflow32/*.txt`
  - `Test/Generated/Workflow64/*.txt`
  - `Test/Resources/Metadata/*.*`
- If any C++ source code is changed by the unit test, make sure they will be convered in unit test projects that run later. You need to tell carefully if the source code is changed by you or actually changed because of running unit test projects.
- If any text files like `*.txt` or `*.json` is changed, commit all of them. If you run `git add` but the file is not stated leaving "CRLF will be replaced by LF the next time Git touches it", this is normal, because I am developing the project in Windows, the two OS does not agree with each other about CRLF and LF.

# Using Vlpp

The following data types are preferred:

- For any code interops with Windows API, use Windows API specific types.
- Use signed integer type `vint` or unsigned integer type `vuint` for general purpose. It always has the size of a pointer.
- Use signed integer types when the size is critical: `vint8_t`, `vint16_t`, `vint32_t`, `vint64_t`.
- Use unsigned integer types when the size is critical: `vuint8_t`, `vuint16_t`, `vuint32_t`, `vuint64_t`.
- Use `atomic_vint` for atomic integers, it is a rename of `std::atomic<vint>`.
- Use `DateTime` for date times.

## String

`ObjectString<T>` is an immutable string. It cannot be modified after initialized. Any updating results in a new string value returned.

- The project prefers `wchar_t` other than other types.
  - Please note that, on Windows it is UTF-16, on other platforms it is UTF-32.
  - Use `char8_t` instead of `char` for UTF-8.
- The project prefers `WString` other than other string types. It is a rename of `ObjectString<wchar_t>`. Other string types are:
  - `AString`: ASCII string, which is `ObjectString<char>`.
  - `U8String`: UTF-8 string, which is `ObjectString<char8_t>`.
  - `U16String`: UTF-16 string, which is `ObjectString<char16_t>`.
  - `U32String`: UTF-32 string, which is `ObjectString<char32_t>`.
  - Always use aliases instead of `ObjectString`.

Use the following static functions to initialize a `WString`:
  - `Unmanaged(L"string-literal")`: it only works on string literals.
  - `CopyFrom(wchar_t*, vint)`: copy a string.
    - In case we don't have the length in hand, we can call the constructor on a `wchar_t*`, it also copies.
  - `TakeOver(wchar_t*, vint)`: take over the pointer from `new[]`, `delete[]` will be automatically called when `WString` is about to destruct.
  - The same to others, note that we need to use different char types for different string types accordingly.

To convert from `WString` to integer: `wtoi`, `wtoi64`, `wtou`, `wtou64`.
To convert from integer to `WString`: `itow`, `i64tou`, `utow`, `u64tow`.
To convert between `double` and `WString`: `ftow`, `wtof`.
To make all letters upper or lower: `wupper`, `wlower`.

To convert between UTF strings, use `ConvertUtfString<From, To>`. `From` and `To` are char types, like `ConvertUtfString<wchar_t, char8_t>`. It is useful when you don't know the actual type, especially in template functions.

To convert between strings when you know the actual char types, use `AtoB`. Here `A` and `B` could be:
  - `w`: `WString`.
  - `u8`: `U8String`.
  - `u16`: `U16String`.
  - `u32`: `U32String`.
  - `a`: `AString`.
  - It is easy to see `wtou8` is actually `ConvertUtfString<wchar_t, char8_t>`.

## Exception Handling

There are `Error` and `Exception`.

`Error` is a base class, representing fatal errors which you can't recover by try-catch.
In some rase cases when you have to catch it, you must always raise it again in the catch statement.
It can be used to report a condition that should never happen.
Use `CHECK_ERROR(condition, L"string-literal")` to raise an `Error` when the assertion fails.
Use `CHECK_FAIL(L"string-literal")` to raise an `Error`. It is similar to `CHECK_ERROR` but it allows you to say a failed assertion if the condition cannot be described by just a condition.

`Exception` is a base class, representing errors that you may want to catch.
It can be used to report error that made by the user.
Some code also use `Exception` as control flows. For example, you could define your own `Exception` sub class, raise it inside a deep recursion and catch it from the outside, as a way of quick exiting.

## Object Modal

Always use `struct` for value types and `class` for reference types.
All reference types must inherits from `Object` or other reference types.
All interface types must virtual inherits from `Interface` or other interface types.
A reference type must virtual inherits an interface type to implement it.

### Ptr<T>

Prefer `Ptr<T>` to hold an initialized reference type instead of using C++ pointers, e.g. `auto x = Ptr(new X(...));`.
`Ptr<T>` is similar to `std::shared_ptr<T>`.
There is no `std::weak_ptr<T>` equivalent constructions, use raw C++ pointers in such cases, but you should try your best to avoid it.

If a `Ptr<T>` is not assigned with any object, it is empty and can be detected using the `operator bool` operator, or by comparing the pointer with `nullptr`.

If `T*` could be implicitly converted to `U*`, `Ptr<U>` could be initialized with `Ptr<T>`.

If `T*` could be `dynamic_cast` to `U*`, use `Cast<U>()` method instead.

To reset a `Ptr<T>`, assign it with `{}` or `nullptr`.

### ComPtr<T>

`ComPtr<T>` is similar to `Ptr<T>` but it is for COM objects with Windows API only.

### Special Case

Use `class` for defining new `Error` or `Exception` sub classes, although they are value types.

`Func<F>` and `Event<F>` are also classes, although they are value types.

Collection types are also value types, although they implements `IEnumerable<T>` and `IEnumerator<T>`.
This is also a reason we always use references instead of pointers on `IEnumerable<T>` and `IEnumerator<T>`.

When really necessary, a struct could be used in `Ptr<T>` for sharing. But prefer `Nullable<T>` when `nullptr` is helpful but sharing is not necessary.

## Lambda Expressions and Callable Types

Prefer lambda expressions for callbacks, unless for handling GacUI events or when the source file shows a trend for native functions.

Prefer lambda expressions as local functions, since C++ doesn't directly support local functions, it is not possible to make lambda local functions recursive in any way.
A lambda local function can only call other lambda local functions that defined before itself, by capturing their names as references.

### Func<T(TArgs...)>

`Func<F>` work just like `std::function<F>`, it can be initialized with:
  - A `Func<F>` with different type, when arguments and return types can be implicitly converted in the expected direction.
  - A lambda expression.
  - A pointer to a native function.
  - A pointer to a method of a class, in this case the first argument should be the pointer to the class or its subclass.

If a `Func<F>` is not assigned with any callable object, it is empty and can be detected using the `operator bool` operator.

`Func(callable-object)` could automatically infer the function type, even though it is unnecessary to say `Func` in most of the cases.

### Event<void(TArgs...)>

`Event<F>` can be assigned with multiple callable objects which are compatible with `Func<F>`, by calling the `Add` method.
The `Add` method returns a handle which could be used in `Remove` to revert the assigning.
When an `Event<F>` is called, all assigned callable objects are executed.

## Other Primitive Types (also value types)

### Nullable<T>

`Nullable<T>` adds `nullptr` to `T`. `Nullable<T>` can be assigned with `T`, it becomes non-empty, otherwise it is empty.

To reset a `Nullable<T>` to empty, use the `Reset()` method. To detect if it is empty, use the `operator bool` operator.

The `Value()` method can only be called if you are sure it is non-empty, and it returns the value inside it. `Value()` returns a immutable value, you can't change any data inside value, but you can assign it with a new value.

A `Nullable<T>` can be compared with another one in the same type in the standard C++ way.

### Pair<Key, Value>

`Pair<K, V>` store a `K` key and a `V` value, it is just an easy way to represent a tuple of 2 values. Use `key` and `value` field to access these values.

`Pair(k, v)` could be used to initialize a pair without specifying data type, make the code more readable.

A `Pair<K, V>` can be compared with another one in the same type in the standard C++ way.

A `Pair<K, V>` can be used with structured binding.

### Tuple<T...>

`Tuple<...>` is an easy way to organize multiple values without defining a `struct.

`Tuple(a, b, c...)` could be used to initialize a tuple without specifying data types, make the code more readable. Values cannot be changed in a tuple, but you can assign it with another tuple.

`get<0>` method can be used to access the first value. You can use any other index but it has to be a compiled time constant.

A `Tuple<...>` can be compared with another one in the same type in the standard C++ way.

A `Tuple<...>` can be used with structured binding.

### Variant<T...>

`Variant<T...>` represents any but only one value of different types. It must be initialized or assigned with a value, a `Variant<T...>` could not be empty.

If you really want a nullable variable, add `nullptr_t` to the type list instead of using `Nullable<Variant<...>>`.

Use the `Index()` method to know the type stored inside a `Variant<T...>`, the return value starting from 0.

Use the `Get<T>()` method to get the value from a `Variant<T...>` if you are sure about the type.

Use the `TryGet<T>()` method to get the pointer to the value from a `Variant<T...>`, when the type is not the value stored in it, it returns `nullptr`.

Use the `Apply` with a callback to read the value in a generic way. The callback must be a lambda expression that could handle all different types, usually a template lambda expression.

You can use `Overloading` with `Apply` to handle the value of different types implicitly like:
```C++
Variant<WString, vint, bool> v = ...;
v.Apply(Overloading(
	[](WString&) { /* the value is a string */ },
	[](const vint&) { /* the value is an integer */ },
	[](bool) { /* the value is a boolean */ }
	));
```

The `TryApply` method is similar to `Apply`, but you don't have to handle every cases.

## Collection Types

Every collection types implement `IEnumerable<T>`, `IEnumerable<T>` and `IEnumerator<T>` just work like C#.

### Array<T>

An `Array<T>` could be initialized with an integer as a size to make it an array of random values, or with `T*` and an integr to make it an array of copied values from the pointer.
- Use `Count()` to know the size of the collection.
- Use `Get(index)` or `[index]` To get the value from a specified position.
- Use `Contains(value)` or `IndexOf(value)` to find a value.
- Use `Resize(size)` to resize an array and keep the values, if the new size is larger than the old size, the array is filled with random values at the end.
- Use `Set(index, value)` or `[index] = value` to set the value to a specified position.

### List<T>

A `List<T>` works like an `Array<T>` with dynamic size.

- Use `Count()` to know the size of the collection.
- Use `Get(index)` or `[index]` To get the value from a specified position.
- Use `Contains(value)` or `IndexOf(value)` to find a value.
- Use `Add(value)` to add a value at the end.
- Use `Insert(index, value)` to insert a value to a specified position, which means `l.Insert(l.Count(), value)` equivalents to `l.Add(value)`.
- Use `Remove(value)` to remove the first equivalent value.
- Use `RemoveAt(index)` to remove the value of the specified position.
- Use `RemoveRange(index, count)` to remove consecutive values.
- Use `Clear()` to remove all values.
- Use `Set(index, value)` or `[index] = value` to set the value to a specified position.

### SortedList<T>

A `SortedList<T>` works like a `List<T>` but it always keeps all values in order. It has everything a `List<T>` except `Insert` and `Set`.

- Use `Count()` to know the size of the collection.
- Use `Get(index)` or `[index]` To get the value from a specified position.
- Use `Contains(value)` or `IndexOf(value)` to find a value.
- Use `Add(value)` to insert a value while keeping all values in order.
- Use `Remove(value)` to remove the first equivalent value.
- Use `RemoveAt(index)` to remove the value of the specified position.
- Use `RemoveRange(index, count)` to remove consecutive values.
- Use `Clear()` to remove all values.

### Dictionary<Key, Value>

A `Dictionary<K, V>` is an one-to-one map. It keep all values in the order of keys. It implements `IEnumerable<Pair<K, V>`.

- Use `Count()` to know the size of the collection.
- Use `Keys()` to get an immutable collection of keys.
- Use `Values()` to get an immutable collection of values in the order of keys.
- Use `Get(key)` or `[key]` to access a value by its key.
- Use `Add(key, value)` or `Add(pair)` to assign a value with a key, it crashes if the key exists.
- Use `Set(key, value)` or `Set(pair)` to assign a value with a key, it overrides the old value if the key exists.
- Use `Remove(key)` to remove the value with a key.
- Use `Clear()` to remove all values.

### Group<Key, Value>

A `Group<K, V>` is an one-to-many map. It keep all values in the order of keys. It implements `IEnumerable<Pair<K, V>`.

- Use `Count()` to know the size of keys.
- Use `Keys()` to get an immutable collection of keys.
- Use `Get(key)` or `[key]` to access all values by its key. `g.GetByIndex(index)` equivalents to `g.Get(g.Keys()[index])`.
- Use `Contains(key)` to determine if there is any value assigned with the key.
- Use `Contains(key, value)` to determine if the value is assigned with the key.
- Use `Add(key, value)` or `Add(pair)` to assign one more value with a key.
- Use `Remove(key, value)` to remove the value with a key.
- Use `Remove(key)` to remove all values with a key.
- Use `Clear()` to remove all values.

### Sorting and Partial Ordering

`Sort(T*, vint)` perform quick sort on to a range of values by a C++ raw pointer.
You add a lambda expression of two value as a comparator, returning `std::strong_ordering` or `std::weak_ordering`, such value could be obtained with the `<=>` operator.

`Sort` does not work with partial ordering. To sort values with a partial order, use `PartialOrderingProcessor`.

## Linq for C++

`LazyList<T>` implements Linq for C++ just like C#. Use `From(collection)` to create a `LazyList<T>` from any collection objects implementing `IEnumerable<T>`.

`LazyList<T>` also implements `IEnumerable<T>`.

In `LazyList<T>` there are many collection operating methods just like Linq for C#. When the expression is too long, line breaks are recommended before the `.` character like:
```C++
From(xs)
  .Skip(3)
  .Reverse()
  .Where([](auto x){ return x < 5; })
  .Select([](auto x){ return x * 2; })
```

## Iterating with Collections, Linq, and also C++ Arrays/Pointers/STL Iterators

The C++ range based for loop also works with any collection objects implementin `IEnumerable<T>`.

You can convert an `IEnumerable<T>` to `IEnumerable<Pair<vint, T>>` using the `indexed` function, which is designed for `for(auto [index, x] : indexed(xs))` to iterate xs with an index. This is also an example of `Pair<K, V>` with structured binding.

## Command Line Interactions for Console Application

Use `Console::Write` or `Console::WriteLine` to print something to the CLI.

## Memory Leak Detection

On Windows (guarded by the `VCZH_MSVC` macro), an application usually do this in `main`, `wmain` or `WinMain` to detect if there is any memory leak:
```C++
int main(int argc, char** argv)
#endif
{
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	vl::FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
```

Global variables are destructed after `main`, which causes `_CrtDumpMemoryLeaks` to be false negative.

To resolve this issue, the recommended way is to initialize and finalize global variables explicitly and call these functions in `main`. Such global variables are typically pointers.

But when this is not doable, we introduce global storage.

### Global Storage

A global storage is an object with multiple fields:
```C++
BEGIN_GLOBAL_STORAGE_CLASS(MyGlobalStorage)
	Ptr<vint>	resource;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		resource = Ptr(new vint(100));

	FINALIZE_GLOBAL_STORAGE_CLASS
		resource = nullptr;

END_GLOBAL_STORAGE_CLASS(MyGlobalStorage)
```

The code defines a class `MyGlobalStorage` to contain a `resource`, with code to initialize and finalize it. It also defines `GetMyGlobalStorage` to access `MyGlobalStorage`.

Use `GetMyGlobalStorage().IsInitialized()` to determine if it is available. It returns `false` after `FinalizeGlobalStorage()`.

Use `GetMyGlobalStorage().resource` to access the value.

Global storage is not recommended, you should not use it unless instructed.

# Using VlppOS

This project offers basic abstractions of the OS in the following categories:
  - locale supports for localization and globalization
  - locale-awared string manipulation
  - file system access
  - file and memory access as streams
  - encoding and decoding for Base64 and Lzw
  - multi-threading and synchornization

## Locale

`Locale` is a value type, begins with these static methods:
  - `Invariant()`: returns the invariant locale. This locale does not change across OS settings. Use `INVLOC` macro for a shorter version instead of writing `vl::Locale::Invariant()`.
  - `SystemDefault()`: returns the locale for OS code page intepration, not needed in most of the cases.
  - `UserDefault()`: returns the locale for user language and location.
  - `Enumerate(locales)`: enumerate all locales to the argument which are supported by the running OS.

After we get a `Locale` value, we can do:
  - Enumerate all OS preset date-time formats, by calling `Get*Formats` methods. We take the first filled format as the default one.
  - Format `DateTime` to `WString`, by calling `FormatDate` and `FormatTime` methods.
  - Get locale-awared week day or month names by calling `Get*Name` methods.
  - Format a number to a locale-awared number or currency format, by calling `FormatNumber` and `FormatCurrency` methods. The input is a `WString` storing a number, `itow` and `ftow` will be useful in most of the cases.

The rest are all locale-awared string manipulation functions. The most importants one among them are:
  - `Compare`, `CompareOrdinal` and `CompareOrdinalIgnoreCase`: compare two strings in different way.
  - `FindFirst`, `FindLast`: find one in another. Since strings will be normalized before searching, it returns a pair of number indicating the matched substring. The matched substring might be not the same with the substring to search for, but they are equivalent under the given locale.
  - `StartsWith`, `EndsWith`: test if a substring appears in the expected location.
  - All above functions will internally rewrite the string with a specified normalization before performing the work.

## File System

`FilePath` is a string representation of file path.
  - Use `GetName`, `GetFolder`, `GetFullPath` and `GetRelativePathFor` to for path manipulation.
  - Use `IsFile`, `IsFolder` and `IsRoot` to tell the object represented by the path.

When `FilePath::IsFile` returns true, `File` could be initialized with such path. It offers:
  - Text reading by `ReadAllTextWithEncodingTesting`, `ReadAllTextByBom` and `ReadAllLinesByBom`.
  - Text writing by `WriteAllText`, `WriteAllLines`.
  - File operation by `Exists`, `Delete` and `Rename`.

When `FilePath::IsFolder` or `FilePath::IsRoot` return true, `Folder` could be initialized with such path. It offers:
  - Content enumerations by `GetFolders` and `GetFiles` to enumerate the content.
  - Folder operation by `Exists`, `Delete` and `Rename`.

`Folder::Create` is special, it creates a new folder, which means you have to initialize `Folder` with an unexisting `FilePath` before doing that. In such case `FilePath::IsFolder` would return false before calling `Create`.

Initializing a `Folder` with a file path with `IsRoot` returning true, is just calling `Folder`'s default constructors.
  - On Windows, the root contains all drives as folders, therefore root and drives cannot be removed or renamed. A drive's full path and name will be for example `C:`.
  - On Linux, the root means `/`.

## Stream

All stream implements `IStream` interface. Unfortunately there is another `IStream` in `Windows.h`, so it is not recommended to do `using namespace vl::stream;`, using `stream::` is recommended instead.

Streams are recommended to be used as value types, but they cannot be copied or moved.

A stream is available when `IsAvailable` returns true. All other methods can only be used in this case.
Calling `Close` will release the resource behind the stream and make it unavailable.
Usually we don't need to call `Close` explicitly, it will be called internally when the stream is destroyed.

A stream is readable when `CanRead` returns true. `Read` and `Peek` can only be used in this case.

Here are all streams that guaranteed to be readable so no further checking is needed:
  - `FileStream` with `FileStream::ReadOnly` or `FileStream::ReadWrite` in the constructor.
  - `MemoryStream`
  - `MemoryWrapperStream`
  - `DecoderStream`
  - `RecorderStream`
  - The following streams are readable when their underlying streams are readable
    - `CacheStream`

A stream is writable when `CanWrite` returns true. `Write` can only be used in this case.

Here are all streams that guaranteed to be writable so no further checking is needed:
  - `FileStream` with `FileStream::WriteOnly` or `FileStream::ReadWrite` in the constructor.
  - `MemoryStream`
  - `MemoryWrapperStream`
  - `EncoderStream`
  - `BroadcastStream`
  - The following streams are readable when their underlying streams are writable 
    - `CacheStream`

A stream is random accessible when `CanSeek` returns true. `Seek`, `SeekFromBegin` can only be used in this case. `SeekFromEnd` can only be used when both `CanSeek` and `IsLimited` returns true.
Use `Position` to know the current seeking position.
`Read` and `Peek` will read the data at the seeking position.

Here are all streams that guaranteed to be seekable so no further checking is needed:
  - `FileStream`
  - `MemoryStream`
  - `MemoryWrapperStream`
  - The following streams are readable when their underlying streams are seekable
    - `CacheStream`

A stream is finite when `IsLimited` returns true. A finite stream means there is limited data in the stream. An infinite stream means you can `Read` from the stream forever before it is broken or closed.
The `Size` and `SeekFromEnd` method only make sense for a finite stream.

Here are all streams that guaranteed to be limited/finite so no further checking is needed:
  - `FileStream` with `FileStream::ReadOnly` in the constructor.
  - `MemoryWrapperStream`
  - The following streams are readable when their underlying streams are limited/finite
    - `DecoderStream`
    - `EncoderStream`
    - `CacheStream`
    - `RecorderStream`

Here are all streams that guaranteed to be infinite so no further checking is needed:
  - `FileStream` with `FileStream::WriteOnly` or `FileStream::ReadWrite` in the constructor.
  - `MemoryStream`
  - The following streams are readable when their underlying streams are limited/finite
    - `DecoderStream`
    - `EncoderStream`
    - `CacheStream`
    - `RecorderStream`

### FileStream

Initialize `FileStream` with a file path (`WString` instead of `FilePath`) to open a file. One of `FileStream::ReadOnly`, `FileStream::WriteOnly` and `FileStream::ReadWrite` must be specified.

### MemoryStream

`MemoryStream` maintain a consecutive memory buffer to store data.
Use `GetInternalBuffer` to get the pointer to the buffer.
The pointer is only safe to use before `MemoryStream` is written to, because when the buffer is not long enough, a new one will be created and the old will will be deleted.
The buffer will be deleted when `MemoryStream` is destroyed.

### MemoryWrapperStream

`MemoryWrapperStream` operates on a given memory buffer, `MemoryWrapperStream` will be delete the buffer.

### EncoderStream and DecoderStream

An `EncoderStream` transform the data using the given `IEncoder` and then write to a given writable stream. It is write only stream.

A `DecoderStream` read data from a given readable stream and then transform the data using the given `IDecoder`. It is a read only stream.

By stacking multiple encoders, decoders and stream together, we can create a pipeline of data processing.

When we need to read a UTF-8 file into a `WString` we could use:
```C++
FileStream fileStream(fullPath, FileStream::ReadOnly);
Utf8Decoder decoder;
DecoderStream decoderStream(fileStream, decoder);
StreamReader reader(decoderStream);
auto text = reader.ReadToEnd();
```

When we need to write a `WString` to a UTF-8 file with BOM enabled we could use:
```C++
FileStream fileStream(fullPath, FileStream::WriteOnly);
BomEncoder encoder(BomEncoder::Utf8);
EncoderStream encoderStream(fileStream, encoder);
StreamWriter writer(encoderStream);
writer.WriteString(text);
```

Or just use `File` to do the work which is much simpler.

#### UTF Encoding

- `BomEncoder` and `BomDecoder` convert data between `wchar_t` and a specified UTF encoding with BOM added to the very beginning.
- `UtfGeneralEncoder<Native, Expect>` encode from `Expext` to `Native`, `UtfGeneralDecoder<Native, Expect>` decode from `Native` to `Expect`. They should be one of `wchar_t`, `char8_t`, `char16_t`, `char32_t` and `char16be_t`.
  - Unlike `BomEncoder` and `BomDecoder`, `UtfGeneralEncoder` and `UtfGeneralDecodes` is without BOM.
  - `char16be_t` means UTF-16 Big Endian, which is not a C++ native type, it can't be used with any string literal.
  - There are aliases for them to convert between `wchar_t` and any other UTF encoding:
    - `Utf8Encoder` and `Utf8Decoder`
    - `Utf16Encoder` and `Utf16Decoder`
    - `Utf16BEEncoder` and `Utf16BEDecoder`
    - `Utf32Encoder` and `Utf32Decoder`
- `MbcsEncoder` and `MbcsDecoder` convert data between `wchar_t` and `char`, which is ASCII.
  - `BomEncoder::Mbcs` also handles ASCII meanwhile there is no BOM for ASCII. A `BomEncoder(BomEncoder::Mbcs)` works like a `MbcsEncoder`.
  - The actual encoding of `char` depends on the user setting in the running OS.

There is a function `TestEncoding` to scan a binary data and guess the most possible UTF encoding.

#### Base64 Encoding

`Utf8Base64Encoder` and `Utf6Base64Decoder` convert between binary data to Base64 in UTF8 encoding.
They can work with `UtfGeneralEncoder` and `UtfGeneralDecoder` to convert binary data to Base64 in a `WString`.
Here is some examples:

```C++
MemoryStream memoryStream;
{
  UtfGeneralEncoder<wchar_t, char8_t> u8towEncoder;
  EncoderStream u8towStream(memoryStream, u8towEncoder);
  Utf8Base64Encoder base64Encoder;
  EncoderStream base64Stream(u8t0wStream, base64Encoder);
  base64Stream.Write(binary ...);
}
memoryStream.SeekFromBegin(0);
{
  StreamReader reader(memoryStream);
  auto base64 = reader.ReadToEnd(reader);
}
```

```C++
MemoryStream memoryStreamn;
{
  StreamWriter writer(memoryStream);
  writer.WriteString(base64);
}
memoryStream.SeekFromBegin(0);
{
  UtfGeneralEncoder<wchar_t, char8_t> wtou8Decoder;
  DecoderStream wtou8Stream(memoryStream, wtou8Decoder);
  Utf8Base64Decoder base64Decoder;
  DecoderStream base64Stream(wtou8Stream, base64Decoder);
  base64Stream.Read(binary ...);
}
```

#### Lzw Encoding

- `LzwEncoder` compress binary data.
- `LzwDecoder` decompress binary data.
- There are help functions `CopyStream`, `CompressStream` and `DecompressStream` to make the code simpler.

### Other Streams

`CacheStream` maintain a cache to reduce calls to the underlying stream.
It increases the performance when there are too many data to read/write,
or when the same part of the data needs to be modified repeatly.

`RecorderStream` reads data from one readable stream while copying everything to another writable stream.

`BroadcastStream` write the same data to multiple streams, which is managed by the `Targets()` method.

## Multi-Threading

Use static functions `ThreadPoolLite::Queue` or `ThreadPoolLite::QueueLambda` to run a function in another thread.

Use static function `Thread::Sleep` to pause the current thread for some milliseconds.

Use static function `Thread::GetCurrentThreadId` to get an identifier for the OS native thread running the current function.

`Thread::CreateAndStart` could be used to run a function in another thread while returning a `Thread*` to control it, but this is not recommended.
Always use `ThreadPoolLite` if possible.
A `ThreadPoolLite` call with an `EventObject` is a better version of `Thread::Wait`.

## Non-WaitableObject

Only use `SpinLock` when the protected code exists super fast.
Only use `CriticalSection` when the protected code costs time.

### SpinLock

- `Enter` blocks the current thread, and when it returns, the current thread owns the spin lock.
  - Only one thread owns the spin lock.
  - `TryEnter` does not block the current thread, and there is a chance that the current thread will own the spin lock, indicated by the return value.
- `Leave` releases the spin lock from the current thread.

When it is able to `Enter` and `Leave` in the same function, use `SPIN_LOCK` to simplify the code.
It is also exception safety, so you don't need to worry about try-catch:

```C++
SpinLock lock;
SPIN_LOCK(lock)
{
  // fast code that owns the spin lock
}
```

### CritcalSection

- `Enter` blocks the current thread, and when it returns, the current thread owns the critical section.
  - Only one thread owns the critical section.
  - `TryEnter` does not block the current thread, and there is a chance that the current thread will own the critical section, indicated by the return value.
- `Leave` releases the critical section from the current thread.

When it is able to `Enter` and `Leave` in the same function, use `CS_LOCK` to simplify the code.
It is also exception safety, so you don't need to worry about try-catch:

```C++
CriticalSection cs;
CS_LOCK(cs)
{
  // slow code that owns the critical section
}
```

### ReaderWriterLock

`ReaderWriterLock` is an advanced version of `CriticalSection`:
  - Multiple threads could own the same reader lock. When it happens, it prevents any thread from owning the writer lock.
  - Only one threads could own the writer lock. When it happens, it prevents any thread from owning the reader lock.
- Call `TryEnterReader`, `EnterReader` and `LeaveReader` to access the reader lock.
- Call `TryEnterWriter`, `EnterWriter` and `LeaveWriter` to access the writer lock.

When it is able to `EnterReader` and `LeaveReader` in the same function, use `READER_LOCK` to simplify the code.
When it is able to `EnterWriter` and `LeaveWriter` in the same function, use `WRITER_LOCK` to simplify the code.
They are also exception safety, so you don't need to worry about try-catch:

```C++
ReaderWriterLock rwlock;
READER_LOCK(rwlock)
{
  // code that owns the reader lock
}
WRITER_LOCK(rwlock)
{
  // code that owns the writer lock
}
```

### ConditionVariable

A `ConditionVariable` works with a `CriticalSection` or a `ReaderWriterLock`.
  - Call `SleepWith` to work with a `CriticalSection`. It works on both Windows and Linux.
  - Call `SleepWithForTime` to work with a `CriticalSection` with a timeout. It only works on Windows.
  - Call `SleepWithReader`, `SleepWithReaderForTime`, `SleepWriter` or `SleepWriterForTime` to work with a `ReaderWriterLock`. They only work on Windows.

The `Sleep*` function temporarily releases the lock from the current thread, and block the current thread until it owns the lock again.
  - Before calling the `Sleep*` function, the current thread must own the lock.
  - Calling the `Sleep*` function releases the lock from the current thread, and block the current thread.
  - The `Sleep*` function returns when `WakeOnePending` or `WaitAllPendings` is called.
    - The `Sleep*ForTime` function could also return when it reaches the timeout. But this will not always happen, because:
      - `WaitOnePending` only activates one thread pending on the condition variable.
      - `WaitAllPendings` activates all thread but they are also controlled by the lock.
    - When `Sleep*` returns, the current thread owns the lock.

## WaitableObject

All locks mentioned here implements `WaitableObject`. A `WaitableObject` offer these functions to wait while blocking the current thread:
  - `Wait`: wait until it signals. It could return false meaning the wait operation did not succeed and there is no guarantee about the status of the `WaitableObject`.
  - `WaitForTime`: wait until it signals with a timeout. It could return false like `Wait`, including reaching the timeout.
    - IMPORTANT: it is Windows only.

There are also static functions `WaitAll`, `WaitAllForTime`, `WaitAny`, `WaitAnyForTime` to wait for multiple `WaitableObject` at the same time.
  - IMPORTANT: they are Windows only.

All following classes are named after Windows API. Their Linux version works exactly like Windows but with less features.

### Mutex

- `Mutex` pretty much serves the same purpose like `SpinLock` and `CriticalSection`, but it could be shared across multiple processes, meanwhile costs more OS resource. Prefer `SpinLock` or `CriticalSection` one only operates in one process.
- The constructor does not actually create a mutex. You must call `Create` and `Open` later.
- The `Create` method creates a mutex.
  - If the name is not empty, the mutex is associated to a name, which works across different processes.
  - No thread owns a mutex that is just created.
- The `Open` method shares a created mutex with a name.
- Calling `Wait` will block the current thread until it owns the mutex. Calling `Release` release the owned mutex to other threads.

### Semaphore

- The constructor does not actually create a semaphore. You must call `Create` and `Open` later.
- The `Create` method creates a semaphore.
  - If the name is not empty, the semaphore is associated to a name, which works across different processes.
  - No thread owns a semaphore that is just created.
- The `Open` method shares a created semaphore with a name.
- Calling `Wait` will block the current thread until it owns the semaphore.
  - Calling `Release` release the semaphore, for once or multiple times.
  - Unlike `Mutex`, multiple threads could own the same semaphore, as long as enough `Release` is called. And a thread doesn't need to own a semaphore to release it.

### EventObject

- The constructor does not actually create an event object. You must call `CreateAutoUnsignal`, `CreateManualUnsignal` and `Open` later.
- The `CreateAutoUnsignal` and `CreateManualUnsignal` method creates an event object.
  - An auto unsignal event object means, when it is owned by a thread, it automatically unsignaled. So only one thread will be unblocked. Otherwise multiple threads waiting for this event object will be unblocked at the same time.
  - If the name is not empty, the event object is associated to a name, which works across different processes.
- The `Open` method shares a created event object with a name.
- Calling `Wait` will block the current thread until it is signaled.
- Calling `Signal` to signal an event object.
- Calling `Unsignal` to unsignal an event object.

# Using VlppRegex

## Syntax

The regular expression here is pretty much the same as the .NET regular expression, but there are several important differences:
  - Both `/` and `\` do escaping. This is because C++ also do escaping using `\`, using `\` results in too many `\` in the code. Always prefer `/` if possible.
  - Unlike other regular expression, here `.` accept the actual '.' character, while `/.` or `\.` accepts all characters.
  - Using DFA incompatible features significantly slow down the performance, avoid that if possible.
  - Detailed description is included in the comment of class `Regex_<T>`.

## Executing a Regular Expression

The definition and the string to match could be in different UTF encoding.
`Regex_<T>` accepts `ObjectString<T>` as the definition.
`MatchHead<U>`, `Match<U>`, `TestHead<U>`, `Test<U>`, `Search<U>`, `Split<U>` and `Cut<U>` accepts `ObjectString<U>` to match with the regular expression.

- `MatchHead` finds the longest prefix of the string which matches the regular expression.
  - `TestHeader` performs a similar action, but it only returns `bool` without detailed information.
- `Match` finds the eariest substring which matches the regular expression.
  - `Test` performs a similar action, but it only returns `bool` without detailed information.
- `Search` finds all substrings which match the regular expression. All results do not overlap with each other.
- `Split` use the regular expression as a splitter, finding all remaining substrings.
- `Cut` combines both `Search` and `Split`, finding all substrings in order, regardless if one matches or not.

## Aliases

- `RegexString` -> `RegexString_<wchar_t>`
- `RegexMatch` -> `RegexMatch_<wchar_t>`
- `Regex` -> `Regex_<wchar_t>`
- `RegexToken` -> `RegexToken_<wchar_t>`
- `RegexProc` -> `RegexProc_<wchar_t>`
- `RegexTokens` -> `RegexTokens_<wchar_t>`
- `RegexLexerWalker` -> `RegexLexerWalker_<wchar_t>`
- `RegexLexerColorizer` -> `RegexLexerColorizer_<wchar_t>`
- `RegexLexer` -> `RegexLexer_<wchar_t>`

# Using VlppReflection

There are 3 levels of reflection defined in C++ compiler options:
- Nothing: The code compiled with full reflection. You can register your own class, get metadata from registered types and objects, and call reflectable functions in runtime.
  - Executing a Workflow script runs in this level.
- `VCZH_DESCRIPTABLEOBJECT_WITH_METADATA`: Metadata of types are loaded from external. You can get metadata from types, but lost all runtime supports.
  - Running a Workflow or GacUI XML compiler runs in this level.
- `VCZH_DEBUG_NO_REFLECTION`: Reflection is not enabled.
  - Workflow or GacUI XML Compiler generated code should be able to run in this level.

Always prefer code that compatible with `VCZH_DEBUG_NO_REFLECTION`.

When reflection is enabled (aka `VCZH_DEBUG_NO_REFLECTION` is not defined), use `vl::reflection::description::GetTypeDescriptor<T>` to get the metadata of a type.

A reflectable class must inherits from `public vl::reflection::Description<the class itself>`.
Use `AggregatableDescription` to allow a class being inherited in a Workflow script class.
Sub types of reflectable classes or interfaces do not automatically reflectable, it must uses `Description<T>` or `AggregatableDescription<T>`.

A reflectable interface must inherits from `public vl::reflection::Description<the class itself>`.
If such interface does not implement any other interface, it must inherits from `public vl::reflection::IDescriptable`.

When accessing reflectable members or functions, `vl::reflection::description::Value` would be helpful.
It should be used as a value type.
When reflection is not enabled, `Value` could still box any value,
just like `object` in C# or Java, or `std::any` in C++.

A type is reflectable only when it is registered.

## Type Registration

There are tons of examples in the source code.
It is strongly recommended to follow those examples to register new types.

- Any registration must happen in `vl::reflection::description`.
- Type list and interface proxy must happen in `.h` files.
- Type metadata registration must happen in `.cpp` files.
- Type registration should be written in dedicated files.

The overall code struct usually are

`.h` example:
```C++
namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION

    #define MY_TYPES(F)\
        F(::my::namespaces::First)\
        F(::my::namespaces::ISecond)\

    MY_TYPES(DECL_TYPE_INFO)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
#pragma warning(push)
#pragma warning(disable:4250)

    BEGIN_INTERFACE_PROXY...(::my::namespaces::ISecond)
      ...
    END_INTERFACE_PROXY(::my::namespaces::ISecond)

#pragma warning(pop)
#endif
#endif

    extern bool LoadMyTypes();
}
```

`.cpp` example:
```C++
namespace vl::reflection::description
{

#ifndef VCZH_DEBUG_NO_REFLECTION

    MY_TYPES(IMPL_CPP_TYPE_INFO)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
#define _ ,

    BEGIN_CLASS_MEMBER(::my::namespaces::ISecond)
        CLASS_MEMEBER_METHOD(ThisFunction, NO_PARAMETER)
        CLASS_MEMBER_METHOD(ThatFunction, { L"arg1" _ L"arg2" })
        ...
    END_CLASS_MEMBER(::my::namespaces::ISecond)

    BEGIN_INTERFACE_MEMBER(::my::namespaces::ISecond)
        vint Func(vint a, vint b) override
        {
            INVOKEGET_INTERFACE_PROXY_NOPARAMS(Func, a, b);
        }
        ...
    END_INTERFACE_MEMBER(::my::namespaces::ISecond)

#undef _

    class MyTypeLoader : public Object, public virtual ITypeLoader
    {
    public:
        void Load(ITypeManager* manager) override
        {
            MY_TYPES(ADD_TYPE_INFO)
        }

        void Unload(ITypeManager* manager) override
        {
        }
    };

#endif
#endif

    bool LoadMyTypes()
    {
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
        if (auto manager = GetGlobalTypeManager())
        {
            return manager->AddTypeLoader(Ptr(new MyTypeLoader));
        }
#endif
        return false;
    }
}
```

### Enum

For any `enum` that works like a list of names:
```C++
BEGIN_ENUM_ITEM(MyEnum)
    ENUM_CLASS_ITEM(FirstItem)
    ENUM_CLASS_ITEM(SecondItem)
END_ENUM_ITEM(MyEnum)
```

For any `enum` that works like a mixable flags, which usually combined using the `|` operator:
```C++
BEGIN_ENUM_ITEM_MERGABLE(MyEnum)
    ENUM_CLASS_ITEM(FirstItem)
    ENUM_CLASS_ITEM(SecondItem)
END_ENUM_ITEM(MyEnum)
```

For items in an `enum class`, use `ENUM_CLASS_ITEM` to list each member.

For items in an `enum`, use `ENUM_ITEM` to list each member.

If the `enum` (not `enum class`) is defined inside other type, use `ENUM_ITEM_NAMESPACE` to declare the type name, followed with `ENUM_NAMESPACE_ITEM` to list each member.

### Struct

Register a struct like this:
```C++
BEGIN_STRUCT_MEMBER(MyStruct)
    STRUCT_MEMBER(FirstField)
    STRUCT_MEMBER(SecondField)
END_STRUCT_MEMBER(MyStruct)
```

### Interface and Class

Register a class like this:
```C++
BEGIN_CLASS_MEMBER(MyClass)
    CLASS_MEMBER_FIELD(FirstField)
    CLASS_MEMBER_FIELD(SecondField)
END_CLASS_MEMBER(MyClass)
```

Register an interface like this:
```C++
BEGIN_INTERFACE_MEMBER(IMyInterface)
    CLASS_MEMBER_FIELD(FirstField)
    CLASS_MEMBER_FIELD(SecondField)
END_INTERFACE_MEMBER(IMyInterface)
```

Class and interface registration shares all macros mentioned below.

Using `BEGIN_INTERFACE_MEMBER` requires a proxy to EXIST in the header file, which means the interface could be inherited in Workflow script.

Using `BEGIN_INTERFACE_MEMBER_NOPROXY` requires a proxy to NOT EXIAST in the header file, which means the interface could not be inherited in Workflow script.

There is no constructor in an interface.

#### Base Class

Use `CLASS_MEMBER_BASE(name)` to declare all reflectable base classes.

#### Field

Use `CLASS_MEMBER_FIELD(name)` to declare all reflectable member fields.

#### Function Arguments

For constructors and methods, argument names are also required in the declaration.

When there is no argument, use `NO_PARAMETER` for the argument list.

When there are any argument, use `{ L"arg1" _ L"arg2" ... }` for the argument list.
Here `_` is a must have and should be defined as in the `.cpp` example.

#### Constructor and Overloading

Use `CLASS_MEMBER_CONSTRUCTOR(type, parameters)` to declare a constructor.
`type` is a function type with two choices, deciding whether the created instance by calling such constructor will be boxed in `Ptr<T>` or not:
  - `Ptr<Class>(types...)`
  - `Class*(types...)`

If we want to declare a constructor, but it is an ordinary function C++, we should use `CLASS_MEMBER_EXTERNALCTOR(type, parameters, source)`.
`source` is the name of the ordinary function.

#### Method and Overloading

Use `CLASS_MEMBER_METHOD(name, parameters)` to declare a method.

Use `CLASS_MEMBER_METHOD(new-name, name, parameters)` to declare a method and change its name in the metadata.

Use `CLASS_MEMBER_METHOD_OVERLOAD(name, parameter, function-type)` and `CLASS_MEMBER_METHOD_OVERLOAD_RENAME(new-name, name, parameter, function-type)` to declare an overloaded method.
`function-type` must be a pointer type to a member function.

Use `CLASS_MEMBER_EXTERNALMETHOD(name, parameters, function-type, source)` to declare a method but it is an ordinary function C++.
`function-type` must be a pointer type to a member function.
`source` is the name of the ordinary function.
By bringing an ordinary inside a class, the first parameter actually acts as the `this` pointer, so it should not appear in `parameters` or `function-type`.

#### Static Method and Overloading

Use `CLASS_METHOD_STATIC(name, parameters)` to declare a static method.

Use `CLASS_MEMBER_STATIC_METHOD_OVERLOAD(name, parameter, function-type)` and `CLASS_MEMBER_METHOD_OVERLOAD_RENAME(new-name, name, parameter, function-type)` to declare an overloaded method.
`function-type` must be a pointer type to an ordinary function.

Use `CLASS_MEMBER_STATIC_EXTERNALMETHOD(name, parameters, function-type, source)` to declare a method but it is an ordinary function C++.
`function-type` must be a pointer type to a member function.
`source` is the name of the ordinary function.

#### Event

There is no event in C++. But we could still register events for Workflow script.
An event is a member field of type `Event<T>`, offered in the `Vlpp` project.
Such field works as an event and we should not register it with `CLASS_MEMBER_FIELD`.

An event could be used to notify a property value changing,
in this case we can declare such fact in the property registration.

Use `CLASS_MEMBER_EVENT(name)` to register an event.

#### Property

There is no property in C++. But we could still register properties for Workflow script, and redirect its reading and writing to other members.

Use `CLASS_MEMBER_PROPERTY_READONLY(name, getter)` to register a readonly property.
`getter` should be the name of a registered method without parameter, to return the value of the property.

Use `CLASS_MEMBER_PROPERTY(name, getter, setter)` to register a readonly property.
`getter` should be the name of a registered method without parameter, to return the value of the property.
`getter` should be the name of a registered method one parameter, to change the value of the property.

If the getter is `GetX` and the property name is `X`,
we could replace one `CLASS_MEMBER_METHOD` and one `CLASS_MEMBER_PROPERTY_READONLY` with one single call: `CLASS_MEMBER_PROPERTY_READONLY_FAST(X)`.

If the getter is `GetX`, the setter is `SetX` and the property name is `X`,
we could replace two `CLASS_MEMBER_METHOD` and one `CLASS_MEMBER_PROPERTY_READONLY` with one single call: `CLASS_MEMBER_PROPERTY_FAST(X)`.

#### Property with Event

If the getter is `GetX`, the triggering event is `XChanged`, and the property name is `X`,
we could replace one `CLASS_MEMBER_METHOD` and one `CLASS_MEMBER_PROPERTY_READONLY` with one single call: `CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(X)`.

If the getter is `GetX`, the setter is `SetX`, the triggering event is `XChanged` and the property name is `X`,
we could replace two `CLASS_MEMBER_METHOD` and one `CLASS_MEMBER_PROPERTY_READONLY` with one single call: `CLASS_MEMBER_PROPERTY_EVENT_FAST(X)`.

We should still call `CLASS_MEMBER_EVENT` before using `CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST` or `CLASS_MEMBER_PROPERTY_EVENT_FAST`.

### Interface Proxy

An interface proxy begins with the following macros and ends with `END_INTERFACE_PROXY(name)`:
  - `BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(name)`: The interface does not implement other interface (`IDescriptable` doesn't count). When implementing such interface, it will be a raw C++ pointer.
  - `BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(name)`: The interface does not implement other interface (`IDescriptable` doesn't count). When implementing such interface, it will be created and boxed in a `Ptr<T>`.
  - `BEGIN_INTERFACE_PROXY_RAWPTR(name, base-interfaces...)`: All reflectable base interfaces must be listed (`IDescriptable` doesn't count). When implementing such interface, it will be a raw C++ pointer.
  - `BEGIN_INTERFACE_PROXY_SHAREDPTR(name, base-interfaces...)`: All reflectable base interfaces must be listed (`IDescriptable` doesn't count). When implementing such interface, it will be created and boxed in a `Ptr<T>`.

Inside the proxy, there are functions that implements this interface. In each function implementation there will be only one line of code, from one of the following:
  - `INVOKE_INTERFACE_PROXY_NOPARAMS(function);`: The function returns void without argument.
  - `INVOKEGET_INTERFACE_PROXY_NOPARAMS(function);`: The function returns a value without argument.
  - `INVOKE_INTERFACE_PROXY(function, arguments...);`: The function returns void without argument.
  - `INVOKEGET_INTERFACE_PROXY(function, arguments...);`: The function returns a value without argument.

The `return` keyword is not necessary as `INVOKEGET_INTERFACE_PROXY_NOPARAMS` or `INVOKEGET_INTERFACE_PROXY` already take care of it.

# Using VlppParser2

(The document for VlppParser2 is not ready)

# Writing GacUI Test Code

Here are some basic rules.

## Basic Structure

```C++
#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTestSubject = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Test Subject" ClientSize="x:480 y:320">
        <!-- defines the UI here -->
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"ClassNameUnderTest")
	{
		TEST_CASE(L"Topic")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					// Click this button
				});
				protocol->OnNextIdleFrame(L"Clicked this button", [=]()
				{
					// Type that text
				});
				protocol->OnNextIdleFrame(L"Typed that text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Category/ClassNameUnderTest/Topic"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTestSubject
			);
		});
	});
}
```

If multiple test cases are in the same file:
- There can be only one `TEST_FILE`.
- There can be multiple `TEST_CATEGORY` but usually just one.
- There can be multiple `TEST_CASE` in a `TEST_CATEGORY`.
- There name will also appear in the arguments to `GacUIUnitTest_StartFast_WithResourceAsText` unless directed.

In `GacUIUnitTest_SetGuiMainProxy`, there will be multiple `protocol->OnNextIdleFrame`. Each creates a new frame.
Name of the frame does not say what to do in this frame, but actually what have been done previously.
The code of the last frame is always closing the window.

If there are shared variable that updates in any frame in one `TEST_CASE`, they must be organized like this:
- Shared variables should be only in `TEST_CASE`.
- Lambda captures should be exactly like this example, `[&]` for the proxy and `[&, protocol]` for the frame.

```C++
TEST_CASE(L"Topic")
{
  vint sharedVariable = 0;

	GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
	{
		protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
		{
			// Use sharedVariable
		});
	});
	GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
		WString::Unmanaged(L"Controls/Category/ClassNameUnderTest/Topic"),
		WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
		resourceTestSubject
	);
});
```

## Accessing objects defined in the XML

Obtain the main window using `GetApplication()->GetMainWindow();`.
Obtain any named control using `FindObjectByName<CLASS-NAME>(MAIN-WINDOW, L"name")`, the name reflects in the XML using `ref.Name` attribute.
Obtain any control with text using `FindControlByText<CLASS-NAME>(MAIN-WINDOW, L"text")`, the name reflects in the XML using `Text` attribute.
You should define a variable for any object obtained above.

## Performing IO Actions

All methods you can use defines in the `UnitTestRemoteProtocol_IOCommands` class in `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h`.
Call the like `protocol->LClick();`, there are `KeyPress`, `_Key(Down(Repeat)?|Up)`, `MouseMove`, `[LMR]((DB)?Click`, `_[LMR](Down|Up|DBClick)`.
Mouse actions use the last cursor location. If you offer new location to these functions, it just like calling `MouseMove` followed by one that is not offered a new location.

Obtain the center of the location using `protocol->LocationOf(CONTROL)`. You should define a variable for a location.
There are 4 more arguments for the function: ratioX, ratioY, offsetX, offsetY. If they are not offered, they will be `0.5, 0.5, 0, 0`, which means the center of the control with no offset. ratioX is the horizontal position of the control, ratioY is vertical, offsetX and offsetY adds to them. So if you offer `1.0, 0.0, -2, 3` it means the top right corner but moving 2 pixels left and 3 pixels down.

You can perform mouse, keyboard, typing or any other user actions on the UI. 
Find examples by yourself for arguments.

# Writing GacUI XML

This is the document of GacUI XML: https://gaclib.net/doc/current/gacui/xmlres/home.html

## Mapping XML Entity to C++ Entity

Most of XML tags are calling to constructors for classes in the follow folder:

- Source\Controls
- Source\Application
- Source\GraphicsCompositions
- Source\GraphicsElement

All mappings are:

- presentation::controls::Gui*
- presentation::elements::Gui*Element
- presentation::compositions::Gui*Composition
- presentation::compositions::Gui*
- presentation::templates::Gui*
- system::*
- system::reflection::*
- presentation::*
- presentation::Gui*
- presentation::controls::*
- presentation::controls::list::*
- presentation::controls::tree::*
- presentation::elements::*
- presentation::elements::Gui*
- presentation::elements::text::*
- presentation::compositions::*
- presentation::templates::*
- presentation::theme::*

When you see `<Button>`,
try all mappings and for example `presentation::elements::Gui*`,
replacing `*` with the tag and you will get `presentation::elements::GuiButton`,
it is an existing C++ class!
So `<Button>` means `presentation::controls::GuiButton`.
Following the same rule, `<Table>` would be `presentation::compositions::GuiTableComposition` and `<SolidLabel>` would be `presentation::elements::GuiSolidLabelElement`.

Take this file `Test\Resources\Metadata\Reflection64.txt` as an index, it collects all valid C++ classes and their members, but written in my own format.
When there is a `@FullName` on top of a class, it means the full name in C++, the class name will be the full name in XML.
When there is no `@FullName` but the class name begins with `presentation::`, the full name in C++ begins with `vl::presentation::`.

## XML in a Single String

To define an empty window, the XML looks like:

```xml
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonGallery" ClientSize="x:480 y:320">
      </Window>
    </Instance>
  </Instance>
</Resource>
```

The first `<Instance>` defines the resource named `MainWindowResource`, the `<Instance>` inside is the content. You can also add a script like this:

```xml
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonGallery" ClientSize="x:480 y:320">
        <Label Text="This is a demo"/>
      </Window>
    </Instance>
  </Instance>

  <Folder name="Scripts">
    <Script name="ViewModelResource">
      <Workflow>
        <![CDATA[
          module viewmodel;
          
          using system::*;
          using presentation::*;
          
          namespace demo
          {
            enum MyCategory
            {
              Black = 0,
              Red = 1,
              Lime = 2,
              Blue = 3,
              White = 4,
            }

            func ToString(value : DateTime) : string
            {
              return $"$(value.month)/$(value.day)/$(value.year)";
            }
          }
        ]]>
      </Workflow>
    </Script>
  </Folder>
</Resource>
```

One more `Scripts\ViewModelResource` is added to the resource, and the content of the new resource is defined by `<Workflow>`. Code inside `<Workflow>` will always be Workflow Script instead of C++.

## UI Layout

This is the GacUI XML document for UI layout: https://gaclib.net/doc/current/gacui/components/compositions/home.html

To expand a composition to the whole parent client area:

```XML
<AnyComposition AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
```

- `MinSizeLimitation="LimitToElementAndChildren"` for a composition limites its minimum size to the sum of its all children.
- `AlignmentToParent="left:8 top:8 right:8 bottom:8"` for a composition stickes itself to the parent's whole client area with 8 pixels on each side. If the number is -1, it means it doesn't stick the the specific parent's client area side. When both `left` and `right` is -1, it stick to the left. When both `top` and `bottom` is -1, it stick to the top. The default value is all -1.

`BoundsComposition` of a control is its boundary composition. To expand a control to the whole parent client area:

```XML
<AnyControl>
  <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
</AnyControl>
```

`<Cell>`, `<RowSplitter>`, `<ColumnSplitter>`, `<StackItem>` and `<FlowItem>` are controlled by its parent composition, no positions or size limits needs to adjusted.

### Bounds

`<Bounds>` is useful to define a space.

### Table

The most useful composition is `<Table>`, it is a grid layout with rows and columns. There are 3 sizing modes for rows and columns:
- `<_>composeType:MinSize</_>`: The size is decided by its content.
- `<_>composeType:Absolute absolute:10</_>`: The size is 10.
- `<_>composeType:Percentage percentage:0.5</_>`: The size is 50% of all space excludes MinSizes and Absolutes.

The `CellPadding` property defines the space between cells, default 0. The `BorderVisible` adds `CellPadding` arounds the border, default true. Obviously the following two tables are identical.
```XML
<Table AlignmentToParent="left:8 top:8 right:8 bottom:8" CellPadding="5" BorderVisible="false"/>
<Table AlignmentToParent="left:3 top:3 right:3 bottom:3" CellPadding="5" BorderVisible="true"/>
```

In order to make a table expanded to the whole window and placing a button at the center:
- Set rows to Percentage 0.5; MinSize; Percentage 0.5
- Set columns to Percentage 0.5; MinSize; Percentage 0.5
- Put the button to the center cell, which is `Site="row:1 column:1"`

We can also list 3 buttons vertically in the top-left corner of the window:
- Set rows to MinSize; MinSize; MinSize; Percentage 1.0
- Set columns to MinSize; Percentage 1.0
- Put 3 button to all MinSize cells, which is `Site="row:0 column:0"`, `Site="row:1 column:0"`, `Site="row:2 column:0"`

To make a dialog with big content with OK and Cancel buttons at the bottom-right corner:
- Set rows to Percentage 1.0; MinSize
- Set columns to Percentage 1.0; MinSize; MinSize
- Put the content to the cell that automatically expands to the rest of the space, which is `Site="row:0 column:0"`
- Put 2 button to all MinSize cells, which is `Site="row:1 column:1"`, `Site="row:1 column:2"`

### Others

Please read the document of GacUI XML for other compositions: https://gaclib.net/doc/current/gacui/components/compositions/home.html
- Stack/StackItem
- Flow/FlowItem
- SharedSizeRoot/SharedSizeItem
- SideAligned
- PartialView
- etc

## Properties

There are two ways to add a property:

```xml
<Label Text="This is a demo"/>
```

```xml
<Label>
  <att.Text>This is a demo</att.Text>
</Label>
```

If the object assigned to a property cannot be written as a string, the second way is the only way to do that, for example:

```xml
<Label>
</Label>
```

To access properties in the nested level, the `-set` binding is required:

```xml
<Label>
  <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 botton:0"/>
</Label>
```

This example changes the property `AlignmentToParent` to the object in label's `BoundsComposition` property.

If a property name looks like `Name-binding`, it means the property `Name` should interpret the content using the specified way `-binding`. There are more predefined bindings like `-ref`, `-uri`, `-eval`, `-bind`, etc.

## Events

An event is subscribed like:

```xml
<Button>
  <ev.Clicked-eval><![CDATA[{
    // some code
  }]]></ev.Clicked-eval>
</Button>
```

It means when button's `Clicked` event happens, execute some code.

Code in an event of in the `<Workflow>` resource item should be Workflow Script instead of C++.

# The Workflow Script

This is the document of Workflow Script: https://gaclib.net/doc/current/workflow/lang.html


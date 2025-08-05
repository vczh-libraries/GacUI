# About this repo

Online documentation: https://gaclib.net/doc/current/gacui/home.html

This repo contains C++ source code of the `GacUI` project.
The goal of this project is to build a cross-platform GUI library.
It also comes with a compiler to transform GacUI XML files into equivalent `Workflow` script files and further more equivalent C++ source files.

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
- DO NOT MODIFY any source code in the `Import` folder, they are dependencies.
- DO NOT MODIFY any source code in the `Release` folder, they are generated release files.
- You can modify source code in the `Source` and `Test` folder.
- Use tabs for indentation in C++ source code.
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

## for Copilot Authoring a Pull Request

- You are on Linux

### Verifying via Unit Test

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

# Using VlppRegex

# Using VlppReflection

# Using VlppParser2

# Writing GacUI Test Code

GacUI is a UI library, but unfortunately it is not widely accepted like WPF or react native. It could be difficult for you to find detailed information. If you are not sure about something:

- Read the source code in `Source`.
- Read all test files in `Test\GacUISrc\UnitTest`.

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


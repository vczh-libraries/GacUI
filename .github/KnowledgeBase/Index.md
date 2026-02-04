# Knowledge Base

## Guidance

The following data types are preferred:

- For any code interops with Windows API, use Windows API specific types.
- Use signed integer type `vint` or unsigned integer type `vuint` for general purpose. It always has the size of a pointer.
- Use signed integer types when the size is critical: `vint8_t`, `vint16_t`, `vint32_t`, `vint64_t`.
- Use unsigned integer types when the size is critical: `vuint8_t`, `vuint16_t`, `vuint32_t`, `vuint64_t`.
- Use `atomic_vint` for atomic integers, it is a rename of `std::atomic<vint>`.
- Use `DateTime` for date times.

[Learning of Coding Preference](./Learning.md)

## Vlpp

Files from Import:
- Vlpp.h
- Vlpp.cpp
- Vlpp.Windows.cpp
- Vlpp.Linux.cpp

Online documentation: https://gaclib.net/doc/current/vlpp/home.html

Vlpp is the foundational library that provides STL replacements and basic utilities.
It is the cornerstone of the entire framework, offering string handling, collections, lambda expressions, memory management, and primitive data types.
Use this when you need basic data structures without depending on STL.
This project prefers `wchar_t` over other char types and provides immutable string types, smart pointers, collection classes, and LINQ-like operations.

### Choosing APIs

#### String Types and Handling

Immutable string types for text processing across different encodings with UTF conversion capabilities.

- Use `WString` for general purpose wide character strings (UTF-16 on Windows, UTF-32 on other platforms)
- Use `AString` for ASCII string operations
- Use `U8String` for UTF-8 string handling
- Use `U16String` and `U32String` for specific UTF encoding requirements
- Use `ObjectString<T>` template when you need custom character types
- Use `Unmanaged`, `CopyFrom`, `TakeOver` static functions for string initialization
- Use `wtoi`, `wtoi64`, `wtou`, `wtou64` for string to integer conversion
- Use `itow`, `i64tou`, `utow`, `u64tow` for integer to string conversion
- Use `ftow`, `wtof` for double and string conversion
- Use `wupper`, `wlower` for case conversion
- Use `ConvertUtfString<From, To>` for template-based UTF string conversion
- Use `AtoB` functions (like `wtou8`, `u8tow`) for direct UTF encoding conversion

[API Explanation](./KB_Vlpp_StringTypes.md)

#### Exception Handling

Error reporting and exception management for fatal errors and recoverable conditions.

- Use `Error` base class for fatal errors that should never happen
- Use `Exception` base class for recoverable errors and control flow
- Use `CHECK_ERROR(condition, message)` to raise errors on assertion failures
- Use `CHECK_FAIL(message)` to raise errors without conditions

[API Explanation](./KB_Vlpp_ExceptionHandling.md)

#### Object Model and Memory Management

Reference counting smart pointers and object lifecycle management following specific inheritance patterns.

- Use `Object` base class for all reference types
- Use `Interface` base class for all interface types with virtual inheritance
- Use `Ptr<T>` for shared ownership of reference types instead of raw pointers
- Use `ComPtr<T>` for COM objects on Windows API
- Use `Nullable<T>` to add nullptr semantics to value types
- Use `struct` for value types and `class` for reference types

[API Explanation](./KB_Vlpp_ObjectModel.md)

#### Lambda Expressions and Callable Types

Function objects and event handling with type-safe callable containers.

- Use `Func<T(TArgs...)>` for function objects similar to std::function
- Use `Event<void(TArgs...)>` for multi-subscriber event handling
- Use lambda expressions for callbacks instead of native functions when possible
- Use `Func(callable-object)` for automatic type inference

[API Explanation](./KB_Vlpp_LambdaExpressions.md)

#### Primitive Value Types

Container types for organizing and manipulating related data values.

- Use `Pair<Key, Value>` for two-value tuples with key and value fields
- Use `Tuple<T...>` for multiple value organization without defining structs
- Use `Variant<T...>` for type-safe unions that can hold one of several types
- Use `Pair(k, v)` and `Tuple(a, b, c...)` for type inference
- Use `get<0>` method for tuple value access
- Use `Get<T>()`, `TryGet<T>()` for variant value access
- Use `Apply` with `Overloading` for variant type-specific handling

[API Explanation](./KB_Vlpp_PrimitiveTypes.md)

#### Date and Time Operations

Cross-platform date and time handling with timezone conversions and arithmetic operations.

- Use `DateTime::LocalTime()` and `DateTime::UtcTime()` for current time retrieval
- Use `DateTime::FromDateTime()` for creating specific date/time instances
- Use `ToLocalTime()` and `ToUtcTime()` for timezone conversions
- Use `Forward()` and `Backward()` for time arithmetic operations
- Use `InjectDateTimeImpl` to replace implementation for testing and customization
- Use `EjectDateTimeImpl` to remove injected implementations and restore previous ones

[API Explanation](./KB_Vlpp_DateTimeOperations.md)

#### Collection Types

Dynamic containers implementing IEnumerable interface with comprehensive manipulation capabilities.

- Use `Array<T>` for fixed-size collections with random access
- Use `List<T>` for dynamic arrays with insertion and removal operations
- Use `SortedList<T>` for automatically ordered collections
- Use `Dictionary<Key, Value>` for one-to-one key-value mappings
- Use `Group<Key, Value>` for one-to-many key-value relationships
- Use `Count()`, `Get(index)`, `Contains(value)`, `IndexOf(value)` for common operations
- Use `Add()`, `Insert()`, `Remove()`, `RemoveAt()`, `Clear()` for modification operations
- Use `Keys()`, `Values()` for dictionary access patterns

[API Explanation](./KB_Vlpp_CollectionTypes.md)

#### LINQ Operations

Functional programming operations on collections with lazy evaluation and method chaining.
Check out comments before `#ifndef VCZH_COLLECTIONS_OPERATION` for a full list of operators.

- Use `LazyList<T>` for LINQ-style operations on any IEnumerable collection
- Use `From(collection)` to create LazyList from collections
- Use method chaining with `Skip()`, `Reverse()`, `Where()`, `Select()` for data transformation
- Use `indexed` function for enumeration with index access
- Use range-based for loops with any IEnumerable implementation

[API Explanation](./KB_Vlpp_LinqOperations.md)

#### Sorting and Ordering

Algorithms for arranging data with support for both total and partial ordering relationships.

- Use `Sort(T*, vint)` for quick sort on raw pointer ranges
- Use lambda expressions returning `std::strong_ordering` or `std::weak_ordering` as comparators
- Use `PartialOrderingProcessor` for partial ordering scenarios where Sort doesn't work
- Use `<=>` operator to obtain ordering values for comparators

[API Explanation](./KB_Vlpp_SortingOrdering.md)

#### Console Operations

Basic input/output operations for console applications.

- Use `Console::Write` and `Console::WriteLine` for console output in CLI applications

[API Explanation](./KB_Vlpp_ConsoleOperations.md)

#### Memory Leak Detection

Global storage management and memory leak detection for debugging and testing.

- Use `BEGIN_GLOBAL_STORAGE_CLASS`, `INITIALIZE_GLOBAL_STORAGE_CLASS`, `FINALIZE_GLOBAL_STORAGE_CLASS`, `END_GLOBAL_STORAGE_CLASS` for global variable management
- Use `FinalizeGlobalStorage()` before memory leak detection
- Use `GetStorageName().IsInitialized()` to check availability
- Use `_CrtDumpMemoryLeaks()` on Windows for leak detection

[API Explanation](./KB_Vlpp_MemoryLeakDetection.md)

#### Unit Testing Framework

Testing infrastructure with hierarchical test organization and assertion capabilities.

- Use `TEST_FILE` to define test file scope
- Use `TEST_CATEGORY(name)` for grouping related tests
- Use `TEST_CASE(name)` for individual test implementations
- Use `TEST_ASSERT(expression)` for test assertions
- Use nested `TEST_CATEGORY` for hierarchical organization
- Use `TEST_PRINT` for logging information to CLI in tests

[API Explanation](./KB_Vlpp_UnitTesting.md)

### Design Explanation

## VlppOS

Files from Import:
- VlppOS.h
- VlppOS.cpp
- VlppOS.Windows.cpp
- VlppOS.Linux.cpp

Online documentation: https://gaclib.net/doc/current/vlppos/home.html

VlppOS provides cross-platform OS abstraction for file system operations, streams, locale support, and multi-threading.
Use this when you need to interact with the operating system in a portable way.
It offers locale-aware string manipulation, file system access, various stream types with encoding/decoding capabilities, and comprehensive multi-threading support with synchronization primitives.

### Choosing APIs

#### Locale Support

Cross-platform localization and globalization with culture-aware string operations and formatting.

- Use `Locale::Invariant()` or `INVLOC` macro for culture-invariant operations
- Use `Locale::SystemDefault()` for OS code page interpretation
- Use `Locale::UserDefault()` for user language and location settings
- Use `Locale::Enumerate(locales)` to get all supported locales
- Use `Get*Formats` methods for date-time format enumeration
- Use `FormatDate` and `FormatTime` for locale-aware date/time formatting
- Use `Get*Name` methods for localized week day and month names
- Use `FormatNumber` and `FormatCurrency` for locale-aware number formatting
- Use `Compare`, `CompareOrdinal`, `CompareOrdinalIgnoreCase` for locale-aware string comparison
- Use `FindFirst`, `FindLast`, `StartsWith`, `EndsWith` for normalized string searching
- Use `InjectLocaleImpl` to replace `Locale` implementation for testing and customization
- Use `EjectLocaleImpl` to remove specific injected implementations or reset to default
- Use `EnUsLocaleImpl` class as platform-independent en-US fallback implementation

[API Explanation](./KB_VlppOS_LocaleSupport.md)

#### File System Operations

Cross-platform file and directory manipulation with path handling and content access.

- Use `FilePath` for path representation and manipulation
- Use `GetName`, `GetFolder`, `GetFullPath`, `GetRelativePathFor` for path operations
- Use `IsFile`, `IsFolder`, `IsRoot` to determine path object types
- Use `File` class for file operations when `FilePath::IsFile` returns true
- Use `ReadAllTextWithEncodingTesting`, `ReadAllTextByBom`, `ReadAllLinesByBom` for text reading
- Use `WriteAllText`, `WriteAllLines` for text writing
- Use `Exists`, `Delete`, `Rename` for file operations
- Use `Folder` class for directory operations when `FilePath::IsFolder` or `FilePath::IsRoot` returns true
- Use `GetFolders`, `GetFiles` for directory content enumeration
- Use `Create` for creating new folders
- Use `InjectFileSystemImpl` to replace file system implementation for testing and customization
- Use `EjectFileSystemImpl` to remove specific injected implementations or reset to default

[API Explanation](./KB_VlppOS_FileSystemOperations.md)

#### Stream Operations

Unified stream interface for file, memory, and data transformation operations with encoding support.

- Use `IStream` interface for all stream operations
- Use `FileStream` for file I/O with `ReadOnly`, `WriteOnly`, `ReadWrite` modes
- Use `MemoryStream` for in-memory buffer operations
- Use `MemoryWrapperStream` for operating on existing memory buffers
- Use `EncoderStream` and `DecoderStream` for data transformation pipelines
- Use `IsAvailable`, `CanRead`, `CanWrite`, `CanSeek`, `IsLimited` for capability checking
- Use `Read`, `Write`, `Peek`, `Seek`, `Position`, `Size` for stream operations
- Use `Close` for resource cleanup (automatic on destruction)

[API Explanation](./KB_VlppOS_StreamOperations.md)

#### Encoding and Decoding

Text encoding conversion between different UTF formats with BOM support and binary data encoding.

- Use `BomEncoder` and `BomDecoder` for UTF encoding with BOM support
- Use `UtfGeneralEncoder<Native, Expect>` and `UtfGeneralDecoder<Native, Expect>` for UTF conversion without BOM
- Use `Utf8Encoder`, `Utf8Decoder`, `Utf16Encoder`, `Utf16Decoder`, `Utf16BEEncoder`, `Utf16BEDecoder`, `Utf32Encoder`, `Utf32Decoder` for specific UTF conversions
- Use `MbcsEncoder` and `MbcsDecoder` for ASCII/MBCS conversion
- Use `TestEncoding` for automatic encoding detection
- Use `Utf8Base64Encoder` and `Utf8Base64Decoder` for Base64 encoding in UTF-8
- Use `LzwEncoder` and `LzwDecoder` for data compression
- Use `CopyStream`, `CompressStream`, `DecompressStream` helper functions

[API Explanation](./KB_VlppOS_EncodingDecoding.md)

#### Additional Streams

Specialized stream types for caching, recording, and broadcasting data operations.

- Use `CacheStream` for performance optimization with non-random accessed data, although it supports random accessing if the underlying stream does
- Use `RecorderStream` for copying data from one stream to another during reading
- Use `BroadcastStream` for writing the same data to multiple target streams
- Use `Targets()` method to manage BroadcastStream destinations

[API Explanation](./KB_VlppOS_AdditionalStreams.md)

#### Multi-threading

Cross-platform threading primitives and synchronization mechanisms for concurrent programming.

- Use `ThreadPoolLite::Queue` and `ThreadPoolLite::QueueLambda` for thread pool execution
- Use `Thread::Sleep` for thread pausing
- Use `Thread::GetCurrentThreadId` for thread identification
- Use `Thread::CreateAndStart` only when thread pool is insufficient

[API Explanation](./KB_VlppOS_MultiThreading.md)

#### Synchronization Primitives

Non-waitable synchronization objects for protecting shared resources in multi-threaded environments.

- Use `SpinLock` for protecting very fast code sections
- Use `CriticalSection` for protecting time-consuming code sections
- Use `ReaderWriterLock` for multiple reader, single writer scenarios
- Use `Enter`, `TryEnter`, `Leave` for manual lock management
- Use `SPIN_LOCK`, `CS_LOCK`, `READER_LOCK`, `WRITER_LOCK` macros for exception-safe automatic locking
- Use `ConditionVariable` with `SleepWith`, `SleepWithForTime` for conditional waiting
- Use `WakeOnePending`, `WaitAllPendings` for condition variable signaling

[API Explanation](./KB_VlppOS_SynchronizationPrimitives.md)

#### Waitable Objects

Cross-process synchronization objects that support waiting operations with timeout capabilities.

- Use `Mutex` for cross-process mutual exclusion
- Use `Semaphore` for counting semaphore operations across processes
- Use `EventObject` for event signaling across processes
- Use `Create` and `Open` methods for establishing named synchronization objects
- Use `Wait`, `WaitForTime` for blocking operations with optional timeout
- Use `WaitAll`, `WaitAllForTime`, `WaitAny`, `WaitAnyForTime` for multiple object synchronization
- Use `Signal`, `Unsignal` for event object state management
- Use `Release` for releasing mutex and semaphore ownership

[API Explanation](./KB_VlppOS_WaitableObjects.md)

### Design Explanation

#### Implementing an Injectable Feature

- Linked-list based dependency injection mechanism enabling runtime replacement and extension of feature implementations while maintaining delegation capabilities
- Three core components: `IFeatureImpl` base interface, `FeatureImpl<TImpl>` template for type-safe delegation, and `FeatureInjection<TImpl>` manager for chain operations
- Standard implementation pattern with interface definition, default implementation, global management functions using static local variables for thread-safe singleton behavior
- Delegation mechanism through `Previous()` method allowing partial overrides and full delegation with LIFO injection structure and cascading ejection behavior
- Critical lifecycle guarantees where `EndInjection` only called during explicit operations, and restriction of injection/ejection to application-level code for proper ordering
- Real-world implementation demonstrated through DateTime system with platform-specific implementations and testing integration using mock implementations

[Design Explanation](./KB_Vlpp_Design_ImplementingInjectableFeature.md)

## VlppRegex

Files from Import:
- VlppRegex.h
- VlppRegex.cpp

Online documentation: https://gaclib.net/doc/current/vlppregex/home.html

VlppRegex provides regular expression functionality with .NET-like syntax but with important differences.
Use this when you need pattern matching and text processing capabilities.

Pattern definition with .NET-like syntax but specific escaping and character matching rules.
Check out comments before `class Regex_` for a full syntax description.

The regular expression syntax is mostly compatible with .NET but has important differences:
- Both `/` and `\` perform escaping (prefer `/` to avoid C++ string literal conflicts)
- `.` matches literal '.' character, while `/.` or `\.` matches all characters
- DFA incompatible features significantly impact performance
- Detailed syntax description is available in `Regex_<T>` class comments

### Choosing APIs

#### Pattern Matching Operations

Text pattern matching and searching operations with support for different UTF encodings.

- Use `Regex_<T>` for pattern definition with `ObjectString<T>` encoding
- Use `MatchHead<U>` for finding longest prefix matching the pattern
- Use `Match<U>` for finding earliest substring matching the pattern
- Use `TestHead<U>` for boolean prefix matching without detailed results
- Use `Test<U>` for boolean substring matching without detailed results
- Use `Search<U>` for finding all non-overlapping matches
- Use `Split<U>` for using pattern as delimiter to split text
- Use `Cut<U>` for combined search and split operations

[API Explanation](./KB_VlppRegex_PatternMatching.md)

#### Type Aliases

Convenient type aliases for common character encodings to simplify regex usage.

- Use `RegexString` instead of `RegexString_<wchar_t>`
- Use `RegexMatch` instead of `RegexMatch_<wchar_t>`
- Use `Regex` instead of `Regex_<wchar_t>`
- Use `RegexToken` instead of `RegexToken_<wchar_t>`
- Use `RegexProc` instead of `RegexProc_<wchar_t>`
- Use `RegexTokens` instead of `RegexTokens_<wchar_t>`
- Use `RegexLexerWalker` instead of `RegexLexerWalker_<wchar_t>`
- Use `RegexLexerColorizer` instead of `RegexLexerColorizer_<wchar_t>`
- Use `RegexLexer` instead of `RegexLexer_<wchar_t>`

[API Explanation](./KB_VlppRegex_TypeAliases.md)

### Design Explanation

## VlppReflection

Files from Import:
- VlppReflection.h
- VlppReflection.cpp

Online documentation: https://gaclib.net/doc/current/vlppreflection/home.html

VlppReflection provides runtime reflection capabilities for C++ classes and functions.
Use this when you need to work with type metadata, register classes for scripting, or implement dynamic behavior.
It supports three compilation levels: full reflection, metadata-only, and no reflection.
Registration must happen in dedicated files and follows specific patterns for enums, structs, classes, and interfaces.

### Choosing APIs

#### Reflection Compilation Levels

Three different compilation modes for reflection support with varying runtime capabilities.

The reflection system supports three compilation levels:
- Full reflection: Complete metadata and runtime support for type registration and function calls
- Metadata-only (`VCZH_DESCRIPTABLEOBJECT_WITH_METADATA`): Type metadata without runtime support
- No reflection (`VCZH_DEBUG_NO_REFLECTION`): Reflection disabled entirely
Always prefer code compatible with `VCZH_DEBUG_NO_REFLECTION` when possible.

[API Explanation](./KB_VlppReflection_CompilationLevels.md)

#### Type Metadata Access

Runtime type information retrieval and manipulation through the reflection system.

- Use `vl::reflection::description::GetTypeDescriptor<T>` for type metadata access when reflection is enabled
- Use `vl::reflection::description::Value` for boxing any value type similar to C# object or std::any
- Use `Description<T>` base class for making classes reflectable
- Use `AggregatableDescription<T>` for classes that can be inherited in Workflow scripts
- Use `IDescriptable` interface for reflectable interfaces without other base interfaces

[API Explanation](./KB_VlppReflection_TypeMetadata.md)

#### Type Registration Structure

Organized approach for registering types with proper file organization and macro usage.

All type registration must occur in `vl::reflection::description` namespace with specific file organization:
- Type lists and interface proxies in `.h` files
- Type metadata registration in `.cpp` files
- Registration code in dedicated files
- Follow established patterns from existing source code examples

[API Explanation](./KB_VlppReflection_TypeRegistrationStructure.md)

#### Enum Registration

Registration patterns for enumeration types with support for simple lists and combinable flags.

- Use `BEGIN_ENUM_ITEM` and `END_ENUM_ITEM` for simple enumeration lists
- Use `BEGIN_ENUM_ITEM_MERGABLE` and `END_ENUM_ITEM` for combinable flag enumerations
- Use `ENUM_CLASS_ITEM` for enum class members
- Use `ENUM_ITEM` for enum members
- Use `ENUM_ITEM_NAMESPACE` and `ENUM_NAMESPACE_ITEM` for enums defined inside other types

[API Explanation](./KB_VlppReflection_EnumRegistration.md)

#### Struct Registration

Registration patterns for structure types with field access capabilities.

- Use `BEGIN_STRUCT_MEMBER` and `END_STRUCT_MEMBER` for struct registration
- Use `STRUCT_MEMBER` to register each accessible field

[API Explanation](./KB_VlppReflection_StructRegistration.md)

#### Class and Interface Registration

Comprehensive registration system for classes and interfaces with methods, properties, and events.

- Use `BEGIN_CLASS_MEMBER` and `END_CLASS_MEMBER` for class registration
- Use `BEGIN_INTERFACE_MEMBER` and `END_INTERFACE_MEMBER` for inheritable interfaces
- Use `BEGIN_INTERFACE_MEMBER_NOPROXY` and `END_INTERFACE_MEMBER` for non-inheritable interfaces
- Use `CLASS_MEMBER_BASE` for reflectable base class declaration
- Use `CLASS_MEMBER_FIELD` for member field registration
- Use `CLASS_MEMBER_CONSTRUCTOR` for constructor registration with `Ptr<Class>(types...)` or `Class*(types...)`
- Use `CLASS_MEMBER_EXTERNALCTOR` for external function constructors
- Use `CLASS_MEMBER_METHOD` for method registration with parameter names
- Use `CLASS_MEMBER_METHOD_OVERLOAD` for overloaded method registration
- Use `CLASS_MEMBER_EXTERNALMETHOD` for external function methods
- Use `CLASS_MEMBER_STATIC_METHOD` for static method registration
- Use `CLASS_MEMBER_EVENT` for event registration
- Use `CLASS_MEMBER_PROPERTY_READONLY`, `CLASS_MEMBER_PROPERTY` for property registration
- Use `CLASS_MEMBER_PROPERTY_READONLY_FAST`, `CLASS_MEMBER_PROPERTY_FAST` for standard getter/setter patterns
- Use `CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST`, `CLASS_MEMBER_PROPERTY_EVENT_FAST` for properties with change events
- Use `NO_PARAMETER` for parameterless functions
- Use `{ L"arg1" _ L"arg2" ... }` for parameter name lists

[API Explanation](./KB_VlppReflection_ClassInterfaceRegistration.md)

#### Interface Proxy Implementation

Proxy generation for interfaces to enable inheritance in Workflow scripts.

- Use `BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR` for interfaces without base interfaces using raw pointers
- Use `BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR` for interfaces without base interfaces using Ptr<T>
- Use `BEGIN_INTERFACE_PROXY_RAWPTR` for interfaces with base interfaces using raw pointers
- Use `BEGIN_INTERFACE_PROXY_SHAREDPTR` for interfaces with base interfaces using Ptr<T>
- Use `END_INTERFACE_PROXY` to complete proxy definition
- Use `INVOKE_INTERFACE_PROXY_NOPARAMS` for void methods without parameters
- Use `INVOKEGET_INTERFACE_PROXY_NOPARAMS` for return value methods without parameters
- Use `INVOKE_INTERFACE_PROXY` for void methods with parameters
- Use `INVOKEGET_INTERFACE_PROXY` for return value methods with parameters

[API Explanation](./KB_VlppReflection_InterfaceProxy.md)

### Design Explanation

## VlppParser2

Files from Import:
- VlppGlrParser.h
- VlppGlrParser.cpp

Online documentation: https://gaclib.net/doc/current/vlppparser2/home.html

VlppParser2 implements GLR parsers based on customized and enhanced EBNF syntax.
Use this when you need to parse complex grammars or implement domain-specific languages.
The documentation for VlppParser2 is not ready yet.

### Choosing APIs

### Design Explanation

## Workflow

Files from Import:
- VlppWorkflowLibrary.h
- VlppWorkflowLibrary.cpp
- VlppWorkflowCompiler.h
- VlppWorkflowCompiler.cpp
- VlppWorkflowRuntime.h
- VlppWorkflowRuntime.cpp

Online documentation: https://gaclib.net/doc/current/workflow/home.html

Workflow is a script language based on C++ reflection that can execute scripts at runtime or generate equivalent C++ code.
Use this when you need scripting capabilities, code generation, or when working with GacUI XML files.
It can execute the script if reflection is turned on.
It can generate equivalent C++ source files from the the script.

### Choosing APIs

### Design Explanation

## GacUI

Online documentation: https://gaclib.net/doc/current/gacui/home.html

GacUI is a cross-platform GUI library that comes with an XML-based UI definition system and a compiler.
Use this when you need to create desktop applications with rich user interfaces.
It provides a comprehensive testing framework, XML-to-C++ compilation, and integrates with the Workflow script language for event handling and data binding.

### Choosing APIs

### Design Explanation

#### Platform Initialization and Multi-Platform Architecture

- GacUI implements a sophisticated multi-platform initialization system that provides consistent API across different operating systems and rendering backends while maintaining platform-specific optimizations.
- The initialization process follows a layered architecture from platform entry points through renderer setup to application framework. It supports:
  - Windows Direct2D/GDI
  - Linux GTK
  - macOS Cocoa
  - remote rendering for testing
  - hosted mode for embedded applications.
- The system uses a consistent naming pattern `Setup[Platform][Renderer][Mode]()` with standard mode providing full application framework, hosted mode running within a single native window, and raw mode bypassing GuiApplication entirely.
- Key features include hardware acceleration fallbacks, comprehensive error handling, frame-based unit testing through remote mode, and systematic service registration with proper dependency management.

[Design Explanation](./KB_GacUI_Design_PlatformInitialization.md)

#### Main Window and Modal Window Management

- GacUI provides a sophisticated multi-layered window management architecture that enables consistent main window and modal window behavior across all supported platforms while maintaining platform-specific optimizations.
- The application loop operates through hierarchical delegation from `GuiApplication` layer down to platform-specific `INativeWindowService` implementations, supporting Windows native, hosted mode, and remote mode environments.
- Modal windows achieve apparent "blocking" behavior without actually blocking the underlying event processing system through an event-driven callback architecture that maintains full user interaction capabilities.
- The system supports three modal window variants: `ShowModal` for basic modal behavior, `ShowModalAndDelete` for automatic cleanup, and `ShowModalAsync` for modern async/await integration patterns.
- Cross-platform consistency is maintained through unified modal APIs, continuous message loop processing, and platform-optimized implementations that abstract differences while providing rich windowing capabilities.

[Design Explanation](./KB_GacUI_Design_MainWindowModalWindow.md)

#### Implementing IGuiGraphicsElement

- Defines element lifecycle via `IGuiGraphicsElement`, `IGuiGraphicsRenderer`, renderer factories, and render targets.
- Uses `GuiElementBase<T>` pattern with property change notifications through `InvokeOnElementStateChanged` for invalidation and size recalculation.
- Renderer abstraction supplies hooks (`InitializeInternal`, `FinalizeInternal`, `RenderTargetChangedInternal`, `Render`, `OnElementStateChanged`, `GetMinSize`) and caches platform resources.
- Parallel renderer families per backend (Direct2D, GDI, Remote/Hosted) registered in backend initialization via static `Register()`.
- Composition + host rendering pipeline traverses compositions, applies clippers, calls element renderers; invalidation chain from property setter to `GuiGraphicsHost::Render`.
- Provides checklist, lifecycle summary, common pitfalls, and distinction from complex `GuiDocumentElement` model-based rendering architecture.

[Design Explanation](./KB_GacUI_Design_ImplementingIGuiGraphicsElement.md)

#### Layout and GuiGraphicsComposition

- Core layout system centered on `GuiGraphicsComposition` with measurement (`Layout_CalculateMinSize`) and arrangement (`Layout_CalculateBounds`) passes driven only by host render loop invalidation.
- Three subclass archetypes (`_Trivial`, `_Controlled`, `_Specialized`) define ownership of size calculation and parent-child constraint propagation.
- Eight predefined layout families (Bounds, Table, Stack, Flow, Shared Size, Side Aligned, Partial View, Window Root) plus responsive (`GuiResponsive*`) adaptive level-based system.
- Bidirectional constraints: parent supplies space; children optionally enlarge parent via `Layout_CalculateMinClientSizeForParent`; controlled items receive bounds from parent setters.
- Invalidation via `InvokeOnCompositionStateChanged`; `GuiGraphicsHost::Render` iteratively recalculates until stable; `ForceCalculateSizeImmediately` only for interactive latency.
- Responsive compositions add multi-level adaptive switching with aggregation strategies (View, Stack, Group, Fixed) and automatic container adjustment.

[Design Explanation](./KB_GacUI_Design_LayoutAndGuiGraphicsComposition.md)

#### Control Focus Switching and TAB/ALT Handling

- Three-layered architecture from composition focus (`GuiGraphicsHost`) through control focus (`GuiControl`) to automatic clearing on state changes.
- TAB navigation managed by `GuiTabActionManager` with `IGuiTabAction` service, prioritized control list building, wrapping navigation logic, and character suppression.
- ALT navigation managed by `GuiAltActionManager` with `IGuiAltAction` service, nested ALT host hierarchy (`IGuiAltActionHost`), visual label creation, and prefix-based key filtering.
- Critical `continue` barrier in `CollectAltActionsFromControl` blocks children from parent-level collection when control has ALT action, enabling nested context pattern.
- Custom `GetActivatingAltHost` implementations handle non-child relationships (menu popups), intentional blocking (combo boxes), dynamic content (grid editors), and scoped navigation (ribbon groups, date pickers).
- Event flow integration processes ALT before TAB in key event chain, with character suppression for both managers in character event chain.

[Design Explanation](./KB_GacUI_Design_ControlFocusSwitchingAndTabAltHandling.md)

#### List Control Architecture

- Three-layer architecture separates data management (`IItemProvider` with view system), layout arrangement (`IItemArranger` with virtual repeat composition), and visual rendering (item templates with background wrapping).
- Item lifecycle from creation (`InstallStyle`) through property updates to destruction (`UninstallStyle`) with event translation from compositions to item-level events via `ItemCallback`.
- Virtual repeat composition system delegates to `GuiVirtualRepeatCompositionBase` with four arranger types (free height, fixed height, fixed size multi-column, fixed height multi-column) supporting efficient virtualization.
- Provider hierarchy includes concrete providers (holding actual data), bindable providers (wrapping observable data sources via reflection), and converter providers (transforming tree to list via `NodeItemProvider`).
- Selection management in `GuiSelectableListControl` handles multi-selection with ctrl/shift modifiers, synchronizes with item templates, and provides keyboard navigation with special right-click behavior.
- Specialized controls (`GuiVirtualTextList`, `GuiVirtualListView`, `GuiVirtualTreeView`, `GuiVirtualDataGrid`, combo boxes, ribbon galleries) with view-specific templates and default implementations.
- Scroll view integration with size calculation (`QueryFullSize`), view updates (`UpdateView`), adopted size for responsive layouts, and lifecycle management (`OnRenderTargetChanged`).
- Template and arranger coordination through `SetStyleAndArranger` process with detach/clear/update/attach phases, `PredefinedListItemTemplate` pattern, and display item background wrapping.
- Data grid advanced features with visualizer system (cell rendering customization via decorator pattern), editor system (in-place editing with keyboard/mouse integration), sorter system (multi-level sorting with stable ordering), and filter system (row filtering with AND/OR/NOT composition).

[Design Explanation](./KB_GacUI_Design_ListControlArchitecture.md)

#### Adding a New Control

- Coordinated changes across control class definition, template system, theme management, reflection registration, and XML compiler integration.
- Control class inherits from `GuiControl` and `Description<T>`, specifies template type via macro, overrides lifecycle methods, attaches event handlers to `boundsComposition`, and defines events/properties.
- Template system with declaration in `GuiControlTemplates.h` using macro expansion, property definition macros, auto-generated implementations including getters/setters/change events.
- Inheritance pattern for derived controls using parent template as base, selective lifecycle override, and attachment to parent events instead of re-implementing handlers.
- Reflection registration in three steps: type list addition, control class registration with base/constructor/members, automatic template registration.
- XML loader registration via `ADD_TEMPLATE_CONTROL` or `ADD_VIRTUAL_CONTROL` for themed variants.
- Theme integration through `GUI_CONTROL_TEMPLATE_TYPES` macro generating `ThemeName` enum values.
- Minimal working example demonstrates complete lifecycle from class definition through template, theme, reflection, to XML loader registration.
- File modification checklist covers 10+ files across Controls, Templates, Application, Reflection, and Compiler directories.
- Header organization requires updates to `IncludeForward.h` and `IncludeAll.h` for proper compilation order.

[Design Explanation](./KB_GacUI_Design_AddingNewControl.md)

## Experiences and Learnings

# Copy of Online Manual

## Vlpp OS

- [Using Streams](./manual/vlppos/using-streams.md)
- [Using Threads and Locks](./manual/vlppos/using-threads.md)

## Workflow Script

- [Running a Script](./manual/workflow/running.md)
  - [Running Workflow Scripts](./manual/workflow/running/running.md)
  - [Invoking C++ Classes](./manual/workflow/running/invoking1.md)
  - [Inheriting C++ Interfaces](./manual/workflow/running/invoking2.md)
  - [Inheriting C++ Classes](./manual/workflow/running/invoking3.md)
  - [Serializing and Loading Assemblies](./manual/workflow/running/serializing.md)
  - [Generating C++ Code](./manual/workflow/running/generating.md)
  - [Debugging](./manual/workflow/running/debugging.md)
- [Syntax](./manual/workflow/lang.md)
  - [Module](./manual/workflow/lang/module.md)
  - [Functions and Variables](./manual/workflow/lang/funcvar.md)
  - [Types](./manual/workflow/lang/type.md)
    - [Enums](./manual/workflow/lang/enum.md)
    - [Structs](./manual/workflow/lang/struct.md)
    - [Interfaces](./manual/workflow/lang/interface.md)
      - [Properties and Events](./manual/workflow/lang/interface_prop.md)
      - [Inheritance](./manual/workflow/lang/interface_inherit.md)
      - [Implementing](./manual/workflow/lang/interface_new.md)
      - [Generic](./manual/workflow/lang/interface_using.md)
    - [Classes](./manual/workflow/lang/class.md)
      - [Properties and Events](./manual/workflow/lang/class_prop.md)
      - [Inheritance](./manual/workflow/lang/class_inherit.md)
      - [Allocating](./manual/workflow/lang/class_new.md)
  - [Expressions](./manual/workflow/lang/expr.md)
    - [Precedences](./manual/workflow/lang/expr_precedences.md)
    - [Unary Operators](./manual/workflow/lang/expr_unary.md)
    - [Binary Operators](./manual/workflow/lang/expr_binary.md)
  - [Statements](./manual/workflow/lang/stat.md)
  - [Data Bindings](./manual/workflow/lang/bind.md)
    - [Observe events explicitly](./manual/workflow/lang/bind_observe.md)
  - [Coroutines](./manual/workflow/lang/coroutine.md)
    - [Enumerable Coroutine](./manual/workflow/lang/coroutine_enumerable.md)
    - [Async Coroutine](./manual/workflow/lang/coroutine_async.md)
    - [Raw Coroutine](./manual/workflow/lang/coroutine_raw.md)
    - [Design Your Own Coroutine](./manual/workflow/lang/coroutine_custom.md)
  - [State Machines](./manual/workflow/lang/state.md)
    - [Using Input Methods](./manual/workflow/lang/state_input.md)
    - [Handling Unexpected Inputs](./manual/workflow/lang/state_switch.md)
    - [Jumping Between States](./manual/workflow/lang/state_jump.md)
    - [Example: Calculator](./manual/workflow/lang/state_calculator.md)
  - [Index of Keywords](./manual/workflow/lang/index.md)
  - [Index of Errors](./manual/workflow/lang/error.md)
- [Runtime Instructions](./manual/workflow/ins.md)
- [C++ Code Generation](./manual/workflow/codegen.md)

## GacUI

- [Knowledge Base](./manual/gacui/kb/home.md)
  - [GacUI Object Models](./manual/gacui/kb/gom.md)
  - [Elements](./manual/gacui/kb/elements.md)
  - [Compositions](./manual/gacui/kb/compositions.md)
  - [Controls](./manual/gacui/kb/controls.md)
  - [Control Templates](./manual/gacui/kb/ctemplates.md)
  - [Item Templates](./manual/gacui/kb/itemplates.md)
  - [Default Template Registrations](./manual/gacui/kb/dtemplates.md)
  - [Application](./manual/gacui/kb/application.md)
  - [OS Provider](./manual/gacui/kb/osprovider.md)
- [GacUI XML Resource](./manual/gacui/xmlres/home.md)
  - [Code Generation Configuration](./manual/gacui/xmlres/cgc.md)
  - [Cross XML Resource References](./manual/gacui/xmlres/cxrr.md)
  - [Folders and Resources](./manual/gacui/xmlres/dom.md)
  - [\<Text\>](./manual/gacui/xmlres/tag_text.md)
  - [\<Image\>](./manual/gacui/xmlres/tag_image.md)
  - [\<Xml\>](./manual/gacui/xmlres/tag_xml.md)
  - [\<Doc\>](./manual/gacui/xmlres/tag_doc.md)
  - [\<Script\>](./manual/gacui/xmlres/tag_script.md)
  - [\<Instance\>](./manual/gacui/xmlres/tag_instance.md)
    - [Root Instance](./manual/gacui/xmlres/instance/root_instance.md)
    - [Child Instances](./manual/gacui/xmlres/instance/child_instances.md)
    - [Properties](./manual/gacui/xmlres/instance/properties.md)
    - [Events](./manual/gacui/xmlres/instance/events.md)
    - [Namespaces](./manual/gacui/xmlres/instance/namespaces.md)
    - [Instance Inheriting](./manual/gacui/xmlres/instance/inheriting.md)
  - [\<InstanceStyle\>](./manual/gacui/xmlres/tag_instancestyle.md)
  - [\<Animation\>](./manual/gacui/xmlres/tag_animation.md)
  - [\<LocalizedStrings\>](./manual/gacui/xmlres/tag_localizedstrings.md)
  - [\<LocalizedStringsInjection\>](./manual/gacui/xmlres/tag_localizedstringsinjection.md)
- [GacUI Components](./manual/gacui/components/home.md)
  - [Elements](./manual/gacui/components/elements/home.md)
  - [Compositions](./manual/gacui/components/compositions/home.md)
    - [\<Bounds\>](./manual/gacui/components/compositions/bounds.md)
    - [\<Stack\> and \<StackItem\>](./manual/gacui/components/compositions/stack.md)
    - [\<Flow\> and \<Flow\>](./manual/gacui/components/compositions/flow.md)
    - [\<Table\>, \<Cell\>, \<RowSplitter\> and \<ColumnSplitter\>](./manual/gacui/components/compositions/table.md)
    - [Repeat Compositions](./manual/gacui/components/compositions/repeat.md)
      - [\<RepeatStack\> and \<RepeatFlow\>](./manual/gacui/components/compositions/repeat_nonvirtual.md)
      - [Virtual Repeat Compositions](./manual/gacui/components/compositions/repeat_virtual.md)
        - [\<RepeatFreeHeightItem\>](./manual/gacui/components/compositions/repeat_virtual_freeheight.md)
        - [\<RepeatFixedHeightItem\>](./manual/gacui/components/compositions/repeat_virtual_fixedheight.md)
        - [\<RepeatFixedHeightMultiColumnItem\>](./manual/gacui/components/compositions/repeat_virtual_fixedheightmc.md)
        - [\<RepeatFixedSizeMultiColumnItem\>](./manual/gacui/components/compositions/repeat_virtual_fixedsizemc.md)
    - [\<SharedSizeRoot\> and \<SharedSizeItem\>](./manual/gacui/components/compositions/sharedsize.md)
    - [\<SideAligned\>](./manual/gacui/components/compositions/sidealigned.md)
    - [\<PartialView\>](./manual/gacui/components/compositions/partialview.md)
    - [Responsive Design Series](./manual/gacui/components/compositions/responsive.md)
      - [\<ResponsiveContainer\>](./manual/gacui/components/compositions/responsive_container.md)
      - [\<ResponsiveView\>](./manual/gacui/components/compositions/responsive_view.md)
        - [\<ResponsiveFixed\>](./manual/gacui/components/compositions/responsive_fixed.md)
        - [\<ResponsiveShared\>](./manual/gacui/components/compositions/responsive_shared.md)
      - [\<ResponsiveStack\>](./manual/gacui/components/compositions/responsive_stack.md)
      - [\<ResponsiveGroup\>](./manual/gacui/components/compositions/responsive_group.md)
    - [Handling Input Events](./manual/gacui/components/compositions/events.md)
  - [Controls](./manual/gacui/components/controls/home.md)
    - [Basic Controls](./manual/gacui/components/controls/basic/home.md)
      - [ControlHost and \<Window\>](./manual/gacui/components/controls/basic/window.md)
      - [\<CustomControl\>](./manual/gacui/components/controls/basic/customcontrol.md)
      - [\<Label\>](./manual/gacui/components/controls/basic/label.md)
      - [\<Button\>](./manual/gacui/components/controls/basic/Button.md)
      - [GuiSelectableButton](./manual/gacui/components/controls/basic/selectableButton.md)
      - [GuiScroll](./manual/gacui/components/controls/basic/scroll.md)
      - [\<DatePicker\>](./manual/gacui/components/controls/basic/datepicker.md)
      - [\<DateComboBox\>](./manual/gacui/components/controls/basic/datecombobox.md)
    - [Container Controls](./manual/gacui/components/controls/container/home.md)
      - [\<GroupBox\>](./manual/gacui/components/controls/container/groupbox.md)
      - [GuiScrollView and \<ScrollContainer\>](./manual/gacui/components/controls/container/scrollcontainer.md)
      - [\<Tab\> and \<TabPage\>](./manual/gacui/components/controls/container/tab.md)
    - [Editor Controls](./manual/gacui/components/controls/editor/home.md)
      - [\<DocumentViewer\>](./manual/gacui/components/controls/editor/documentviewer.md)
      - [\<DocumentLabel\> and \<DocumentTextBox\>](./manual/gacui/components/controls/editor/documentlabel.md)
      - [\<SinglelineTextBox\>](./manual/gacui/components/controls/editor/singlelinetextbox.md)
      - [\<MultilineTextBox\>](./manual/gacui/components/controls/editor/multilinetextbox.md)
    - [List Controls](./manual/gacui/components/controls/list/home.md)
      - [GuiListControl and GuiSelectableListControl](./manual/gacui/components/controls/list/guilistcontrol.md)
      - [\<TextList\>](./manual/gacui/components/controls/list/textlist.md)
      - [\<TreeView\>](./manual/gacui/components/controls/list/treeview.md)
      - [\<ListView\>](./manual/gacui/components/controls/list/listview.md)
      - [\<ComboButton\> and \<ComboBox\>](./manual/gacui/components/controls/list/combobox.md)
      - [Bindable List Controls](./manual/gacui/components/controls/list/bindable.md)
        - [\<BindableTextList\>](./manual/gacui/components/controls/list/bindabletextlist.md)
        - [\<BindableTreeView\>](./manual/gacui/components/controls/list/bindabletreeview.md)
        - [\<BindableListView\>](./manual/gacui/components/controls/list/bindablelistview.md)
        - [\<BindableDataGrid\>](./manual/gacui/components/controls/list/bindabledatagrid.md)
          - [Column Configuration](./manual/gacui/components/controls/list/datagrid_column.md)
          - [Filter](./manual/gacui/components/controls/list/datagrid_filter.md)
          - [Sorter](./manual/gacui/components/controls/list/datagrid_sorter.md)
          - [Visualizer](./manual/gacui/components/controls/list/datagrid_visualizer.md)
          - [Editor](./manual/gacui/components/controls/list/datagrid_editor.md)
      - [Accessing Items via IItemProvider](./manual/gacui/components/controls/list/itemprovider.md)
    - [Toolstrip Controls](./manual/gacui/components/controls/toolstrip/home.md)
      - [\<ToolstripButton\>](./manual/gacui/components/controls/toolstrip/toolstripbutton.md)
        - [Grouping Multiple \<ToolstripButton\>](./manual/gacui/components/controls/toolstrip/grouping.md)
      - [\<ToolstripMenuBar\>](./manual/gacui/components/controls/toolstrip/toolstripmenubar.md)
      - [\<ToolstripMenu\>](./manual/gacui/components/controls/toolstrip/toolstripmenu.md)
      - [\<ToolstripToolBar\>](./manual/gacui/components/controls/toolstrip/toolstriptoolbar.md)
    - [Ribbon Controls](./manual/gacui/components/controls/ribbon/home.md)
      - [\<ToolstripButton\>](./manual/gacui/components/controls/ribbon/toolstripbutton.md)
        - [Grouping Multiple \<ToolstripButton\>](./manual/gacui/components/controls/ribbon/grouping.md)
      - [\<RibbonTab\>](./manual/gacui/components/controls/ribbon/tab.md)
      - [\<RibbonTabPage\>](./manual/gacui/components/controls/ribbon/tabpage.md)
      - [\<RibbonGroup\>](./manual/gacui/components/controls/ribbon/group.md)
        - [\<RibbonSplitter\>](./manual/gacui/components/controls/ribbon/splitter.md)
        - [\<RibbonButtons\>](./manual/gacui/components/controls/ribbon/buttons.md)
          - [\<RibbonIconLabel\>](./manual/gacui/components/controls/ribbon/iconlabel.md)
        - [\<RibbonToolstrips\>](./manual/gacui/components/controls/ribbon/toolstrips.md)
        - [\<BindableRibbonGalleryList\>](./manual/gacui/components/controls/ribbon/gallerylist.md)
          - [\<RibbonToolstripHeader\>](./manual/gacui/components/controls/ribbon/header.md)
  - [Control Templates](./manual/gacui/components/ctemplates/home.md)
    - [\<CommonScrollBehavior\>](./manual/gacui/components/ctemplates/commonscrollbehavior.md)
    - [\<CommonScrollViewLook\>](./manual/gacui/components/ctemplates/commonscrollviewlook.md)
    - [\<CommonDatePicker\>](./manual/gacui/components/ctemplates/commondatepickerlook.md)
  - [Item Templates](./manual/gacui/components/itemplates/home.md)
    - [\<ListItemTemplate\>](./manual/gacui/components/itemplates/listitemtemplate.md)
    - [\<TextListItemTemplate\>](./manual/gacui/components/itemplates/textlistitemtemplate.md)
    - [\<TreeItemTemplate\>](./manual/gacui/components/itemplates/treeitemtemplate.md)
    - [\<GridVisualizerTemplate\>](./manual/gacui/components/itemplates/gridvisualizertemplate.md)
    - [\<GridEditorTemplate\>](./manual/gacui/components/itemplates/grideditortemplate.md)
  - [Components](./manual/gacui/components/components/home.md)
    - [\<MessageDialog\>](./manual/gacui/components/components/messagedialog.md)
    - [\<ColorDialog\>](./manual/gacui/components/components/colordialog.md)
    - [\<FontDialog\>](./manual/gacui/components/components/fontdialog.md)
    - [\<OpenFileDialog\>](./manual/gacui/components/components/openfiledialog.md)
    - [\<SaveFileDialog\>](./manual/gacui/components/components/savefiledialog.md)
- [Advanced Topics](./manual/gacui/advanced/home.md)
  - [Interop with C++ View Model](./manual/gacui/advanced/vm.md)
  - [Data Bindings](./manual/gacui/advanced/bindings.md)
  - [Animations](./manual/gacui/advanced/animations.md)
  - [Localization](./manual/gacui/advanced/localization.md)
  - [Cross XML Resource References](./manual/gacui/advanced/cxrr.md)
  - [ALT Sequence and Control Focus](./manual/gacui/advanced/alt.md)
  - [TAB and Control Focus](./manual/gacui/advanced/tab.md)
  - [Creating New List Controls](./manual/gacui/advanced/impllistcontrol.md)
  - [Porting GacUI to Other Platforms](./manual/gacui/advanced/implosprovider.md)
- [Hosted Mode and Remote Protocol](./manual/gacui/modes/home.md)
  - [Remote Protocol Core Application](./manual/gacui/modes/remote_core.md)
  - [Remote Protocol Client Application](./manual/gacui/modes/remote_client.md)
  - [Implementing a Communication Protocol](./manual/gacui/modes/remote_communication.md)

## Unit Testing

- [Using Unit Test Framework](./manual/unittest/vlpp.md)
- [Running GacUI in Unit Test Framework](./manual/unittest/gacui.md)
  - [Snapshots and Frames](./manual/unittest/gacui_frame.md)
  - [IO Interaction](./manual/unittest/gacui_io.md)
  - [Accessing Controls](./manual/unittest/gacui_controls.md)
  - [Snapshot Viewer](./manual/unittest/gacui_snapshots.md)

# VlppOS Knowledge Base

Project introduction remains in [Index.md](./Index.md#vlppos).

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
- Use `GetPathDelimiter`, `operator/`, `GetName`, `GetFolder`, `GetFullPath`, `GetRelativePathFor` for path operations
- Use `IsFile`, `IsFolder`, `IsRoot` to determine path object types
- Use `File` class for file operations when `FilePath::IsFile` returns true
- Use `ReadAllTextWithEncodingTesting`, `ReadAllTextByBom`, `ReadAllLinesByBom` for text reading
- Use `WriteAllText`, `WriteAllLines` for text writing
- Use `Exists`, `Delete`, `Rename` for file operations
- Use `Folder` class for directory operations when `FilePath::IsFolder` or `FilePath::IsRoot` returns true
- Use `GetFolders`, `GetFiles` for directory content enumeration
- Use `Create(bool recursively)`, `Delete(bool recursively)`, `Rename` for folder operations
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
- Use `IsAvailable`, `CanRead`, `CanWrite`, `CanSeek`, `CanPeek`, `IsLimited` for capability checking
- Use `Read`, `Write`, `Peek`, `Seek`, `SeekFromBegin`, `SeekFromEnd`, `Position`, `Size` for stream operations
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
- Use `TaskQueue` when queued work must run on one blocking task loop instead of the thread pool
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
- Use `WakeOnePending`, `WakeAllPendings` for condition variable signaling

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

#### Inter-Process Network Protocols and Channels

Production-usable asynchronous text-protocol and typed named-channel abstractions. Every concrete network-protocol implementation supplied by this repository, including the async-socket-based Mini HTTP transport, is testing-only; production applications should provide their own `INetworkProtocol*` implementation.

- Use `INetworkProtocolServer`, `INetworkProtocolClient`, `INetworkProtocolConnection` and `INetworkProtocolCallback` for asynchronous `WString` messages.
- Use `IChannelServer<TPackage>`, `IChannelClient<TPackage>`, `IChannel<TPackage>` and `IChannelReader<TPackage>` for typed named channels with client ids, direct sends, broadcasts and batched writes.
- Use `NetworkProtocolChannelServer<TPackage, TSerialization, TServerBase>`, `NetworkProtocolChannelClient<TPackage, TSerialization>` and `NetworkProtocolLocalChannelClient<TPackage, TSerialization>` to run the channel implementation over a production-ready custom protocol.
- Use `vl::inter_process::async_tcp_socket::SocketHttpServer` and `vl::inter_process::async_tcp_socket::SocketHttpClient` only for testing as the portable Mini HTTP implementation of the raw network-protocol interfaces.

[API Explanation](./KB_VlppOS_InterProcessNetworkProtocolsAndChannels.md)

#### Async-Socket-Based Mini HTTP API

Testing-only layered loopback TCP and HTTP/1.1 APIs for asynchronous binary streams, parsed request/response connections and prefix-dispatched Mini HTTP services. Do not use these APIs in production code.

- Use `vl::inter_process::async_tcp_socket::IAsyncSocketServer`, `vl::inter_process::async_tcp_socket::IAsyncSocketClient`, `vl::inter_process::async_tcp_socket::IAsyncSocketConnection`, `vl::inter_process::async_tcp_socket::IAsyncSocketCallback` and `vl::inter_process::async_tcp_socket::AsyncSocketBuffer` for asynchronous loopback byte streams. Each server and client exposes its immutable construction port through `GetPort()`, and a client creates a fresh transport-preserving lane through `CreateSameEndpointClient()`.
- Use the platform-neutral `vl::inter_process::async_tcp_socket::CreateDefaultAsyncSocketServer` and `vl::inter_process::async_tcp_socket::CreateDefaultAsyncSocketClient` factories at the composition boundary.
- Use `vl::inter_process::async_tcp_socket::windows_socket::AsyncSocketServer` / `vl::inter_process::async_tcp_socket::windows_socket::AsyncSocketClient`, `vl::inter_process::async_tcp_socket::linux_socket::AsyncSocketServer` / `vl::inter_process::async_tcp_socket::linux_socket::AsyncSocketClient`, or `vl::inter_process::async_tcp_socket::macos_socket::AsyncSocketServer` / `vl::inter_process::async_tcp_socket::macos_socket::AsyncSocketClient` for the current platform.
- Use `vl::inter_process::async_tcp_socket::HttpRequest`, `vl::inter_process::async_tcp_socket::HttpResponse`, `vl::inter_process::async_tcp_socket::HttpRequestServer`, `vl::inter_process::async_tcp_socket::HttpRequestClient`, `vl::inter_process::async_tcp_socket::IHttpRequestConnection` and `vl::inter_process::async_tcp_socket::IHttpRequestCallback` for binary-safe sequential HTTP/1.1 exchanges.
- Use `vl::inter_process::async_tcp_socket::SocketHttpServerApi`, `vl::inter_process::async_tcp_socket::SocketHttpRequestContext` and `vl::inter_process::async_tcp_socket::SocketHttpClientApi` for prefix-dispatched Mini HTTP request/response work; inject socket dependencies explicitly, derive the port from the injected socket, reuse the same server pointer when multiple prefixes must share one listener, and treat client-side 404 as a terminal `ResponseNotFound` error.

[API Explanation](./KB_VlppOS_InterProcessAsyncSocketBasedMiniHttpApi.md)

### Design Explanation

#### Implementing an Injectable Feature

- Linked-list based dependency injection mechanism enabling runtime replacement and extension of feature implementations while maintaining delegation capabilities
- Three core components: `IFeatureImpl` base interface, `FeatureImpl<TImpl>` template for type-safe delegation, and `FeatureInjection<TImpl>` manager for chain operations
- Standard implementation pattern with interface definition, default implementation, global management functions using static local variables for thread-safe singleton behavior
- Delegation mechanism through `Previous()` method allowing partial overrides and full delegation with LIFO injection structure and cascading ejection behavior
- Critical lifecycle guarantees where `EndInjection` only called during explicit operations, and restriction of injection/ejection to application-level code for proper ordering
- Real-world implementation demonstrated through DateTime system with platform-specific implementations and testing integration using mock implementations

[Design Explanation](./KB_Vlpp_Design_ImplementingInjectableFeature.md)

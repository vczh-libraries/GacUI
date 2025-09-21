# Stream Operations

Unified stream interface for file, memory, and data transformation operations with encoding support.

## IStream Interface

All stream implements `IStream` interface. Unfortunately there is another `IStream` in `Windows.h`, so it is not recommended to do `using namespace vl::stream;`, using `stream::` is recommended instead.

Streams are recommended to be used as value types, but they cannot be copied or moved.

### Stream Availability

A stream is available when `IsAvailable` returns true. All other methods can only be used in this case.
Calling `Close` will release the resource behind the stream and make it unavailable.
Usually we don't need to call `Close` explicitly, it will be called internally when the stream is destroyed.

### Stream Capabilities

Use `IsAvailable`, `CanRead`, `CanWrite`, `CanSeek`, `IsLimited` for capability checking.

#### Readable Streams

A stream is readable when `CanRead` returns true. `Read` and `Peek` can only be used in this case.

Here are all streams that guaranteed to be readable so no further checking is needed:
- `FileStream` with `FileStream::ReadOnly` or `FileStream::ReadWrite` in the constructor.
- `MemoryStream`
- `MemoryWrapperStream`
- `DecoderStream`
- `RecorderStream`
- The following streams are readable when their underlying streams are readable
  - `CacheStream`

#### Writable Streams

A stream is writable when `CanWrite` returns true. `Write` can only be used in this case.

Here are all streams that guaranteed to be writable so no further checking is needed:
- `FileStream` with `FileStream::WriteOnly` or `FileStream::ReadWrite` in the constructor.
- `MemoryStream`
- `MemoryWrapperStream`
- `EncoderStream`
- `BroadcastStream`
- The following streams are readable when their underlying streams are writable 
  - `CacheStream`

#### Seekable Streams

A stream is random accessible when `CanSeek` returns true. `Seek`, `SeekFromBegin` can only be used in this case. `SeekFromEnd` can only be used when both `CanSeek` and `IsLimited` returns true.
Use `Position` to know the current seeking position.
`Read` and `Peek` will read the data at the seeking position.

Here are all streams that guaranteed to be seekable so no further checking is needed:
- `FileStream`
- `MemoryStream`
- `MemoryWrapperStream`
- The following streams are readable when their underlying streams are seekable
  - `CacheStream`

#### Limited/Finite Streams

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

### Basic Stream Operations

Use `Read`, `Write`, `Peek`, `Seek`, `Position`, `Size` for stream operations.
Use `Close` for resource cleanup (automatic on destruction).

## FileStream

Initialize `FileStream` with a file path (`WString` instead of `FilePath`) to open a file. One of `FileStream::ReadOnly`, `FileStream::WriteOnly` and `FileStream::ReadWrite` must be specified.

## MemoryStream

`MemoryStream` maintain a consecutive memory buffer to store data.
Use `GetInternalBuffer` to get the pointer to the buffer.
The pointer is only safe to use before `MemoryStream` is written to, because when the buffer is not long enough, a new one will be created and the old will will be deleted.
The buffer will be deleted when `MemoryStream` is destroyed.

## MemoryWrapperStream

`MemoryWrapperStream` operates on a given memory buffer, `MemoryWrapperStream` will be delete the buffer.

## EncoderStream and DecoderStream

An `EncoderStream` transform the data using the given `IEncoder` and then write to a given writable stream. It is write only stream.

A `DecoderStream` read data from a given readable stream and then transform the data using the given `IDecoder`. It is a read only stream.

By stacking multiple encoders, decoders and stream together, we can create a pipeline of data processing.

### Example: Reading UTF-8 File

When we need to read a UTF-8 file into a `WString` we could use:
```cpp
FileStream fileStream(fullPath, FileStream::ReadOnly);
Utf8Decoder decoder;
DecoderStream decoderStream(fileStream, decoder);
StreamReader reader(decoderStream);
auto text = reader.ReadToEnd();
```

### Example: Writing UTF-8 File with BOM

When we need to write a `WString` to a UTF-8 file with BOM enabled we could use:
```cpp
FileStream fileStream(fullPath, FileStream::WriteOnly);
BomEncoder encoder(BomEncoder::Utf8);
EncoderStream encoderStream(fileStream, encoder);
StreamWriter writer(encoderStream);
writer.WriteString(text);
```

Or just use `File` to do the work which is much simpler.

## Extra Content

### Stream Design Philosophy

The stream system in VlppOS follows a layered design pattern where basic streams provide fundamental I/O operations, and decorator streams add specific functionality like encoding/decoding, caching, or broadcasting.

### Performance Considerations

When working with large amounts of data, prefer streaming operations over loading entire content into memory. Use `MemoryStream` for temporary buffers and `FileStream` for persistent storage. Or use `CacheStream` to improve `FileStream` performance when random access is not required.

### Error Handling

Stream operations may throw exceptions when encountering I/O errors, permission issues, or when operations are performed on streams that don't support them (e.g., writing to a read-only stream).

### Resource Management

Streams automatically manage their resources through RAII. The `Close()` method is called automatically in destructors, but can be called explicitly when immediate resource cleanup is needed.
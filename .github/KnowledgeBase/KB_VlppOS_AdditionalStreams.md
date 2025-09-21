# Additional Streams

Specialized stream types for caching, recording, and broadcasting data operations.

## CacheStream

Use `CacheStream` for performance optimization with frequently accessed data.

`CacheStream` maintain a cache to reduce calls to the underlying stream.
It increases the performance when there are too many data to read/write,
or when the same part of the data needs to be modified repeatly.

### CacheStream Capabilities

`CacheStream` inherits the capabilities of its underlying stream:
- Readable when the underlying stream is readable
- Writable when the underlying stream is writable  
- Seekable when the underlying stream is seekable
- Limited/finite when the underlying stream is limited/finite

### Use Cases

CacheStream is particularly useful for:
- Random access patterns on large files
- Repeated reading/writing of the same data sections
- Reducing I/O operations on slow storage devices
- Optimizing performance when working with network streams

## RecorderStream

Use `RecorderStream` for copying data from one stream to another during reading.

`RecorderStream` reads data from one readable stream while copying everything to another writable stream.

### RecorderStream Behavior

- It is a read-only stream that wraps another readable stream
- Every read operation is simultaneously written to a target stream
- Useful for creating backups or logs of data as it's being processed
- The recorded data can be written to any writable stream (file, memory, etc.)

### Use Cases

RecorderStream is ideal for:
- Creating audit trails of data processing
- Simultaneous backup during data migration
- Logging network communications
- Splitting data streams for parallel processing

## BroadcastStream

Use `BroadcastStream` for writing the same data to multiple target streams.

`BroadcastStream` write the same data to multiple streams, which is managed by the `Targets()` method.

### Managing Broadcast Targets

Use `Targets()` method to manage BroadcastStream destinations.

The `Targets()` method returns a collection that allows you to:
- Add new target streams for broadcasting
- Remove existing target streams
- Query the current list of active targets

### BroadcastStream Behavior

- It is a write-only stream that distributes writes to multiple targets
- Each write operation is replicated to all registered target streams
- If any target stream fails, the broadcast operation may fail
- All target streams should be writable for proper operation

### Use Cases

BroadcastStream is useful for:
- Creating multiple backup copies simultaneously
- Writing to both file and network destinations
- Distributing data to multiple processing pipelines
- Creating redundant storage for critical data

## Extra Content

### Stream Combination Patterns

These additional streams can be combined with basic streams and encoding streams to create sophisticated data processing pipelines:

1. **Cached File Processing**: `CacheStream` wrapping a `FileStream` for improved random access performance
2. **Recorded Encoding**: `RecorderStream` capturing the output of an `EncoderStream` for debugging
3. **Broadcast with Compression**: `BroadcastStream` sending data to both compressed and uncompressed destinations

### Performance Considerations

- **CacheStream**: Trades memory usage for reduced I/O operations
- **RecorderStream**: Doubles the write operations but provides valuable audit capabilities  
- **BroadcastStream**: Multiplies write operations by the number of targets

### Error Handling

Each specialized stream type has specific error scenarios:
- **CacheStream**: May fail if the underlying stream becomes unavailable
- **RecorderStream**: Can fail if either the source or target stream has issues
- **BroadcastStream**: May partially succeed if only some target streams fail

### Memory Management

- **CacheStream**: Automatically manages cache size and eviction policies
- **RecorderStream**: Doesn't store data itself, just passes it through
- **BroadcastStream**: Minimal memory overhead as it doesn't buffer data

### Thread Safety

These stream types inherit the thread safety characteristics of their underlying streams. Additional synchronization may be needed when using them in multi-threaded scenarios, especially with `BroadcastStream` where multiple targets might be accessed concurrently.
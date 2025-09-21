# Multi-threading

Cross-platform threading primitives and synchronization mechanisms for concurrent programming.

## Thread Pool Operations

Use `ThreadPoolLite::Queue` and `ThreadPoolLite::QueueLambda` for thread pool execution.

Use static functions `ThreadPoolLite::Queue` or `ThreadPoolLite::QueueLambda` to run a function in another thread.

### ThreadPoolLite Benefits

The thread pool provides several advantages over manual thread creation:
- Automatic thread lifecycle management
- Resource pooling to avoid thread creation overhead
- Better system resource utilization
- Simplified concurrent programming model

### Usage Examples

```cpp
// Queue a function pointer
ThreadPoolLite::Queue(someFunction);

// Queue a lambda expression
ThreadPoolLite::QueueLambda([]() {
    // Work to be done in background thread
});
```

## Thread Control Operations

### Thread Pausing

Use `Thread::Sleep` for thread pausing.

Use static function `Thread::Sleep` to pause the current thread for some milliseconds.

### Thread Identification

Use `Thread::GetCurrentThreadId` for thread identification.

Use static function `Thread::GetCurrentThreadId` to get an identifier for the OS native thread running the current function.

## Manual Thread Creation

Use `Thread::CreateAndStart` only when thread pool is insufficient.

`Thread::CreateAndStart` could be used to run a function in another thread while returning a `Thread*` to control it, but this is not recommended.
Always use `ThreadPoolLite` if possible.

### When to Use Manual Threads

Manual thread creation should only be considered when:
- You need fine-grained control over thread lifecycle
- Thread-specific configurations are required
- The thread pool doesn't meet specific timing requirements
- Working with legacy code that requires direct thread handles

## Thread Pool with Synchronization

A `ThreadPoolLite` call with an `EventObject` is a better version of `Thread::Wait`.

This approach provides better resource management and avoids the complexities of manual thread synchronization.

## Extra Content

### Threading Best Practices

1. **Prefer Thread Pool**: Use `ThreadPoolLite` for most concurrent operations
2. **Avoid Thread Creation**: Manual thread creation adds overhead and complexity
3. **Use Synchronization Primitives**: Combine threading with proper synchronization objects
4. **Handle Exceptions**: Ensure proper exception handling in threaded code

### Performance Considerations

- **Thread Pool**: Minimal overhead for task scheduling and execution
- **Thread Creation**: Significant overhead for creating and destroying threads
- **Context Switching**: Consider the cost of frequent context switches
- **Resource Contention**: Be aware of shared resource access patterns

### Cross-Platform Considerations

The threading API provides a unified interface across platforms:
- Windows: Uses Windows thread API internally
- Linux: Uses pthread API internally  
- Behavior remains consistent across platforms
- Thread IDs are platform-specific but the API is uniform

### Memory Management

- Thread pool automatically manages worker threads
- Manual threads require explicit cleanup
- Lambda captures should be carefully managed for thread safety
- Avoid capturing references to stack variables in threaded lambdas

### Error Handling

Threading operations can fail due to:
- System resource exhaustion
- Permission restrictions
- Platform-specific limitations

Always check return values and handle potential failures gracefully.

### Integration with Synchronization

Multi-threading works best when combined with appropriate synchronization primitives:
- Use with `Mutex` for cross-process synchronization
- Combine with `CriticalSection` for in-process protection
- Integrate with `EventObject` for thread coordination
- Apply `ConditionVariable` for complex waiting scenarios
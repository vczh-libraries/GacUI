# Waitable Objects

Cross-process synchronization objects that support waiting operations with timeout capabilities.

## WaitableObject Interface

All locks mentioned here implements `WaitableObject`. A `WaitableObject` offer these functions to wait while blocking the current thread:

- `Wait`: wait until it signals. It could return false meaning the wait operation did not succeed and there is no guarantee about the status of the `WaitableObject`.
- `WaitForTime`: wait until it signals with a timeout. It could return false like `Wait`, including reaching the timeout.
  - IMPORTANT: it is Windows only.

### Multiple Object Waiting

There are also static functions `WaitAll`, `WaitAllForTime`, `WaitAny`, `WaitAnyForTime` to wait for multiple `WaitableObject` at the same time.
- IMPORTANT: they are Windows only.

## Platform Naming Convention

All following classes are named after Windows API. Their Linux version works exactly like Windows but with less features.

## Mutex

Use `Mutex` for cross-process mutual exclusion.

`Mutex` pretty much serves the same purpose like `SpinLock` and `CriticalSection`, but it could be shared across multiple processes, meanwhile costs more OS resource. Prefer `SpinLock` or `CriticalSection` one only operates in one process.

### Mutex Lifecycle

- The constructor does not actually create a mutex. You must call `Create` and `Open` later.
- The `Create` method creates a mutex.
  - If the name is not empty, the mutex is associated to a name, which works across different processes.
  - No thread owns a mutex that is just created.
- The `Open` method shares a created mutex with a name.

### Mutex Operations

Use `Create` and `Open` methods for establishing named synchronization objects.
Use `Wait`, `WaitForTime` for blocking operations with optional timeout.
Use `Release` for releasing mutex and semaphore ownership.

Calling `Wait` will block the current thread until it owns the mutex. Calling `Release` release the owned mutex to other threads.

## Semaphore

Use `Semaphore` for counting semaphore operations across processes.

### Semaphore Lifecycle

- The constructor does not actually create a semaphore. You must call `Create` and `Open` later.
- The `Create` method creates a semaphore.
  - If the name is not empty, the semaphore is associated to a name, which works across different processes.
  - No thread owns a semaphore that is just created.
- The `Open` method shares a created semaphore with a name.

### Semaphore Operations

Calling `Wait` will block the current thread until it owns the semaphore.
- Calling `Release` release the semaphore, for once or multiple times.
- Unlike `Mutex`, multiple threads could own the same semaphore, as long as enough `Release` is called. And a thread doesn't need to own a semaphore to release it.

## EventObject

Use `EventObject` for event signaling across processes.

### EventObject Lifecycle

- The constructor does not actually create an event object. You must call `CreateAutoUnsignal`, `CreateManualUnsignal` and `Open` later.
- The `CreateAutoUnsignal` and `CreateManualUnsignal` method creates an event object.
  - An auto unsignal event object means, when it is owned by a thread, it automatically unsignaled. So only one thread will be unblocked. Otherwise multiple threads waiting for this event object will be unblocked at the same time.
  - If the name is not empty, the event object is associated to a name, which works across different processes.
- The `Open` method shares a created event object with a name.

### EventObject Operations

Use `Signal`, `Unsignal` for event object state management.

- Calling `Wait` will block the current thread until it is signaled.
- Calling `Signal` to signal an event object.
- Calling `Unsignal` to unsignal an event object.

## Multiple Object Synchronization

Use `WaitAll`, `WaitAllForTime`, `WaitAny`, `WaitAnyForTime` for multiple object synchronization.

These static functions allow coordination with multiple waitable objects simultaneously:
- `WaitAll`: Block until all specified objects are signaled
- `WaitAny`: Block until any one of the specified objects is signaled  
- Timeout versions provide time-limited waiting capabilities

## Extra Content

### Cross-Process Synchronization

Waitable objects provide the foundation for inter-process communication and synchronization:

- **Named Objects**: Objects with names can be shared across process boundaries
- **Security**: Named objects inherit security attributes from the creating process
- **Lifetime**: Objects persist as long as any process holds a reference

### Performance Considerations

Waitable objects have different performance characteristics compared to in-process synchronization:

- **System Overhead**: Cross-process synchronization involves kernel mode switches
- **Scalability**: Limited by system-wide synchronization object limits
- **Network Transparency**: Not suitable for distributed synchronization

### Error Handling

Waitable object operations can fail due to various conditions:

- **Abandoned Objects**: When owning process terminates unexpectedly
- **Timeout Conditions**: When wait operations exceed specified time limits
- **System Limits**: When maximum number of handles is exceeded
- **Permission Issues**: When processes lack appropriate access rights

### Best Practices

Guidelines for effective use of waitable objects:

1. **Prefer In-Process**: Use non-waitable primitives when cross-process sync isn't needed
2. **Name Carefully**: Use descriptive, unique names for shared objects
3. **Handle Timeouts**: Always handle timeout scenarios gracefully
4. **Resource Management**: Properly close handles to avoid resource leaks
5. **Security Aware**: Consider security implications of named objects

### Platform-Specific Features

Feature availability varies by platform:

- **Windows**: Full support for all waitable object features
- **Linux**: Limited timeout support and fewer object types
- **Portability**: Stick to basic features for cross-platform compatibility

### Integration Patterns

Common usage patterns with waitable objects:

- **Process Coordination**: Use `EventObject` for startup/shutdown signaling
- **Resource Limiting**: Use `Semaphore` for controlling access to limited resources
- **Mutual Exclusion**: Use `Mutex` for cross-process critical sections
- **Complex Coordination**: Combine multiple objects with `WaitAny`/`WaitAll`

### Debugging and Monitoring

Tools and techniques for troubleshooting waitable objects:

- **Handle Tracking**: Monitor handle counts and leaks
- **Deadlock Detection**: Watch for circular wait conditions
- **Performance Profiling**: Measure synchronization overhead
- **Event Tracing**: Use system tools to trace synchronization events
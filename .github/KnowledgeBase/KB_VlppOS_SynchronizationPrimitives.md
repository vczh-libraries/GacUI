# Synchronization Primitives

Non-waitable synchronization objects for protecting shared resources in multi-threaded environments.

## Lock Selection Guidelines

Only use `SpinLock` when the protected code exists super fast.
Only use `CriticalSection` when the protected code costs time.

## SpinLock

Use `SpinLock` for protecting very fast code sections.

### SpinLock Operations

- `Enter` blocks the current thread, and when it returns, the current thread owns the spin lock.
  - Only one thread owns the spin lock.
  - `TryEnter` does not block the current thread, and there is a chance that the current thread will own the spin lock, indicated by the return value.
- `Leave` releases the spin lock from the current thread.

### SpinLock Automation

Use `SPIN_LOCK`, `CS_LOCK`, `READER_LOCK`, `WRITER_LOCK` macros for exception-safe automatic locking.

When it is able to `Enter` and `Leave` in the same function, use `SPIN_LOCK` to simplify the code.
It is also exception safety, so you don't need to worry about try-catch:

```cpp
SpinLock lock;
SPIN_LOCK(lock)
{
  // fast code that owns the spin lock
}
```

## CriticalSection

Use `CriticalSection` for protecting time-consuming code sections.

### CriticalSection Operations

- `Enter` blocks the current thread, and when it returns, the current thread owns the critical section.
  - Only one thread owns the critical section.
  - `TryEnter` does not block the current thread, and there is a chance that the current thread will own the critical section, indicated by the return value.
- `Leave` releases the critical section from the current thread.

### CriticalSection Automation

When it is able to `Enter` and `Leave` in the same function, use `CS_LOCK` to simplify the code.
It is also exception safety, so you don't need to worry about try-catch:

```cpp
CriticalSection cs;
CS_LOCK(cs)
{
  // slow code that owns the critical section
}
```

## ReaderWriterLock

Use `ReaderWriterLock` for multiple reader, single writer scenarios.

`ReaderWriterLock` is an advanced version of `CriticalSection`:
- Multiple threads could own the same reader lock. When it happens, it prevents any thread from owning the writer lock.
- Only one threads could own the writer lock. When it happens, it prevents any thread from owning the reader lock.

### ReaderWriterLock Operations

- Call `TryEnterReader`, `EnterReader` and `LeaveReader` to access the reader lock.
- Call `TryEnterWriter`, `EnterWriter` and `LeaveWriter` to access the writer lock.

### ReaderWriterLock Automation

When it is able to `EnterReader` and `LeaveReader` in the same function, use `READER_LOCK` to simplify the code.
When it is able to `EnterWriter` and `LeaveWriter` in the same function, use `WRITER_LOCK` to simplify the code.
They are also exception safety, so you don't need to worry about try-catch:

```cpp
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

## ConditionVariable

Use `ConditionVariable` with `SleepWith`, `SleepWithForTime` for conditional waiting.

A `ConditionVariable` works with a `CriticalSection` or a `ReaderWriterLock`.

### ConditionVariable with CriticalSection

- Call `SleepWith` to work with a `CriticalSection`. It works on both Windows and Linux.
- Call `SleepWithForTime` to work with a `CriticalSection` with a timeout. It only works on Windows.

### ConditionVariable with ReaderWriterLock

- Call `SleepWithReader`, `SleepWithReaderForTime`, `SleepWriter` or `SleepWriterForTime` to work with a `ReaderWriterLock`. They only work on Windows.

### ConditionVariable Behavior

The `Sleep*` function temporarily releases the lock from the current thread, and block the current thread until it owns the lock again.

- Before calling the `Sleep*` function, the current thread must own the lock.
- Calling the `Sleep*` function releases the lock from the current thread, and block the current thread.
- The `Sleep*` function returns when `WakeOnePending` or `WaitAllPendings` is called.
  - The `Sleep*ForTime` function could also return when it reaches the timeout. But this will not always happen, because:
    - `WaitOnePending` only activates one thread pending on the condition variable.
    - `WaitAllPendings` activates all thread but they are also controlled by the lock.
  - When `Sleep*` returns, the current thread owns the lock.

### ConditionVariable Signaling

Use `WakeOnePending`, `WaitAllPendings` for condition variable signaling.

## Extra Content

### Lock Performance Characteristics

Different synchronization primitives have varying performance profiles:

- **SpinLock**: Lowest overhead for very short critical sections, but wastes CPU cycles if held too long
- **CriticalSection**: Higher overhead but efficient for longer critical sections
- **ReaderWriterLock**: Most complex but allows concurrent reads

### Choosing the Right Primitive

Selection criteria for synchronization primitives:

1. **SpinLock**: Use when critical section execution time is less than a context switch (~50-100 microseconds)
2. **CriticalSection**: Use for general-purpose mutual exclusion with longer critical sections  
3. **ReaderWriterLock**: Use when reads are frequent and writes are infrequent
4. **ConditionVariable**: Use when threads need to wait for specific conditions

### Exception Safety

All automation macros (`SPIN_LOCK`, `CS_LOCK`, `READER_LOCK`, `WRITER_LOCK`) provide RAII-style exception safety:
- Locks are automatically released when leaving the scope
- Proper cleanup even if exceptions are thrown
- No manual lock management required

### Platform Differences

Synchronization behavior varies across platforms:
- Windows: Full support for all features including timeout operations
- Linux: Limited timeout support for some operations
- Cross-platform code should avoid Windows-only features when possible

### Deadlock Prevention

Best practices to avoid deadlocks:
- Always acquire locks in the same order across all threads
- Use timeout versions of lock operations when available
- Keep critical sections as short as possible
- Consider using higher-level synchronization abstractions

### Performance Optimization

Tips for optimal synchronization performance:
- Minimize lock contention by keeping critical sections short
- Use appropriate lock granularity (not too fine, not too coarse)
- Consider lock-free alternatives for high-performance scenarios
- Profile lock contention in performance-critical applications

### Integration with Threading

Synchronization primitives work best when combined with the multi-threading APIs:
- Use with `ThreadPoolLite` for concurrent task execution
- Combine with `ConditionVariable` for complex coordination scenarios
- Apply appropriate synchronization for shared data structures
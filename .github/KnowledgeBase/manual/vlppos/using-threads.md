# Using Threads and Locks

**Lock protects resources, lock does not protect code.**

[The synchronization topic in MSDN](https://docs.microsoft.com/en-us/windows/win32/sync/synchronization)is recommended for learning how to write concurrent programs correctly in details.

## Thread and thread pool

**ThreadPoolLite**is always recommended to use whenever possible, instead of using**Thread**. Waiting for a thread to exit is a little bit tricky, it is much safer to use**EventObject**for this purpose. This is why you are not recommended to use**Thread**directly.

To start a background task, this is the easiest way:
```
ThreadPoolLite::Queue([]()
{
    // do something
});
```


An auto-delete thread is also able to do the same job:
```
Thread::CreateAndStart([]()
{
    // do something
}, true);
```
By passing**true**as the second argument, the**Thread**object will be deleted after the function finishes. It is very important to notice that, when you get the returned**Thread**object from the**CreateAndStart**function, the thread could have finished and then the object was deleted.

To wait for a task to complete, A manual-unsignal**EventObject**is always recommended. Manual-unsignal means that, when the event object is signaled and something pick up the event, the event object will not be automatically unsignaled.
```
EventObject stopEvent;
stopEvent.CreateManualUnsignal(false);

ThreadPoolLite::Queue([&]()
{
    // do something
    stopEvent.Signal();
});

stopEvent.Wait();
```


## Thread local variables

By using**ThreadVariable\<T\>**as a global variable, a thread local variable is created.
```
ThreadVariable<int> v;

ThreadPoolLite::Queue([&]()
{
    // this thread has a copy of v
    v.Set(1);
});

ThreadPoolLite::Queue([&]()
{
    // this thread has another copy of v too
    v.Set(2);
});

// the main thread also has a different copy of v
v.Set(3);
```


## Mutex, CriticalSection and SpinLock

**Mutex**,**CriticalSection**and**SpinLock**is very similar. When multiple threads are waiting for a**Mutex**, a**CriticalSection**or a**SpinLock**, only one thread could pick it up at the same time.

**Mutex**uses system resource. It is recommended when:
- Owning a mutex for a long time could happen.
- Synchronize multiple operations across processes. A global mutex requires a name to create.

**CriticalSection**is a bit faster than**Mutex**. It is recommended when:
- Owning a critical section for a long time could happen.
- **(Windows only)**You need a composable lock. When a critical section is owned by a thread, this particular thread can pick up the critical section again without result in a deadlock.

**CS_LOCK**macro is a convenient way to wait for a critical section:
```
CriticalSection cs;

CS_LOCK(cs)
{
    // do something when owning the lock
}
```


**SpinLock**is recommended when.
- Always owning a spin lock for a very short time.

**SPIN_LOCK**macro is a convenient way to wait for a spin lock:
```
SpinLock lock;

SPIN_LOCK(lock)
{
    // do something when owning the lock
}
```


## Semaphore

**Semaphore**is simlar to**Mutex**, the main difference is that, multiple threads can pick up the same semaphore at the same time. You can set a maximum number of owning threads.

A semaphore could be created as a global object to use across processes, it requires a name to create.

## EventObject

**EventObject**cannot be owned by a thread. Instead, it is used to notify that something happens.

**(Windows only)**An event object could be created as a global object to use across processes, it requires a name to create.

A auto-unsignal event is initialized using**CreateAutoUnsignal**. When multiple threads is waiting on an auto-unsignal event, if this event is signaled, only one thread get noticed, and the event is automatically unsignaled.

A manual-unsignal event is initialized using**CreateManualUnsignal**. When multiple threads is waiting on an auto-unsignal event, if this event is signaled, all threads get noticed.

## ReaderWriterLock

**ReaderWriterLock**can be owned by multiple thread in**reader mode**. If any thread owns a**ReaderWriterLock**in**reader mode**, no thread can own it in**writer mode**.

**ReaderWriterLock**can be owned by only one thread in**writer mode**. If any thread owns a**ReaderWriterLock**in**writer mode**, no thread can own it in**reader mode**.

It is recommended to use**ReaderWriterLock**in this way:
```
ReaderWriterLock lock;

READER_LOCK(lock)
{
    // do something when owning the lock in reader mode
}

WRITER_LOCK(lock)
{
    // do something when owning the lock in writer mode
}
```


## ConditionVariable

**ReaderWriterLock**is used when you need to temporarily give up a**CriticalSection**or a**ReaderWriterLock**to wait for a signal.

You are not recommended to use**EventObject**in such scenario, because it is very easy to run into a deadlock.

The**SleepWith**method is used to termporarily give up a**CriticalSeciont**.**(Windows only)**There are also**SleepWithReader**and**SleepWithWriter**for**ReaderWriterLock**. Functions above are only allowed to call when a thread actually owns the lock to give up.

For example:
```
CriticalSection lock;
ConditionVariable cv;

CS_LOCK(lock)
{
    // do something before sleeping.
    DoSomething();

    // give up the lock and become pending on the condition variable,
    // so that another thread could own this lock.
    // cv.WaitOnePending(); or cv.WaitAllPendings(); could wake up this thread.
    // but the lock need to be owned by this thread again in order to actually wake up.
    cv.SleepWith(lock);

    // do something after sleeping, lock is owned by this thread again.
    DoSomething();
}

WRITER_LOCK(lock)
{
    // do something when owning the lock in writer mode
}
```


You can wake one pending thread by calling the**WaitOnePending**method, or**WakeAllPendings**for all threads.


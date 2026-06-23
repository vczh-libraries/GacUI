# Coding Convention (Multi-Threading)

- Most of the code do not require thread safety, DO NOT over engineering.
- `SpinLock` is only for protecting a piece of code or data in a super flashy short time.
  - When defining a `SpinLock` field, names it begins with `lock`, a `// covers a, b, c` comment is recommended to put above it, a empty line is recommended to put around it.
  - When doing `read-process-write` but the `write` part doesn't depend on the result of `process`:
    - The below rules only involve when `process` is heavy, if anything is simple, keep it simple.
    - You can copy or move the heavy structure in `SPIN_LOCK`, and `process` after `SPIN_LOCK`.
    - Especially for scenario when a container should be processed and cleared, `std::move` would be the best choice to copy and clean `Vlpp` containers. Using moved `Vlpp` containers is not undefined behavior.
- When using other locks, try your best to only use methods that available to all platforms.
  - Write cross platform code when it is performance optimal for all platforms.
  - If Windows specific methods could make Windows implementation much better, then you are allowed to implement them in different ways.
  - Use `lock` (both `SpinLock` and `CriticalSection`), `mutex`, `semaphore`, `event`, `semaphore`, `rwlock`, `cv` as lock variable prefixes.
- Prefer lock free construction only when the code would be simple, DO NOT involve complex lock free trick unless explicitly required.
- `std::atomic<T>` should be considered and use it precisely. The code should be correct when running parallelly while I don't want unnecessary `std::atomic<T>`.
  - `atomic_vint` is widely used in the library, use it for `vint`.
- Avoid polling at all cost, I strongly prefer scheduling in async way.

## Platform Independence

- Avoid using any platform specific methods.
- `ThreadPoolLite::Stop` is an exception, if it is used in the application, this function is expected to call at the end of `main`.

## Spawning Threads

`vl::Thread` class is for heavy resource and time consuming work work:
- Heave time consuming usually means the thread lasts for the lifecycle of the process.
- Inherit from `Thread` and override `Run`.
- Avoid using `CreateAndStart` as your best effort, and strictly avoid in unit test projects, as there are small memory leaks that could pollute the memory leaks detecting.

`vl::ThreadPoolLite` class is for medium time consuming work:
- Medium time consuming usually means it is spawned to complete a task, and although the task takes time but it is expected to end.
- Avoid using this class **too parallelly**, it has a limited threads running at the background, queuing too many tasks would cause traffic.

`RepeatingTaskExecutor` queues inputs to trigger the same task when outdated input is discardable:
- Inherit from `RepeatingTaskExecutor` and override `Execute`.
- Calling `SubmitTask` cause the task to begin:
  - If the previous task is running, the new task would wait, otherwise it starts immediately.
  - If the last submitted task did not start, it is discarded and replaced by the new one, aka the queue only stores one pending task.
- One submission executes the task once, when there is no task running, the system resource will be released.
- One example is editor auto completion with editing text as an input, if the user is typing too fast, outdated queued text is discardable because we only response to the last editor state.

`TaskQueue` queues multiple tasks to execute in a single thread:
- `RunTaskQueue` blocks the thread forever until `QueueExitTask` is called. Before calling `QueueExitTask`, it blocks even when no task is running.
- `QueueExitTask` does not cancel any pending `QueueTask`, all queued task will be executed.

`ThreadVariable<T>` can be used on global variables, different threads see different copy even when using the same `ThreadVariable<T>` global variables.

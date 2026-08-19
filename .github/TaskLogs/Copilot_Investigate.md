# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

One more investigation to do, the FileDialogTaskQueue uses a lock,  it it seems that the lock is used during view model calling and InvokeInMainThread execution, all in UI thread. Since this is not a public class so  it is easy to limit the analysis in this cpp file. Confirm that they are actually all in UI thread, and if so, you can remove the lock. Run unit test to make sure the change actually works. Commit and push all local changes once finishing.

# UPDATES

# TEST [CONFIRMED]

Audit every `FileDialogTaskQueue` construction, `Queue` call, and access to
`tasks` and `executing` in
`Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`.
The class is file-local, one queue is owned by each `FileDialogViewModel`, and
the only two `Queue` calls are from `FileDialogFolder::GetType` and
`FileDialogViewModel::RefreshFiles`. Both are GacUI view-model operations and
run on the UI thread; completion callbacks from their queued filesystem work
also return through `GuiApplication::InvokeInMainThread`.

The current implementation is not completely UI-thread-confined, however:
`FileDialogTaskQueue::Execute` invokes a worker with
`GuiApplication::InvokeAsync`, and that worker removes entries from `tasks`
and clears `executing`. Therefore deleting only `SpinLock` would introduce a
data race between UI-thread submissions and worker-thread queue consumption.

The proposed implementation must leave only the copied filesystem task on the
worker. All `tasks` and `executing` reads and writes must happen either in the
known UI-thread `Queue` call sites or in an explicit `InvokeInMainThread`
continuation. Verification requires a successful Debug x64 build, a complete
Debug x64 `UnitTest` run with all test files and cases passing and no memory
leak dump, stable file-dialog snapshots, and no unexpected working-tree
changes.

# PROPOSALS

- No.1 Wait for each UI completion before continuing [DENIED]
- No.2 Batch UI-thread submissions before background execution [DENIED]
- No.3 Retain the cross-thread lock [CONFIRMED]

## No.1 Keep queue state on the UI thread and remove `SpinLock`

Have `Queue` append on the UI thread and start execution when the queue is
idle. `Execute` removes one task while still on the UI thread, passes that
copied task alone to `InvokeAsync`, and posts a main-thread continuation after
the task finishes. The continuation either starts the next queued task or
marks the queue idle. Each existing filesystem task posts its own UI completion
before this continuation, so task completions retain request order and any
tasks queued by a completion join the same FIFO.

This makes `tasks` and `executing` UI-thread-confined while preserving
background filesystem enumeration. The `SpinLock`, `SPIN_LOCK` blocks, and
temporary cross-thread completion flags then become unnecessary.

### CODE CHANGE

- Change the private queue task type from `Func<void()>` to a background
  function returning its UI completion callback.
- Remove `SpinLock` and both `SPIN_LOCK` sections.
- In `Execute`, remove the next task on the UI thread, run only that copied
  task through `InvokeAsync`, then invoke its returned callback through
  `InvokeInMainThread`. After the callback finishes, either start the next
  queued task or clear `executing`, still on the UI thread.
- Change the two filesystem-enumeration tasks to return their existing UI
  mutation lambdas instead of posting those lambdas themselves.

### DENIED

The Debug x64 solution built successfully with zero warnings and zero errors,
and the complete `UnitTest` run passed all 88 files and all 1713 cases without
a memory-leak dump. However, waiting for one UI callback before starting the
next background task changed the observable scheduling contract: 165 tracked
file-dialog snapshot files were rewritten with equal insertion/deletion counts.
The visible test semantics still passed, but element allocation and rendering
order changed throughout all 17 file-dialog cases. The generated changes were
restored rather than accepted as an incidental snapshot migration.

The lock can only be removed if background tasks remain able to run
sequentially and post their UI callbacks without waiting for those callbacks
to execute.

## No.2 Batch UI-thread submissions before background execution

Keep the existing task type and its existing behavior: each task performs
filesystem enumeration in a worker and posts its own UI completion callback.
When the queue becomes active, defer `Execute` through
`InvokeInMainThread`. That collects all submissions from the current UI work
item before execution begins. `Execute` then copies the pending tasks into a
private batch and clears the file-local queue on the UI thread. One
`InvokeAsync` worker executes the copied batch sequentially, preserving FIFO
posting of completion callbacks without waiting for them.

After the batch has posted all completions, the worker posts one
`InvokeInMainThread` continuation. Because it was posted after the task
callbacks, the continuation observes any tasks queued by those callbacks,
starts the next batch if necessary, or marks the queue idle. Consequently all
reads and writes of `tasks` and `executing` are on the UI thread, while the
background/completion ordering remains the same as the locked implementation.

### CODE CHANGE

- Restore `Func<void()>` tasks and the two existing task call sites.
- Remove `SpinLock` and both `SPIN_LOCK` sections.
- Defer the first `Execute` call through `InvokeInMainThread` so same-cycle UI
  submissions form one batch.
- Copy and clear pending tasks on the UI thread, execute that copied batch on
  one worker, and post a final UI continuation that starts another batch or
  clears `executing`.

### DENIED

The Debug x64 solution again built with zero warnings and zero errors, and a
second complete `UnitTest` run again passed all 88 files and all 1713 cases
without a memory-leak dump. Nevertheless, this batching boundary rewrote 166
tracked file-dialog snapshots (8411 insertions and 6723 deletions). Deferring
and batching submissions changes which filesystem requests are already in the
worker batch before UI completions are processed, so it does not preserve the
established element-allocation and rendering order.

The snapshot changes were generated by the experiment and must be restored.
Together with No.1, this demonstrates that moving either queue consumption or
batch formation to the UI thread changes observable behavior.

## No.3 Retain the cross-thread lock

Keep the existing `FileDialogTaskQueue` implementation. Its two producers are
UI-thread view-model paths, and task completion mutations are UI-thread
callbacks, but the consumer is intentionally different: the `InvokeAsync`
worker repeatedly removes pending tasks and clears `executing`. That dynamic
producer/consumer overlap is what lets later UI submissions join the active
worker without introducing another UI scheduling boundary.

`SpinLock` therefore protects real cross-thread state. Retaining it is simpler
and preserves the stable snapshots established by the preceding fix. The
conditional premise for removing it (all queue-state access occurs on the UI
thread) is false.

### CODE CHANGE

Restore the production source and all generated snapshots to the established
implementation. No source-code change is appropriate. Run the full unit test
once more with the restored implementation and require no tracked snapshot
changes.

### CONFIRMED

The source and generated snapshots were restored after both experiments. A
final Debug x64 solution build completed with zero warnings and zero errors.
The final complete `UnitTest` run passed all 88 test files and all 1713 test
cases, produced no memory-leak dump, and left every file-dialog snapshot
unchanged.

The file-local audit establishes the exact thread split:

- `FileDialogFolder::GetType` and `FileDialogViewModel::RefreshFiles` are the
  only `Queue` callers. They submit from UI-thread view-model execution.
- Each submitted task runs filesystem enumeration in the non-UI worker created
  by `InvokeAsync`, then posts its model mutation through `InvokeInMainThread`.
- The same worker also removes tasks from the queue and changes `executing`,
  concurrently with later UI-thread `Queue` calls. Those are the accesses
  protected by `SpinLock`.

No.1 proved that moving consumption behind each UI completion changes 165
snapshots. No.2 proved that moving batch formation to the UI thread changes
166 snapshots. Both full runs passed functional assertions and leak checks,
so the snapshot diffs specifically expose the scheduling change rather than a
test crash. Retaining the lock is the only examined implementation that keeps
the established FIFO scheduling and byte-stable snapshots without adding a
new UI scheduling boundary.

# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `src/sharedApi.ts`
- `src/taskApi.ts`
- `src/index.ts`

Data structures about jobs and tasks are in `src/jobsDef.ts`.
Its spec is in `JobsData.md`.

## Helpers (taskApi.ts) -----------------------------------------------------------------------------------------------------------------------------

All helper functions and types are exported and API implementations should use them.

### SESSION_CRASH_PREFIX

**Referenced by**:
- JobsData.md: `## Running Tasks`

`const SESSION_CRASH_PREFIX = "The session crashed, please redo and here is the last request:\n";`
- Internal constant (not exported). Prefix added to prompts when resending after a session crash.

### sendMonitoredPrompt (crash retry)

**Referenced by**:
- API_Task.md: `### copilot/task/start/{task-name}/session/{session-id}`, `### copilot/task/{task-id}/stop`, `### copilot/task/{task-id}/live`
- API_Job.md: `### copilot/job/start/{job-name}`, `### copilot/job/{job-id}/stop`, `### copilot/job/{job-id}/live`
- JobsData.md: `## Running Tasks`, `## Running Jobs`, `### Task.availability`, `### Task.criteria`

A private method on the `CopilotTaskImpl` class used by both task execution and condition evaluation.
Sends a prompt to a session, retrying up to 5 consecutive crashes (except in borrowing mode where retry is 1).
On retry, prepends `SESSION_CRASH_PREFIX` to the prompt.
Also pushes `onGeneratedUserPrompt` to the driving session's response queue.

### errorToDetailedString

`errorToDetailedString(err: unknown): string;`
- Convert any error into a detailed JSON string representation including name, message, stack, and cause.

### installJobsEntry

`async installJobsEntry(entry: Entry): Promise<void>;`
- Use the entry. It could be `entry` from `jobsData.ts` or whatever.
- This function can only be called when no session is running, otherwise throws.

```typescript
interface ICopilotTask {
  readonly drivingSession: ICopilotSession;
  readonly status: "Executing" | "Succeeded" | "Failed";
  readonly crashError?: any;
  // stop all running sessions, no further callback issues.
  stop(): void;
}

interface ICopilotTaskCallback {
  // Called when this task succeeded
  taskSucceeded(): void;
  // Called when this task failed
  taskFailed(): void;
  // Called when the driving session finishes a test or makes a decision
  taskDecision(reason: string): void;
  // This callback is unavailable if it is running with borrowing session mode
  // Called when a task session started, with its session id
  // When this session is created as a driving session, the isDrivingSession argument is true.
  taskSessionStarted(taskSession: ICopilotSession, taskId: string, isDrivingSession: boolean): void;
  // This callback is unavailable if it is running with borrowing session mode
  // Called when a task session stopped, with its session id
  // If the task succeeded, the succeeded argument is true
  taskSessionStopped(taskSession: ICopilotSession, taskId: string, succeeded: boolean): void;
}

async function startTask(
  taskName: string,
  userInput: string,
  drivingSession: ICopilotSession | undefined,
  ignorePrerequisiteCheck: boolean,
  callback: ICopilotTaskCallback,
  taskModelIdOverride: string | undefined,
  workingDirectory: string | undefined,
  exceptionHandler: (err: any) => void
): Promise<ICopilotTask>
```
- Start a task.
- Throw an error if `installJobsEntry` has not been called.
- When `drivingSession` is defined:
  - the task is in borrowing session mode
- When `drivingSession` is not defined:
  - `startTask` needs to create and maintain all sessions it needs.
- The `exceptionHandler` is called if the task execution throws an unhandled exception.

## API (taskApi.ts) ---------------------------------------------------------------------------------------------------------------------------------

**TEST-NOTE-BEGIN**
`yarn launch-for-test` will be used for unit testing therefore you have a chance to specify your own entry file.
DO NOT use the default entry for testing.

The free model "gpt-5.1-mini" must be used in every `Entry.models`.
If the model is no longer available, choose a new one and update both spec and custom entry JSON files.
**TEST-NOTE-END**

### copilot/test/installJobsEntry

Only available when `src/index.ts` is launched with `--test`.
The body will be an absolute path of a custom JSON file for entry.
The API will first check if the JSON file is in the `test` folder.
It reads the JSON file, called `validateEntry` followed by `installJobsEntry`.

Return in this schema:
```typescript
{
  result: "OK" | "InvalidatePath" | "InvalidateEntry" | "Rejected",
  error?: string
}
```

Return "InvalidatePath" when the file is not in the `test` folder.
Return "InvalidateEntry" when `validateEntry` throws.
Return "Rejected" when `installJobsEntry` throws.
"error" stores the exception message.

### copilot/task

**Referenced by**:
- Index.md: `#### Request Part`

List all tasks passed to `installJobsEntry` in this schema:
```typescript
{
  tasks: {
    name: string;
    requireUserInput: boolean;
  }[]
}
```

### copilot/task/start/{task-name}/session/{session-id}

**Referenced by**:
- Index.md: `#### Request Part`

**TEST-NOTE-BEGIN**
Besides of testing API failures, it is important to make sure task running works.

Create test cases for running a task, focused on the `criteria` stuff.
Test every fields in `criteria` and ensure:
- If the test succeeded, retry won't happen.
- If the task failed, retry happens.
- Retry has a budget limit.
- Whenever the task succeeded or failed, the live api responses correctly.
- Retrying should be observable from the live api.

Skip testing against session crashes scenarios because it is difficult to make it crash.

It is able to make up a failed test by:
- Does nothing
- In `criteria` specify `toolExecuted`, since the task does nothing, this will never satisfies.
- Set a retry budget limit to 0 so no retrying should happen.
- Therefore it fails because of not being able to pass the criteria check.
**TEST-NOTE-END**

The body will be user input.

Start a new task and return in this schema.
borrowing session mode is forced with an existing session id.
Prerequisite checking is skipped.

After the task finishes, it stops automatically, the task id will be unavailable immediately.
Keep the session alive.

```typescript
{
  taskId: string;
}
```

or when error happens:

```typescript
{
  error: "SessionNotFound"
}
```

### copilot/task/{task-id}/stop

The API will ignore the action and return `TaskCannotClose` if the task is started with borrowing session mode.

A task will automatically stops when finishes,
this api forced the task to stop.

Stop the task and return in this schema.

```typescript
{
  result: "Closed"
}
```

or when error happens:

```typescript
{
  error: "TaskNotFound" | "TaskCannotClose"
}
```

### copilot/task/{task-id}/live

**Referenced by**:
- Jobs.md: `### jobTracking.html`

It works like `copilot/session/{session-id}/live` but it reacts to `ICopilotTaskCallback`.
They should be implemented in the same way, but only respond in schemas mentioned below.

Returns in this schema if any error happens

```typescript
{
  error: "TaskNotFound" | "TaskClosed" | "HttpRequestTimeout" | "ParallelCallNotSupported"
}
```

Special `TaskNotFound` and `TaskClosed` handling for this API:
- Works in the same way as `SessionNotFound` and `SessionClosed` in `copilot/session/{session-id}/live`. 

**TEST-NOTE-BEGIN**
Can't trigger "HttpRequestTimeout" stably in unit test so it is not covered.
It requires the underlying copilot agent to not generate any response for 5 seconds,
which is almost impossible.
**TEST-NOTE-END**

Returns in this schema if an exception it thrown from inside the session

```typescript
{
  taskError: string
}
```

Other response maps to all methods in `ICopilotTaskCallback` in `src/taskApi.ts`.

When a task is created by a job's `executeWork`, the `taskSessionStarted` response additionally includes `sessionId` (string) and `isDriving` (boolean) fields so the frontend can poll `copilot/session/{session-id}/live` and distinguish between driving and task sessions.

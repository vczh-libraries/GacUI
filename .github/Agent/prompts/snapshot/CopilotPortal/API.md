# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `src/copilotSession.ts`
- `src/sharedApi.ts`
- `src/copilotApi.ts`
- `src/taskApi.ts`
- `src/jobsApi.ts`
- `src/index.ts`

Data structures about jobs and tasks are in `src/jobsDef.ts`.
It's spec is in `JobsData.md`.

## Starting the HTTP Server

**Referenced by**:
- API.md: `### copilot/test/installJobsEntry`

- This package starts an http server, serving a website as well as a set of RESTful API.
- In src/index.ts it accepts command line options like this:
  - `--port 8888`: Specify the http server port. `8888` by default.
  - `--test`: `installJobsEntry` is not called, an extra API `copilot/test/installJobsEntry` is available only in this mode.
- Website entry is http://localhost:port
- API entry is http://localhost:port/api/...
- "yarn portal" to run src/index.ts.
- "yarn portal-for-test" to run src/index.ts in test model

It starts both Website and RESTful API. Awaits for api/stop to stops.
Prints the following URL for shortcut:
- http://localhost:port
- http://localhost:port/api/stop

## Running the Website

- http://localhost:port is equivalent to http://localhost:port/index.html.
- In the assets folder there stores all files for the website.
- Requesting for http://localhost:port/index.html returns assets/index.html.

## Helpers (copilotApi.ts)

All helper functions and types are exported and API implementations should use them.

### helperGetModels

**Referenced by**:
- API.md: `### copilot/models`

`async helperGetModels(): Promise<ModelInfo[]>;`
- List all models.

### helperSessionStart

**Referenced by**:
- API.md: `### copilot/session/start/{model-id}`

`async helperSessionStart(modelId: string, workingDirectory?: string): Promise<[ICopilotSession, string]>;`
- Start a session, return the session object and its id.

### helperSessionStop

**Referenced by**:
- API.md: `### copilot/session/{session-id}/stop`, `### stop`

`async helperSessionStop(session: ICopilotSession): Promise<void>;`
- Stop a session.

### helperGetSession

**Referenced by**:
- API.md: `### copilot/session/{session-id}/stop`, `### copilot/session/{session-id}/query`, `### copilot/session/{session-id}/live`

`helperGetSession(sessionId: string): ICopilotSession | undefined;`
- Get a session by its id.

### helperPushSessionResponse

**Referenced by**:
- API.md: `### copilot/session/{session-id}/live`

`helperPushSessionResponse(session: ICopilotSession, response: LiveResponse): void;`
- Push a response to a session's response queue.

### hasRunningSessions

**Referenced by**:
- API.md: `### stop`

`hasRunningSessions(): boolean;`
- Returns true if any sessions are currently running.

## Helpers (copilotSession.ts)

Wraps `@github/copilot-sdk` to provide a simplified session interface with event callbacks.

### ICopilotSession

**Referenced by**:
- API.md: `### helperSessionStart`, `### helperSessionStop`, `### helperGetSession`, `### helperPushSessionResponse`, `### startSession`

```typescript
interface ICopilotSession {
  get rawSection(): CopilotSession;
  sendRequest(message: string, timeout?: number): Promise<void>;
  stop(): Promise<void>
}
```
- Interface for interacting with a Copilot session.
- When a session is no longer using, `stop` must be called. `CopilotSession.destroy` should be called by `stop`.

### ICopilotSessionCallbacks

**Referenced by**:
- API.md: `### copilot/session/{session-id}/live`, `### startSession`
- Index.md: `#### Session Interaction`, `#### Request Part`
- Shared.md: `#### Session Response Rendering`

```typescript
interface ICopilotSessionCallbacks {
  onStartReasoning(reasoningId: string): void;
  onReasoning(reasoningId: string, delta: string): void;
  onEndReasoning(reasoningId: string, completeContent: string): void;
  onStartMessage(messageId: string): void;
  onMessage(messageId: string, delta: string): void;
  onEndMessage(messageId: string, completeContent: string): void;
  onStartToolExecution(toolCallId: string, parentToolCallId: string | undefined, toolName: string, toolArguments: string): void;
  onToolExecution(toolCallId: string, delta: string): void;
  onEndToolExecution(toolCallId: string, result: { content: string; detailedContent?: string } | undefined, error: { message: string; code?: string } | undefined): void;
  onAgentStart(turnId: string): void;
  onAgentEnd(turnId: string): void;
  onIdle(): void;
}
```
- Callback interface for all session events.

### startSession

**Referenced by**:
- API.md: `### copilot/session/start/{model-id}`

`async startSession(client: CopilotClient, modelId: string, callback: ICopilotSessionCallbacks, workingDirectory?: string): Promise<ICopilotSession>;`
- Create a session with the given model, register job tools, wire up all event handlers, and return an `ICopilotSession`.

## Helpers (taskApi.ts)

All helper functions and types are exported and API implementations should use them.

### SESSION_CRASH_PREFIX

**Referenced by**:
- JobsData.md: `## Running Tasks`

`const SESSION_CRASH_PREFIX = "The session crashed, please redo and here is the last request:\n";`
- Internal constant (not exported). Prefix added to prompts when resending after a session crash.

### sendMonitoredPrompt (crash retry)

**Referenced by**:
- API.md: `### copilot/test/installJobsEntry`, `### copilot/task/start/{task-name}/session/{session-id}`, `### copilot/task/{task-id}/stop`, `### copilot/task/{task-id}/live`, `### copilot/job/start/{job-name}`, `### copilot/job/{job-id}/stop`, `### copilot/job/{job-id}/live`
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

## Helpers (jobsApi.ts)

```typescript
interface ICopilotJob {
  get runningWorkIds(): number[];
  get status(): "Executing" | "Succeeded" | "Failed";
  // stop all running tasks, no further callback issues.
  stop(): void;
}

interface ICopilotJobCallback {
  // Called when this job succeeded
  jobSucceeded(): void;
  // Called when this job failed
  jobFailed(): void;
  // Called when a TaskWork started, taskId is the registered task for live polling
  workStarted(workId: number, taskId: string): void;
  // Called when a TaskWork stopped
  workStopped(workId: number, succeeded: boolean): void;
}

async function startJob(
  jobName: string,
  userInput: string,
  workingDirectory: string,
  callback: ICopilotJobCallback
): Promise<ICopilotJob>
```

## API (copilotApi.ts)

All restful read arguments from the path and returns a JSON document.

All title names below represents http://localhost:port/api/TITLE

Copilot hosting is implemented by `@github/copilot-sdk` and `src/copilotSession.ts`.

### config

**Referenced by**:
- Index.md: `#### Starting an Copilot Session`

Returns the repo root path (detected by walking up from the server's directory until a `.git` folder is found).

```typescript
{
  repoRoot: string;
}
```

### test

**Referenced by**:
- Test.md: `### test.html`

Returns `{"message":"Hello, world!"}`

### stop

**Referenced by**:
- Index.md: `#### Request Part`
- Jobs.md: `### Matrix Part`

Stop any running sessions.
Returns `{}` and stops.

### copilot/models

**Referenced by**:
- Index.md: `#### Starting an Copilot Session`

Returns all copilot sdk supported models in this schema

```typescript
{
  models: {
    name: string;
    id: string;
    multiplier: number;
  }[]
}
```

### copilot/session/start/{model-id}

**Referenced by**:
- Index.md: `##### Start Button`

The body will be an absolute path for working directory

Start a new copilot session and return in this schema

```typescript
{
  sessionId: string;
}
```

or when error happens:

```typescript
{
  error: "ModelIdNotFound" | "WorkingDirectoryNotAbsolutePath" | "WorkingDirectoryNotExists"
}
```

Multiple sessions could be running parallelly, start a `CopilotClient` if it is not started yet, it shares between all sessions.
The `CopilotClient` will be closed when the server is shutting down.

### copilot/session/{session-id}/stop

**Referenced by**:
- Index.md: `#### Request Part`

Stop the session and return in this schema

```typescript
{result:"Closed"} | {error:"SessionNotFound"}
```

### copilot/session/{session-id}/query

**Referenced by**:
- Index.md: `#### Request Part`

The body will be the query prompt string.

Send the query to the session, and the session begins to work.

Returns in this schema

```typescript
{
  error?:"SessionNotFound"
}
```

### copilot/session/{session-id}/live

**Referenced by**:
- Index.md: `#### Session Interaction`
- Shared.md: `#### Session Response Rendering`
- API.md: `### copilot/task/{task-id}/live`, `### copilot/job/{job-id}/live`

This is a query to wait for one response back for this session.
Each session generates many responses, storing in a queue.
When the api comes, it pop one response and send back. Responses must be send back in its generating orders.
If there is no response, do not reply the API. If there is no response after 5 seconds, send back a time out error.
Be aware of that api requests and session responses could happen in any order.

This api does not support parallel calling on the same id.
If a call with a session-id is pending,
the second call with the same session-id should return an error.

Returns in this schema if any error happens

```typescript
{
  error: "SessionNotFound" | "SessionClosed" | "HttpRequestTimeout" | "ParallelCallNotSupported"
}
```

Special `SessionNotFound` and `SessionClosed` handling for this API:
- `SessionNotFound` returns when the `session-id` is never used.
- When a session is closed, but responses for this session is not drained by the API yet, the API still responses.
- When there is no more response and the session already stopped, it returns `SessionClosed`.
- After `SessionClosed`, the `session-id` will be no longer available for this api, future calls returns `SessionNotFound`.
- Even the client received notification that a session stops from another API, it is recommended to keep reading until `SessionClosed` returns.

**TEST-NOTE-BEGIN**
Can't trigger "HttpRequestTimeout" stably in unit test so it is not covered.
It requires the underlying copilot agent to not generate any response for 5 seconds,
which is almost impossible.
**TEST-NOTE-END**

Returns in this schema if an exception it thrown from inside the session

```typescript
{
  sessionError: string
}
```

Other response maps to all methods in `ICopilotSessionCallbacks` in `src/copilotSession.ts` in this schema

```typescript
{
  callback: string,
  argument1: ...,
  ...
}
```

For example, when `onReasoning(reasoningId: string, delta: string): void;` is called, it returns

```typescript
{
  callback: "onReasoning",
  reasoningId: string,
  delta: string
}
```

When running a task, any driving session generated prompts will be reported in this schema

```typescript
{
  callback: "onGeneratedUserPrompt",
  prompt: string
}
```

## API (jobsApi.ts)

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

It works likes `copilot/session/{session-id}/live` but it reacts to `ICopilotTaskCallback`.
They should be implemented in the same way, but only response in schemas mentioned below.

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

Other response maps to all methods in `ICopilotTaskCallback` in `src/jobsApi.ts`.

When a task is created by a job's `executeWork`, the `taskSessionStarted` response additionally includes `sessionId` (string) and `isDriving` (boolean) fields so the frontend can poll `copilot/session/{session-id}/live` and distinguish between driving and task sessions.

### copilot/job

**Referenced by**:
- Jobs.md: `### jobs.html`, `### Matrix Part`

List all jobs passed to `installJobsEntry` in this schema:
```typescript
{
  grid: GridRow[];
  jobs: { [key in string]: Job };
  chart: { [key in string]: ChartGraph };
}
```

Basically means it only keeps `grid` and `jobs` and drops all other fields, and calculate `chart` with `generateChartNodes`.

### copilot/job/start/{job-name}

**Referenced by**:
- Jobs.md: `#### Clicking Start Job Button`

**TEST-NOTE-BEGIN**
Besides of testing API failures, it is important to make sure job running works.

Create test cases for running a job, focused on different types of `Work`s.
Test every kinds of `Work` and ensure:
- It succeeds when all involved tasks succeed. Should test against every type of `Work`.
- If fails properly. Should test against every type of `Work`, in each possible failure opsition.
- Whenever the job succeeded or failed, the live api responses correctly.
- Execution of tasks in `Work` should be observable from the live api.

Skip testing against task crashes scenarios because it is difficult to make it crash.
**TEST-NOTE-END**

The first line will be an absolute path for working directory
The rest of the body will be user input.

Start a new job and return in this schema.

```typescript
{
  jobId: string;
}
```

or when error happens:

```typescript
{
  error: "JobNotFound"
}
```

### copilot/job/{job-id}/stop

**Referenced by**:
- Jobs.md: `### Job Part`

A job will automatically stops when finishes,
this api forced the job to stop.

Stop the job and return in this schema.

```typescript
{
  result: "Closed"
}
```

or when error happens:

```typescript
{
  error: "JobNotFound"
}
```

### copilot/job/{job-id}/live

**Referenced by**:
- Jobs.md: `### Job Part`, `### Session Response Part`

It works likes `copilot/session/{session-id}/live` but it reacts to `ICopilotJobCallback`.
They should be implemented in the same way, but only response in schemas mentioned below.

Returns in this schema if any error happens

```typescript
{
  error: "JobNotFound" | "JobsClosed" | "HttpRequestTimeout" | "ParallelCallNotSupported"
}
```

Special `JobNotFound` and `JobsClosed` handling for this API:
- Works in the same way as `SessionNotFound` and `SessionClosed` in `copilot/session/{session-id}/live`. 

**TEST-NOTE-BEGIN**
Can't trigger "HttpRequestTimeout" stably in unit test so it is not covered.
It requires the underlying copilot agent to not generate any response for 5 seconds,
which is almost impossible.
**TEST-NOTE-END**

Returns in this schema if an exception it thrown from inside the session

```typescript
{
  jobError: string
}
```

Other response maps to all methods in `ICopilotJobCallback` in `src/jobsApi.ts`.

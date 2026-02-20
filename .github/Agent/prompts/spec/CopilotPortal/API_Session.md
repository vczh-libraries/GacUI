# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `src/copilotSession.ts`
- `src/sharedApi.ts`
- `src/copilotApi.ts`
- `src/index.ts`

Data structures about jobs and tasks are in `src/jobsDef.ts`.
Its spec is in `JobsData.md`.

If an api requires `Entry`:
- The first argument of the api helper function must be `entry: Entry`.
- `index.ts` will call `ensureInstalledEntry` for this argument.

**TEST-NOTE-BEGIN**
`yarn launch-for-test` will be used for unit testing therefore you have a chance to specify your own entry file.
DO NOT use the default entry for testing.

The free model "gpt-5-mini" must be used in every `Entry.models`.
If the model is no longer available, choose a new one and update both spec and custom entry JSON files.
**TEST-NOTE-END**

## Helpers (copilotApi.ts) --------------------------------------------------------------------------------------------------------------------------

All helper functions and types are exported and API implementations should use them.

### helperSessionStart

**Referenced by**:
- API_Session.md: `### copilot/session/start/{model-id}`

`async helperSessionStart(modelId: string, workingDirectory?: string): Promise<[ICopilotSession, string]>;`
- Start a session, return the session object and its id.

### helperSessionStop

**Referenced by**:
- API_Session.md: `### copilot/session/{session-id}/stop`
- API.md: `### stop`

`async helperSessionStop(session: ICopilotSession): Promise<void>;`
- Stop a session.

### helperGetSession

**Referenced by**:
- API_Session.md: `### copilot/session/{session-id}/stop`, `### copilot/session/{session-id}/query`, `### copilot/session/{session-id}/live`

`helperGetSession(sessionId: string): ICopilotSession | undefined;`
- Get a session by its id.

### helperPushSessionResponse

**Referenced by**:
- API_Session.md: `### copilot/session/{session-id}/live`

`helperPushSessionResponse(session: ICopilotSession, response: LiveResponse): void;`
- Push a response to a session's response queue.

### hasRunningSessions

**Referenced by**:
- API.md: `### stop`

`hasRunningSessions(): boolean;`
- Returns true if any sessions are currently running.

## Helpers (copilotSession.ts) ----------------------------------------------------------------------------------------------------------------------

Wraps `@github/copilot-sdk` to provide a simplified session interface with event callbacks.

### ICopilotSession

**Referenced by**:
- API_Session.md: `### helperSessionStart`, `### helperSessionStop`, `### helperGetSession`, `### helperPushSessionResponse`, `### startSession`

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
- API_Session.md: `### copilot/session/{session-id}/live`, `### startSession`
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
- API_Session.md: `### copilot/session/start/{model-id}`

`async startSession(client: CopilotClient, modelId: string, callback: ICopilotSessionCallbacks, workingDirectory?: string): Promise<ICopilotSession>;`
- Create a session with the given model, register job tools, wire up all event handlers, and return an `ICopilotSession`.

## API (copilotApi.ts) ------------------------------------------------------------------------------------------------------------------------------

All restful read arguments from the path and returns a JSON document.

All title names below represents http://localhost:port/api/TITLE

Copilot hosting is implemented by `@github/copilot-sdk` and `src/copilotSession.ts`.

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
- API_Task.md: `### copilot/task/{task-id}/live`
- API_Job.md: `### copilot/job/{job-id}/live`
- Jobs.md: `### jobTracking.html`

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

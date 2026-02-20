# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `src/sharedApi.ts`
- `src/jobsApi.ts`
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

## Helpers (jobsApi.ts) -----------------------------------------------------------------------------------------------------------------------------

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
  entry: Entry,
  jobName: string,
  userInput: string,
  workingDirectory: string,
  callback: ICopilotJobCallback
): Promise<ICopilotJob>
```

## API (jobsApi.ts) ---------------------------------------------------------------------------------------------------------------------------------

### copilot/job

**Referenced by**:
- Jobs.md: `### jobs.html`, `### Matrix Part`
- Jobs.md: `### jobTracking.html`

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
- If fails properly. Should test against every type of `Work`, in each possible failure position.
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
- Jobs.md: `### Job Part`, `### Session Response Part`, `### jobTracking.html`

It works like `copilot/session/{session-id}/live` but it reacts to `ICopilotJobCallback`.
They should be implemented in the same way, but only respond in schemas mentioned below.

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

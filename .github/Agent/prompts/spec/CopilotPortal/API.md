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
Its spec is in `JobsData.md`.

## Starting the HTTP Server

**Referenced by**:
- API_Task.md: `### copilot/test/installJobsEntry`

- This package starts an http server, serving a website as well as a set of RESTful API.
- In src/index.ts it accepts command line options like this:
  - `--port 8888`: Specify the http server port. `8888` by default.
  - `--test`: `installJobsEntry` is not called, an extra API `copilot/test/installJobsEntry` is available only in this mode.
- Website entry is http://localhost:port
- API entry is http://localhost:port/api/...
- "yarn portal" to run src/index.ts.
- "yarn portal-for-test" to run src/index.ts in test mode

It starts both Website and RESTful API. Awaits for api/stop to stop.
Prints the following URL for shortcut:
- http://localhost:port
- http://localhost:port/api/stop

## Running the Website

- http://localhost:port is equivalent to http://localhost:port/index.html.
- In the assets folder there stores all files for the website.
- Requesting for http://localhost:port/index.html returns assets/index.html.

## Helpers (index.ts) --------------------------------------------------------------------------------------------------------------------------

### installJobsEntry

`async installJobsEntry(entry: Entry): Promise<void>;`
- Use the entry. It could be `entry` from `jobsData.ts` or whatever.
- This function can only be called when no session is running, otherwise throws.

### ensureInstalledEntry

**Referenced by**:
- API_Task.md: `### copilot/task/start/{task-name}/session/{session-id}`
- API_Job.md: `copilot/job/start/{job-name}`

`ensureInstalledEntry(): Entry`
- Throw an error if `installJobsEntry` has not been called.
- Return the installed entry.

## Helpers (copilotApi.ts) --------------------------------------------------------------------------------------------------------------------------

All helper functions and types are exported and API implementations should use them.

### helperGetModels

**Referenced by**:
- API.md: `### copilot/models`

`async helperGetModels(): Promise<ModelInfo[]>`
- List all models.

## API (copilotApi.ts) ------------------------------------------------------------------------------------------------------------------------------

All restful read arguments from the path and returns a JSON document.

All title names below represents http://localhost:port/api/TITLE

Copilot hosting is implemented by `@github/copilot-sdk` and `src/copilotSession.ts`.

### config

**Referenced by**:
- Index.md: `#### Starting a Copilot Session`

Returns the repo root path (detected by walking up from the server's directory until a `.git` folder is found).

```typescript
{
  repoRoot: string;
}
```

### stop

**Referenced by**:
- Index.md: `#### Request Part`
- Jobs.md: `### Matrix Part`

Stop any running sessions.
Returns `{}` and stops.

### copilot/models

**Referenced by**:
- Index.md: `#### Starting a Copilot Session`

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

## API (index.ts) ------------------------------------------------------------------------------------------------------------------------------

### test

**Referenced by**:
- Test.md: `### test.html`

Returns `{"message":"Hello, world!"}`

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

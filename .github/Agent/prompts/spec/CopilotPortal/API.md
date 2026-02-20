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

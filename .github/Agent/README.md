# Copilot Agent

A TypeScript-based application for interacting with GitHub Copilot models, featuring a web-based portal.

## Prerequisites

- [GitHub Copilot CLI](https://docs.github.com/en/copilot/how-tos/set-up/install-copilot-cli) must be installed and authenticated
- Node.js 18+ and Yarn

## Setup

From the `.github/Agent` directory, run:

```bash
yarn install
```

## Build

To build and test the application:

```bash
yarn build
```

This runs three steps in sequence:
1. `yarn compile` — compiles all TypeScript packages
2. `yarn testStart` — starts the portal server in the background
3. `yarn testExecute` — runs all API and Playwright tests, then stops the server

The server is always stopped via `api/stop` after tests, regardless of pass/fail.

To compile only (no tests):

```bash
yarn compile
```

## Usage

### Portal (Web UI)

```bash
yarn portal
```

Starts an HTTP server at `http://localhost:8888` serving a web UI and RESTful API.

- Open `http://localhost:8888` to launch the portal.
- Select a model and working directory, then click Start.
- Send requests via the text box (Ctrl+Enter or Send button).
- Session responses (reasoning, tool calls, messages) appear as collapsible message blocks.
- "Close Session" ends the session and closes the tab; "Stop Server" also shuts down the server.
- Use `http://localhost:8888/index.html?project=XXX` to default working directory to the sibling folder `XXX` next to the repo root.
- Without `project` parameter, working directory defaults to the repo root.

#### CLI Options

- `--port <number>` — Set the HTTP server port (default: `8888`)
- `--test` — Start in test mode (entry is not pre-installed; enables `copilot/test/installJobsEntry` API for test-driven entry loading)

### Portal for Test

```bash
yarn portal-for-test
```

Starts the portal in test mode (`--test` flag). In test mode, the jobs entry is not installed at startup, and the `copilot/test/installJobsEntry` API is available for tests to install entries dynamically.

## Specification Structure

There are two folders storing specification:
- `prompts/snapshot`: The specification that the project implemented, it reflects the current state.
- `prompts/spec`: The specification that the project need to be.

File organization in these two folders are identical:
- `CopilotPortal` folder: about `packages/CopilotPortal`
  - `JobsData.md`: Definitions of jobs data.
  - `API.md`: RESTful API, and how to start the project. More specific content in these files:
    - `API_Session.md`
    - `API_Task.md`
    - `API_Job.md`
  - `Index.md`: index.html page.
  - `Jobs.md`: jobs.html and jobTracking.html pages.
  - `Test.md`: test.html page.
  - `Shared.md`: Shared components between multiple web pages.

## Project Structure

```
.github/Agent/
├── package.json              # Workspace configuration (yarn workspaces)
├── tsconfig.json             # Base TypeScript configuration
├── packages/
│   ├── CopilotPortal/        # Web UI + RESTful API server
│   │   ├── src/
│   │   │   ├── copilotSession.ts # Copilot SDK session wrapper
│   │   │   ├── copilotApi.ts  # Copilot session API routes and helpers
│   │   │   ├── taskApi.ts     # Task execution engine (startTask, crash retry)
│   │   │   ├── jobsApi.ts     # Job/task API routes and job execution
│   │   │   ├── jobsDef.ts     # Jobs/tasks data definitions and validation
│   │   │   ├── jobsChart.ts   # Flow chart graph generation from work trees
│   │   │   ├── jobsData.ts    # Preloaded jobs/tasks data
│   │   │   ├── sharedApi.ts   # Shared HTTP/live-polling utilities
│   │   │   └── index.ts       # HTTP server, API routing, static files
│   │   ├── assets/           # Static website files
│   │   │   ├── index.html    # Main portal page
│   │   │   ├── index.js      # Portal JS (session interaction, live polling)
│   │   │   ├── index.css     # Portal styles
│   │   │   ├── jobs.html     # Jobs selection page
│   │   │   ├── jobs.js       # Jobs page JS (matrix rendering, job selection)
│   │   │   ├── jobs.css      # Jobs page styles
│   │   │   ├── jobTracking.html  # Job tracking page
│   │   │   ├── jobTracking.js    # Job tracking JS (logic + renderer dispatch)
│   │   │   ├── jobTracking.css   # Job tracking styles
│   │   │   ├── flowChartMermaid.js # Mermaid flow chart renderer
│   │   │   ├── messageBlock.js       # MessageBlock component
│   │   │   ├── messageBlock.css      # MessageBlock styles
│   │   │   ├── sessionResponse.js    # SessionResponseRenderer component
│   │   │   ├── sessionResponse.css   # SessionResponseRenderer styles
│   │   │   └── test.html     # Simple API test page
│   │   ├── test/             # Test files
│   │   │   ├── startServer.mjs       # Starts server in test mode for testing
│   │   │   ├── runTests.mjs          # Test runner (always stops server)
│   │   │   ├── testEntry.json        # Test entry with simple tasks/jobs for API tests
│   │   │   ├── jobsData.test.mjs     # Jobs data validation tests
│   │   │   ├── api.test.mjs          # RESTful API tests (incl. task/job execution)
│   │   │   ├── work.test.mjs         # Work tree execution tests
│   │   │   ├── web.test.mjs          # Playwright UI tests (test.html)
│   │   │   ├── web.index.mjs         # Playwright tests for index.html
│   │   │   ├── web.jobs.mjs          # Playwright tests for jobs.html and jobTracking.html
│   │   │   └── windowsHide.cjs       # Windows process hiding helper
│   │   └── package.json
```

## Maintaining the Project

- **Build**: `yarn build` compiles all packages and runs tests.
- **Compile only**: `yarn compile` compiles all packages via TypeScript.
- **Run portal**: `yarn portal` starts the web server (default port 8888).
- **Run portal in test mode**: `yarn portal-for-test` starts in test mode.
- **Run tests only**: `yarn testStart && yarn testExecute` starts server in test mode and runs tests.
- **Playwright**: Install with `npx playwright install chromium`. Used for testing the portal UI.
- **Spec-driven**: Portal features are defined in `prompts/spec/CopilotPortal/`.

## Features

- **Web Portal**: Browser-based UI for Copilot sessions with real-time streaming
- **Message Blocks**: User, Reasoning, Tool, and Message blocks with expand/collapse behavior
- **Markdown Rendering**: Completed message blocks (except Tool) render markdown content as formatted HTML using marked.js
- **Awaiting Status**: "Awaits responses ..." indicator shown in the session part while the agent is working
- **Lazy CopilotClient**: Client starts on demand and closes when the server shuts down
- **Multiple Sessions**: Supports parallel sessions sharing a single CopilotClient
- **Live Polling**: Sequential long-polling for real-time session callbacks
- **Task System**: Job/task execution engine with availability checks, criteria validation, and retry logic
- **Session Crash Retry**: `sendMonitoredPrompt` (private method on `CopilotTaskImpl`) automatically retries up to 5 times if a Copilot session crashes during prompt execution, creating new sessions when needed
- **Detailed Error Reporting**: `errorToDetailedString` helper converts errors to detailed JSON with name, message, stack, and recursive cause chain for comprehensive crash diagnostics
- **Jobs API**: RESTful API for listing, starting, stopping, and monitoring tasks and jobs via live polling
- **Live Polling Drain**: Live APIs (session/task/job) use a drain model — clients continue polling until receiving terminal `*Closed` or `*NotFound` errors, ensuring all buffered responses are consumed
- **Closed State Management**: Sessions, tasks, and jobs transition through a `closed` state that buffers remaining responses before cleanup, preventing lost data
- **Test Mode API**: `copilot/test/installJobsEntry` endpoint (test mode only) for dynamically installing job entries during testing
- **Job Workflow Engine**: Composable work tree execution supporting sequential, parallel, loop, and conditional (alt) work patterns
- **Task Selection UI**: Combo box in the portal to select and run tasks within an active session
- **Tool Registration**: Custom job tools (e.g. `job_boolean_true`, `job_prepare_document`) are automatically registered with Copilot sessions
- **Flow Chart Renderers**: Job tracking page uses Mermaid for declarative flowchart rendering
- **Job Status Tracking**: Live polling of job execution status with visual status bar (RUNNING/SUCCEEDED/FAILED/CANCELED) and Stop Job button
- **Flow Chart Status Indicators**: Running tasks display a green triangle indicator; succeeded tasks display a green tick indicator; failed tasks display a red cross indicator on the flow chart
- **Task Inspection**: Clicking a TaskNode in the flow chart opens a tab control showing session responses for that task's sessions
- **Job Preview Mode**: Job tracking page supports a preview mode (no jobId) showing the flow chart without tracking, with no Stop Job button and "JOB: PREVIEW" status
- **Job-Created Tasks**: Jobs create tasks in managed session mode; `startTask` manages driving session creation internally. Task live API provides real-time session updates with `sessionId` and `isDriving` fields
- **Task Decision Reporting**: `taskDecision` callback reports all driving session decisions with categorized prefixes (`[OPERATION]`, `[CRITERIA]`, `[AVAILABILITY]`, `[SESSION CRASHED]`, `[TASK SUCCEEDED]`, `[TASK FAILED]`, `[DECISION]`) as User message blocks in the driving session tab
- **Driving Session Consolidation**: All driving sessions for a task are consolidated into a single "Driving" tab; when a driving session is replaced (e.g., due to crash retry), the new session reuses the same tab and renderer
- **Borrowing Session Mode**: Tasks can run with an externally-provided session (borrowing mode); crashes in borrowing mode fail immediately without retry
- **Managed Session Mode**: Tasks in managed mode create their own sessions — single model mode reuses one session, multiple models mode creates ephemeral sessions per mission
- **Separated Retry Budgets**: Crash retries (per-call, 5 max) and criteria retries (per failure action) are independent; a crash during a criteria retry loop is treated as a failed iteration rather than killing the task

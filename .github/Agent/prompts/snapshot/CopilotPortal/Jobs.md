# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `assets`
  - `jobs.css`
  - `jobs.js`
  - `jobs.html`
  - `jobTracking.css`
  - `jobTracking.js`
  - `jobTracking.html`
  - `flowChartMermaid.js`

### jobs.css

Put jobs.html specific css file in jobs.css.

### jobs.js

Put jobs.html specific javascript file in jobs.js.

### jobs.html

This page should only be opened by `/index.html`'s "Jobs" button in order to obtain the working directory.
If it is directly loaded, it should redirect itself to `/index.html`.
To tell this, find the `wb` argument in the url passing from `/index.html`.

Call `api/copilot/job` to obtain jobs definition.
- `grid` defines the `matrix part` of the UI.
- `jobs` offers details for each job in order to render the tracking UI.

The webpage is splitted to two part:
- The left part is `matrix part`.
- The right part is `user input part`.
- The left and right part should always fill the whole webpage.
- Between two parts there is a bar to drag vertically to adjust the width of the right part which defaults to 800.

The look-and-feel must be similar to `/index.html`, but DO NOT share any css file.

### Matrix Part

It renders a larget table of buttons according to `grid`.
The first row is a title "Available Jobs", followed by a button "Stop Server" at the very right doing exactly what the button in `/index.html` does.
The first column shows `keyword`.
All other columns are for `grid[index].jobs`, `name` will be the text of the button.
`undefined` in any `grid[index].jobs` element renders an empty cell.

If any cell has no button, leave it blanks.
The table is supposed to fill all `matrix part` but leave margins to the border and between buttons.
The table is also rendered with light color lines.

Font size should be large enough to reduce blanks, prepare to fill about 1000x1000 or a even larger space. The complete content can be read in `jobsData.ts`, it helps to guess a font size as it will but rarely changes.

Besides automate buttons, other buttons renders as buttons but it works like radio buttons:
- Clicking a job button renders an exclusive selected state.
  - Its `jobName` (not the `name` in the button text) becomes `selectedJobName`.
  - The "Start Job" button is enabled.
  - Only when `requireUserInput` is true, the text box is enabled
- Clicking a selected job button unselect it
  - The "Start Job" button is disabled.
  - The text box is disabled.

#### Actions of Automate Button

(to be editing...)

### User Input Part

There is a text box fills the page. Disabled by default.

At the bottom there are buttons aligned to the right:
- "Start Job: ${selectedJobName}" or "Job Not Selected". Disabled by default.
- "Preview".

#### Clicking Start Job Button

When I hit the "Start Job" button, call `copilot/job/start/{job-name}`.
When a job id is returned, it jumpts to `/jobTracking.html` in a new window.
The selected job directory and the job id should be brought to `/jobTracking.html`.
No need to bring other information.

#### Clicking Preview Button

Sync "Preview"'s enability to "Start Job".
Call `/jobTracking.html` without starting the job and not passing the job id.
This triggers the preview mode.

(to be editing...)

### jobTracking.css

Put jobTracking.html specific css file in jobTracking.css.

### jobTracking.js

Put jobTracking.html specific javascript file in jobTracking.js.

### jobTracking.html

This page should only be opened by `/jobs.html`'s "Start Job" button in order to obtain the working directory.
If it is directly loaded, it should redirect itself to `/index.html`.
To tell this, find the `jobName` and `jobId` argument in the url passing from `/jobs.html`.

Call `api/copilot/job` to obtain the specified job's definition.
`jobs[jobName]` and `chart[jobName]` is used to render the flow chart.

#### When jobId argument presents

Call `api/copilot/job/{job-id}/live`, `api/copilot/task/{task-id}/live` and `copilot/session/{session-id}/live` to update the running status of the job:
- job live api notifies when a new task is started, task live api notifies when a new session is started.
- Drain all response from live apis, no more issue until `(Sessoin|Task|Job)(Closed|NotDefined)` returns.
- On receiving `ICopilotTaskCallback.taskDecision`
  - Create a "User" message block with `title` set to `TaskDecision`, copying the `reason`, put it in the driving session.

#### When jobId argument not presents

The webpage preview the job only, no tracking is performed.
The "Stop Job" button disappears.
The job status becomes "JOB: PREVIEW".
Clicking `ChartNode` does nothing.

#### Layout

The webpage is splitted to two part:
- The left part is `job part`.
- The right part is `session response part`.
- The left and right part should always fill the whole webpage.
- Between two parts there is a bar to drag vertically to adjust the width of the right part which defaults to 800.

The look-and-feel must be similar to `/index.html`, but DO NOT share any css file.

### Job Part

You can find the `Job` definition in `jobsDef.ts`.
`Job.work` is a simple control flow AST.
Render it like a flow chart expanding to fill the whole `job part`.#### Flow Chart Rendering Note

#### Tracking Job Status

When the status of a task running is changed,
update the rendering of a `ChartNode` of which `hint` is `TaskNode` or `CondNode`:
- If a symbol attaches to a node, it is at the center of the left border but outside.
  - Use emoji if possible.
  - If it is technically impossible to touch the left border you can choose another place.
- When a task begins running, attach a green triangle.
- When a task finishes running, the triangle is removed.
  - If it succeeds, a green tick replaces the triangle.
  - If it fails, a red cross replaces the triangle.
- There could be loops in the flow chart, which means a task could starts and stops multiple times.

When a task runs, track all status of sessions in this task:
- The driving session names `Driving`.
- Multiple occurances of the task session names beginning from `Attemp #1`.
  - If a task is being inspected, when a session starts, `session response part` should display the new tab header, but do not switch to the new tab.
  - Responses in a tab should keep updating if new data comes.
- Whenever the task is being inspected or not, status should keep recording. The user will inspect here and there, data should persist.
  - For convenience, whenever a session starts, a `Session Response Rendering` from `Shared.md` could be created and adding message blocks.
  - When the task is being inspected, display it, otherwise removes it from the DOM.
- When a task begins **AGAIN**, cached data will be deleted and replaced with status of the latest run.

The `Flow Chart Rendering` should keep centered horizontally and vertically.
At the very top of the `job part`:
- At the left there is a big label showing the job that:
  - `JOB: RUNNING`
  - `JOB: SUCCEEDED`
  - `JOB: FAILED`
- At the right there is a button in the same label size "Stop Job"
  - It is enabled when the job is running.
  - Clicking the button should call `api/copilot/job/{job-id}/stop` and the big label becomes `JOB: CANCELED`.

#### Flow Chart Rendering

**TEST-NOTE-BEGIN**
No need to create unit test to assert the chart is in a correct layout.
Ensure every `TaskWork` has a `ChartNode` with `TaskNode` or `CondNode` hint.
**TEST-NOTE-END**

The `api/copilot/job` has an extra `chart` node which is a `ChartGraph`.
It is already a generated flow chart but has no layout information.

Each `ChartNode` is a node in the flow chart, and each hint maps to a graph:
- `TaskNode`: A blue rectangle with the task id, the text would be the `TaskWork` with that `workIdInJob`.
- `CondNode`: A yellow hexagon with the task id, the text would be the `TaskWork` with that `workIdInJob`.
- `ParBegin`, `ParEnd`: A small black rectangle bar.
- `AltEnd`: A small pink rectangle bar.
- `CondBegin`: A small yellow rectangle bar.
- `CondEnd`: A small yellow diamind.
- `LoopEnd`: A small gray circle.

For `TaskNode` and `CondNode`:
  - The text would be the `TaskWork` with that `workIdInJob`.
  - If `modelOverride` is specified, render it like `task (model)`.

Each graph must have a border, and the background color would be lighter, except the black rectangle bar which has the same border and background color.

There will be multiple arrows connecting between nodes:
- `ChartArrow.id` is the target `ChartNode` with that `id`.
- When `ChartArrow.loopBack` is true, it hints a arrow pointing upwards. All others should be downwards.
- `ChartArrow.label` is the label attached to the arrow.

Arrows would be gray.

#### Rendering with Mermaid

Implementation stores in:
- `flowChartMermaid.js`

No separate CSS file is needed; node styles are embedded as inline Mermaid `style` directives in the generated definition.

Use [Mermaid.js](https://mermaid.js.org/) (loaded from CDN) for declarative flowchart rendering:
- Initialize Mermaid with `startOnLoad: false` so rendering is controlled programmatically.
- Build a Mermaid `graph TD` definition string from the `ChartGraph`.
- Each `ChartNode` becomes a Mermaid node with shape syntax matching its hint (rectangles, hexagons `{{}}`, circles `(())`).
- Each `ChartArrow` becomes a Mermaid edge with optional label.
- Per-node inline `style` directives set fill, stroke, and text color.
- Call `mermaid.render("mermaid-chart", definition)` to produce an SVG, then insert it into the container.
- The SVG viewBox must have enough padding (at least 24px) so that status indicator emojis on leftmost nodes are not clipped.

#### Ctrl+Scroll Zoom

When the user holds **Ctrl** and scrolls the mouse wheel over the `#chart-container`:
- The flow chart SVG scales up (scroll up) or down (scroll down) in discrete steps of 0.1.
- Zoom range is clamped between 0.2x and 3x, defaulting to 1x.
- CSS `transform: scale(...)` with `transform-origin: top left` is applied to the SVG element.
- The default browser scroll/zoom behavior is suppressed (`preventDefault`).

#### Interaction with `ChartNode` which has a `TaskNode` or `CondNode` hint

Clicking it select (exclusive) or unselect the text:
- When it becomes selected, the task is being inspected, `session response part` should display this task.
- When it becomes unselected, the task is not being inspected. `session response part` should restore.
- The border of the node becomes obviously thicker when selected.

### Session Response Part

When no task is being inspected, print `JSON.stringify(jobToRender and chartToRender, undefined, 4)` inside a `<pre>` element.

When a task is being inspected:
- It becomes a tab control.
- Each tab is a session, tab headers are names of sessions.
  - The first tab will always be `Driving` and all driving sessions come to here.
  - Each task session has its own tab.
- Clicking a shows responses from a session using `Session Response Rendering` from `Shared.md`.

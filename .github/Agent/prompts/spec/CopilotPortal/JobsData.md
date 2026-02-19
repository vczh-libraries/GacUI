# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `src/jobsDef.ts`
- `src/jobsChart.ts`
- `src/jobsData.ts`
- `src/jobsApi.ts`

## Functions

### expandPromptStatic

**Referenced by**:
- JobsData.md: `### expandPromptDynamic`, `### validateEntry`

A function convert from `Prompt` to `Prompt` with only one string.
`Prompt` is a string array, the function should join them with LF character.
In each string there might be variables.
An variable is $ followed by one or multiple words connected with hypens.
When an variable is in `entry.promptVariables`, replace the variable with its values.
When an variable is in `runtimeVariables`, keep them.
When it fails to look up the value, report an error `${codePath}: Cannot find prompt variable: ${variableName}.`.
Be aware of that, the value is still a `Prompt`, so it is recursive.

When calls `expandPromptStatic` recursively for a resolved prompt variable,
the `codePath` becomes `${codePath}/$${variableName}`.

Report an error if `prompt` is empty.

### expandPromptDynamic

**Referenced by**:
- JobsData.md: `## Running Tasks`

It works like `expandPromptStatic`, but assert `prompt` has exactly one item.
Look up all `runtimeVariables` in `values` argument.
Be aware of that, not all variable has an value assigned.
When it fails to look up the value, use `<MISSING>` as its value.

### validateEntry

**Referenced by**:
- API.md: `### copilot/test/installJobsEntry`, `### sendMonitoredPrompt (crash retry)`

Perform all verifications, verify and update all prompts with `expandPromptStatic`:
- entry.tasks[name].prompt
- entry.tasks[name].availability.condition (run extra verification)
- entry.tasks[name].criteria.condition (run extra verification)
- entry.tasks[name].criteria.failureAction.additionalPrompt

When extra verification is needed,
`expandPromptStatic`'s `requiresBooleanTool` will be set to true,
it verifies that `job_boolean_true` or `job_boolean_false` must be mentioned in the expanded prompt.

Here are all checks that `validateEntry` needs to do:
- `entry.models.driving`:
  - Should exist.
- `entry.grid[rowIndex].jobs[columnIndex].jobName`:
  - Must be in keys of `entry.jobs`.
- `entry.tasks[name].model.category`;
  - Must be in fields of `entry.models`.
- `entry.tasks[name].requireUserInput`:
  - If it is true, its evaluated `prompt` should use `$user-input`, otherwise should not use.
- `entry.tasks[name].availability.previousTasks[index]`:
  - Must be in keys of `entry.tasks`.
- `entry.tasks[name].criteria.toolExecuted[index]`:
  - Must be in `availableTools`.
- Any `TaskWork` in `entry.jobs[name]`, needs to inspect recursively:
  - `TaskWork.taskId` must be in `entry.tasks`.
  - `TaskWork.modelOverride.category` must be in fields of `entry.models`.
  - `TaskWork.modelOverride` must be defined if that task has no specified model.
- Any `SequencialWork` and `ParallelWork`:
  - `works` should have at least one element.
- `entry.jobs[name].requireUserInput`.
  - Find out if a job requires user input by inspecting all `Task` record referenced by any `TaskWork` in this job.
  - If any task `requireUserInput`, then the job `requireUserInput`, otherwise the job does not `requireUserInput`.
  - If `Job.requireUserInput` is defined, it should reflect the correct value.
  - If `requireUserInput` is undefined, fill it.
  - After `validateEntry` finishes, all `Job.requireUserInput` should be filled.
- `entry.jobs[name].work`:
  - Simplies the `work` tree:
    - Expand nested `SequencialWork`, that a `SequencialWork` directly in another `SequencialWork` should be flattened. They could be multiple level.
    - Expand nested `ParallelWork`, that a `ParallelWork` directly in another `ParallelWork` should be flattened. They could be multiple level.

If any validation runs directly in this function fails:
- The error code path will be `codePath` appended by the javascript expression without any variable.
  - For example, if `entry.tasks[name].model` fails with task `scrum-problem-task`, the error code path becomes `${codePath}entry.tasks["scrum-problem-task"].model`.
- It must throws an error like "${errorCodePath}: REASON".
- The functions must also use the error code path for any `Prompt` expression to call `expandPromptStatic`.

## Running Tasks

**Referenced by**:
- API.md: `### sendMonitoredPrompt (crash retry)`
- JobsData.md: `### TaskWork`

A task is represented by type `Task`.

A task can be running with `borrowing session mode` or `managed session mode`.

### Borrowing Session Mode

Every prompt will be running in the given session.
If the session crashes, fail the task immediately.

### Managed Session Mode

Before starting a task, it needs to decide if the task is running in single model or multiple models.
Single model option will be enabled when one of the following conditions satisfies:
- `Task.criteria` is undefined.
- `Task.criteria.runConditionInSameSession` is undefined or it is true.

Every session is created and managed by the task.
No matter single model or multiple models is selected:
- If a session crashes, new session must be created to replace it.
- If executing the same prompt results in 5 consecutive crashes, fail the task immediately.
- Add `SESSION_CRASH_PREFIX` (const from `taskApi.ts`: `"The session crashed, please redo and here is the last request:\n"`) before the prompt when resend.
- The crash retry logic is implemented in the private `sendMonitoredPrompt` method on `CopilotTaskImpl` in `taskApi.ts`.
- The exception cannot be consumed silently, and every exception should be reported by `ICopilotTaskCallback.taskDecision`.

#### Managed Session Mode (single model)

A session will be created, and it serves both a driving session and a task session.
`Entry.models[Task.model]` will be used. When `Task.model` is undefined, the model id should be assigned from `startTask`.

#### Managed Session Mode (multiple models)

Session are created only when needed, and it is closed when its mission finishes.

The mission of a driving session is:
- Perform one `Task.availability.condition` check.
- Perform one `Task.criteria.condition` check.
- Finishing the check closes the driving session

The mission of a task session is:
- Perform one `Task.prompt`.
- Finishing one `Task.prompt` closes the task session

The driving session uses `Entry.models.driving`.
The task session uses `Entry.models[Task.model]`. When `Task.model` is undefined, the model id should be assigned.

#### Preprocess Prompt

Before sending any prompt to the driving or task session,
`expandPromptDynamic` will be called to apply runtime variables.
This function must be called every time a prompt is sent to the session,
because runtime variables could change.

### Tools and Runtime Variables

**Referenced by**:
- JobsData.md: `### expandPromptDynamic`, `### Task.availability`, `### Task.criteria`, `### validateEntry`

`$user-input` will be passed from the user directly.
`$task-model` will be the model name selected to run the task session. It is not the category, it is the actual model name.

The following tools could be called in the driving or task session.
- When `job_prepare_document` is called, its argument becomes `$reported-document`. If there is multiple line, only keep the first line and trim and space characters before and after it.
- When the `job_boolean_true` tool is called, the condition satisfies.
  - The argument will be assigned to the `$reported-true-reason` runtime variable.
  - `$reported-false-reason` will be deleted.
- When the `job_boolean_false` tool is called, the condition fails.
  - The argument will be assigned to the `$reported-false-reason` runtime variable.
  - `$reported-true-reason` will be deleted.

### Task.availability

**Referenced by**:
- JobsData.md: `### TaskWork`, `### validateEntry`
- API.md: `### copilot/task/start/{task-name}/session/{session-id}`

If `Task.availability` is not defined,
there will be no prerequisite checking,
the task just run.

All conditions must satisfy at the same time to run the task:
- When `Task.availability.previousTasks` is defined, the previous task's name must be in the list.
- When `Task.availability.condition` is defined, the condition must satisfy.
  - The driving session will run the prompt.
  - The condition satisfies when the `job_boolean_true` is called in this round of driving session response.

otherwise the `job_prerequisite_failed` tool will be called in the driving session,
indicating the task fails.

### Task.criteria

**Referenced by**:
- JobsData.md: `### TaskWork`, `### validateEntry`
- API.md: `### copilot/task/start/{task-name}/session/{session-id}`

If `Task.criteria` is not defined,
there will be no criteria checking,
the task is treat as succeeded.

All conditions must be satisfy to indicate that the task succeeded:
- When `Task.criteria.toolExecuted` is defined, all tools in the list should have been executed in the last round of task session response.
  - When retrying the task due to `toolExecuted`, append `## Required Tool Not Called: {tool names ...}`.
- When `Task.criteria.condition` is defined:
  - The driving session will run the prompt.
  - The condition satisfies when the `job_boolean_true` is called in this round of driving session response.

The task should react to `Task.criteria.failureAction` when task execution does not satisfy the condition:
- Retry at most `retryTimes` times.
- Send the original prompt, extra prompt is appended if:
  - The criteria test fails due to `toolExecuted`: append `## Required Tool Not Called: {tool names ...}`.
  - `additionalPrompt` defines: append `"## You accidentally Stopped"` followed by the `additionalPrompt`.

### Calling ICopilotTaskCallback.taskDecision

In above sessions there are a lot of thing happenes in the driving session. A reason should be provided to `taskDecision`, including but not limited to:
- The availability test passed.
- The availability test failed with details.
- The criteria condition test passed.
- The criteria condition test failed with details.
- Starting a retry with retry number.
- Retry budget drained because of availability or criteria failure.
- Retry budget drained because of crashing.
  - These two budgets are separated: crash retries are per-call (5 max in `sendMonitoredPrompt`), criteria retries are per failure action loop. A crash exhausting its per-call budget during a criteria retry loop is treated as a failed iteration rather than killing the task.
- Any error generated in the copilot session.
- A final decision about the task succeeded or failed.

Information passing to `taskDecision` should include the following prefix in order to better categories:
- `[SESSION CRASHED]` with detailed information from the exception.
- `[TASK SUCCEEDED]`
- `[TASK FAILED]`
- `[AVAILABILITY]`
- `[CRITERIA]`
- `[OPERATION]`
- `[DECISION]`
- `[MISC]`

For `availability` and `criteria`, the information should at least contain:
- The task does not match which field and what is the content of the field (field in `availability` and `criteria` properties)
- Why it does not match

## Running Jobs

**Referenced by**:
- API.md: `### sendMonitoredPrompt (crash retry)`

A `Job` is workflow of multiple `Task`. If its work fails, the job fails.

### Work

**Referenced by**:
- JobsData.md: `### TaskWork`, `### Determine TaskWork.workId`
- API.md: `### copilot/job/start/{job-name}`

- `TaskWork`: run the task, if `modelOverride` is defined that model is used.
  - If `category` is defined, the model id is `entry.models[category]`.
  - Otherwise `id` is the model id.
- `SequentialWork`, run each work sequentially, any failing work immediately fails the `SequentialWork` without continuation.
  - Empty `works` makes `SequentialWork` succeeds.
- `ParallelWork`, run all works parallelly, any failing work fails the `ParallelWork`, but it needs to wait until all works finishes.
  - Empty `works` makes `ParallelWork` succeeds.
- `LoopWork`:
  - Before running `body`, if `preCondition` succeeds (first element is true) or fails (first element is false), run `body`, otherwise `LoopWork` finishes as succeeded.
  - After running `body`, if `postCondition` succeeds (first element is true) or fails (first element is false), redo `LoopWork`, otherwise `LoopWork` finishes as succeeded.
  - If `body` fails, `LoopWork` finishes and fail.
- `AltWork`:
  - If `condition` succeeds, choose `trueWork`, otherwise choose `falseWork`.
  - If the chosen work is undefined, `AltWork` finishes as succeeded.
  - If the chosen work succeeds, `AltWork` finishes as succeeded.

**TEST-NOTE-BEGIN**
Need individual test cases for each type of `Work` in `work.test.mjs`, verifying details of each statement in the above bullet-point.
Such test case could be implemented by making up a job and calls `api/copilot/job/start/{job-name}` to start a work.
You can firstly obtain the updated work by calling `api/copilot/job`, find your target job, `workIdForJob` should have been attached to each `TaskWork`.
By calling the `api/copilot/job/{job-id}/live` api, you are able to see the starting and ending order of each `TaskWork`, by their own `workIdForJob`.

With such information, you can verify:
- If the job succeeded or failed as expected.
- If each `TaskWork` actually triggered in the expected order or logic.
  - For `AltWork`, only one of `trueWork` or `falseWork` triggers.
  - For `ParallelWork`, all `works` should trigger but the order may vary.
- Any task could fail, assert its side effect on the control flow.
  - For example, if `AltWork.condition` succeeds but a defined `trueWork` does not happen, there should be problems.
  - You need to check all possible equivalence classes of execution paths according to the control flow.

More details for api and additional test notes could be found in `API.md`.
**TEST-NOTE-END**

### TaskWork

**Referenced by**:
- JobsData.md: `### Determine TaskWork.workId`

When a task is executed by a `TaskWork`, it is in `managed session model`.
The job has to start all sessions.
`TaskWork` fails if the last retry:
- Does not pass `Task.availability` checking. Undefined means the check always succeeds.
- Does not pass `Task.criteria` checking. Undefined means the check always succeeds.

### Determine TaskWork.workId

**Referenced by**:
- API.md: `### copilot/job`

Any `TaskWork` must have an unique `workIdInJob` in a Job.
The `assignWorkId` function converts a `Work<never>` to `Work<number>` with property `workIdInJob` assigned.
When creating a `Work` AST, you can create one in `Work<never>` without worrying about `workIdInJob`, and call `assignWorkId` to fix that for you.

### Exception Handling

**Referenced by**:
- API.md: `### sendMonitoredPrompt (crash retry)`

If any task crashes:
- The job stops immediately and marked failed.
- The exception cannot be consumed silently.

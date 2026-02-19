# TODO

## Job Execution

Needs to support multiple client watching for the same job running.
A client can make a random guid and the server would treat them separately.
Data will be deleted after all guids drain all responses or 5 minutes after the target is closed.

- List all running jobs.
  - Task status will be updated to job (add api to pull).
    - Rendering icons only, do not filtering.
  - job live api:
    - A running job is able to see all history of responses.
    - Durning receiving history responses that a finished task is started, trying to pull the task will fail.
    - The UI needs to handle it, by not creating any data structure for this task.
  - task live api:
    - A running task is able to see all history of responses.
    - During receiving history responses that a finished session is started, trying to pull the session will fail.
    - The UI needs to handle it, tabs still created but leaving some words saying the history is invisible.
  - session live api:
    - A running session is able to see all history of responses.
- List all running session, which is not initiated by a job/task.
- No task listing as there is no UI for just a task.
- List will be displayed in `/index.html` before clicking "Start" button and will not refresh.
  - This serves as an entry of opening existing jobs/session.
  - The name will be the time starting the job/session

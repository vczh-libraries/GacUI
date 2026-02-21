# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `assets`
  - `index.css`
  - `index.js`
  - `index.html`

### index.css

Put index.html specific css file in index.css.

### index.js

Put index.html specific javascript file in index.js.

### index.html

#### Starting a Copilot Session

**Referenced by**:
- Jobs.md: `### jobs.html`

When the webpage is loaded, it renders a UI in the middle to let me input:
- Model. A combo box with contents retrieved from `api/copilot/models`.
  - Items must be sorted.
  - Names instead of ids are displayed, but be aware of that other API needs the id.
  - Default to the model whose id is "gpt-5.2"
- Working Directory. A text box receiving an absolute full path.
  - When the url is `index.html?project=XXX`, the text box defaults to `REPO-ROOT\..\XXX`
  - When there is no `project` argument, it becomes `REPO-ROOT`.
  - `REPO-ROOT` is the root folder of the repo that the source code of this project is in (no hardcoding).

There are a row of buttons with proper margin:
- On the very left: "New Job", "Refresh"
- On the very right: "Start"

Below there is a list, displaying all running job's name, status, time.
The list only refresh when the webpage is loaded or the "Refresh" button is clicked.
It can be listed by `copilot/job/running`.
At the very left of each item, there is a "View" button. It starts `/jobTracking.html` to inspect into the job.

The list must be in the same width with the above part.

##### Start Button

When I hit the "Start" button, the UI above disappears and show the session UI.
Send `api/copilot/session/start/{model-id}` to get the session id.

Only after the model list is loaded, "Start" is enabled.

##### Jobs Button

**Referenced by**:
- Jobs.md: `### jobs.html`

When I hit the "New Job" button, it jumps to `/jobs.html`.
The selected model is ignored, but the working directory should be brought to `/jobs.html`.

#### Session Interaction

The agent UI has two part.

The upper part (session part) renders what the session replies.
The lower part (request part) renders a text box to send my request to the session.
The session part and the request part should always fill the whole webpage.
Between two parts there is a bar to drag vertically to adjust the height of the request part which defaults to 300px.

After the UI is loaded,
call `api/token` for a token,
the page must keep sending `api/copilot/session/{session-id}/live/{token}` to the server sequentially (aka not parallelly).
When a timeout happens, resend the api.
When it returns any response, process it and still keep sending the api.
Whenever `ICopilotSessionCallbacks::METHOD` is mentioned, it means a response from this api.

There are additional callbacks:
- `onGeneratedUserPrompt`: Create a "User" message block with `title` set to `Task`, pretending that the user is talking.

After "Stop Server" or "Close Session" is pressed, responses from this api will be ignored and no more such api is sending.

#### Session Part

The session part div is passed to a `SessionResponseRenderer` (from `sessionResponse.js`) which handles all rendering within it.
See `Shared.md` for `SessionResponseRenderer` specification.

Live polling callbacks are forwarded to `sessionRenderer.processCallback(data)`.
When its return value is `onIdle`, the send button is re-enabled.

When not running a task, any user request should call `sessionRenderer.addUserMessage(text)` to create a "User" message block.
Call `sessionRenderer.setAwaiting(true)` when waiting for responses, and `sessionRenderer.setAwaiting(false)` when done (sync with "Send" button's enabled state).

#### Request Part

**Referenced by**:
- Jobs.md: `### Matrix Part`

At the very top there is a label "Choose a Task: " followed by a combo box in a proper weight, listing all tasks.
Keep the visual looks similar and good margin between controls.
The first item is "(none)". When it is selected, "Send" will talk to the session directly instead of starting a task.
If a task is selected, the same session will be reused to start a task.

It is a multiline text box. I can type any text, and press CTRL+ENTER to send the request.

There is a "Send" button at the right bottom corner of the text box.
It does the same thing as pressing CTRL+ENTER.
When the button is disabled, pressing CTRL+ENTER should also does nothing.
`api/copilot/session/{session-id}/query` is used here.

User request should generate a "User" message block, append the request and immediately complete it.

When a request is sent, the button is disabled.
When `ICopilotSessionCallbacks::onIdle` triggers, it is enabled again.

There is a "Stop Server" and "Close Session" button (in the mentioning order) at the left bottom corner of the text box.
When "Close Session" is clicked:
- Ends the session with `api/copilot/session/{session-id}/stop`.
- Do whatever needs for finishing.
- Try to close the current webpage window or tab. If the browser blocks it (e.g. Chrome blocks `window.close()` for tabs not opened by script), replace the page content with a message indicating the session has ended and the tab may be closed manually.
When "Stop Server" is clicked:
- It does what "Close Session" does, with an extra `api/stop` to stop the server before attempting to close the webpage.

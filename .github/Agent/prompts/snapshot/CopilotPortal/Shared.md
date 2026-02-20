# Specification

Root folder of the project is `REPO-ROOT/.github/Agent`.
Read `README.md` to understand the whole picture of the project as well as specification organizations.

## Related Files

- `assets`
  - `messageBlock.css`
  - `messageBlock.js`
  - `sessionResponse.css`
  - `sessionResponse.js`

### messageBlock.css

Put messageBlock.js specific css file in messageBlock.css.

### messageBlock.js

**Referenced by**:
- Shared.md: `### sessionResponse.js`

It exposes some APIs in this schema

```typescript
export class MessageBlock {
  constructor(blockType: "User" | "Reasoning" | "Tool" | "Message");
  appendData(data: string): void;
  complete(): void;
  get isCompleted(): boolean;
  get divElement(): HTMLDivElement;
  get title(): string;
  set title(value: string);
}

export function getMessageBlock(div: HTMLDivElement): MessageBlock | undefined;
```

Each `MessageBlock` has a title, displaying: "blockType (title) [receiving...]" or "blockType (title)".
When `title` is empty, "()" and the space before it must be omitted.
Receiving appears when it is not completed yet.

When a `MessageBlock` is created and receiving data, the height is limited to 150px, clicking the header does nothing

When a `MessageBlock` is completed:
- If this `MessageBlock` is "User" and "Message", it will expand, otherwise collapse.
- Completing a `MessageBlock` should not automatically expand or collapse other blocks.
- Clicking the header of a completed `MessageBlock` switch between expanding or collapsing.
- There is no more height limit, it should expands to render all data.
- A button appears at the very right of the header, it should fills full height.
  - When the content is rendering as markdown, it shows "View Raw Data", clicking it shows the raw data.
  - When the content is raw data, it shows "View Markdown", clicking it shows the markdown rendering of the raw data.

Before a `MessageBlock` is completed, raw data should render.
After it is completed, assuming the data is markdown document and render it properly:
- Except for "Tool" block, and "Tool" block should not render the button switching between raw data and markdown.
- Try to tell if the raw content is markdown or just ordinary text, if it doesn't look like a markdown, do not do the markdown rendering automatically.

Inside the `MessageBlock`, it holds a `<div/>` to change the rendering.
And it should also put itself in the element (e.g. in a field with a unique name) so that the object will not be garbage-collected.

### sessionResponse.css

Put sessionResponse.js specific css file in sessionResponse.css.

### sessionResponse.js

**Referenced by**:
- Index.md: `#### Session Part`, `#### Session Interaction`, `#### Request Part`

It exposes a `SessionResponseRenderer` class in this schema:

```typescript
export class SessionResponseRenderer {
  constructor(div: HTMLDivElement);
  processCallback(data: object): string;
  addUserMessage(text: string, title?: string): void;
  setAwaiting(awaiting: boolean): void;
  scrollToBottom(): void;
}
```

`SessionResponseRenderer` is a pure rendering component that does not touch any Copilot related API.
An empty div element is passed to the constructor; all child elements are dynamically created inside it.
The container div gets a CSS class `session-response-container` for styling (scrollable, padded).

It manages an internal map of `MessageBlock` instances keyed by `"blockType-blockId"`.
An "Awaits responses ..." status element is created and appended to the container.

- `processCallback(data)`: Processes a callback object (from the live polling API) and handles creating/updating/completing message blocks for Reasoning, Message, Tool, and onGeneratedUserPrompt callbacks. For `onGeneratedUserPrompt`, it calls `addUserMessage(data.prompt, "Task")`. Returns the callback name string so the caller can react to lifecycle events (e.g. `"onAgentEnd"`).
- `addUserMessage(text)`: Creates a "User" `MessageBlock`, appends the text, completes it, and scrolls to bottom.
- `setAwaiting(awaiting)`: Shows or hides the "Awaits responses ..." status text.
- `scrollToBottom()`: Scrolls the container to the bottom.

#### Session Response Rendering

Session responses generates 3 types of message block:
- Reasoning
- Tool
- Message
Multiple of them could happen parallelly.

When `ICopilotSessionCallbacks::onStartXXX` happens, a new message block should be created.
When `ICopilotSessionCallbacks::onXXX` happens, the data should be appended to the message block.
When `ICopilotSessionCallbacks::onEndXXX` happens, the message block is completed, no data needs to append to the message block.

The content of a "Tool" `MessageBlock` needs to be taken care of specially:
- The first line should be in its title. It is easy to tell when the `title` property is empty.
- `ICopilotSessionCallbacks::onEndToolExecution` will give you optional extra information.
Responses for different message blocks are identified by its id.

Message blocks stack vertically from top to bottom.
`MessageBlock` in messageBlock.js should be used to control any message block.

You are recommended to maintain a list of message blocks in a map with key "blockType-blockId" in its rendering order.

When `setAwaiting(true)` is called,
there must be a text at the left bottom corner of the session part saying "Awaits responses ...".
When `setAwaiting(false)` is called, this text disappears.

The session response container is scrollable.

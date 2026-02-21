import { CopilotClient, defineTool, type CopilotSession } from "@github/copilot-sdk";

export interface ICopilotSession {
  get rawSection(): CopilotSession;
  sendRequest(message: string, timeout?: number): Promise<void>;
  stop(): Promise<void>;
}

export interface ICopilotSessionCallbacks {
  // assistant.reasoning_delta with a new id
  onStartReasoning(reasoningId: string): void;
  // assistant.reasoning_delta with an existing id
  onReasoning(reasoningId: string, delta: string): void;
  // assistant.reasoning with an existing id
  onEndReasoning(reasoningId: string, completeContent: string): void;

  // assistant.message_delta with a new id
  onStartMessage(messageId: string): void;
  // assistant.message_delta with an existing id
  onMessage(messageId: string, delta: string): void;
  // assistant.message with an existing id
  onEndMessage(messageId: string, completeContent: string): void;

  // tool.execution_start with a new id
  onStartToolExecution(
    toolCallId: string,
    parentToolCallId: string | undefined,
    toolName: string,
    toolArguments: string
  ): void;
  // tool.execution_partial_result with an existing id
  onToolExecution(toolCallId: string, delta: string): void;
  // tool.execution_complete with an existing id
  onEndToolExecution(
    toolCallId: string,
    result: { content: string; detailedContent?: string } | undefined,
    error: { message: string; code?: string } | undefined
  ): void;

  // assistant.turn_start
  onAgentStart(turnId: string): void;

  // assistant.turn_end
  onAgentEnd(turnId: string): void;

  // session.idle
  onIdle(): void;
}

// DOCUMENT: https://github.com/github/copilot-sdk/blob/main/nodejs/README.md
export async function startSession(
  client: CopilotClient,
  modelId: string,
  callback: ICopilotSessionCallbacks,
  workingDirectory?: string
): Promise<ICopilotSession> {
  const jobTools = [
    defineTool("job_prepare_document", {
      description: "When required, use this tool to report a document path that you are about to create or update.",
      parameters: { type: "object" as const, properties: { argument: { type: "string", description: "An absolute path of the document" } }, required: ["argument"] },
      handler: async (args: { argument?: string }) => args.argument ?? "",
    }),
    defineTool("job_boolean_true", {
      description: "When required, use this tool to report that a boolean condition is true, with the reason.",
      parameters: { type: "object" as const, properties: { argument: { type: "string", description: "The reason" } }, required: ["argument"] },
      handler: async (args: { argument?: string }) => args.argument ?? "",
    }),
    defineTool("job_boolean_false", {
      description: "When required, use this tool to report that a boolean condition is false, with the reason.",
      parameters: { type: "object" as const, properties: { argument: { type: "string", description: "The reason" } }, required: ["argument"] },
      handler: async (args: { argument?: string }) => args.argument ?? "",
    }),
    defineTool("job_prerequisite_failed", {
      description: "When required, use this tool to report that a prerequisite check has failed.",
      parameters: { type: "object" as const, properties: { argument: { type: "string", description: "The reason" } }, required: ["argument"] },
      handler: async (args: { argument?: string }) => args.argument ?? "",
    }),
  ];

  const session = await client.createSession({
    model: modelId,
    streaming: true,
    workingDirectory,
    tools: jobTools,
    hooks: {
      onPreToolUse: async (input) => {
        if (input.toolName === "glob") {
          return {
            permissionDecision: "deny",
            permissionDecisionReason: "Glob does not work on Windows."
          }
        }
      }
    }
  });

  const reasoningContentById = new Map<string, string>();
  const messageContentById = new Map<string, string>();
  const toolOutputById = new Map<string, string>();

  session.on("assistant.turn_start", (event) => {
    callback.onAgentStart(event.data.turnId);
  });

  session.on("assistant.turn_end", (event) => {
    callback.onAgentEnd(event.data.turnId);
  });

  session.on("assistant.reasoning_delta", (event) => {
    const existing = reasoningContentById.get(event.data.reasoningId);
    if (existing === undefined) {
      reasoningContentById.set(event.data.reasoningId, event.data.deltaContent);
      callback.onStartReasoning(event.data.reasoningId);
      callback.onReasoning(event.data.reasoningId, event.data.deltaContent);
      return;
    }

    reasoningContentById.set(
      event.data.reasoningId,
      existing + event.data.deltaContent
    );
    callback.onReasoning(event.data.reasoningId, event.data.deltaContent);
  });

  session.on("assistant.reasoning", (event) => {
    reasoningContentById.set(event.data.reasoningId, event.data.content);
    callback.onEndReasoning(event.data.reasoningId, event.data.content);
  });

  session.on("assistant.message_delta", (event) => {
    const existing = messageContentById.get(event.data.messageId);
    if (existing === undefined) {
      messageContentById.set(event.data.messageId, event.data.deltaContent);
      callback.onStartMessage(event.data.messageId);
      callback.onMessage(event.data.messageId, event.data.deltaContent);
      return;
    }

    messageContentById.set(
      event.data.messageId,
      existing + event.data.deltaContent
    );
    callback.onMessage(event.data.messageId, event.data.deltaContent);
  });

  session.on("assistant.message", (event) => {
    messageContentById.set(event.data.messageId, event.data.content);
    callback.onEndMessage(event.data.messageId, event.data.content);
  });

  session.on("tool.execution_start", (event) => {
    callback.onStartToolExecution(
      event.data.toolCallId,
      event.data.parentToolCallId,
      event.data.toolName,
      (event.data.arguments ? JSON.stringify(event.data.arguments, undefined, 2) : "")
    );
  });

  session.on("tool.execution_partial_result", (event) => {
    const existing = toolOutputById.get(event.data.toolCallId) ?? "";
    toolOutputById.set(event.data.toolCallId, existing + event.data.partialOutput);
    callback.onToolExecution(event.data.toolCallId, event.data.partialOutput);
  });

  session.on("tool.execution_complete", (event) => {
    callback.onEndToolExecution(
      event.data.toolCallId,
      event.data.result,
      event.data.error
    );
  });

  session.on("session.idle", () => {
    callback.onIdle();
  });

  return {
    get rawSection(): CopilotSession {
      return session;
    },

    async sendRequest(message: string, timeout: number = 2147483647): Promise<void> {
      await session.sendAndWait({ prompt: message }, timeout);
    },

    async stop(): Promise<void> {
      await session.destroy();
    },
  };
}

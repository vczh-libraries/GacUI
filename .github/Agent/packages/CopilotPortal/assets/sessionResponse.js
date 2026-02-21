import { MessageBlock } from "./messageBlock.js";

/**
 * SessionResponseRenderer handles rendering of multiple stacking session responses.
 * It is a pure rendering component that does not touch any Copilot API.
 * Pass an empty div element into the constructor; all child elements are created dynamically.
 */
export class SessionResponseRenderer {
    #containerDiv;
    #awaitingStatus;
    #messageBlocks; // Map keyed by "blockType-blockId"

    /**
     * @param {HTMLDivElement} div - An empty div element to render into.
     */
    constructor(div) {
        this.#containerDiv = div;
        this.#containerDiv.classList.add("session-response-container");
        this.#messageBlocks = new Map();

        // Create awaiting status element
        this.#awaitingStatus = document.createElement("div");
        this.#awaitingStatus.classList.add("session-response-awaiting");
        this.#awaitingStatus.textContent = "Awaits responses ...";
        this.#awaitingStatus.style.display = "none";
        this.#containerDiv.appendChild(this.#awaitingStatus);
    }

    /**
     * Get or create a MessageBlock by type and id.
     * @param {string} blockType
     * @param {string} blockId
     * @returns {MessageBlock}
     */
    #getOrCreateBlock(blockType, blockId) {
        const key = `${blockType}-${blockId}`;
        let block = this.#messageBlocks.get(key);
        if (!block) {
            block = new MessageBlock(blockType);
            this.#messageBlocks.set(key, block);
            this.#containerDiv.insertBefore(block.divElement, this.#awaitingStatus);
        }
        return block;
    }

    /**
     * Process a callback response from the live polling API.
     * This handles all session response types: Reasoning, Message, Tool, and lifecycle events.
     *
     * @param {object} data - The callback data with a `callback` property.
     * @returns {string} The callback name, so the caller can react (e.g. to "onAgentEnd").
     */
    processCallback(data) {
        const cb = data.callback;

        // Reasoning
        if (cb === "onStartReasoning") {
            this.#getOrCreateBlock("Reasoning", data.reasoningId);
        } else if (cb === "onReasoning") {
            const block = this.#getOrCreateBlock("Reasoning", data.reasoningId);
            block.appendData(data.delta);
        } else if (cb === "onEndReasoning") {
            const block = this.#getOrCreateBlock("Reasoning", data.reasoningId);
            block.replaceData(data.completeContent);
            block.complete();
        }

        // Message
        else if (cb === "onStartMessage") {
            this.#getOrCreateBlock("Message", data.messageId);
        } else if (cb === "onMessage") {
            const block = this.#getOrCreateBlock("Message", data.messageId);
            block.appendData(data.delta);
        } else if (cb === "onEndMessage") {
            const block = this.#getOrCreateBlock("Message", data.messageId);
            block.replaceData(data.completeContent);
            block.complete();
        }

        // Tool
        else if (cb === "onStartToolExecution") {
            const block = this.#getOrCreateBlock("Tool", data.toolCallId);
            block.title = data.toolName;
            block.appendData(data.toolArguments);
            block.appendData("\n");
        } else if (cb === "onToolExecution") {
            const block = this.#getOrCreateBlock("Tool", data.toolCallId);
            block.appendData(data.delta);
        } else if (cb === "onEndToolExecution") {
            const block = this.#getOrCreateBlock("Tool", data.toolCallId);
            if (data.result) {
                block.appendData(`\nResult: ${data.result.content}`);
                if (data.result.detailedContent) {
                    block.appendData(`\nDetails: ${data.result.detailedContent}`);
                }
            }
            if (data.error) {
                block.appendData(`\nError: ${data.error.message}`);
            }
            block.complete();
        }

        // Generated user prompt (sent to driving/task session by the engine)
        else if (cb === "onGeneratedUserPrompt") {
            this.addUserMessage(data.prompt, "Task");
        }

        // Auto-scroll to bottom
        this.scrollToBottom();

        return cb;
    }

    /**
     * Add a user message block. Appends data and immediately completes it.
     * @param {string} text - The user request text.
     * @param {string} [title] - Optional title for the message block.
     */
    addUserMessage(text, title) {
        const userBlock = new MessageBlock("User");
        if (title) {
            userBlock.title = title;
        }
        const userKey = `User-request-${Date.now()}`;
        this.#messageBlocks.set(userKey, userBlock);
        this.#containerDiv.insertBefore(userBlock.divElement, this.#awaitingStatus);
        userBlock.appendData(text);
        userBlock.complete();
        this.scrollToBottom();
    }

    /**
     * Show or hide the "Awaits responses ..." status.
     * @param {boolean} awaiting
     */
    setAwaiting(awaiting) {
        this.#awaitingStatus.style.display = awaiting ? "block" : "none";
    }

    /**
     * Scroll the container to the bottom.
     */
    scrollToBottom() {
        this.#containerDiv.scrollTop = this.#containerDiv.scrollHeight;
    }
}

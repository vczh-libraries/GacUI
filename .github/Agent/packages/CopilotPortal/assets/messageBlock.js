const MESSAGE_BLOCK_FIELD = "__copilotMessageBlock";

function looksLikeMarkdown(text) {
    // Check for common markdown patterns
    const patterns = [
        /^#{1,6}\s/m,           // headings
        /\*\*.+?\*\*/,          // bold
        /\*.+?\*/,              // italic
        /\[.+?\]\(.+?\)/,      // links
        /^[-*+]\s/m,            // unordered lists
        /^\d+\.\s/m,            // ordered lists
        /^>\s/m,                // blockquotes
        /```[\s\S]*?```/,       // fenced code blocks
        /`[^`]+`/,              // inline code
        /^\|.*\|.*\|/m,         // tables
        /^---+$/m,              // horizontal rules
        /!\[.*?\]\(.*?\)/,      // images
    ];
    let matchCount = 0;
    for (const p of patterns) {
        if (p.test(text)) matchCount++;
    }
    return matchCount >= 2;
}

export class MessageBlock {
    #blockType;
    #title = "";
    #completed = false;
    #collapsed = false;
    #rawData = "";
    #showingMarkdown = false;
    #divElement;
    #headerElement;
    #headerTextElement;
    #toggleButton;
    #bodyElement;

    constructor(blockType) {
        this.#blockType = blockType;
        this.#title = "";
        this.#completed = false;
        this.#collapsed = false;
        this.#rawData = "";
        this.#showingMarkdown = false;

        this.#divElement = document.createElement("div");
        this.#divElement.classList.add("message-block", "receiving");
        this.#divElement[MESSAGE_BLOCK_FIELD] = this;

        this.#headerElement = document.createElement("div");
        this.#headerElement.classList.add("message-block-header");

        this.#headerTextElement = document.createElement("span");
        this.#headerTextElement.classList.add("message-block-header-text");
        this.#headerElement.appendChild(this.#headerTextElement);

        this.#toggleButton = document.createElement("button");
        this.#toggleButton.classList.add("message-block-toggle");
        this.#toggleButton.style.display = "none";
        this.#toggleButton.addEventListener("click", (e) => {
            e.stopPropagation();
            this.#toggleView();
        });
        this.#headerElement.appendChild(this.#toggleButton);

        this.#updateHeader();
        this.#divElement.appendChild(this.#headerElement);

        this.#bodyElement = document.createElement("div");
        this.#bodyElement.classList.add("message-block-body");
        this.#divElement.appendChild(this.#bodyElement);
    }

    #updateHeader() {
        const titlePart = this.#title ? ` (${this.#title})` : "";
        const receiving = this.#completed ? "" : " [receiving...]";
        this.#headerTextElement.textContent = `${this.#blockType}${titlePart}${receiving}`;
    }

    #renderMarkdown() {
        this.#showingMarkdown = true;
        this.#bodyElement.classList.add("markdown-rendered");
        this.#bodyElement.innerHTML = marked.parse(this.#rawData);
        this.#toggleButton.textContent = "View Raw Data";
    }

    #renderRawData() {
        this.#showingMarkdown = false;
        this.#bodyElement.classList.remove("markdown-rendered");
        this.#bodyElement.textContent = this.#rawData;
        this.#toggleButton.textContent = "View Markdown";
    }

    #toggleView() {
        if (this.#showingMarkdown) {
            this.#renderRawData();
        } else {
            this.#renderMarkdown();
        }
    }

    get title() {
        return this.#title;
    }

    set title(value) {
        this.#title = value;
        this.#updateHeader();
    }

    appendData(data) {
        this.#rawData += data;
        this.#bodyElement.textContent = this.#rawData;
        // Auto-scroll to bottom while receiving
        this.#bodyElement.scrollTop = this.#bodyElement.scrollHeight;
    }

    replaceData(data) {
        this.#rawData = data;
        this.#bodyElement.textContent = this.#rawData;
        this.#bodyElement.scrollTop = this.#bodyElement.scrollHeight;
    }

    complete() {
        this.#completed = true;
        this.#collapsed = false;
        this.#updateHeader();
        this.#headerElement.classList.add("completed");
        this.#divElement.classList.remove("receiving");
        this.#divElement.classList.add("completed");

        // For non-Tool blocks: render markdown if content looks like markdown, show toggle button
        if (this.#blockType !== "Tool" && typeof marked !== "undefined") {
            this.#toggleButton.style.display = "";
            if (looksLikeMarkdown(this.#rawData)) {
                this.#renderMarkdown();
            } else {
                this.#renderRawData();
            }
        }

        // "User" and "Message" blocks expand, others collapse
        // Completing a block should NOT automatically expand or collapse other blocks
        const shouldExpand = this.#blockType === "User" || this.#blockType === "Message";
        if (shouldExpand) {
            this.#collapsed = false;
            this.#divElement.classList.remove("collapsed");
        } else {
            this.#collapsed = true;
            this.#divElement.classList.add("collapsed");
        }

        this.#headerElement.onclick = () => {
            if (!this.#completed) return;
            this.#collapsed = !this.#collapsed;
            if (this.#collapsed) {
                this.#divElement.classList.add("collapsed");
            } else {
                this.#divElement.classList.remove("collapsed");
            }
        };
    }

    get isCompleted() {
        return this.#completed;
    }

    get divElement() {
        return this.#divElement;
    }
}

export function getMessageBlock(div) {
    return div?.[MESSAGE_BLOCK_FIELD];
}

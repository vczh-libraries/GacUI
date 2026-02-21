import * as http from "node:http";
import { CopilotClient } from "@github/copilot-sdk";

// ---- Helpers ----

export function readBody(req: http.IncomingMessage): Promise<string> {
    return new Promise((resolve, reject) => {
        const chunks: Buffer[] = [];
        req.on("data", (chunk: Buffer) => chunks.push(chunk));
        req.on("end", () => resolve(Buffer.concat(chunks).toString("utf-8")));
        req.on("error", reject);
    });
}

export function jsonResponse(res: http.ServerResponse, statusCode: number, data: unknown): void {
    res.writeHead(statusCode, { "Content-Type": "application/json" });
    res.end(JSON.stringify(data));
}

// ---- Test Mode ----

let _testMode = false;
export function setTestMode(value: boolean): void { _testMode = value; }
export function isTestMode(): boolean { return _testMode; }
export function getCountDownMs(): number { return _testMode ? 5000 : 60000; }

// ---- Token-based Live State ----

export interface LiveResponse {
    [key: string]: unknown;
}

export interface TokenState {
    position: number;
    lastResponseTime: number | undefined;
    pendingResolve: ((response: LiveResponse) => void) | null;
    pendingClosedError: string;
    httpTimeout: ReturnType<typeof setTimeout> | null;
    batchTimeout: ReturnType<typeof setTimeout> | null;
}

export interface LiveEntityState {
    responses: LiveResponse[];
    tokens: Map<string, TokenState>;
    closed: boolean;
    countDownBegin: number | undefined; // undefined while entity is running
    countDownMs: number;
    onDelete?: () => void;
}

export function createLiveEntityState(countDownMs: number, onDelete?: () => void): LiveEntityState {
    return {
        responses: [],
        tokens: new Map(),
        closed: false,
        countDownBegin: undefined,
        countDownMs,
        onDelete,
    };
}

function tryCleanupEntity(entity: LiveEntityState, token: string): void {
    entity.tokens.delete(token);
    if (entity.tokens.size === 0 && entity.onDelete) {
        entity.onDelete();
    }
}

// Shared helper: drain all available responses, update position/lastResponseTime, and resolve.
// Used by pushLiveResponse (immediate or via batchTimeout), httpTimeout, and closeLiveEntity.
function resolveToken(entity: LiveEntityState, token: string, tokenState: TokenState): void {
    if (!tokenState.pendingResolve) return;

    if (tokenState.httpTimeout) { clearTimeout(tokenState.httpTimeout); tokenState.httpTimeout = null; }
    if (tokenState.batchTimeout) { clearTimeout(tokenState.batchTimeout); tokenState.batchTimeout = null; }

    const resolve = tokenState.pendingResolve;
    tokenState.pendingResolve = null;

    if (tokenState.position < entity.responses.length) {
        const responses = entity.responses.slice(tokenState.position);
        tokenState.position = entity.responses.length;
        tokenState.lastResponseTime = Date.now();
        resolve({ responses });
    } else if (entity.closed) {
        tryCleanupEntity(entity, token);
        resolve({ error: tokenState.pendingClosedError });
    } else {
        resolve({ error: "HttpRequestTimeout" });
    }
}

export function pushLiveResponse(entity: LiveEntityState, response: LiveResponse): void {
    // Optimization: when onEndReasoning/onEndMessage arrives, remove all matching delta entries.
    // The end response carries the complete content so deltas are redundant.
    if (response.callback === "onEndReasoning" || response.callback === "onEndMessage") {
        const isReasoning = response.callback === "onEndReasoning";
        const deltaCallback = isReasoning ? "onReasoning" : "onMessage";
        const idKey = isReasoning ? "reasoningId" : "messageId";
        const id = response[idKey] as string;

        for (let i = entity.responses.length - 1; i >= 0; i--) {
            if (entity.responses[i].callback === deltaCallback && entity.responses[i][idKey] === id) {
                entity.responses.splice(i, 1);
                // Adjust token positions for the removed entry
                for (const [, ts] of entity.tokens) {
                    if (ts.position > i) {
                        ts.position--;
                    }
                }
            }
        }
    }

    entity.responses.push(response);

    // Notify pending tokens that have unread data and no batchTimeout already scheduled
    for (const [tok, ts] of entity.tokens) {
        if (ts.pendingResolve && ts.position < entity.responses.length && !ts.batchTimeout) {
            const now = Date.now();
            if (ts.lastResponseTime === undefined || now - ts.lastResponseTime >= 5000) {
                // No recent response or window elapsed — resolve immediately
                resolveToken(entity, tok, ts);
            } else {
                // Schedule delayed batch to accumulate more responses
                const remaining = 5000 - (now - ts.lastResponseTime);
                const ent = entity;
                const token = tok;
                const tokenState = ts;
                ts.batchTimeout = setTimeout(() => {
                    tokenState.batchTimeout = null;
                    resolveToken(ent, token, tokenState);
                }, remaining);
            }
        }
    }
}

export function closeLiveEntity(entity: LiveEntityState): void {
    entity.closed = true;
    entity.countDownBegin = Date.now();
    // Wake up all tokens that are waiting and fully drained
    for (const [token, tokenState] of entity.tokens) {
        if (tokenState.pendingResolve && tokenState.position >= entity.responses.length) {
            resolveToken(entity, token, tokenState);
        }
    }
}

export function waitForLiveResponse(
    entity: LiveEntityState | undefined,
    token: string,
    timeoutMs: number,
    notFoundError: string,
    closedError: string,
): Promise<LiveResponse> {
    // Entity doesn't exist
    if (!entity) {
        return Promise.resolve({ error: notFoundError });
    }

    // Check if token is known
    let tokenState = entity.tokens.get(token);
    if (!tokenState) {
        // New token — check lifecycle
        if (entity.countDownBegin !== undefined) {
            if (Date.now() - entity.countDownBegin > entity.countDownMs) {
                return Promise.resolve({ error: notFoundError });
            }
        }
        tokenState = { position: 0, lastResponseTime: undefined, pendingResolve: null, pendingClosedError: "", httpTimeout: null, batchTimeout: null };
        entity.tokens.set(token, tokenState);
    }

    // Check parallel call on same (entity, token)
    if (tokenState.pendingResolve) {
        return Promise.resolve({ error: "ParallelCallNotSupported" });
    }

    // Batch drain: return ALL available responses from current position
    if (tokenState.position < entity.responses.length) {
        const responses = entity.responses.slice(tokenState.position);
        tokenState.position = entity.responses.length;
        tokenState.lastResponseTime = Date.now();
        return Promise.resolve({ responses });
    }

    // Closed and fully drained?
    if (entity.closed && tokenState.position >= entity.responses.length) {
        tryCleanupEntity(entity, token);
        return Promise.resolve({ error: closedError });
    }

    // Wait for next response or timeout
    const ts = tokenState;
    const ent = entity;
    const tok = token;
    return new Promise((resolve) => {
        ts.pendingResolve = resolve;
        ts.pendingClosedError = closedError;
        ts.httpTimeout = setTimeout(() => {
            ts.httpTimeout = null;
            resolveToken(ent, tok, ts);
        }, timeoutMs);
    });
}

export function shutdownLiveEntity(entity: LiveEntityState): void {
    // Resolve all pending token waits and clear
    for (const [, tokenState] of entity.tokens) {
        if (tokenState.httpTimeout) { clearTimeout(tokenState.httpTimeout); tokenState.httpTimeout = null; }
        if (tokenState.batchTimeout) { clearTimeout(tokenState.batchTimeout); tokenState.batchTimeout = null; }
        if (tokenState.pendingResolve) {
            const resolve = tokenState.pendingResolve;
            tokenState.pendingResolve = null;
            resolve({ error: "HttpRequestTimeout" });
        }
    }
    entity.tokens.clear();
}

// ---- Copilot Client ----

let copilotClient: CopilotClient | null = null;
let copilotClientPromise: Promise<CopilotClient> | null = null;

export async function ensureCopilotClient(): Promise<CopilotClient> {
    if (copilotClient) return copilotClient;
    if (!copilotClientPromise) {
        copilotClientPromise = (async () => {
            const client = new CopilotClient();
            await client.start();
            copilotClient = client;
            copilotClientPromise = null;
            return client;
        })();
    }
    return copilotClientPromise;
}

export function stopCoplilotClient(): void {
    copilotClientPromise = null;
    if (copilotClient) {
        copilotClient.stop();
        copilotClient = null;
    }
}

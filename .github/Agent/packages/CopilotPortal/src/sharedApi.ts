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

// ---- Live State ----

export interface LiveResponse {
    [key: string]: unknown;
}

export interface LiveState {
    responseQueue: LiveResponse[];
    waitingResolve: ((response: LiveResponse) => void) | null;
}

export function pushResponse(state: LiveState, response: LiveResponse): void {
    if (state.waitingResolve) {
        const resolve = state.waitingResolve;
        state.waitingResolve = null;
        resolve(response);
    } else {
        state.responseQueue.push(response);
    }
}

export function waitForResponse(state: LiveState, timeoutMs: number): Promise<LiveResponse | null> {
    return new Promise((resolve) => {
        if (state.responseQueue.length > 0) {
            resolve(state.responseQueue.shift()!);
            return;
        }
        state.waitingResolve = resolve as (response: LiveResponse) => void;
        setTimeout(() => {
            if (state.waitingResolve === resolve) {
                state.waitingResolve = null;
                resolve(null);
            }
        }, timeoutMs);
    });
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

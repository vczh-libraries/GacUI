import * as http from "node:http";
import * as fs from "node:fs";
import * as path from "node:path";
import { fileURLToPath } from "node:url";
import {
    jsonResponse,
    readBody,
    setTestMode,
} from "./sharedApi.js";
import {
    apiConfig,
    apiStop,
    apiToken,
    apiCopilotModels,
    apiCopilotSessionStart,
    apiCopilotSessionStop,
    apiCopilotSessionQuery,
    apiCopilotSessionLive,
    hasRunningSessions,
} from "./copilotApi.js";
import {
    apiTaskList,
    apiTaskStart,
    apiTaskStop,
    apiTaskLive,
} from "./taskApi.js";
import {
    apiJobList,
    apiJobRunning,
    apiJobStatus,
    apiJobStart,
    apiJobStop,
    apiJobLive,
} from "./jobsApi.js";
import type { Entry } from "./jobsDef.js";
import { validateEntry } from "./jobsDef.js";
import { entry } from "./jobsData.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Parse command-line options
let port = 8888;
let testMode = false;
const args = process.argv.slice(2);
for (let i = 0; i < args.length; i++) {
    if (args[i] === "--port" && i + 1 < args.length) {
        port = parseInt(args[i + 1], 10);
        i++;
    } else if (args[i] === "--test") {
        testMode = true;
    }
}

if (testMode) {
    setTestMode(true);
}

const mimeTypes: Record<string, string> = {
    ".html": "text/html",
    ".css": "text/css",
    ".js": "application/javascript",
    ".json": "application/json",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".gif": "image/gif",
    ".svg": "image/svg+xml",
    ".ico": "image/x-icon",
};

// assets folder is at packages/CopilotPortal/assets, __dirname is packages/CopilotPortal/dist
const assetsDir = path.resolve(__dirname, "..", "assets");

// REPO-ROOT: walk up from __dirname until we find a .git folder
function findRepoRoot(startDir: string): string {
    let dir = startDir;
    while (true) {
        if (fs.existsSync(path.join(dir, ".git"))) {
            return dir;
        }
        const parent = path.dirname(dir);
        if (parent === dir) {
            // Reached filesystem root without finding .git; fall back to startDir
            return startDir;
        }
        dir = parent;
    }
}
const repoRoot = findRepoRoot(__dirname);

// ---- Entry Management ----

let installedEntry: Entry | null = null;

async function installJobsEntry(entryValue: Entry): Promise<void> {
    if (hasRunningSessions()) {
        throw new Error("Cannot call installJobsEntry while sessions are running.");
    }
    installedEntry = entryValue;
}

function ensureInstalledEntry(): Entry {
    if (!installedEntry) {
        throw new Error("installJobsEntry has not been called.");
    }
    return installedEntry;
}

// ---- Static File Serving ----

function serveStaticFile(res: http.ServerResponse, filePath: string): void {
    if (!fs.existsSync(filePath)) {
        res.writeHead(404, { "Content-Type": "text/plain" });
        res.end("Not Found");
        return;
    }
    const ext = path.extname(filePath).toLowerCase();
    const contentType = mimeTypes[ext] ?? "application/octet-stream";
    const content = fs.readFileSync(filePath);
    res.writeHead(200, { "Content-Type": contentType });
    res.end(content);
}

// ---- API Handler ----

async function handleApi(req: http.IncomingMessage, res: http.ServerResponse, apiPath: string): Promise<void> {
    // api/test
    if (apiPath === "test") {
        jsonResponse(res, 200, { message: "Hello, world!" });
        return;
    }

    // api/config
    if (apiPath === "config") {
        await apiConfig(req, res, repoRoot);
        return;
    }

    // api/stop
    if (apiPath === "stop") {
        await apiStop(req, res, server);
        return;
    }

    // api/token
    if (apiPath === "token") {
        await apiToken(req, res);
        return;
    }

    // api/copilot/models
    if (apiPath === "copilot/models") {
        await apiCopilotModels(req, res);
        return;
    }

    // api/copilot/test/installJobsEntry (only in test mode)
    if (apiPath === "copilot/test/installJobsEntry" && testMode) {
        const body = await readBody(req);
        const entryFilePath = body.trim();

        // Check if file is in the test folder
        const testFolder = path.resolve(__dirname, "..", "test");
        const resolvedPath = path.resolve(entryFilePath);
        if (!resolvedPath.startsWith(testFolder + path.sep) && resolvedPath !== testFolder) {
            jsonResponse(res, 200, { result: "InvalidatePath", error: `File is not in the test folder: ${resolvedPath}` });
            return;
        }

        try {
            const fileContent = fs.readFileSync(resolvedPath, "utf-8");
            const entryData = JSON.parse(fileContent);
            try {
                const validatedEntry = validateEntry(entryData, "testEntry");
                try {
                    await installJobsEntry(validatedEntry);
                    jsonResponse(res, 200, { result: "OK" });
                } catch (err) {
                    jsonResponse(res, 200, { result: "Rejected", error: String(err instanceof Error ? err.message : err) });
                }
            } catch (err) {
                jsonResponse(res, 200, { result: "InvalidateEntry", error: String(err instanceof Error ? err.message : err) });
            }
        } catch (err) {
            jsonResponse(res, 200, { result: "InvalidatePath", error: String(err instanceof Error ? err.message : err) });
        }
        return;
    }

    // api/copilot/session/start/{model-id}
    const startMatch = apiPath.match(/^copilot\/session\/start\/(.+)$/);
    if (startMatch) {
        await apiCopilotSessionStart(req, res, startMatch[1]);
        return;
    }

    // api/copilot/session/{session-id}/stop
    const stopMatch = apiPath.match(/^copilot\/session\/([^\/]+)\/stop$/);
    if (stopMatch) {
        await apiCopilotSessionStop(req, res, stopMatch[1]);
        return;
    }

    // api/copilot/session/{session-id}/query
    const queryMatch = apiPath.match(/^copilot\/session\/([^\/]+)\/query$/);
    if (queryMatch) {
        await apiCopilotSessionQuery(req, res, queryMatch[1]);
        return;
    }

    // api/copilot/session/{session-id}/live/{token}
    const liveMatch = apiPath.match(/^copilot\/session\/([^\/]+)\/live\/([^\/]+)$/);
    if (liveMatch) {
        await apiCopilotSessionLive(req, res, liveMatch[1], liveMatch[2]);
        return;
    }

    // api/copilot/task (list all tasks)
    if (apiPath === "copilot/task") {
        await apiTaskList(ensureInstalledEntry(), req, res);
        return;
    }

    // api/copilot/task/start/{task-name}/session/{session-id}
    const taskStartMatch = apiPath.match(/^copilot\/task\/start\/([^\/]+)\/session\/([^\/]+)$/);
    if (taskStartMatch) {
        await apiTaskStart(ensureInstalledEntry(), req, res, taskStartMatch[1], taskStartMatch[2]);
        return;
    }

    // api/copilot/task/{task-id}/stop
    const taskStopMatch = apiPath.match(/^copilot\/task\/([^\/]+)\/stop$/);
    if (taskStopMatch) {
        await apiTaskStop(req, res, taskStopMatch[1]);
        return;
    }

    // api/copilot/task/{task-id}/live/{token}
    const taskLiveMatch = apiPath.match(/^copilot\/task\/([^\/]+)\/live\/([^\/]+)$/);
    if (taskLiveMatch) {
        await apiTaskLive(req, res, taskLiveMatch[1], taskLiveMatch[2]);
        return;
    }

    // api/copilot/job (list all jobs)
    if (apiPath === "copilot/job") {
        await apiJobList(ensureInstalledEntry(), req, res);
        return;
    }

    // api/copilot/job/running
    if (apiPath === "copilot/job/running") {
        await apiJobRunning(req, res);
        return;
    }

    // api/copilot/job/start/{job-name}
    const jobStartMatch = apiPath.match(/^copilot\/job\/start\/([^\/]+)$/);
    if (jobStartMatch) {
        await apiJobStart(ensureInstalledEntry(), req, res, jobStartMatch[1]);
        return;
    }

    // api/copilot/job/{job-id}/status
    const jobStatusMatch = apiPath.match(/^copilot\/job\/([^\/]+)\/status$/);
    if (jobStatusMatch) {
        await apiJobStatus(req, res, jobStatusMatch[1]);
        return;
    }

    // api/copilot/job/{job-id}/stop
    const jobStopMatch = apiPath.match(/^copilot\/job\/([^\/]+)\/stop$/);
    if (jobStopMatch) {
        await apiJobStop(req, res, jobStopMatch[1]);
        return;
    }

    // api/copilot/job/{job-id}/live/{token}
    const jobLiveMatch = apiPath.match(/^copilot\/job\/([^\/]+)\/live\/([^\/]+)$/);
    if (jobLiveMatch) {
        await apiJobLive(req, res, jobLiveMatch[1], jobLiveMatch[2]);
        return;
    }

    jsonResponse(res, 404, { error: "Unknown API endpoint" });
}

const server = http.createServer((req, res) => {
    const url = new URL(req.url ?? "/", `http://localhost:${port}`);
    let pathname = url.pathname;

    // API routes
    if (pathname.startsWith("/api/")) {
        const apiPath = pathname.slice("/api/".length);
        handleApi(req, res, apiPath).catch((err) => {
            console.error("API error:", err);
            jsonResponse(res, 500, { error: String(err) });
        });
        return;
    }

    // Website routes
    if (pathname === "/") {
        pathname = "/index.html";
    }

    const filePath = path.join(assetsDir, pathname);
    serveStaticFile(res, filePath);
});

// Install the jobs entry (only if not in test mode)
if (!testMode) {
    installJobsEntry(entry);
}

server.listen(port, () => {
    console.log(`http://localhost:${port}`);
    console.log(`http://localhost:${port}/api/stop`);
});

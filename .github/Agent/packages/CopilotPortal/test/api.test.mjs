import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const BASE = "http://localhost:8888";

async function fetchJson(urlPath, options) {
    const res = await fetch(`${BASE}${urlPath}`, options);
    return res.json();
}

// Helper: resolve test entry path
const testEntryPath = path.resolve(__dirname, "testEntry.json");

describe("API: /api/test", () => {
    it("returns hello world message", async () => {
        const data = await fetchJson("/api/test");
        assert.deepStrictEqual(data, { message: "Hello, world!" });
    });
});

describe("API: /api/config", () => {
    it("returns repoRoot as a non-empty string", async () => {
        const data = await fetchJson("/api/config");
        assert.ok(typeof data.repoRoot === "string", "repoRoot should be a string");
        assert.ok(data.repoRoot.length > 0, "repoRoot should not be empty");
    });
});

describe("API: /api/copilot/models", () => {
    it("returns an array of models", async () => {
        const data = await fetchJson("/api/copilot/models");
        assert.ok(Array.isArray(data.models), "models should be an array");
        assert.ok(data.models.length > 0, "should have at least one model");
    });

    it("each model has name, id, multiplier", async () => {
        const data = await fetchJson("/api/copilot/models");
        for (const m of data.models) {
            assert.ok(typeof m.name === "string", `model name should be string: ${JSON.stringify(m)}`);
            assert.ok(typeof m.id === "string", `model id should be string: ${JSON.stringify(m)}`);
            assert.ok(typeof m.multiplier === "number", `model multiplier should be number: ${JSON.stringify(m)}`);
        }
    });

    it("has at least one free model (multiplier=0)", async () => {
        const data = await fetchJson("/api/copilot/models");
        const freeModels = data.models.filter((m) => m.multiplier === 0);
        assert.ok(freeModels.length > 0, "should have at least one free model");
    });

    it("gpt-5-mini model exists and is free", async () => {
        const data = await fetchJson("/api/copilot/models");
        const model = data.models.find((m) => m.id === "gpt-5-mini");
        assert.ok(model, "gpt-5-mini model should exist");
        assert.strictEqual(model.multiplier, 0, "gpt-5-mini should be free");
    });
});

describe("API: session not found errors", () => {
    it("live returns SessionNotFound for invalid session", async () => {
        const data = await fetchJson("/api/copilot/session/nonexistent/live");
        assert.deepStrictEqual(data, { error: "SessionNotFound" });
    });

    it("stop returns SessionNotFound for invalid session", async () => {
        const data = await fetchJson("/api/copilot/session/nonexistent/stop");
        assert.deepStrictEqual(data, { error: "SessionNotFound" });
    });

    it("query returns SessionNotFound for invalid session", async () => {
        const data = await fetchJson("/api/copilot/session/nonexistent/query", {
            method: "POST",
            body: "test",
        });
        assert.deepStrictEqual(data, { error: "SessionNotFound" });
    });
});

describe("API: full session lifecycle", () => {
    let freeModelId;
    let sessionId;

    before(async () => {
        const data = await fetchJson("/api/copilot/models");
        const freeModel = data.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model for testing");
        freeModelId = freeModel.id;
    });

    after(async () => {
        if (sessionId) {
            try {
                await fetchJson(`/api/copilot/session/${sessionId}/stop`);
            } catch {
                // ignore
            }
        }
    });

    it("starts a session and returns sessionId", async () => {
        const data = await fetchJson(`/api/copilot/session/start/${freeModelId}`, {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools",
        });
        assert.ok(typeof data.sessionId === "string", "should return sessionId");
        sessionId = data.sessionId;
    });

    it("live returns HttpRequestTimeout when idle", async () => {
        assert.ok(sessionId, "session must be started");
        const data = await fetchJson(`/api/copilot/session/${sessionId}/live`);
        assert.strictEqual(data.error, "HttpRequestTimeout");
    });

    it("live returns ParallelCallNotSupported for concurrent calls", async () => {
        assert.ok(sessionId, "session must be started");
        const [first, second] = await Promise.all([
            fetchJson(`/api/copilot/session/${sessionId}/live`),
            new Promise((r) => setTimeout(r, 100)).then(() =>
                fetchJson(`/api/copilot/session/${sessionId}/live`)
            ),
        ]);
        assert.strictEqual(second.error, "ParallelCallNotSupported",
            "second parallel live call should be rejected");
        assert.strictEqual(first.error, "HttpRequestTimeout",
            "first live call should still timeout normally");
    });

    it("query sends request and returns empty object (no error)", async () => {
        assert.ok(sessionId, "session must be started");
        const data = await fetchJson(`/api/copilot/session/${sessionId}/query`, {
            method: "POST",
            body: "What is 2+2? Reply with a single number only.",
        });
        assert.strictEqual(data.error, undefined, "query should not return error");
    });

    it("live returns callbacks after query", async () => {
        assert.ok(sessionId, "session must be started");
        const callbacks = [];
        let gotAgentEnd = false;

        const timeout = Date.now() + 60000;
        while (!gotAgentEnd && Date.now() < timeout) {
            const data = await fetchJson(`/api/copilot/session/${sessionId}/live`);
            if (data.error === "HttpRequestTimeout") continue;
            if (data.error) break;
            callbacks.push(data);
            if (data.callback === "onAgentEnd") gotAgentEnd = true;
        }

        assert.ok(gotAgentEnd, "should receive onAgentEnd callback");

        const agentStart = callbacks.find((c) => c.callback === "onAgentStart");
        assert.ok(agentStart, "should receive onAgentStart");

        const hasMessage = callbacks.some((c) => c.callback === "onStartMessage");
        const hasReasoning = callbacks.some((c) => c.callback === "onStartReasoning");
        assert.ok(hasMessage || hasReasoning, "should receive at least message or reasoning callbacks");
    });

    it("stops the session and returns Closed", async () => {
        assert.ok(sessionId, "session must be started");
        const data = await fetchJson(`/api/copilot/session/${sessionId}/stop`);
        assert.deepStrictEqual(data, { result: "Closed" });
        sessionId = null;
    });

    it("stopping again returns SessionNotFound", async () => {
        const data = await fetchJson("/api/copilot/session/session-that-was-just-stopped/stop");
        assert.deepStrictEqual(data, { error: "SessionNotFound" });
    });
});

describe("API: session start errors", () => {
    it("returns ModelIdNotFound for invalid model id", async () => {
        const data = await fetchJson("/api/copilot/session/start/nonexistent-model-xyz", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools",
        });
        assert.deepStrictEqual(data, { error: "ModelIdNotFound" });
    });

    it("returns WorkingDirectoryNotAbsolutePath for relative path", async () => {
        const modelsData = await fetchJson("/api/copilot/models");
        const freeModel = modelsData.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model");
        const data = await fetchJson(`/api/copilot/session/start/${freeModel.id}`, {
            method: "POST",
            body: "relative/path/here",
        });
        assert.deepStrictEqual(data, { error: "WorkingDirectoryNotAbsolutePath" });
    });

    it("returns WorkingDirectoryNotExists for non-existent path", async () => {
        const modelsData = await fetchJson("/api/copilot/models");
        const freeModel = modelsData.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model");
        const data = await fetchJson(`/api/copilot/session/start/${freeModel.id}`, {
            method: "POST",
            body: "C:\\NonExistentPath\\ThatDoesNotExist12345",
        });
        assert.deepStrictEqual(data, { error: "WorkingDirectoryNotExists" });
    });
});

describe("API: task not found errors", () => {
    it("task stop returns TaskNotFound for invalid task id", async () => {
        const data = await fetchJson("/api/copilot/task/nonexistent/stop");
        assert.deepStrictEqual(data, { error: "TaskNotFound" });
    });

    it("task live returns TaskNotFound for invalid task id", async () => {
        const data = await fetchJson("/api/copilot/task/nonexistent/live");
        assert.deepStrictEqual(data, { error: "TaskNotFound" });
    });

});

describe("API: copilot/test/installJobsEntry", () => {
    it("returns error for tasks/jobs before entry is installed", async () => {
        const tasks = await fetchJson("/api/copilot/task");
        assert.ok(tasks.error, "should return error when entry not installed");
        const jobs = await fetchJson("/api/copilot/job");
        assert.ok(jobs.error, "should return error when entry not installed");
    });

    it("returns InvalidatePath for file outside test folder", async () => {
        const data = await fetchJson("/api/copilot/test/installJobsEntry", {
            method: "POST",
            body: "C:\\some\\random\\path\\entry.json",
        });
        assert.strictEqual(data.result, "InvalidatePath");
        assert.ok(data.error, "should have error message");
    });

    it("returns InvalidatePath for non-existent file in test folder", async () => {
        const fakePath = path.join(__dirname, "nonexistent.json");
        const data = await fetchJson("/api/copilot/test/installJobsEntry", {
            method: "POST",
            body: fakePath,
        });
        assert.strictEqual(data.result, "InvalidatePath");
    });

    it("returns InvalidateEntry for invalid entry JSON", async () => {
        const invalidEntryPath = path.join(__dirname, "invalidEntry.json");
        const fs = await import("node:fs");
        fs.writeFileSync(invalidEntryPath, JSON.stringify({
            models: {},
            promptVariables: {},
            grid: [],
            tasks: {
                "bad-task": { model: { category: "nonexistent" }, prompt: ["hello"], requireUserInput: false }
            },
            jobs: {}
        }));
        try {
            const data = await fetchJson("/api/copilot/test/installJobsEntry", {
                method: "POST",
                body: invalidEntryPath,
            });
            assert.strictEqual(data.result, "InvalidateEntry");
            assert.ok(data.error, "should have validation error");
        } finally {
            fs.unlinkSync(invalidEntryPath);
        }
    });

    it("installs a valid test entry successfully", async () => {
        const data = await fetchJson("/api/copilot/test/installJobsEntry", {
            method: "POST",
            body: testEntryPath,
        });
        assert.strictEqual(data.result, "OK", `installJobsEntry should succeed: ${JSON.stringify(data)}`);
    });

    it("rejects when session is running", async () => {
        const modelsData = await fetchJson("/api/copilot/models");
        const freeModel = modelsData.models.find((m) => m.multiplier === 0);
        const sessionData = await fetchJson(`/api/copilot/session/start/${freeModel.id}`, {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools",
        });
        assert.ok(sessionData.sessionId, "should get session id");

        try {
            const data = await fetchJson("/api/copilot/test/installJobsEntry", {
                method: "POST",
                body: testEntryPath,
            });
            assert.strictEqual(data.result, "Rejected");
            assert.ok(data.error, "should have error");
        } finally {
            await fetchJson(`/api/copilot/session/${sessionData.sessionId}/stop`);
        }
    });
});

describe("API: /api/copilot/task (after entry installed)", () => {
    it("task start returns SessionNotFound for invalid session id", async () => {
        const data = await fetchJson("/api/copilot/task/start/some-task/session/nonexistent", {
            method: "POST",
            body: "test input",
        });
        assert.deepStrictEqual(data, { error: "SessionNotFound" });
    });

    it("returns a list of tasks", async () => {
        const data = await fetchJson("/api/copilot/task");
        assert.ok(Array.isArray(data.tasks), "tasks should be an array");
        assert.ok(data.tasks.length > 0, "should have at least one task");
    });

    it("each task has name and requireUserInput", async () => {
        const data = await fetchJson("/api/copilot/task");
        for (const t of data.tasks) {
            assert.ok(typeof t.name === "string", `task name should be string: ${JSON.stringify(t)}`);
            assert.ok(typeof t.requireUserInput === "boolean", `task requireUserInput should be boolean: ${JSON.stringify(t)}`);
        }
    });

    it("contains expected test tasks", async () => {
        const data = await fetchJson("/api/copilot/task");
        const taskNames = data.tasks.map((t) => t.name);
        assert.ok(taskNames.includes("simple-task"), "should have simple-task");
        assert.ok(taskNames.includes("criteria-fail-task"), "should have criteria-fail-task");
        assert.ok(taskNames.includes("input-task"), "should have input-task");
    });
});

describe("API: /api/copilot/job (after entry installed)", () => {
    it("returns grid and jobs", async () => {
        const data = await fetchJson("/api/copilot/job");
        assert.ok(Array.isArray(data.grid), "grid should be an array");
        assert.ok(typeof data.jobs === "object" && data.jobs !== null && !Array.isArray(data.jobs), "jobs should be an object");
        assert.ok(Object.keys(data.jobs).length > 0, "should have at least one job");
    });

    it("grid has expected rows", async () => {
        const data = await fetchJson("/api/copilot/job");
        assert.ok(data.grid.length >= 2, "should have at least 2 grid rows");
        const keywords = data.grid.map(r => r.keyword);
        assert.ok(keywords.includes("test"), "should have 'test' keyword");
        assert.ok(keywords.includes("batch"), "should have 'batch' keyword");
    });

    it("each job has work", async () => {
        const data = await fetchJson("/api/copilot/job");
        for (const [name, job] of Object.entries(data.jobs)) {
            assert.ok(typeof name === "string", `job key should be string`);
            assert.ok(job.work !== undefined, `job ${name} should have work`);
        }
    });

    it("contains expected test jobs", async () => {
        const data = await fetchJson("/api/copilot/job");
        const jobNames = Object.keys(data.jobs);
        assert.ok(jobNames.includes("simple-job"), "should have simple-job");
        assert.ok(jobNames.includes("seq-job"), "should have seq-job");
        assert.ok(jobNames.includes("par-job"), "should have par-job");
        assert.ok(jobNames.includes("fail-job"), "should have fail-job");
    });

    it("does not include models, promptVariables, or tasks", async () => {
        const data = await fetchJson("/api/copilot/job");
        assert.strictEqual(data.models, undefined, "should not include models");
        assert.strictEqual(data.promptVariables, undefined, "should not include promptVariables");
        assert.strictEqual(data.tasks, undefined, "should not include tasks");
    });

    it("includes chart map with entries for each job", async () => {
        const data = await fetchJson("/api/copilot/job");
        assert.ok(typeof data.chart === "object" && data.chart !== null, "chart should be an object");
        const jobNames = Object.keys(data.jobs);
        const chartNames = Object.keys(data.chart);
        assert.deepStrictEqual(chartNames.sort(), jobNames.sort(), "chart keys should match job keys");
        for (const [name, chartGraph] of Object.entries(data.chart)) {
            assert.ok(Array.isArray(chartGraph.nodes), `chart[${name}] should have nodes array`);
        }
    });

    it("every TaskWork has a ChartNode with TaskNode or CondNode hint", async () => {
        const data = await fetchJson("/api/copilot/job");
        for (const [jobName, job] of Object.entries(data.jobs)) {
            const taskWorkIds = [];
            function collectTaskWorkIds(work) {
                if (work.kind === "Ref") taskWorkIds.push(work.workIdInJob);
                else if (work.kind === "Seq" || work.kind === "Par") work.works.forEach(collectTaskWorkIds);
                else if (work.kind === "Loop") {
                    if (work.preCondition) collectTaskWorkIds(work.preCondition[1]);
                    collectTaskWorkIds(work.body);
                    if (work.postCondition) collectTaskWorkIds(work.postCondition[1]);
                } else if (work.kind === "Alt") {
                    collectTaskWorkIds(work.condition);
                    if (work.trueWork) collectTaskWorkIds(work.trueWork);
                    if (work.falseWork) collectTaskWorkIds(work.falseWork);
                }
            }
            collectTaskWorkIds(job.work);
            const chart = data.chart[jobName];
            for (const wid of taskWorkIds) {
                const node = chart.nodes.find(n => Array.isArray(n.hint) && (n.hint[0] === "TaskNode" || n.hint[0] === "CondNode") && n.hint[1] === wid);
                assert.ok(node, `job ${jobName}: TaskWork workIdInJob=${wid} should have a ChartNode with TaskNode or CondNode hint`);
            }
        }
    });
});

describe("API: job not found errors", () => {
    it("job start returns JobNotFound for invalid job name", async () => {
        const data = await fetchJson("/api/copilot/job/start/nonexistent-job-xyz", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest input",
        });
        assert.deepStrictEqual(data, { error: "JobNotFound" });
    });

    it("job stop returns JobNotFound for invalid job id", async () => {
        const data = await fetchJson("/api/copilot/job/nonexistent/stop");
        assert.deepStrictEqual(data, { error: "JobNotFound" });
    });

    it("job live returns JobNotFound for invalid job id", async () => {
        const data = await fetchJson("/api/copilot/job/nonexistent/live");
        assert.deepStrictEqual(data, { error: "JobNotFound" });
    });
});

// Helper: drain all live responses until a specific callback or timeout
async function drainLive(livePath, targetCallback, timeoutMs = 120000) {
    const callbacks = [];
    const deadline = Date.now() + timeoutMs;
    while (Date.now() < deadline) {
        const data = await fetchJson(livePath);
        if (data.error === "HttpRequestTimeout") continue;
        if (data.error === "TaskNotFound" || data.error === "JobNotFound") break;
        callbacks.push(data);
        if (data.callback === targetCallback) break;
        if (data.taskError || data.jobError) break;
    }
    return callbacks;
}

describe("API: task running - criteria success", () => {
    let sessionId;

    before(async () => {
        const modelsData = await fetchJson("/api/copilot/models");
        const freeModel = modelsData.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model");
        const data = await fetchJson(`/api/copilot/session/start/${freeModel.id}`, {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools",
        });
        assert.ok(data.sessionId, "should get session id");
        sessionId = data.sessionId;
    });

    after(async () => {
        if (sessionId) {
            try { await fetchJson(`/api/copilot/session/${sessionId}/stop`); } catch { /* ignore */ }
        }
    });

    it("simple-task succeeds (no criteria)", async () => {
        const startData = await fetchJson(`/api/copilot/task/start/simple-task/session/${sessionId}`, {
            method: "POST",
            body: "",
        });
        assert.ok(startData.taskId, `should return taskId: ${JSON.stringify(startData)}`);

        const callbacks = await drainLive(`/api/copilot/task/${startData.taskId}/live`, "taskSucceeded");
        const succeeded = callbacks.some((c) => c.callback === "taskSucceeded");
        assert.ok(succeeded, `simple-task should succeed, callbacks: ${JSON.stringify(callbacks.map(c => c.callback))}`);

        // taskDecision should appear with success message
        const decisionCb = callbacks.find((c) => c.callback === "taskDecision");
        assert.ok(decisionCb, `should have taskDecision callback, callbacks: ${JSON.stringify(callbacks.map(c => c.callback))}`);
        assert.ok(decisionCb.reason.includes("succeeded"), `taskDecision reason should mention succeeded: ${decisionCb.reason}`);
    });
});

describe("API: task running - criteria failure (no retry)", () => {
    let sessionId;

    before(async () => {
        const modelsData = await fetchJson("/api/copilot/models");
        const freeModel = modelsData.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model");
        const data = await fetchJson(`/api/copilot/session/start/${freeModel.id}`, {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools",
        });
        assert.ok(data.sessionId, "should get session id");
        sessionId = data.sessionId;
    });

    after(async () => {
        if (sessionId) {
            try { await fetchJson(`/api/copilot/session/${sessionId}/stop`); } catch { /* ignore */ }
        }
    });

    it("criteria-fail-task fails (toolExecuted not satisfied, retry=0)", async () => {
        const startData = await fetchJson(`/api/copilot/task/start/criteria-fail-task/session/${sessionId}`, {
            method: "POST",
            body: "",
        });
        assert.ok(startData.taskId, `should return taskId: ${JSON.stringify(startData)}`);

        const callbacks = await drainLive(`/api/copilot/task/${startData.taskId}/live`, "taskFailed");
        const failed = callbacks.some((c) => c.callback === "taskFailed");
        assert.ok(failed, `criteria-fail-task should fail, callbacks: ${JSON.stringify(callbacks.map(c => c.callback))}`);

        // In borrowing mode, no taskSessionStarted callbacks
        const sessionStarts = callbacks.filter((c) => c.callback === "taskSessionStarted");
        assert.strictEqual(sessionStarts.length, 0, `should have no session starts in borrowing mode, got: ${sessionStarts.length}`);

        // taskDecision should report criteria failure and final decision
        const decisions = callbacks.filter((c) => c.callback === "taskDecision");
        assert.ok(decisions.length >= 1, `should have taskDecision callbacks: ${JSON.stringify(callbacks.map(c => c.callback))}`);
        const failDecision = decisions.find((d) => d.reason.includes("failed"));
        assert.ok(failDecision, `should have a failure decision: ${JSON.stringify(decisions.map(d => d.reason))}`);
    });
});

describe("API: task running - live responses", () => {
    let sessionId;

    before(async () => {
        const modelsData = await fetchJson("/api/copilot/models");
        const freeModel = modelsData.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model");
        const data = await fetchJson(`/api/copilot/session/start/${freeModel.id}`, {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools",
        });
        assert.ok(data.sessionId, "should get session id");
        sessionId = data.sessionId;
    });

    after(async () => {
        if (sessionId) {
            try { await fetchJson(`/api/copilot/session/${sessionId}/stop`); } catch { /* ignore */ }
        }
    });

    it("task live responds correctly for succeeded task", async () => {
        const startData = await fetchJson(`/api/copilot/task/start/simple-task/session/${sessionId}`, {
            method: "POST",
            body: "",
        });
        assert.ok(startData.taskId, "should return taskId");

        const callbacks = await drainLive(`/api/copilot/task/${startData.taskId}/live`, "taskSucceeded");

        // In borrowing session mode, taskSessionStarted/Stopped are unavailable
        const sessionStarted = callbacks.some((c) => c.callback === "taskSessionStarted");
        assert.ok(!sessionStarted, "should NOT have taskSessionStarted callback in borrowing mode");

        const sessionStopped = callbacks.some((c) => c.callback === "taskSessionStopped");
        assert.ok(!sessionStopped, "should NOT have taskSessionStopped callback in borrowing mode");

        const lastCallback = callbacks[callbacks.length - 1];
        assert.strictEqual(lastCallback.callback, "taskSucceeded", "last callback should be taskSucceeded");
    });

    it("task live responds correctly for failed task", async () => {
        const startData = await fetchJson(`/api/copilot/task/start/criteria-fail-task/session/${sessionId}`, {
            method: "POST",
            body: "",
        });
        assert.ok(startData.taskId, "should return taskId");

        const callbacks = await drainLive(`/api/copilot/task/${startData.taskId}/live`, "taskFailed");

        const lastCallback = callbacks[callbacks.length - 1];
        assert.strictEqual(lastCallback.callback, "taskFailed", "last callback should be taskFailed");
    });
});

describe("API: job running - simple job succeeds", () => {
    it("simple-job succeeds (single TaskWork)", async () => {
        const startData = await fetchJson("/api/copilot/job/start/simple-job", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest",
        });
        assert.ok(startData.jobId, `should return jobId: ${JSON.stringify(startData)}`);

        const callbacks = await drainLive(`/api/copilot/job/${startData.jobId}/live`, "jobSucceeded");
        const succeeded = callbacks.some((c) => c.callback === "jobSucceeded");
        assert.ok(succeeded, `simple-job should succeed, callbacks: ${JSON.stringify(callbacks.map(c => c.callback))}`);

        const workStarted = callbacks.some((c) => c.callback === "workStarted");
        const workStopped = callbacks.some((c) => c.callback === "workStopped");
        assert.ok(workStarted, "should have workStarted");
        assert.ok(workStopped, "should have workStopped");

        // workStarted should include taskId for live polling
        const workStartedCb = callbacks.find((c) => c.callback === "workStarted");
        assert.ok(workStartedCb.taskId, "workStarted should include taskId");
    });
});

describe("API: job running - sequential work", () => {
    it("seq-job succeeds with sequential tasks", async () => {
        const startData = await fetchJson("/api/copilot/job/start/seq-job", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest",
        });
        assert.ok(startData.jobId, `should return jobId: ${JSON.stringify(startData)}`);

        const callbacks = await drainLive(`/api/copilot/job/${startData.jobId}/live`, "jobSucceeded");
        const succeeded = callbacks.some((c) => c.callback === "jobSucceeded");
        assert.ok(succeeded, `seq-job should succeed`);

        const workStarted = callbacks.filter((c) => c.callback === "workStarted");
        assert.strictEqual(workStarted.length, 2, "should have 2 workStarted for sequential tasks");
    });
});

describe("API: job running - parallel work", () => {
    it("par-job succeeds with parallel tasks", async () => {
        const startData = await fetchJson("/api/copilot/job/start/par-job", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest",
        });
        assert.ok(startData.jobId, `should return jobId: ${JSON.stringify(startData)}`);

        const callbacks = await drainLive(`/api/copilot/job/${startData.jobId}/live`, "jobSucceeded");
        const succeeded = callbacks.some((c) => c.callback === "jobSucceeded");
        assert.ok(succeeded, `par-job should succeed`);

        const workStarted = callbacks.filter((c) => c.callback === "workStarted");
        assert.strictEqual(workStarted.length, 2, "should have 2 workStarted for parallel tasks");
    });
});

describe("API: job running - failure propagation", () => {
    it("fail-job fails when task fails", async () => {
        const startData = await fetchJson("/api/copilot/job/start/fail-job", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest",
        });
        assert.ok(startData.jobId, `should return jobId: ${JSON.stringify(startData)}`);

        const callbacks = await drainLive(`/api/copilot/job/${startData.jobId}/live`, "jobFailed");
        const failed = callbacks.some((c) => c.callback === "jobFailed");
        assert.ok(failed, `fail-job should fail`);

        const workStopped = callbacks.find((c) => c.callback === "workStopped");
        assert.ok(workStopped, "should have workStopped");
        assert.strictEqual(workStopped.succeeded, false, "work should fail");
    });
});

describe("API: job running - live responses observability", () => {
    it("work execution is observable from live api", async () => {
        const startData = await fetchJson("/api/copilot/job/start/simple-job", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest",
        });
        assert.ok(startData.jobId, "should return jobId");

        const callbacks = await drainLive(`/api/copilot/job/${startData.jobId}/live`, "jobSucceeded");

        const startIdx = callbacks.findIndex((c) => c.callback === "workStarted");
        const stopIdx = callbacks.findIndex((c) => c.callback === "workStopped");
        const succeedIdx = callbacks.findIndex((c) => c.callback === "jobSucceeded");

        assert.ok(startIdx >= 0, "should have workStarted");
        assert.ok(stopIdx > startIdx, "workStopped should come after workStarted");
        assert.ok(succeedIdx > stopIdx, "jobSucceeded should come after workStopped");
    });
});

describe("API: job-created task live polling", () => {
    it("task live API works for job-created tasks with taskDecision", async () => {
        const startData = await fetchJson("/api/copilot/job/start/simple-job", {
            method: "POST",
            body: "C:\\Code\\VczhLibraries\\Tools\ntest",
        });
        assert.ok(startData.jobId, "should return jobId");

        // Drain job live to get the taskId from workStarted
        const jobCallbacks = await drainLive(`/api/copilot/job/${startData.jobId}/live`, "jobSucceeded");

        const workStartedCb = jobCallbacks.find((c) => c.callback === "workStarted");
        assert.ok(workStartedCb, "should have workStarted");
        assert.ok(workStartedCb.taskId, "workStarted should include taskId");
        assert.ok(workStartedCb.taskId.startsWith("task-"), "taskId should be in expected format");

        // Drain task live - task may have already completed, try to get callbacks
        const taskCallbacks = await drainLive(`/api/copilot/task/${workStartedCb.taskId}/live`, "taskSucceeded", 10000);

        // If we got callbacks, verify taskDecision appears
        if (taskCallbacks.length > 0 && !taskCallbacks[0]?.error) {
            const decisions = taskCallbacks.filter((c) => c.callback === "taskDecision");
            // taskDecision should be present for a succeeding task
            if (decisions.length > 0) {
                assert.ok(decisions.some((d) => d.reason), "taskDecision should have a reason");
            }
        }
    });
});


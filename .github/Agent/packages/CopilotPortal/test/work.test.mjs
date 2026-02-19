import { describe, it, before } from "node:test";
import assert from "node:assert/strict";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const BASE = "http://localhost:8888";

async function fetchJson(urlPath, options) {
    const res = await fetch(`${BASE}${urlPath}`, options);
    return res.json();
}

// Drain live responses until a specific callback or timeout
async function drainLive(livePath, targetCallback, timeoutMs = 120000) {
    const callbacks = [];
    const deadline = Date.now() + timeoutMs;
    while (Date.now() < deadline) {
        const data = await fetchJson(livePath);
        if (data.error === "HttpRequestTimeout") continue;
        if (data.error === "JobNotFound") break;
        callbacks.push(data);
        if (data.callback === targetCallback) break;
        if (data.jobError) break;
    }
    return callbacks;
}

// Start a job and drain its live responses until completion
async function runJob(jobName) {
    const startData = await fetchJson(`/api/copilot/job/start/${jobName}`, {
        method: "POST",
        body: "C:\\Code\\VczhLibraries\\Tools\ntest",
    });
    assert.ok(startData.jobId, `should return jobId for ${jobName}: ${JSON.stringify(startData)}`);

    // Drain until jobSucceeded or jobFailed
    const callbacks = [];
    const deadline = Date.now() + 120000;
    while (Date.now() < deadline) {
        const data = await fetchJson(`/api/copilot/job/${startData.jobId}/live`);
        if (data.error === "HttpRequestTimeout") continue;
        if (data.error === "JobNotFound") break;
        callbacks.push(data);
        if (data.callback === "jobSucceeded" || data.callback === "jobFailed") break;
        if (data.jobError) break;
    }
    return { jobId: startData.jobId, callbacks };
}

// Extract workStarted/workStopped info from callbacks
function getWorkEvents(callbacks) {
    const started = callbacks
        .filter(c => c.callback === "workStarted")
        .map(c => c.workId);
    const stopped = callbacks
        .filter(c => c.callback === "workStopped")
        .map(c => ({ workId: c.workId, succeeded: c.succeeded }));
    return { started, stopped };
}

// Look up workIdInJob values from the /api/copilot/job endpoint for a given job
async function getJobWorkIds(jobName) {
    const jobsData = await fetchJson("/api/copilot/job");
    const job = jobsData.jobs[jobName];
    assert.ok(job, `job ${jobName} should exist`);

    // Collect all TaskWork workIdInJob values from the work tree
    const workIds = [];
    function collectWorkIds(work) {
        if (work.kind === "Ref") {
            workIds.push({ workId: work.workIdInJob, taskId: work.taskId });
        } else if (work.kind === "Seq" || work.kind === "Par") {
            for (const w of work.works) collectWorkIds(w);
        } else if (work.kind === "Loop") {
            if (work.preCondition) collectWorkIds(work.preCondition[1]);
            collectWorkIds(work.body);
            if (work.postCondition) collectWorkIds(work.postCondition[1]);
        } else if (work.kind === "Alt") {
            collectWorkIds(work.condition);
            if (work.trueWork) collectWorkIds(work.trueWork);
            if (work.falseWork) collectWorkIds(work.falseWork);
        }
    }
    collectWorkIds(job.work);
    return workIds;
}

// ============================================================
// TaskWork
// ============================================================

describe("Work: TaskWork - succeeds", () => {
    it("simple-job succeeds and workStarted/workStopped are observed", async () => {
        const { callbacks } = await runJob("simple-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 1, "should have 1 workStarted");
        assert.strictEqual(stopped.length, 1, "should have 1 workStopped");
        assert.strictEqual(stopped[0].succeeded, true, "work should succeed");
    });
});

describe("Work: TaskWork - fails", () => {
    it("fail-job fails when task fails", async () => {
        const { callbacks } = await runJob("fail-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail");

        const { stopped } = getWorkEvents(callbacks);
        assert.strictEqual(stopped.length, 1, "should have 1 workStopped");
        assert.strictEqual(stopped[0].succeeded, false, "work should fail");
    });
});

// ============================================================
// SequentialWork
// ============================================================

describe("Work: SequentialWork - all succeed", () => {
    it("seq-job succeeds with all sequential tasks", async () => {
        const { callbacks } = await runJob("seq-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 2, "should have 2 workStarted");
        assert.strictEqual(stopped.length, 2, "should have 2 workStopped");
        assert.ok(stopped.every(s => s.succeeded), "all works should succeed");

        // Verify sequential order: first work stops before second starts
        const startIdx0 = callbacks.findIndex(c => c.callback === "workStarted" && c.workId === started[0]);
        const stopIdx0 = callbacks.findIndex(c => c.callback === "workStopped" && c.workId === started[0]);
        const startIdx1 = callbacks.findIndex(c => c.callback === "workStarted" && c.workId === started[1]);
        assert.ok(stopIdx0 < startIdx1, "first work should stop before second starts (sequential)");
    });
});

describe("Work: SequentialWork - first fails, second does not run", () => {
    it("seq-fail-first-job fails and second task never starts", async () => {
        const { callbacks } = await runJob("seq-fail-first-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail");

        const workIds = await getJobWorkIds("seq-fail-first-job");
        const secondWorkId = workIds[1].workId;

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 1, "only first work should start");
        assert.ok(!started.includes(secondWorkId), "second work should never start");
        assert.strictEqual(stopped.length, 1, "only one work stopped");
        assert.strictEqual(stopped[0].succeeded, false, "first work should fail");
    });
});

describe("Work: SequentialWork - first succeeds, second fails", () => {
    it("seq-fail-second-job fails at the second task", async () => {
        const { callbacks } = await runJob("seq-fail-second-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 2, "both works should start");
        assert.strictEqual(stopped.length, 2, "both works should stop");

        // First succeeded, second failed
        assert.strictEqual(stopped[0].succeeded, true, "first work should succeed");
        assert.strictEqual(stopped[1].succeeded, false, "second work should fail");
    });
});

// ============================================================
// ParallelWork
// ============================================================

describe("Work: ParallelWork - all succeed", () => {
    it("par-job succeeds with all parallel tasks", async () => {
        const { callbacks } = await runJob("par-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 2, "should have 2 workStarted");
        assert.strictEqual(stopped.length, 2, "should have 2 workStopped");
        assert.ok(stopped.every(s => s.succeeded), "all works should succeed");
    });
});

describe("Work: ParallelWork - one fails, job fails, but all run", () => {
    it("par-fail-one-job fails but both tasks run", async () => {
        const { callbacks } = await runJob("par-fail-one-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail when any parallel task fails");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 2, "both works should start (parallel)");
        assert.strictEqual(stopped.length, 2, "both works should stop (parallel waits for all)");

        const succeededCount = stopped.filter(s => s.succeeded).length;
        const failedCount = stopped.filter(s => !s.succeeded).length;
        assert.strictEqual(succeededCount, 1, "one work should succeed");
        assert.strictEqual(failedCount, 1, "one work should fail");
    });
});

// ============================================================
// AltWork
// ============================================================

describe("Work: AltWork - condition succeeds, trueWork runs", () => {
    it("alt-true-job: condition succeeds → trueWork triggers, job succeeds", async () => {
        const workIds = await getJobWorkIds("alt-true-job");
        const condWorkId = workIds.find(w => w.taskId === "simple-task" && w.workId === workIds[0].workId).workId;

        const { callbacks } = await runJob("alt-true-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed");

        const { started, stopped } = getWorkEvents(callbacks);
        // condition (workId 0) + trueWork (workId 1) = 2 tasks started
        assert.strictEqual(started.length, 2, "condition + trueWork should both start");
        assert.ok(stopped.every(s => s.succeeded), "all works should succeed");

        // The second TaskWork that starts should be trueWork (workId 1)
        const trueWorkId = workIds[1].workId;
        assert.ok(started.includes(trueWorkId), "trueWork should be triggered");
    });
});

describe("Work: AltWork - condition fails, falseWork runs", () => {
    it("alt-false-job: condition fails → falseWork triggers, job succeeds", async () => {
        const workIds = await getJobWorkIds("alt-false-job");

        const { callbacks } = await runJob("alt-false-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed");

        const { started, stopped } = getWorkEvents(callbacks);
        // condition (workId 0, fails) + falseWork (workId 1, succeeds) = 2 started
        assert.strictEqual(started.length, 2, "condition + falseWork should both start");

        const falseWorkId = workIds[1].workId;
        assert.ok(started.includes(falseWorkId), "falseWork should be triggered");

        // Condition task fails, but falseWork succeeds
        const condStopped = stopped.find(s => s.workId === workIds[0].workId);
        assert.strictEqual(condStopped.succeeded, false, "condition task should fail");
        const falseStopped = stopped.find(s => s.workId === falseWorkId);
        assert.strictEqual(falseStopped.succeeded, true, "falseWork should succeed");
    });
});

describe("Work: AltWork - condition succeeds, trueWork undefined → succeeds", () => {
    it("alt-true-undef-job: condition succeeds, no trueWork → job succeeds", async () => {
        const workIds = await getJobWorkIds("alt-true-undef-job");

        const { callbacks } = await runJob("alt-true-undef-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed when chosen work is undefined");

        const { started } = getWorkEvents(callbacks);
        // condition runs (workId 0), trueWork is undefined so falseWork (workId 1) should NOT run
        assert.strictEqual(started.length, 1, "only condition should start, chosen branch is undefined");

        const falseWorkId = workIds[1].workId;
        assert.ok(!started.includes(falseWorkId), "falseWork should not be triggered when condition succeeds");
    });
});

describe("Work: AltWork - condition fails, falseWork undefined → succeeds", () => {
    it("alt-false-undef-job: condition fails, no falseWork → job succeeds", async () => {
        const workIds = await getJobWorkIds("alt-false-undef-job");

        const { callbacks } = await runJob("alt-false-undef-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed when chosen work is undefined");

        const { started } = getWorkEvents(callbacks);
        // condition runs (workId 0, fails), falseWork is undefined so trueWork (workId 1) should NOT run
        assert.strictEqual(started.length, 1, "only condition should start, chosen branch is undefined");

        const trueWorkId = workIds[1].workId;
        assert.ok(!started.includes(trueWorkId), "trueWork should not be triggered when condition fails");
    });
});

describe("Work: AltWork - condition succeeds, trueWork fails → job fails", () => {
    it("alt-true-fail-job: condition succeeds, trueWork fails → job fails", async () => {
        const workIds = await getJobWorkIds("alt-true-fail-job");

        const { callbacks } = await runJob("alt-true-fail-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail when chosen work fails");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 2, "condition + trueWork should both start");

        const trueWorkId = workIds[1].workId;
        assert.ok(started.includes(trueWorkId), "trueWork should be triggered");

        const trueStopped = stopped.find(s => s.workId === trueWorkId);
        assert.strictEqual(trueStopped.succeeded, false, "trueWork should fail");
    });
});

describe("Work: AltWork - condition fails, falseWork fails → job fails", () => {
    it("alt-false-fail-job: condition fails, falseWork fails → job fails", async () => {
        const workIds = await getJobWorkIds("alt-false-fail-job");

        const { callbacks } = await runJob("alt-false-fail-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail when chosen work fails");

        const { started, stopped } = getWorkEvents(callbacks);
        assert.strictEqual(started.length, 2, "condition + falseWork should both start");

        const falseWorkId = workIds[1].workId;
        assert.ok(started.includes(falseWorkId), "falseWork should be triggered");

        const falseStopped = stopped.find(s => s.workId === falseWorkId);
        assert.strictEqual(falseStopped.succeeded, false, "falseWork should fail");
    });
});

// ============================================================
// LoopWork
// ============================================================

describe("Work: LoopWork - preCondition mismatch, body does not run", () => {
    it("loop-pre-exit-job: preCondition expected true but fails → body skipped, job succeeds", async () => {
        const workIds = await getJobWorkIds("loop-pre-exit-job");

        const { callbacks } = await runJob("loop-pre-exit-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed when preCondition causes exit");

        const { started } = getWorkEvents(callbacks);
        // preCondition task (workId 0) runs; body task (workId 1) should NOT run
        const bodyWorkId = workIds.find(w => w.workId !== workIds[0].workId).workId;
        assert.ok(!started.includes(bodyWorkId), "body should not start when preCondition causes exit");

        // Only condition task should have started
        assert.strictEqual(started.length, 1, "only preCondition task should start");
    });
});

describe("Work: LoopWork - preCondition matches, body runs once (no postCondition)", () => {
    it("loop-pre-enter-job: preCondition expected true and succeeds → body runs, job succeeds", async () => {
        const workIds = await getJobWorkIds("loop-pre-enter-job");

        const { callbacks } = await runJob("loop-pre-enter-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed");

        const { started } = getWorkEvents(callbacks);
        // preCondition (workId 0) + body (workId 1) = 2 started
        assert.strictEqual(started.length, 2, "preCondition + body should both start");
        assert.ok(started.includes(workIds[0].workId), "preCondition task should start");
        assert.ok(started.includes(workIds[1].workId), "body task should start");
    });
});

describe("Work: LoopWork - body fails → loop fails", () => {
    it("loop-body-fail-job: body fails → job fails", async () => {
        const workIds = await getJobWorkIds("loop-body-fail-job");

        const { callbacks } = await runJob("loop-body-fail-job");
        const failed = callbacks.some(c => c.callback === "jobFailed");
        assert.ok(failed, "job should fail when body fails");

        const { started, stopped } = getWorkEvents(callbacks);
        // preCondition (workId 0, succeeds) + body (workId 1, fails) = 2
        assert.strictEqual(started.length, 2, "preCondition + body should both start");

        const bodyStopped = stopped.find(s => s.workId === workIds[1].workId);
        assert.ok(bodyStopped, "body should have stopped");
        assert.strictEqual(bodyStopped.succeeded, false, "body should fail");
    });
});

describe("Work: LoopWork - postCondition mismatch → exit after body", () => {
    it("loop-post-exit-job: body runs, postCondition mismatch → job succeeds", async () => {
        const workIds = await getJobWorkIds("loop-post-exit-job");

        const { callbacks } = await runJob("loop-post-exit-job");
        const succeeded = callbacks.some(c => c.callback === "jobSucceeded");
        assert.ok(succeeded, "job should succeed when postCondition causes exit");

        const { started, stopped } = getWorkEvents(callbacks);
        // body (workId 0) + postCondition (workId 1, fails, expected true → mismatch → exit)
        assert.strictEqual(started.length, 2, "body + postCondition should both start");

        const bodyWorkId = workIds[0].workId;
        const bodyStopped = stopped.find(s => s.workId === bodyWorkId);
        assert.ok(bodyStopped, "body should have stopped");
        assert.strictEqual(bodyStopped.succeeded, true, "body should succeed");
    });
});

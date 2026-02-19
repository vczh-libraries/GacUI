import * as http from "node:http";
import * as path from "node:path";
import * as fs from "node:fs";
import type { ICopilotSession } from "./copilotSession.js";
import type { Entry, Work, TaskWork, SequentialWork, ParallelWork, LoopWork, AltWork } from "./jobsDef.js";
import { getModelId } from "./jobsDef.js";
import { generateChartNodes } from "./jobsChart.js";
import {
    helperGetSession,
    jsonResponse,
} from "./copilotApi.js";
import {
    readBody,
    pushResponse,
    waitForResponse,
    type LiveState,
    type LiveResponse,
} from "./sharedApi.js";
import {
    type ICopilotTask,
    type ICopilotTaskCallback,
    installedEntry,
    installJobsEntry,
    startTask,
    errorToDetailedString,
} from "./taskApi.js";

export type { ICopilotTask, ICopilotTaskCallback };
export { installJobsEntry };

// ---- Types ----

export interface ICopilotJob {
    get runningWorkIds(): number[];
    get status(): "Executing" | "Succeeded" | "Failed";
    stop(): void;
}

export interface ICopilotJobCallback {
    jobSucceeded(): void;
    jobFailed(): void;
    workStarted(workId: number, taskId: string): void;
    workStopped(workId: number, succeeded: boolean): void;
}

// ---- Task State ----

interface TaskState extends LiveState {
    taskId: string;
    task: ICopilotTask;
    taskError: string | null;
    borrowingSessionMode: boolean;
    closed: boolean;
}

const tasks = new Map<string, TaskState>();
let nextTaskId = 1;

// ---- API Handlers ----

export async function apiTaskList(
    req: http.IncomingMessage,
    res: http.ServerResponse,
): Promise<void> {
    if (!installedEntry) {
        jsonResponse(res, 200, { tasks: [] });
        return;
    }
    const taskList = Object.entries(installedEntry.tasks).map(([name, task]) => ({
        name,
        requireUserInput: task.requireUserInput,
    }));
    jsonResponse(res, 200, { tasks: taskList });
}

export async function apiTaskStart(
    req: http.IncomingMessage,
    res: http.ServerResponse,
    taskName: string,
    sessionId: string,
): Promise<void> {
    const session = helperGetSession(sessionId);
    if (!session) {
        jsonResponse(res, 200, { error: "SessionNotFound" });
        return;
    }

    const body = await readBody(req);
    const userInput = body;

    try {
        const taskId = `task-${nextTaskId++}`;
        const state: TaskState = {
            taskId,
            task: null as unknown as ICopilotTask,
            responseQueue: [],
            waitingResolve: null,
            taskError: null,
            borrowingSessionMode: true,
            closed: false,
        };

        const taskCallback: ICopilotTaskCallback = {
            taskSucceeded() {
                pushResponse(state, { callback: "taskSucceeded" });
                state.closed = true;
            },
            taskFailed() {
                pushResponse(state, { callback: "taskFailed" });
                state.closed = true;
            },
            taskDecision(reason: string) {
                pushResponse(state, { callback: "taskDecision", reason });
            },
            // Unavailable in borrowing session mode - won't be called
            taskSessionStarted(taskSession: ICopilotSession, taskId: string, isDrivingSession: boolean) {
                pushResponse(state, { callback: "taskSessionStarted", sessionId: taskId, isDriving: isDrivingSession });
            },
            taskSessionStopped(taskSession: ICopilotSession, taskId: string, succeeded: boolean) {
                pushResponse(state, { callback: "taskSessionStopped", sessionId: taskId, succeeded });
            },
        };

        const copilotTask = await startTask(taskName, userInput, session, true, taskCallback, undefined, undefined, (err: unknown) => {
            state.taskError = errorToDetailedString(err);
            pushResponse(state, { taskError: state.taskError });
            state.closed = true;
        });
        state.task = copilotTask;
        tasks.set(taskId, state);

        jsonResponse(res, 200, { taskId });
    } catch (err) {
        jsonResponse(res, 200, { taskError: errorToDetailedString(err) });
    }
}

export async function apiTaskStop(
    req: http.IncomingMessage,
    res: http.ServerResponse,
    taskId: string,
): Promise<void> {
    const state = tasks.get(taskId);
    if (!state) {
        jsonResponse(res, 200, { error: "TaskNotFound" });
        return;
    }
    if (state.borrowingSessionMode) {
        jsonResponse(res, 200, { error: "TaskCannotClose" });
        return;
    }
    state.task.stop();
    tasks.delete(taskId);
    if (state.waitingResolve) {
        const resolve = state.waitingResolve;
        state.waitingResolve = null;
        resolve({ error: "TaskNotFound" });
    }
    jsonResponse(res, 200, { result: "Closed" });
}

export async function apiTaskLive(
    req: http.IncomingMessage,
    res: http.ServerResponse,
    taskId: string,
): Promise<void> {
    const state = tasks.get(taskId);
    if (!state) {
        jsonResponse(res, 200, { error: "TaskNotFound" });
        return;
    }
    if (state.waitingResolve) {
        jsonResponse(res, 200, { error: "ParallelCallNotSupported" });
        return;
    }
    // If closed and no more responses in queue, return TaskClosed and remove
    if (state.closed && state.responseQueue.length === 0) {
        tasks.delete(taskId);
        jsonResponse(res, 200, { error: "TaskClosed" });
        return;
    }
    const response = await waitForResponse(state, 5000);
    if (response === null) {
        jsonResponse(res, 200, { error: "HttpRequestTimeout" });
    } else if (state.taskError) {
        jsonResponse(res, 200, { taskError: state.taskError });
    } else {
        jsonResponse(res, 200, response);
    }
}

// ---- Job State ----

interface JobState extends LiveState {
    jobId: string;
    job: ICopilotJob;
    jobError: string | null;
    closed: boolean;
}

const jobs = new Map<string, JobState>();
let nextJobId = 1;

// ---- executeWork ----

async function executeWork(
    entry: Entry,
    work: Work<number>,
    userInput: string,
    workingDirectory: string,
    runningIds: Set<number>,
    stopped: { readonly value: boolean },
    activeTasks: ICopilotTask[],
    callback: ICopilotJobCallback
): Promise<boolean> {
    if (stopped.value) return false;

    switch (work.kind) {
        case "Ref": {
            const taskWork = work as TaskWork<number>;
            const taskName = taskWork.taskId;

            // Determine model override
            let taskModelId: string | undefined;
            if (taskWork.modelOverride) {
                taskModelId = getModelId(taskWork.modelOverride, entry);
            }

            // Register task state for live API
            const taskId = `task-${nextTaskId++}`;
            const taskState: TaskState = {
                taskId,
                task: null as unknown as ICopilotTask,
                responseQueue: [],
                waitingResolve: null,
                taskError: null,
                borrowingSessionMode: false,
                closed: false,
            };
            tasks.set(taskId, taskState);

            runningIds.add(taskWork.workIdInJob);
            callback.workStarted(taskWork.workIdInJob, taskId);

            try {
                const result = await new Promise<boolean>((resolve, reject) => {
                    const taskCallback: ICopilotTaskCallback = {
                        taskSucceeded() {
                            pushResponse(taskState, { callback: "taskSucceeded" });
                            taskState.closed = true;
                            const crashErr = startedTask?.crashError;
                            if (crashErr) { reject(crashErr); } else { resolve(true); }
                        },
                        taskFailed() {
                            pushResponse(taskState, { callback: "taskFailed" });
                            taskState.closed = true;
                            const crashErr = startedTask?.crashError;
                            if (crashErr) { reject(crashErr); } else { resolve(false); }
                        },
                        taskDecision(reason: string) {
                            pushResponse(taskState, { callback: "taskDecision", reason });
                        },
                        taskSessionStarted(taskSession: ICopilotSession, taskId: string, isDrivingSession: boolean) {
                            pushResponse(taskState, { callback: "taskSessionStarted", sessionId: taskId, isDriving: isDrivingSession });
                        },
                        taskSessionStopped(taskSession: ICopilotSession, taskId: string, succeeded: boolean) {
                            pushResponse(taskState, { callback: "taskSessionStopped", sessionId: taskId, succeeded });
                        },
                    };

                    let startedTask: ICopilotTask | null = null;
                    startTask(
                        taskName,
                        userInput,
                        undefined, // managed session mode for jobs
                        false, // not ignoring prerequisites
                        taskCallback,
                        taskModelId,
                        workingDirectory,
                        () => { }
                    ).then(t => {
                        startedTask = t;
                        taskState.task = t;
                        activeTasks.push(t);
                    }).catch(err => {
                        taskState.taskError = errorToDetailedString(err);
                        pushResponse(taskState, { taskError: taskState.taskError });
                        taskState.closed = true;
                        reject(err);
                    });
                });

                runningIds.delete(taskWork.workIdInJob);
                callback.workStopped(taskWork.workIdInJob, result);
                return result;
            } catch (err) {
                runningIds.delete(taskWork.workIdInJob);
                callback.workStopped(taskWork.workIdInJob, false);
                throw err; // Propagate crash to job level
            }
        }
        case "Seq": {
            const seqWork = work as SequentialWork<number>;
            for (const w of seqWork.works) {
                if (stopped.value) return false;
                const result = await executeWork(entry, w, userInput, workingDirectory, runningIds, stopped, activeTasks, callback);
                if (!result) return false;
            }
            return true;
        }
        case "Par": {
            const parWork = work as ParallelWork<number>;
            if (parWork.works.length === 0) return true;
            const results = await Promise.all(
                parWork.works.map(w => executeWork(entry, w, userInput, workingDirectory, runningIds, stopped, activeTasks, callback))
            );
            return results.every(r => r);
        }
        case "Loop": {
            const loopWork = work as LoopWork<number>;
            while (true) {
                if (stopped.value) return false;

                // Check pre-condition
                if (loopWork.preCondition) {
                    const [expected, condWork] = loopWork.preCondition;
                    const condResult = await executeWork(entry, condWork, userInput, workingDirectory, runningIds, stopped, activeTasks, callback);
                    if (condResult !== expected) {
                        return true; // LoopWork finishes as succeeded
                    }
                }

                // Run body
                const bodyResult = await executeWork(entry, loopWork.body, userInput, workingDirectory, runningIds, stopped, activeTasks, callback);
                if (!bodyResult) return false; // body fails → LoopWork fails

                // Check post-condition
                if (loopWork.postCondition) {
                    const [expected, condWork] = loopWork.postCondition;
                    const condResult = await executeWork(entry, condWork, userInput, workingDirectory, runningIds, stopped, activeTasks, callback);
                    if (condResult !== expected) {
                        return true; // LoopWork finishes as succeeded
                    }
                    // condition matches expected → redo loop
                } else {
                    return true; // No post-condition, loop body ran once successfully
                }
            }
        }
        case "Alt": {
            const altWork = work as AltWork<number>;
            const condResult = await executeWork(entry, altWork.condition, userInput, workingDirectory, runningIds, stopped, activeTasks, callback);
            const chosen = condResult ? altWork.trueWork : altWork.falseWork;
            if (!chosen) return true; // No chosen work = success
            return executeWork(entry, chosen, userInput, workingDirectory, runningIds, stopped, activeTasks, callback);
        }
    }
}

// ---- startJob ----

export async function startJob(
    jobName: string,
    userInput: string,
    workingDirectory: string,
    callback: ICopilotJobCallback
): Promise<ICopilotJob> {
    if (!installedEntry) {
        throw new Error("installJobsEntry has not been called.");
    }

    const entry = installedEntry;
    const job = entry.jobs[jobName];
    if (!job) {
        throw new Error(`Job "${jobName}" not found.`);
    }

    let status: "Executing" | "Succeeded" | "Failed" = "Executing";
    let stopped = false;
    const runningIds = new Set<number>();
    const activeTasks: ICopilotTask[] = [];

    const copilotJob: ICopilotJob = {
        get runningWorkIds() { return Array.from(runningIds); },
        get status() { return status; },
        stop() {
            if (stopped) return;
            stopped = true;
            status = "Failed";
            for (const task of activeTasks) {
                task.stop();
            }
        },
    };

    const executionPromise = (async () => {
        try {
            const result = await executeWork(
                entry, job.work, userInput, workingDirectory,
                runningIds, { get value() { return stopped; } },
                activeTasks, callback
            );
            if (result) {
                status = "Succeeded";
                callback.jobSucceeded();
            } else {
                status = "Failed";
                callback.jobFailed();
            }
        } catch (err) {
            if (status === "Executing") {
                status = "Failed";
            }
            // Stop all running tasks
            for (const task of activeTasks) {
                task.stop();
            }
            callback.jobFailed();
            throw err; // Don't consume silently
        }
    })();

    (copilotJob as any)._executionPromise = executionPromise;
    executionPromise.catch(() => { }); // Prevent unhandled rejection; callers should handle

    return copilotJob;
}

// ---- Job API Handlers ----

export async function apiJobList(
    req: http.IncomingMessage,
    res: http.ServerResponse,
): Promise<void> {
    if (!installedEntry) {
        jsonResponse(res, 200, { grid: [], jobs: {}, chart: {} });
        return;
    }
    // Build a combined chart from all jobs
    const chart: Record<string, ReturnType<typeof generateChartNodes>> = {};
    for (const [jobName, job] of Object.entries(installedEntry.jobs)) {
        chart[jobName] = generateChartNodes(job.work);
    }
    jsonResponse(res, 200, { grid: installedEntry.grid, jobs: installedEntry.jobs, chart });
}

export async function apiJobStart(
    req: http.IncomingMessage,
    res: http.ServerResponse,
    jobName: string,
): Promise<void> {
    if (!installedEntry || !(jobName in installedEntry.jobs)) {
        jsonResponse(res, 200, { error: "JobNotFound" });
        return;
    }

    const body = await readBody(req);
    const lines = body.split("\n");
    const workingDirectory = lines[0].trim();
    const userInput = lines.slice(1).join("\n");

    if (!workingDirectory || !path.isAbsolute(workingDirectory)) {
        jsonResponse(res, 200, { error: "JobNotFound" });
        return;
    }
    if (!fs.existsSync(workingDirectory)) {
        jsonResponse(res, 200, { error: "JobNotFound" });
        return;
    }

    try {
        const jobId = `job-${nextJobId++}`;
        const state: JobState = {
            jobId,
            job: null as unknown as ICopilotJob,
            responseQueue: [],
            waitingResolve: null,
            jobError: null,
            closed: false,
        };

        const jobCallback: ICopilotJobCallback = {
            jobSucceeded() {
                pushResponse(state, { callback: "jobSucceeded" });
                state.closed = true;
            },
            jobFailed() {
                pushResponse(state, { callback: "jobFailed" });
                state.closed = true;
            },
            workStarted(workId: number, taskId: string) {
                pushResponse(state, { callback: "workStarted", workId, taskId });
            },
            workStopped(workId: number, succeeded: boolean) {
                pushResponse(state, { callback: "workStopped", workId, succeeded });
            },
        };

        const copilotJob = await startJob(jobName, userInput, workingDirectory, jobCallback);
        state.job = copilotJob;
        jobs.set(jobId, state);

        // Catch execution crashes
        const execPromise = (copilotJob as any)._executionPromise as Promise<void> | undefined;
        if (execPromise) {
            execPromise.catch((err: unknown) => {
                state.jobError = errorToDetailedString(err);
                pushResponse(state, { jobError: state.jobError });
                state.closed = true;
            });
        }

        jsonResponse(res, 200, { jobId });
    } catch (err) {
        jsonResponse(res, 200, { jobError: errorToDetailedString(err) });
    }
}

export async function apiJobStop(
    req: http.IncomingMessage,
    res: http.ServerResponse,
    jobId: string,
): Promise<void> {
    const state = jobs.get(jobId);
    if (!state) {
        jsonResponse(res, 200, { error: "JobNotFound" });
        return;
    }
    state.job.stop();
    jobs.delete(jobId);
    if (state.waitingResolve) {
        const resolve = state.waitingResolve;
        state.waitingResolve = null;
        resolve({ error: "JobNotFound" });
    }
    jsonResponse(res, 200, { result: "Closed" });
}

export async function apiJobLive(
    req: http.IncomingMessage,
    res: http.ServerResponse,
    jobId: string,
): Promise<void> {
    const state = jobs.get(jobId);
    if (!state) {
        jsonResponse(res, 200, { error: "JobNotFound" });
        return;
    }
    if (state.waitingResolve) {
        jsonResponse(res, 200, { error: "ParallelCallNotSupported" });
        return;
    }
    // If closed and no more responses in queue, return JobsClosed and remove
    if (state.closed && state.responseQueue.length === 0) {
        jobs.delete(jobId);
        jsonResponse(res, 200, { error: "JobsClosed" });
        return;
    }
    const response = await waitForResponse(state, 5000);
    if (response === null) {
        jsonResponse(res, 200, { error: "HttpRequestTimeout" });
    } else if (state.jobError) {
        jsonResponse(res, 200, { jobError: state.jobError });
    } else {
        jsonResponse(res, 200, response);
    }
}

// ---- Entry Export ----

export { entry } from "./jobsData.js";

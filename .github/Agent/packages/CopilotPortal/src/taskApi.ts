import type { ICopilotSession } from "./copilotSession.js";
import type { Entry, Task, Prompt } from "./jobsDef.js";
import { expandPromptDynamic, getModelId, MAX_CRASH_RETRIES, SESSION_CRASH_PREFIX } from "./jobsDef.js";
import {
    helperSessionStart,
    helperSessionStop,
    helperPushSessionResponse,
    hasRunningSessions,
} from "./copilotApi.js";

// ---- Error Formatting Helper ----

export function errorToDetailedString(err: unknown): string {
    if (err instanceof Error) {
        const info: Record<string, unknown> = {};
        if (err.name !== undefined) info.name = err.name;
        if (err.message !== undefined) info.message = err.message;
        if (err.stack !== undefined) info.stack = err.stack;
        if ((err as any).cause !== undefined) {
            info.cause = errorToDetailedString((err as any).cause);
        }
        return JSON.stringify(info);
    }
    if (typeof err === "string") return err;
    try { return JSON.stringify(err); } catch { return String(err); }
}

// ---- Types ----

export interface ICopilotTask {
    readonly drivingSession: ICopilotSession;
    readonly status: "Executing" | "Succeeded" | "Failed";
    readonly crashError?: any;
    stop(): void;
}

export interface ICopilotTaskCallback {
    taskSucceeded(): void;
    taskFailed(): void;
    taskDecision(reason: string): void;
    // Unavailable in borrowing session mode
    taskSessionStarted(taskSession: ICopilotSession, taskId: string, isDrivingSession: boolean): void;
    // Unavailable in borrowing session mode
    taskSessionStopped(taskSession: ICopilotSession, taskId: string, succeeded: boolean): void;
}

// ---- Entry Management ----

export let installedEntry: Entry | null = null;

export async function installJobsEntry(entryValue: Entry): Promise<void> {
    if (hasRunningSessions()) {
        throw new Error("Cannot call installJobsEntry while sessions are running.");
    }
    installedEntry = entryValue;
}

// For testing: reset the installed entry
export function resetJobsEntry(): void {
    installedEntry = null;
}

// ---- Runtime Variable Helpers ----

function expandPrompt(entry: Entry, prompt: Prompt, runtimeValues: Record<string, string>): string {
    const result = expandPromptDynamic(entry, prompt, runtimeValues);
    return result[0];
}

// ---- Tool Monitoring ----

interface ToolMonitor {
    toolsCalled: Set<string>;
    booleanResult: boolean | null;
    cleanup: () => void;
}

function monitorSessionTools(session: ICopilotSession, runtimeValues: Record<string, string>): ToolMonitor {
    const toolsCalled = new Set<string>();
    let booleanResult: boolean | null = null;
    let active = true;

    const raw = session.rawSection;

    const onToolStart = (event: { data: { toolName: string; arguments?: unknown } }) => {
        if (!active) return;
        const toolName = event.data.toolName;
        toolsCalled.add(toolName);

        // Extract argument as string
        let argStr = "";
        if (event.data.arguments) {
            if (typeof event.data.arguments === "string") {
                argStr = event.data.arguments;
            } else if (typeof event.data.arguments === "object") {
                const values = Object.values(event.data.arguments as Record<string, unknown>);
                const strVal = values.find(v => typeof v === "string");
                argStr = strVal ? String(strVal) : JSON.stringify(event.data.arguments);
            }
        }

        if (toolName === "job_prepare_document") {
            // Only keep first line and trim spaces
            const lines = argStr.split("\n");
            runtimeValues["reported-document"] = lines[0].trim();
        } else if (toolName === "job_boolean_true") {
            runtimeValues["reported-true-reason"] = argStr;
            delete runtimeValues["reported-false-reason"];
            booleanResult = true;
        } else if (toolName === "job_boolean_false") {
            runtimeValues["reported-false-reason"] = argStr;
            delete runtimeValues["reported-true-reason"];
            booleanResult = false;
        }
    };

    raw.on("tool.execution_start", onToolStart);

    return {
        toolsCalled,
        get booleanResult() { return booleanResult; },
        set booleanResult(v: boolean | null) { booleanResult = v; },
        cleanup() {
            active = false;
        },
    } as ToolMonitor;
}

// ---- startTask ----

class CopilotTaskImpl implements ICopilotTask {

    private readonly entry: Entry;
    private readonly task: Task;
    private readonly criteria?: Task["criteria"];
    private readonly borrowingMode: boolean;
    private readonly singleModel: boolean;
    private taskModelId?: string;
    private runtimeValues: Record<string, string> = {};
    private stopped = false;
    private readonly activeSessions = new Map<string, ICopilotSession>(); // sessionId -> session
    private primaryDrivingSession?: ICopilotSession;

    public status: "Executing" | "Succeeded" | "Failed" = "Executing";
    public crashError?: any;

    public get drivingSession(): ICopilotSession {
        return this.primaryDrivingSession || this.assignedDrivingSession!;
    }

    public stop() {
        if (this.stopped) return;
        this.stopped = true;
        this.status = "Failed";
        for (const [, session] of this.activeSessions) {
            helperSessionStop(session).catch(() => { });
        }
        this.activeSessions.clear();
    }

    constructor(
        taskName: string,
        userInput: string,
        private assignedDrivingSession: ICopilotSession | undefined,
        private ignorePrerequisiteCheck: boolean,
        private callback: ICopilotTaskCallback,
        taskModelIdOverride?: string,
        private workingDirectory?: string) {
        if (!installedEntry) {
            throw new Error("installJobsEntry has not been called.");
        }
        this.entry = installedEntry;
        this.task = this.entry.tasks[taskName];
        if (!this.task) {
            throw new Error(`Task "${taskName}" not found.`);
        }

        this.criteria = this.task.criteria;
        this.borrowingMode = assignedDrivingSession !== undefined;
        this.singleModel = this.borrowingMode ||
            !this.criteria ||
            !("runConditionInSameSession" in this.criteria) ||
            (this.criteria as any).runConditionInSameSession === undefined ||
            (this.criteria as any).runConditionInSameSession === true;

        // Determine task model ID
        this.taskModelId = taskModelIdOverride;
        if (!this.taskModelId && this.task.model) {
            this.taskModelId = getModelId(this.task.model, this.entry);
        }
        if (!this.taskModelId && !this.borrowingMode && this.singleModel) {
            this.taskModelId = this.entry.models.driving;
        }
        if (!this.taskModelId && !this.borrowingMode && !this.singleModel) {
            this.taskModelId = this.entry.models.driving;
        }

        if (userInput) this.runtimeValues["user-input"] = userInput;
        this.primaryDrivingSession = assignedDrivingSession;
    }

    // ---- Session management helpers ----

    private async openSession(modelId: string, isDriving: boolean): Promise<[ICopilotSession, string]> {
        const [session, sessionId] = await helperSessionStart(modelId, this.workingDirectory);
        this.activeSessions.set(sessionId, session);
        this.callback.taskSessionStarted(session, sessionId, isDriving);
        if (isDriving && !this.primaryDrivingSession) this.primaryDrivingSession = session;
        return [session, sessionId];
    }

    private async closeExistingSession(session: ICopilotSession, sessionId: string, succeeded: boolean): Promise<void> {
        this.activeSessions.delete(sessionId);
        await helperSessionStop(session).catch(() => { });
        this.callback.taskSessionStopped(session, sessionId, succeeded);
    }

    // ---- Send prompt with monitoring and crash retry ----

    private async sendMonitoredPrompt(
        sessionRef: { session: ICopilotSession; id: string },
        prompt: string,
        modelId: string,
        isDriving: boolean,
    ): Promise<{ toolsCalled: Set<string>; booleanResult: boolean | null }> {
        const maxAttempts = this.borrowingMode ? 1 : MAX_CRASH_RETRIES;
        let lastError: unknown;

        for (let attempt = 0; attempt < maxAttempts; attempt++) {
            const actualPrompt = attempt === 0 ? prompt : SESSION_CRASH_PREFIX + prompt;
            const monitor = monitorSessionTools(sessionRef.session, this.runtimeValues);

            try {
                helperPushSessionResponse(sessionRef.session, { callback: "onGeneratedUserPrompt", prompt: actualPrompt });
                await sessionRef.session.sendRequest(actualPrompt);
                monitor.cleanup();
                return { toolsCalled: monitor.toolsCalled, booleanResult: monitor.booleanResult };
            } catch (err) {
                monitor.cleanup();
                lastError = err;
                this.callback.taskDecision(`[SESSION CRASHED] ${errorToDetailedString(err)}`);

                if (!this.borrowingMode && attempt < maxAttempts - 1) {
                    try {
                        await this.closeExistingSession(sessionRef.session, sessionRef.id, false);
                        const [newSession, newId] = await this.openSession(modelId, isDriving);
                        sessionRef.session = newSession;
                        sessionRef.id = newId;
                    } catch (sessionErr) {
                        this.callback.taskDecision(`[SESSION CRASHED] Failed to create replacement session: ${errorToDetailedString(sessionErr)}`);
                        throw sessionErr;
                    }
                }
            }
        }

        throw lastError;
    }

    // ---- Criteria checking ----

    private async checkCriteriaResult(
        toolsCalled: Set<string>,
        drivingRef: { session: ICopilotSession; id: string } | undefined,
        drivingModelId: string,
    ): Promise<{ passed: boolean; missingTools: string[] }> {
        if (!this.criteria) return { passed: true, missingTools: [] };

        // Check toolExecuted
        let missingTools: string[] = [];
        if (this.criteria.toolExecuted) {
            for (const tool of this.criteria.toolExecuted) {
                if (!toolsCalled.has(tool)) {
                    missingTools.push(tool);
                }
            }
            if (missingTools.length > 0) {
                this.callback.taskDecision(`[CRITERIA] toolExecuted check failed: missing tools: ${missingTools.join(", ")}`);
                if (!("condition" in this.criteria) || !(this.criteria as any).condition) {
                    return { passed: false, missingTools };
                }
            }
        }

        // Check condition
        if ("condition" in this.criteria && (this.criteria as any).condition) {
            let condRef: { session: ICopilotSession; id: string };
            let needsClose = false;

            if (drivingRef) {
                // Single model or borrowing: use existing session
                condRef = drivingRef;
            } else {
                // Multiple models: create new driving session for condition check
                const [ds, dsId] = await this.openSession(drivingModelId, true);
                condRef = { session: ds, id: dsId };
                needsClose = true;
            }

            const condPrompt = expandPrompt(this.entry, this.criteria.condition, this.runtimeValues);
            let condPassed: boolean;
            try {
                const result = await this.sendMonitoredPrompt(condRef, condPrompt, drivingModelId, true);
                condPassed = result.booleanResult === true;
            } catch (err) {
                if (needsClose) await this.closeExistingSession(condRef.session, condRef.id, false);
                throw err;
            }

            if (needsClose) await this.closeExistingSession(condRef.session, condRef.id, condPassed);

            if (condPassed) {
                this.callback.taskDecision("[CRITERIA] Criteria condition passed");
            } else {
                this.callback.taskDecision(`[CRITERIA] Criteria condition failed. Condition: ${JSON.stringify((this.criteria as any).condition)}`);
                return { passed: false, missingTools };
            }
        }

        // If toolsOk was false but condition passed, tools still failed
        if (missingTools.length > 0) {
            return { passed: false, missingTools };
        }

        return { passed: true, missingTools: [] };
    }

    private buildRetryPrompt(missingTools: string[]): string {
        let prompt = expandPrompt(this.entry, this.task.prompt, this.runtimeValues);
        if (missingTools.length > 0) {
            prompt += `\n## Required Tool Not Called: ${missingTools.join(", ")}`;
        }
        if (this.criteria?.failureAction?.additionalPrompt) {
            const expandedAdditional = expandPrompt(this.entry, this.criteria.failureAction.additionalPrompt, this.runtimeValues);
            prompt += `\n## You accidentally Stopped\n${expandedAdditional}`;
        }
        return prompt;
    }

    // ---- Execution ----

    public async executeBorrowing(): Promise<void> {
        // === BORROWING SESSION MODE ===
        this.runtimeValues["task-model"] = this.taskModelId || "";

        // Execute prompt directly on the given session
        const promptText = expandPrompt(this.entry, this.task.prompt, this.runtimeValues);
        const borrowedSession = this.drivingSession!;

        const monitor = monitorSessionTools(borrowedSession, this.runtimeValues);
        try {
            helperPushSessionResponse(borrowedSession, { callback: "onGeneratedUserPrompt", prompt: promptText });
            await borrowedSession.sendRequest(promptText);
        } catch (err) {
            monitor.cleanup();
            this.callback.taskDecision(`[SESSION CRASHED] Task crashed in borrowing session mode: ${errorToDetailedString(err)}`);
            throw err;
        }
        monitor.cleanup();

        // Check criteria using the borrowed session
        const borrowedRef = { session: borrowedSession, id: "" };
        let { passed, missingTools } = this.criteria
            ? await this.checkCriteriaResult(monitor.toolsCalled, borrowedRef, "")
            : { passed: true, missingTools: [] as string[] };

        // Retry loop (borrowing mode: same session, crash = immediate fail)
        if (!passed && this.criteria?.failureAction) {
            const maxRetries = this.criteria.failureAction.retryTimes;
            for (let i = 0; i < maxRetries && !passed; i++) {
                if (this.stopped) return;
                this.callback.taskDecision(`[OPERATION] Starting retry #${i + 1}`);

                const retryPrompt = this.buildRetryPrompt(missingTools);
                const retryMonitor = monitorSessionTools(borrowedSession, this.runtimeValues);
                try {
                    helperPushSessionResponse(borrowedSession, { callback: "onGeneratedUserPrompt", prompt: retryPrompt });
                    await borrowedSession.sendRequest(retryPrompt);
                } catch (err) {
                    retryMonitor.cleanup();
                    this.callback.taskDecision(`[SESSION CRASHED] Crash during retry in borrowing mode: ${errorToDetailedString(err)}`);
                    throw err;
                }
                retryMonitor.cleanup();

                const retResult = await this.checkCriteriaResult(retryMonitor.toolsCalled, borrowedRef, "");
                passed = retResult.passed;
                missingTools = retResult.missingTools;

                this.callback.taskDecision(passed
                    ? `[CRITERIA] Criteria passed on retry #${i + 1}`
                    : `[CRITERIA] Criteria failed on retry #${i + 1}`);
            }
            if (!passed) {
                this.callback.taskDecision(`[DECISION] Retry budget drained after ${maxRetries} retries`);
            }
        }

        // Report result
        if (passed) {
            this.status = "Succeeded";
            this.callback.taskDecision("[TASK SUCCEEDED] Decision: task succeeded");
            this.callback.taskSucceeded();
        } else {
            this.status = "Failed";
            this.callback.taskDecision("[TASK FAILED] Decision: task failed (criteria not satisfied)");
            this.callback.taskFailed();
        }
    }

    public async executeSingle(): Promise<void> {
        // === MANAGED SESSION MODE (SINGLE MODEL) ===
        const modelId = this.taskModelId || this.entry.models.driving;
        this.runtimeValues["task-model"] = modelId;

        const [session, sessionId] = await this.openSession(modelId, true);
        const ref = { session, id: sessionId };

        if (this.stopped) return;

        // Check availability
        if (this.task.availability && !this.ignorePrerequisiteCheck) {
            if (this.task.availability.condition) {
                const condPrompt = expandPrompt(this.entry, this.task.availability.condition, this.runtimeValues);
                const result = await this.sendMonitoredPrompt(ref, condPrompt, modelId, true);
                if (result.booleanResult !== true) {
                    this.callback.taskDecision(`[AVAILABILITY] Availability check failed: condition not satisfied. Condition: ${JSON.stringify(this.task.availability.condition)}`);
                    this.status = "Failed";
                    await this.closeExistingSession(ref.session, ref.id, false);
                    this.callback.taskFailed();
                    return;
                }
                this.callback.taskDecision("[AVAILABILITY] Availability check passed");
            }
        }

        if (this.stopped) return;

        // Execute prompt
        const promptText = expandPrompt(this.entry, this.task.prompt, this.runtimeValues);
        const { toolsCalled } = await this.sendMonitoredPrompt(ref, promptText, modelId, true);

        // Check criteria
        let { passed, missingTools } = this.task.criteria
            ? await this.checkCriteriaResult(toolsCalled, ref, modelId)
            : { passed: true, missingTools: [] as string[] };

        // Retry loop
        if (!passed && this.task.criteria?.failureAction) {
            const maxRetries = this.task.criteria.failureAction.retryTimes;
            for (let i = 0; i < maxRetries && !passed; i++) {
                if (this.stopped) return;
                this.callback.taskDecision(`[OPERATION] Starting retry #${i + 1}`);

                const retryPrompt = this.buildRetryPrompt(missingTools);
                const retryResult = await this.sendMonitoredPrompt(ref, retryPrompt, modelId, true);

                const checkResult = await this.checkCriteriaResult(retryResult.toolsCalled, ref, modelId);
                passed = checkResult.passed;
                missingTools = checkResult.missingTools;

                this.callback.taskDecision(passed
                    ? `[CRITERIA] Criteria passed on retry #${i + 1}`
                    : `[CRITERIA] Criteria failed on retry #${i + 1}`);
            }
            if (!passed) {
                this.callback.taskDecision(`[DECISION] Retry budget drained after ${maxRetries} retries`);
            }
        }

        // Report result
        if (passed) {
            this.status = "Succeeded";
            this.callback.taskDecision("[TASK SUCCEEDED] Decision: task succeeded");
            await this.closeExistingSession(ref.session, ref.id, true);
            this.callback.taskSucceeded();
        } else {
            this.status = "Failed";
            this.callback.taskDecision("[TASK FAILED] Decision: task failed (criteria not satisfied)");
            await this.closeExistingSession(ref.session, ref.id, false);
            this.callback.taskFailed();
        }
    }

    public async executeMultiple(): Promise<void> {
        // === MANAGED SESSION MODE (MULTIPLE MODELS) ===
        const drivingModelId = this.entry.models.driving;
        const tModelId = this.taskModelId || this.entry.models.driving;
        this.runtimeValues["task-model"] = tModelId;

        if (this.stopped) return;

        // Check availability
        if (this.task.availability && !this.ignorePrerequisiteCheck) {
            if (this.task.availability.condition) {
                const [ds, dsId] = await this.openSession(drivingModelId, true);
                const dRef = { session: ds, id: dsId };

                const condPrompt = expandPrompt(this.entry, this.task.availability.condition, this.runtimeValues);
                let condPassed: boolean;
                try {
                    const result = await this.sendMonitoredPrompt(dRef, condPrompt, drivingModelId, true);
                    condPassed = result.booleanResult === true;
                } catch (err) {
                    await this.closeExistingSession(dRef.session, dRef.id, false);
                    throw err;
                }

                await this.closeExistingSession(dRef.session, dRef.id, condPassed);

                if (!condPassed) {
                    this.callback.taskDecision(`[AVAILABILITY] Availability check failed: condition not satisfied. Condition: ${JSON.stringify(this.task.availability.condition)}`);
                    this.status = "Failed";
                    this.callback.taskFailed();
                    return;
                }
                this.callback.taskDecision("[AVAILABILITY] Availability check passed");
            }
        }

        if (this.stopped) return;

        // Execute prompt (task session)
        const [ts, tsId] = await this.openSession(tModelId, false);
        let taskRef = { session: ts, id: tsId };

        const promptText = expandPrompt(this.entry, this.task.prompt, this.runtimeValues);
        let { toolsCalled } = await this.sendMonitoredPrompt(taskRef, promptText, tModelId, false);

        // Close task session (mission done)
        await this.closeExistingSession(taskRef.session, taskRef.id, true);

        // Check criteria (new driving session for condition if needed)
        let { passed, missingTools } = this.criteria
            ? await this.checkCriteriaResult(toolsCalled, undefined, drivingModelId)
            : { passed: true, missingTools: [] as string[] };

        // Retry loop
        if (!passed && this.criteria?.failureAction) {
            const maxRetries = this.criteria.failureAction.retryTimes;
            for (let i = 0; i < maxRetries && !passed; i++) {
                if (this.stopped) return;
                this.callback.taskDecision(`[OPERATION] Starting retry #${i + 1}`);

                // New task session for retry
                const [rts, rtsId] = await this.openSession(tModelId, false);
                taskRef = { session: rts, id: rtsId };

                const retryPrompt = this.buildRetryPrompt(missingTools);
                let retryToolsCalled: Set<string>;
                try {
                    const retryResult = await this.sendMonitoredPrompt(taskRef, retryPrompt, tModelId, false);
                    retryToolsCalled = retryResult.toolsCalled;
                } catch (err) {
                    // Crash exhausting per-call budget is a failed iteration
                    this.callback.taskDecision(`[SESSION CRASHED] Session crash during retry #${i + 1}: ${errorToDetailedString(err)}`);
                    continue;
                }

                // Close task session
                await this.closeExistingSession(taskRef.session, taskRef.id, true);

                // Re-check criteria
                const checkResult = await this.checkCriteriaResult(retryToolsCalled, undefined, drivingModelId);
                passed = checkResult.passed;
                missingTools = checkResult.missingTools;

                this.callback.taskDecision(passed
                    ? `[CRITERIA] Criteria passed on retry #${i + 1}`
                    : `[CRITERIA] Criteria failed on retry #${i + 1}`);
            }
            if (!passed) {
                this.callback.taskDecision(`[DECISION] Retry budget drained after ${maxRetries} retries`);
            }
        }

        // Report result
        if (passed) {
            this.status = "Succeeded";
            this.callback.taskDecision("[TASK SUCCEEDED] Decision: task succeeded");
            this.callback.taskSucceeded();
        } else {
            this.status = "Failed";
            this.callback.taskDecision("[TASK FAILED] Decision: task failed (criteria not satisfied)");
            this.callback.taskFailed();
        }
    }

    public async handleExecutionError(err: any): Promise<void> {
        if (this.status === "Executing") {
            this.status = "Failed";
        }
        this.crashError = err;
        this.callback.taskDecision(`[TASK FAILED] Task error: ${errorToDetailedString(err)}`);

        // Close all remaining active sessions
        for (const [id, session] of this.activeSessions) {
            await helperSessionStop(session).catch(() => { });
            if (!this.borrowingMode) this.callback.taskSessionStopped(session, id, false);
        }
        this.activeSessions.clear();

        this.callback.taskFailed();
        throw err;
    }

    public async execute(): Promise<void> {
        try {
            if (this.borrowingMode) {
                await this.executeBorrowing();
            } else if (this.singleModel) {
                await this.executeSingle();
            } else {
                await this.executeMultiple();
            }
        } catch (err) {
            await this.handleExecutionError(err);
        }
    }
}

export async function startTask(
    taskName: string,
    userInput: string,
    drivingSession: ICopilotSession | undefined,
    ignorePrerequisiteCheck: boolean,
    callback: ICopilotTaskCallback,
    taskModelIdOverride: string | undefined,
    workingDirectory: string | undefined,
    exceptionHandler: (err: any) => void
): Promise<ICopilotTask> {

    const copilotTask = new CopilotTaskImpl(
        taskName,
        userInput,
        drivingSession,
        ignorePrerequisiteCheck,
        callback,
        taskModelIdOverride,
        workingDirectory
    );

    copilotTask.execute().catch(exceptionHandler);
    return copilotTask;
}

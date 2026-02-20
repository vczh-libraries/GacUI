// ---- Task ----

export type Prompt = string[];

export type Model =
    | { category: string; }
    | { id: string; }
    ;

export interface TaskRetry {
    retryTimes: number;
    additionalPrompt?: Prompt;
}

export interface Task {
    model?: Model;
    prompt: Prompt;
    requireUserInput: boolean;
    availability?: {
        previousTasks?: string[];
        condition?: Prompt;
    };
    criteria?: {
        toolExecuted?: string[];
        failureAction: TaskRetry;
    } & ({
        condition: Prompt;
        runConditionInSameSession: boolean;
    } | never);
}

// ---- Work ----

export interface TaskWork<T> {
    kind: "Ref";
    workIdInJob: T;
    taskId: string;
    modelOverride?: Model;
}

export interface SequentialWork<T> {
    kind: "Seq";
    works: Work<T>[];
}

export interface ParallelWork<T> {
    kind: "Par";
    works: Work<T>[];
}

export interface LoopWork<T> {
    kind: "Loop";
    preCondition?: [boolean, Work<T>];
    postCondition?: [boolean, Work<T>];
    body: Work<T>;
}

export interface AltWork<T> {
    kind: "Alt";
    condition: Work<T>;
    trueWork?: Work<T>;
    falseWork?: Work<T>;
}

export type Work<T> = TaskWork<T> | SequentialWork<T> | ParallelWork<T> | LoopWork<T> | AltWork<T>;

export function assignWorkId(work: Work<never>): Work<number> {
    function helper(w: Work<never>, nextId: number[]): Work<number> {
        switch (w.kind) {
            case "Ref": {
                return { ...w, workIdInJob: nextId[0]++ };
            }
            case "Seq": {
                return { ...w, works: w.works.map(work => helper(work, nextId)) };
            }
            case "Par": {
                return { ...w, works: w.works.map(work => helper(work, nextId)) };
            }
            case "Loop": {
                return {
                    ...w,
                    preCondition: w.preCondition ? [w.preCondition[0], helper(w.preCondition[1], nextId)] : undefined,
                    postCondition: w.postCondition ? [w.postCondition[0], helper(w.postCondition[1], nextId)] : undefined,
                    body: helper(w.body, nextId)
                };
            }
            case "Alt": {
                return {
                    ...w,
                    condition: helper(w.condition, nextId),
                    trueWork: w.trueWork ? helper(w.trueWork, nextId) : undefined,
                    falseWork: w.falseWork ? helper(w.falseWork, nextId) : undefined
                };
            }
        }   
    }
    return helper(work, [0]);
}

// ---- Job ----

export interface Job {
    requireUserInput?: boolean;
    work: Work<number>;
}

export interface GridColumn {
    name: string;
    jobName: string;
}

export interface GridRow {
    keyword: string;
    jobs: (GridColumn | undefined)[];
}

// ---- Entry ----

export interface Entry {
    models: { [key in string]: string };
    promptVariables: { [key in string]: string[] };
    tasks: { [key in string]: Task };
    jobs: { [key in string]: Job };
    grid: GridRow[];
}

// ---- Validation and Helpers ----

export const availableTools: string[] = [
    "job_prepare_document",
    "job_boolean_true",
    "job_boolean_false",
    "job_prerequisite_failed"
];

export const runtimeVariables: string[] = [
    "$user-input",
    "$task-model",
    "$reported-document",
    "$reported-true-reason",
    "$reported-false-reason"
];

export function getModelId(model: Model, entry: Entry): string {
    if ("category" in model) {
        return entry.models[model.category];
    } else {
        return model.id;
    }
}


export const SESSION_CRASH_PREFIX = "The session crashed, please redo and here is the last request:\n";
export const MAX_CRASH_RETRIES = 5;
export const DEFAULT_CRITERIA_RETRIES = 5;

export function retryWithNewSessionCondition(retryTimes: number = 3): TaskRetry {
    return { retryTimes };
}

export function retryFailed(retryTimes: number = DEFAULT_CRITERIA_RETRIES): TaskRetry {
    return { retryTimes, additionalPrompt: ["Please continue as you seemed to be accidentally stopped."] };
}

export function retryFailedCondition(retryTimes: number = DEFAULT_CRITERIA_RETRIES): TaskRetry {
    return { retryTimes, additionalPrompt: ["Please continue as you seemed to be accidentally stopped, because I spotted that: $reported-false-reason"] };
}

export function expandPromptStatic(entry: Entry, codePath: string, prompt: Prompt, requiresBooleanTool?: boolean): Prompt {
    if (prompt.length === 0) {
        throw new Error(`${codePath}: Prompt is empty.`);
    }
    const joined = prompt.join("\n");
    const resolved = resolveVariablesStatic(entry, codePath, joined);
    if (requiresBooleanTool) {
        if (!resolved.includes("job_boolean_true") && !resolved.includes("job_boolean_false")) {
            throw new Error(`${codePath}: Boolean tool (job_boolean_true or job_boolean_false) must be mentioned.`);
        }
    }
    return [resolved];
}

function resolveVariablesStatic(entry: Entry, codePath: string, text: string): string {
    return text.replace(/\$[a-zA-Z]+(?:-[a-zA-Z]+)*/g, (match) => {
        const variableName = match;
        if (runtimeVariables.includes(variableName)) {
            return variableName;
        }
        const key = variableName.slice(1); // remove leading $
        if (key in entry.promptVariables) {
            const childCodePath = `${codePath}/${variableName}`;
            const childPrompt = entry.promptVariables[key];
            const expanded = expandPromptStatic(entry, childCodePath, childPrompt);
            return expanded[0];
        }
        throw new Error(`${codePath}: Cannot find prompt variable: ${variableName}.`);
    });
}

export function expandPromptDynamic(entry: Entry, prompt: Prompt, values: Record<string, string>): Prompt {
    if (prompt.length !== 1) {
        throw new Error(`expandPromptDynamic: Prompt must have exactly one item, got ${prompt.length}.`);
    }
    const text = prompt[0];
    const resolved = text.replace(/\$[a-zA-Z]+(?:-[a-zA-Z]+)*/g, (match) => {
        const variableName = match;
        const key = variableName.slice(1);
        if (key in values) {
            return values[key];
        }
        return "<MISSING>";
    });
    return [resolved];
}

export function validateEntry(entry: Entry, codePath: string): Entry {
    // Validate models.driving exists
    if (!("driving" in entry.models)) {
        throw new Error(`${codePath}entry.models.driving: Should exist.`);
    }

    const modelKeys = Object.keys(entry.models).filter(k => k !== "reviewers");
    const gridKeywords = entry.grid.map(row => row.keyword);
    const jobKeys = entry.jobs ? Object.keys(entry.jobs) : [];

    // Validate grid jobNames
    for (let rowIndex = 0; rowIndex < entry.grid.length; rowIndex++) {
        const row = entry.grid[rowIndex];
        for (let columnIndex = 0; columnIndex < row.jobs.length; columnIndex++) {
            const col = row.jobs[columnIndex];
            if (col && !jobKeys.includes(col.jobName)) {
                throw new Error(`${codePath}entry.grid[${rowIndex}].jobs[${columnIndex}].jobName: "${col.jobName}" is not a valid job name.`);
            }
        }
    }

    for (const [taskName, task] of Object.entries(entry.tasks)) {
        const taskBase = `${codePath}entry.tasks["${taskName}"]`;

        // Validate model
        if (task?.model && "category" in task.model) {
            if (!modelKeys.includes(task.model.category)) {
                throw new Error(`${taskBase}.model.category: "${task.model.category}" is not a valid model key.`);
            }
        }

        // Expand and validate prompt
        task.prompt = expandPromptStatic(entry, `${taskBase}.prompt`, task.prompt);

        // Validate requireUserInput
        const expandedPromptText = task.prompt[0];
        if (task.requireUserInput) {
            if (!expandedPromptText.includes("$user-input")) {
                throw new Error(`${taskBase}.requireUserInput: Prompt should use $user-input when requireUserInput is true.`);
            }
        } else {
            if (expandedPromptText.includes("$user-input")) {
                throw new Error(`${taskBase}.requireUserInput: Prompt should not use $user-input when requireUserInput is false.`);
            }
        }

        // Validate availability
        if (task.availability) {
            if (task.availability.previousTasks) {
                for (let i = 0; i < task.availability.previousTasks.length; i++) {
                    const pt = task.availability.previousTasks[i];
                    if (!(pt in entry.tasks)) {
                        throw new Error(`${taskBase}.availability.previousTasks[${i}]: "${pt}" is not a valid task name.`);
                    }
                }
            }
            if (task.availability.condition) {
                task.availability.condition = expandPromptStatic(entry, `${taskBase}.availability.condition`, task.availability.condition, true);
            }
        }

        // Validate criteria
        if (task.criteria) {
            if (task.criteria.toolExecuted) {
                for (let i = 0; i < task.criteria.toolExecuted.length; i++) {
                    const tool = task.criteria.toolExecuted[i];
                    if (!availableTools.includes(tool)) {
                        throw new Error(`${taskBase}.criteria.toolExecuted[${i}]: "${tool}" is not an available tool.`);
                    }
                }
            }
            if (task.criteria.condition) {
                task.criteria.condition = expandPromptStatic(entry, `${taskBase}.criteria.condition`, task.criteria.condition, true);
            }
            if (task.criteria.failureAction.additionalPrompt) {
                task.criteria.failureAction.additionalPrompt = expandPromptStatic(entry, `${taskBase}.criteria.failureAction.additionalPrompt`, task.criteria.failureAction.additionalPrompt);
            }
        }
    }

    // Validate jobs
    if (entry.jobs) {
        const allModelKeys = Object.keys(entry.models);
        for (const [jobName, job] of Object.entries(entry.jobs)) {
            const jobBase = `${codePath}entry.jobs["${jobName}"]`;

            // Simplify work tree: flatten nested Seq/Par
            job.work = simplifyWork(job.work);

            // Assign unique workIdInJob to each Ref node
            job.work = assignWorkId(job.work as unknown as Work<never>);

            validateWork(entry, job.work, jobBase + ".work", allModelKeys);

            // Compute requireUserInput for the job
            const jobRequiresInput = collectTaskIdsFromWork(job.work).some(
                taskId => entry.tasks[taskId]?.requireUserInput
            );
            if (job.requireUserInput !== undefined && job.requireUserInput !== jobRequiresInput) {
                throw new Error(`${jobBase}.requireUserInput: Should be ${jobRequiresInput} but is ${job.requireUserInput}.`);
            }
            job.requireUserInput = jobRequiresInput;
        }
    }

    return entry;
}

// ---- Helper: ensure cross references in Entry correct ----

function validateWork(entry: Entry, work: Work<unknown>, codePath: string, modelKeys: string[]): void {
    switch (work.kind) {
        case "Ref": {
            const tw = work as TaskWork<unknown>;
            if (!(tw.taskId in entry.tasks)) {
                throw new Error(`${codePath}.taskId: "${tw.taskId}" is not a valid task name.`);
            }
            if (tw.modelOverride) {
                if ("category" in tw.modelOverride) {
                    if (!modelKeys.includes(tw.modelOverride.category)) {
                        throw new Error(`${codePath}.modelOverride.category: "${tw.modelOverride.category}" is not a valid model key.`);
                    }
                }
            } else {
                // modelOverride must be defined if the task has no specified model
                const task = entry.tasks[tw.taskId];
                if (!task.model) {
                    throw new Error(`${codePath}.modelOverride: must be defined because task "${tw.taskId}" has no specified model.`);
                }
            }
            break;
        }
        case "Seq": {
            const sw = work as SequentialWork<unknown>;
            if (sw.works.length === 0) {
                throw new Error(`${codePath}.works: should have at least one element.`);
            }
            for (let i = 0; i < sw.works.length; i++) {
                validateWork(entry, sw.works[i], `${codePath}.works[${i}]`, modelKeys);
            }
            break;
        }
        case "Par": {
            const pw = work as ParallelWork<unknown>;
            if (pw.works.length === 0) {
                throw new Error(`${codePath}.works: should have at least one element.`);
            }
            for (let i = 0; i < pw.works.length; i++) {
                validateWork(entry, pw.works[i], `${codePath}.works[${i}]`, modelKeys);
            }
            break;
        }
        case "Loop": {
            const lw = work as LoopWork<unknown>;
            if (lw.preCondition) {
                validateWork(entry, lw.preCondition[1], `${codePath}.preCondition[1]`, modelKeys);
            }
            if (lw.postCondition) {
                validateWork(entry, lw.postCondition[1], `${codePath}.postCondition[1]`, modelKeys);
            }
            validateWork(entry, lw.body, `${codePath}.body`, modelKeys);
            break;
        }
        case "Alt": {
            const aw = work as AltWork<unknown>;
            validateWork(entry, aw.condition, `${codePath}.condition`, modelKeys);
            if (aw.trueWork) {
                validateWork(entry, aw.trueWork, `${codePath}.trueWork`, modelKeys);
            }
            if (aw.falseWork) {
                validateWork(entry, aw.falseWork, `${codePath}.falseWork`, modelKeys);
            }
            break;
        }
    }
}

// ---- Helper: collect all TaskWork taskIds from a work tree ----

function collectTaskIdsFromWork(work: Work<unknown>): string[] {
    const ids: string[] = [];
    function collect(w: Work<unknown>): void {
        switch (w.kind) {
            case "Ref":
                ids.push((w as TaskWork<unknown>).taskId);
                break;
            case "Seq":
            case "Par":
                for (const child of (w as SequentialWork<unknown> | ParallelWork<unknown>).works) {
                    collect(child);
                }
                break;
            case "Loop": {
                const lw = w as LoopWork<unknown>;
                if (lw.preCondition) collect(lw.preCondition[1]);
                collect(lw.body);
                if (lw.postCondition) collect(lw.postCondition[1]);
                break;
            }
            case "Alt": {
                const aw = w as AltWork<unknown>;
                collect(aw.condition);
                if (aw.trueWork) collect(aw.trueWork);
                if (aw.falseWork) collect(aw.falseWork);
                break;
            }
        }
    }
    collect(work);
    return ids;
}

// ---- Helper: simplify work tree (flatten nested Seq/Par) ----

function simplifyWork<T>(work: Work<T>): Work<T> {
    switch (work.kind) {
        case "Ref":
            return work;
        case "Seq": {
            const flatWorks: Work<T>[] = [];
            for (const child of (work as SequentialWork<T>).works) {
                const simplified = simplifyWork(child);
                if (simplified.kind === "Seq") {
                    flatWorks.push(...(simplified as SequentialWork<T>).works);
                } else {
                    flatWorks.push(simplified);
                }
            }
            return { ...work, works: flatWorks } as SequentialWork<T>;
        }
        case "Par": {
            const flatWorks: Work<T>[] = [];
            for (const child of (work as ParallelWork<T>).works) {
                const simplified = simplifyWork(child);
                if (simplified.kind === "Par") {
                    flatWorks.push(...(simplified as ParallelWork<T>).works);
                } else {
                    flatWorks.push(simplified);
                }
            }
            return { ...work, works: flatWorks } as ParallelWork<T>;
        }
        case "Loop": {
            const lw = work as LoopWork<T>;
            return {
                ...lw,
                preCondition: lw.preCondition ? [lw.preCondition[0], simplifyWork(lw.preCondition[1])] : undefined,
                body: simplifyWork(lw.body),
                postCondition: lw.postCondition ? [lw.postCondition[0], simplifyWork(lw.postCondition[1])] : undefined,
            } as LoopWork<T>;
        }
        case "Alt": {
            const aw = work as AltWork<T>;
            return {
                ...aw,
                condition: simplifyWork(aw.condition),
                trueWork: aw.trueWork ? simplifyWork(aw.trueWork) : undefined,
                falseWork: aw.falseWork ? simplifyWork(aw.falseWork) : undefined,
            } as AltWork<T>;
        }
    }
}

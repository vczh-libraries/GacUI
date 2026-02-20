import { describe, it } from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";

const { expandPromptStatic, expandPromptDynamic, validateEntry, availableTools, runtimeVariables } =
    await import("../dist/jobsDef.js");
const { generateChartNodes } =
    await import("../dist/jobsChart.js");
const { entry } =
    await import("../dist/jobsData.js");

// Load and validate the test entry for tests that reference test-specific jobs
const testEntryRaw = JSON.parse(readFileSync(new URL("./testEntry.json", import.meta.url), "utf-8"));
const validatedTestEntry = validateEntry(JSON.parse(JSON.stringify(testEntryRaw)), "testEntry:");

describe("expandPromptStatic", () => {
    it("joins prompt array with LF", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        const result = expandPromptStatic(testEntry, "test", ["line1", "line2", "line3"]);
        assert.deepStrictEqual(result, ["line1\nline2\nline3"]);
    });

    it("resolves prompt variables recursively", () => {
        const testEntry = {
            ...entry,
            promptVariables: {
                greeting: ["Hello"],
                full: ["$greeting", "World"],
            },
        };
        const result = expandPromptStatic(testEntry, "test", ["$full"]);
        assert.deepStrictEqual(result, ["Hello\nWorld"]);
    });

    it("keeps runtime variables as-is", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        const result = expandPromptStatic(testEntry, "test", ["Say $user-input please"]);
        assert.deepStrictEqual(result, ["Say $user-input please"]);
    });

    it("throws on unknown variable", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        assert.throws(
            () => expandPromptStatic(testEntry, "test", ["$unknown-var"]),
            { message: "test: Cannot find prompt variable: $unknown-var." }
        );
    });

    it("throws on empty prompt", () => {
        assert.throws(
            () => expandPromptStatic(entry, "test", []),
            { message: "test: Prompt is empty." }
        );
    });

    it("codePath includes variable chain for nested errors", () => {
        const testEntry = {
            ...entry,
            promptVariables: {
                outer: ["$inner"],
                inner: ["$missing"],
            },
        };
        assert.throws(
            () => expandPromptStatic(testEntry, "root", ["$outer"]),
            { message: "root/$outer/$inner: Cannot find prompt variable: $missing." }
        );
    });
});

describe("expandPromptDynamic", () => {
    it("resolves runtime variables from values", () => {
        const result = expandPromptDynamic(entry, ["Hello $user-input"], { "user-input": "world" });
        assert.deepStrictEqual(result, ["Hello world"]);
    });

    it("throws when prompt has more than one item", () => {
        assert.throws(
            () => expandPromptDynamic(entry, ["a", "b"], {}),
            /must have exactly one item/
        );
    });

    it("returns <MISSING> when variable not in values", () => {
        const result = expandPromptDynamic(entry, ["$user-input"], {});
        assert.deepStrictEqual(result, ["<MISSING>"]);
    });
});

describe("validateEntry (entry export)", () => {
    it("entry is exported and has expected structure", () => {
        assert.ok(entry.models, "should have models");
        assert.ok(entry.grid, "should have grid");
        assert.ok(entry.tasks, "should have tasks");
    });

    it("all task prompts are expanded to single item", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            assert.strictEqual(task.prompt.length, 1, `task ${name} prompt should be single item`);
        }
    });

    it("all availability conditions are expanded to single item", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            if (task.availability?.condition) {
                assert.strictEqual(
                    task.availability.condition.length, 1,
                    `task ${name} availability.condition should be single item`
                );
            }
        }
    });

    it("all criteria conditions are expanded to single item", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            if (task.criteria?.condition) {
                assert.strictEqual(
                    task.criteria.condition.length, 1,
                    `task ${name} criteria.condition should be single item`
                );
            }
        }
    });

    it("all criteria failureAction prompts are expanded to single item", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            if (task.criteria?.failureAction && task.criteria.failureAction.length === 3) {
                const prompt = task.criteria.failureAction[2];
                assert.strictEqual(
                    prompt.length, 1,
                    `task ${name} criteria.failureAction[2] should be single item`
                );
            }
        }
    });

    it("expanded prompts do not contain resolvable variables (only runtime ones)", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            const text = task.prompt[0];
            const vars = text.match(/\$[a-zA-Z]+(?:-[a-zA-Z]+)*/g) || [];
            for (const v of vars) {
                assert.ok(
                    runtimeVariables.includes(v),
                    `task ${name} prompt contains unresolved variable: ${v}`
                );
            }
        }
    });

    it("validation error paths use JS expression format", () => {
        // Build a minimal entry with an invalid model to test error path format
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2", coding: "gpt-5.2-codex", reviewers: [] },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "test-task": { model: { category: "nonexistent" }, prompt: ["hello"] }
            }
        };
        assert.throws(
            () => {
                validateEntry(badEntry, "root:");
            },
            (err) => {
                return err.message.includes('root:entry.tasks["test-task"].model');
            }
        );
    });

    it("availableTools is a non-empty array of strings", () => {
        assert.ok(Array.isArray(availableTools));
        assert.ok(availableTools.length > 0);
        for (const t of availableTools) {
            assert.ok(typeof t === "string");
        }
    });

    it("runtimeVariables is a non-empty array of strings starting with $", () => {
        assert.ok(Array.isArray(runtimeVariables));
        assert.ok(runtimeVariables.length > 0);
        for (const v of runtimeVariables) {
            assert.ok(v.startsWith("$"), `${v} should start with $`);
        }
    });
});

describe("expandPromptStatic requiresBooleanTool", () => {
    it("passes when boolean tool is mentioned", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        const result = expandPromptStatic(testEntry, "test", ["You must call job_boolean_true or job_boolean_false"], true);
        assert.deepStrictEqual(result, ["You must call job_boolean_true or job_boolean_false"]);
    });

    it("passes when only job_boolean_true is mentioned", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        const result = expandPromptStatic(testEntry, "test", ["Call job_boolean_true to confirm"], true);
        assert.ok(result[0].includes("job_boolean_true"));
    });

    it("throws when no boolean tool is mentioned and requiresBooleanTool is true", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        assert.throws(
            () => expandPromptStatic(testEntry, "test", ["No boolean tool here"], true),
            /Boolean tool/
        );
    });

    it("does not throw when requiresBooleanTool is false or unset", () => {
        const testEntry = {
            ...entry,
            promptVariables: {},
        };
        const result = expandPromptStatic(testEntry, "test", ["No boolean tool here"], false);
        assert.deepStrictEqual(result, ["No boolean tool here"]);
        const result2 = expandPromptStatic(testEntry, "test", ["No boolean tool here"]);
        assert.deepStrictEqual(result2, ["No boolean tool here"]);
    });

    it("all availability conditions mention boolean tools", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            if (task.availability?.condition) {
                const text = task.availability.condition[0];
                assert.ok(
                    text.includes("job_boolean_true") || text.includes("job_boolean_false"),
                    `task ${name} availability.condition should mention boolean tool`
                );
            }
        }
    });

    it("all criteria conditions mention boolean tools", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            if (task.criteria?.condition) {
                const text = task.criteria.condition[0];
                assert.ok(
                    text.includes("job_boolean_true") || text.includes("job_boolean_false"),
                    `task ${name} criteria.condition should mention boolean tool`
                );
            }
        }
    });
});

describe("validateEntry requireUserInput", () => {
    it("throws when requireUserInput is true but prompt does not use $user-input", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2", coding: "gpt-5.2-codex", reviewers: [] },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "test-task": { model: { category: "planning" }, requireUserInput: true, prompt: ["hello world"] }
            }
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /requireUserInput.*should use \$user-input/
        );
    });

    it("throws when requireUserInput is false but prompt uses $user-input", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2", coding: "gpt-5.2-codex", reviewers: [] },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "test-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello $user-input"] }
            }
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /requireUserInput.*should not use \$user-input/
        );
    });

    it("passes when requireUserInput matches prompt usage", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2", coding: "gpt-5.2-codex", reviewers: [] },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "task-with-input": { model: { category: "planning" }, requireUserInput: true, prompt: ["do $user-input"] },
                "task-without-input": { model: { category: "planning" }, requireUserInput: false, prompt: ["do something"] }
            }
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });

    it("validated entry tasks have correct requireUserInput for tasks using $user-input", () => {
        for (const [name, task] of Object.entries(entry.tasks)) {
            const text = task.prompt[0];
            const usesUserInput = text.includes("$user-input");
            assert.strictEqual(
                task.requireUserInput, usesUserInput,
                `task ${name}: requireUserInput=${task.requireUserInput} but prompt ${usesUserInput ? "uses" : "does not use"} $user-input`
            );
        }
    });
});

const { assignWorkId } = await import("../dist/jobsDef.js");

describe("assignWorkId", () => {
    it("assigns sequential ids to TaskWork nodes", () => {
        const work = {
            kind: "Seq",
            works: [
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "a" },
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "b" },
            ],
        };
        const result = assignWorkId(work);
        assert.strictEqual(result.works[0].workIdInJob, 0);
        assert.strictEqual(result.works[1].workIdInJob, 1);
    });

    it("assigns ids across nested structures", () => {
        const work = {
            kind: "Par",
            works: [
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "a" },
                {
                    kind: "Seq",
                    works: [
                        { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "b" },
                        { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "c" },
                    ],
                },
            ],
        };
        const result = assignWorkId(work);
        assert.strictEqual(result.works[0].workIdInJob, 0);
        assert.strictEqual(result.works[1].works[0].workIdInJob, 1);
        assert.strictEqual(result.works[1].works[1].workIdInJob, 2);
    });
});

describe("validateEntry models.driving", () => {
    it("throws when models.driving is missing", () => {
        const badEntry = {
            models: { planning: "gpt-5.2" },
            promptVariables: {},
            grid: [],
            tasks: {},
            jobs: {},
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /entry\.models\.driving.*Should exist/
        );
    });

    it("passes when models.driving exists", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {},
            jobs: {},
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });

    it("throws when drivingSessionRetries[0].modelId doesn't match models.driving", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "wrong-model", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {},
            jobs: {},
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /drivingSessionRetries.*first modelId should equal/
        );
    });

    it("passes when drivingSessionRetries[0].modelId matches models.driving", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {},
            jobs: {},
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });

    it("throws when drivingSessionRetries is empty", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [],
            promptVariables: {},
            grid: [],
            tasks: {},
            jobs: {},
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /drivingSessionRetries.*at least one item/
        );
    });

    it("throws when drivingSessionRetries is undefined", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            promptVariables: {},
            grid: [],
            tasks: {},
            jobs: {},
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /drivingSessionRetries.*at least one item/
        );
    });
});

describe("validateEntry jobs", () => {
    it("validates TaskWork.taskId must be in entry.tasks", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "real-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    printedName: "Test Job",
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "nonexistent-task" }),
                },
            },
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /taskId.*nonexistent-task.*not a valid task name/
        );
    });

    it("validates TaskWork.modelOverride.category must be in entry.models", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "real-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    printedName: "Test Job",
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "real-task", modelOverride: { category: "nonexistent-model" } }),
                },
            },
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /modelOverride.category.*nonexistent-model.*not a valid model key/
        );
    });

    it("validates TaskWork.modelOverride must be defined if task has no model", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "no-model-task": { requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    printedName: "Test Job",
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "no-model-task" }),
                },
            },
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /modelOverride.*must be defined/
        );
    });

    it("passes validation for valid job with TaskWork", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "real-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    printedName: "Test Job",
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "real-task" }),
                },
            },
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });

    it("passes validation for valid job with modelOverride on task without model", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "no-model-task": { requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    printedName: "Test Job",
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "no-model-task", modelOverride: { category: "planning" } }),
                },
            },
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });
});

describe("validateEntry grid jobName", () => {
    it("throws when grid jobName is not in entry.jobs", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [{
                keyword: "test",
                jobs: [{ name: "col", jobName: "nonexistent-job" }]
            }],
            tasks: {},
            jobs: {},
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /entry\.grid\[0\]\.jobs\[0\]\.jobName.*nonexistent-job.*not a valid job name/
        );
    });

    it("passes when grid jobName exists in entry.jobs", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [{
                keyword: "test",
                jobs: [{ name: "col", jobName: "real-job" }]
            }],
            tasks: {
                "real-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "real-job": {
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "real-task" }),
                },
            },
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });

    it("all grid jobNames in entry are valid", () => {
        const jobKeys = Object.keys(entry.jobs);
        for (let rowIndex = 0; rowIndex < entry.grid.length; rowIndex++) {
            const row = entry.grid[rowIndex];
            for (let colIndex = 0; colIndex < row.jobs.length; colIndex++) {
                const col = row.jobs[colIndex];
                if (col === undefined || col === null) continue; // undefined renders an empty cell
                assert.ok(
                    jobKeys.includes(col.jobName),
                    `grid[${rowIndex}].jobs[${colIndex}].jobName "${col.jobName}" should be a valid job name`
                );
            }
        }
    });
});

describe("validateEntry job requireUserInput", () => {
    it("fills requireUserInput=false for job referencing only non-input tasks", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "no-input-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "no-input-task" }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        assert.strictEqual(result.jobs["test-job"].requireUserInput, false);
    });

    it("fills requireUserInput=true for job referencing an input task", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "input-task": { model: { category: "planning" }, requireUserInput: true, prompt: ["do $user-input"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "input-task" }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        assert.strictEqual(result.jobs["test-job"].requireUserInput, true);
    });

    it("fills requireUserInput=true for job with mixed tasks (Seq)", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "no-input-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
                "input-task": { model: { category: "planning" }, requireUserInput: true, prompt: ["do $user-input"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Seq",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "no-input-task" },
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "input-task" },
                        ]
                    }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        assert.strictEqual(result.jobs["test-job"].requireUserInput, true);
    });

    it("passes when requireUserInput is defined correctly", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "input-task": { model: { category: "planning" }, requireUserInput: true, prompt: ["do $user-input"] },
            },
            jobs: {
                "test-job": {
                    requireUserInput: true,
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "input-task" }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        assert.strictEqual(result.jobs["test-job"].requireUserInput, true);
    });

    it("throws when requireUserInput is defined incorrectly", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "no-input-task": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    requireUserInput: true,
                    work: assignWorkId({ kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "no-input-task" }),
                },
            },
        };
        assert.throws(
            () => validateEntry(testEntry, "test:"),
            /requireUserInput.*Should be false but is true/
        );
    });

    it("all jobs in entry have requireUserInput filled after validation", () => {
        for (const [jobName, job] of Object.entries(entry.jobs)) {
            assert.ok(
                job.requireUserInput !== undefined,
                `job ${jobName} should have requireUserInput filled`
            );
        }
    });

    it("input-job in test entry has requireUserInput=true", () => {
        assert.strictEqual(validatedTestEntry.jobs["input-job"].requireUserInput, true);
    });

    it("simple-job in test entry has requireUserInput=false", () => {
        assert.strictEqual(validatedTestEntry.jobs["simple-job"].requireUserInput, false);
    });
});

describe("validateEntry work simplification", () => {
    it("flattens nested SequentialWork", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Seq",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                            {
                                kind: "Seq",
                                works: [
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                ]
                            }
                        ]
                    }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        const work = result.jobs["test-job"].work;
        assert.strictEqual(work.kind, "Seq");
        assert.strictEqual(work.works.length, 3, "nested Seq should be flattened");
        for (const w of work.works) {
            assert.strictEqual(w.kind, "Ref");
        }
    });

    it("flattens multi-level nested SequentialWork", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Seq",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                            {
                                kind: "Seq",
                                works: [
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                    {
                                        kind: "Seq",
                                        works: [
                                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                        ]
                                    }
                                ]
                            }
                        ]
                    }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        const work = result.jobs["test-job"].work;
        assert.strictEqual(work.kind, "Seq");
        assert.strictEqual(work.works.length, 3, "multi-level nested Seq should be fully flattened");
    });

    it("flattens nested ParallelWork", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Par",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                            {
                                kind: "Par",
                                works: [
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                ]
                            }
                        ]
                    }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        const work = result.jobs["test-job"].work;
        assert.strictEqual(work.kind, "Par");
        assert.strictEqual(work.works.length, 3, "nested Par should be flattened");
    });

    it("does not flatten Par inside Seq or vice versa", () => {
        const testEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Seq",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                            {
                                kind: "Par",
                                works: [
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                    { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                                ]
                            }
                        ]
                    }),
                },
            },
        };
        const result = validateEntry(testEntry, "test:");
        const work = result.jobs["test-job"].work;
        assert.strictEqual(work.kind, "Seq");
        assert.strictEqual(work.works.length, 2, "Par inside Seq should not be flattened");
        assert.strictEqual(work.works[1].kind, "Par");
    });

    it("nested-seq-job in test entry has flattened work", () => {
        const work = validatedTestEntry.jobs["nested-seq-job"].work;
        assert.strictEqual(work.kind, "Seq");
        assert.strictEqual(work.works.length, 3, "nested-seq-job should be fully flattened");
    });

    it("nested-par-job in test entry has flattened work", () => {
        const work = validatedTestEntry.jobs["nested-par-job"].work;
        assert.strictEqual(work.kind, "Par");
        assert.strictEqual(work.works.length, 3, "nested-par-job should be fully flattened");
    });
});

describe("validateEntry empty works rejection", () => {
    it("throws when SequentialWork has empty works", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({ kind: "Seq", works: [] }),
                },
            },
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /works.*should have at least one element/
        );
    });

    it("throws when ParallelWork has empty works", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({ kind: "Par", works: [] }),
                },
            },
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /works.*should have at least one element/
        );
    });

    it("throws when nested ParallelWork inside SequentialWork has empty works", () => {
        const badEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Seq",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                            { kind: "Par", works: [] },
                        ],
                    }),
                },
            },
        };
        assert.throws(
            () => validateEntry(badEntry, "test:"),
            /works.*should have at least one element/
        );
    });

    it("passes when SequentialWork has at least one element", () => {
        const goodEntry = {
            models: { driving: "gpt-5-mini", planning: "gpt-5.2" },
            drivingSessionRetries: [{ modelId: "gpt-5-mini", retries: 3 }],
            promptVariables: {},
            grid: [],
            tasks: {
                "t": { model: { category: "planning" }, requireUserInput: false, prompt: ["hello"] },
            },
            jobs: {
                "test-job": {
                    work: assignWorkId({
                        kind: "Seq",
                        works: [
                            { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "t" },
                        ],
                    }),
                },
            },
        };
        const result = validateEntry(goodEntry, "test:");
        assert.ok(result);
    });
});

describe("generateChartNodes", () => {
    it("generates TaskNode for each TaskWork", () => {
        const work = assignWorkId({
            kind: "Seq",
            works: [
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "a" },
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "b" },
            ],
        });
        const chart = generateChartNodes(work);
        const taskNodes = chart.nodes.filter(n => Array.isArray(n.hint) && n.hint[0] === "TaskNode");
        assert.strictEqual(taskNodes.length, 2, "should have 2 TaskNode nodes");
        const workIds = taskNodes.map(n => n.hint[1]).sort();
        assert.deepStrictEqual(workIds, [0, 1]);
    });

    it("generates ParBegin and ParEnd for ParallelWork", () => {
        const work = assignWorkId({
            kind: "Par",
            works: [
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "a" },
                { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "b" },
            ],
        });
        const chart = generateChartNodes(work);
        assert.ok(chart.nodes.some(n => n.hint === "ParBegin"), "should have ParBegin");
        assert.ok(chart.nodes.some(n => n.hint === "ParEnd"), "should have ParEnd");
    });

    it("generates LoopEnd for LoopWork", () => {
        const work = assignWorkId({
            kind: "Loop",
            body: { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "a" },
        });
        const chart = generateChartNodes(work);
        assert.ok(chart.nodes.some(n => n.hint === "LoopEnd"), "should have LoopEnd");
    });

    it("generates AltEnd for AltWork", () => {
        const work = assignWorkId({
            kind: "Alt",
            condition: { kind: "Ref", workIdInJob: /** @type {never} */ (undefined), taskId: "a" },
        });
        const chart = generateChartNodes(work);
        assert.ok(chart.nodes.some(n => n.hint === "AltEnd"), "should have AltEnd");
    });

    it("every TaskWork in test entry jobs has a ChartNode with TaskNode or CondNode hint", () => {
        function collectTaskWorkIds(work) {
            const ids = [];
            if (work.kind === "Ref") ids.push(work.workIdInJob);
            else if (work.kind === "Seq" || work.kind === "Par") work.works.forEach(w => ids.push(...collectTaskWorkIds(w)));
            else if (work.kind === "Loop") {
                if (work.preCondition) ids.push(...collectTaskWorkIds(work.preCondition[1]));
                ids.push(...collectTaskWorkIds(work.body));
                if (work.postCondition) ids.push(...collectTaskWorkIds(work.postCondition[1]));
            } else if (work.kind === "Alt") {
                ids.push(...collectTaskWorkIds(work.condition));
                if (work.trueWork) ids.push(...collectTaskWorkIds(work.trueWork));
                if (work.falseWork) ids.push(...collectTaskWorkIds(work.falseWork));
            }
            return ids;
        }
        for (const [jobName, job] of Object.entries(validatedTestEntry.jobs)) {
            const workIds = collectTaskWorkIds(job.work);
            const chart = generateChartNodes(job.work);
            for (const wid of workIds) {
                const node = chart.nodes.find(n => Array.isArray(n.hint) && (n.hint[0] === "TaskNode" || n.hint[0] === "CondNode") && n.hint[1] === wid);
                assert.ok(node, `job ${jobName}: TaskWork workIdInJob=${wid} should have a ChartNode with TaskNode or CondNode hint`);
            }
        }
    });
});

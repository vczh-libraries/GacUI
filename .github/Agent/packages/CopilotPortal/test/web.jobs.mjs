import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import { chromium } from "playwright";

const BASE = "http://localhost:8888";

async function fetchJson(urlPath, options) {
    const res = await fetch(`${BASE}${urlPath}`, options);
    return res.json();
}

// Verify the entry is already installed (by api.test.mjs which runs earlier)
async function verifyEntryInstalled() {
    const data = await fetchJson("/api/copilot/job");
    assert.ok(Array.isArray(data.grid), "grid should be an array (entry must be installed by api.test.mjs)");
    assert.ok(data.grid.length > 0, "grid should have rows (entry must be installed by api.test.mjs)");
}

describe("Web: jobs.html redirect", () => {
    let browser;
    let page;

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
    });

    after(async () => {
        await browser?.close();
    });

    it("redirects to index.html when no wb parameter", async () => {
        await page.goto(`${BASE}/jobs.html`);
        await page.waitForTimeout(1000);
        const url = new URL(page.url());
        assert.strictEqual(url.pathname, "/index.html", "should redirect to index.html");
    });
});

describe("Web: jobs.html layout", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobs.html?wb=C%3A%5CCode%5CVczhLibraries%5CTools`);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("loads jobs.css stylesheet", async () => {
        const links = await page.evaluate(() =>
            Array.from(document.querySelectorAll('link[rel="stylesheet"]')).map((l) => l.getAttribute("href"))
        );
        assert.ok(links.includes("jobs.css"), "should include jobs.css");
    });

    it("has left-part and right-part", async () => {
        const leftVisible = await page.locator("#left-part").isVisible();
        const rightVisible = await page.locator("#right-part").isVisible();
        assert.ok(leftVisible, "left part should be visible");
        assert.ok(rightVisible, "right part should be visible");
    });

    it("has horizontal resize bar", async () => {
        const visible = await page.locator("#resize-bar").isVisible();
        assert.ok(visible, "resize bar should be visible");
    });

    it("matrix-part is visible at start", async () => {
        const visible = await page.locator("#matrix-part").isVisible();
        assert.ok(visible, "matrix part should be visible");
    });

    it("user-input-part is visible at start", async () => {
        const visible = await page.locator("#user-input-part").isVisible();
        assert.ok(visible, "user input part should be visible");
    });

    it("user-input-textarea is disabled by default", async () => {
        const disabled = await page.locator("#user-input-textarea").isDisabled();
        assert.ok(disabled, "textarea should be disabled by default");
    });
});

describe("Web: jobs.html matrix rendering", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobs.html?wb=C%3A%5CCode%5CVczhLibraries%5CTools`);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("renders matrix table with title", async () => {
        const title = await page.locator(".matrix-title").textContent();
        assert.strictEqual(title, "Available Jobs", "should have title row");
    });

    it("has Stop Server button in title row", async () => {
        const btn = page.locator("#stop-server-button");
        const visible = await btn.isVisible();
        assert.ok(visible, "Stop Server button should be visible");
        const text = await btn.textContent();
        assert.strictEqual(text, "Stop Server");
    });

    it("renders keyword columns", async () => {
        const keywords = await page.locator(".matrix-keyword").allTextContents();
        assert.ok(keywords.includes("test"), "should have 'test' keyword");
        assert.ok(keywords.includes("batch"), "should have 'batch' keyword");
    });

    it("renders job buttons with correct text", async () => {
        const jobBtnTexts = await page.locator(".matrix-job-btn").allTextContents();
        assert.ok(jobBtnTexts.includes("run"), "should have 'run' button");
        assert.ok(jobBtnTexts.includes("fail"), "should have 'fail' button");
        assert.ok(jobBtnTexts.includes("sequence"), "should have 'sequence' button");
        assert.ok(jobBtnTexts.includes("parallel"), "should have 'parallel' button");
    });

    it("job buttons have data-job-name attribute", async () => {
        const jobNames = await page.evaluate(() =>
            Array.from(document.querySelectorAll(".matrix-job-btn")).map(b => b.dataset.jobName)
        );
        assert.ok(jobNames.includes("simple-job"), "should have simple-job");
        assert.ok(jobNames.includes("fail-job"), "should have fail-job");
        assert.ok(jobNames.includes("seq-job"), "should have seq-job");
        assert.ok(jobNames.includes("par-job"), "should have par-job");
    });
});

describe("Web: jobs.html job selection", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobs.html?wb=C%3A%5CCode%5CVczhLibraries%5CTools`);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("Start Job button shows 'Job Not Selected' and is disabled initially", async () => {
        const text = await page.locator("#start-job-button").textContent();
        assert.strictEqual(text, "Job Not Selected");
        const disabled = await page.locator("#start-job-button").isDisabled();
        assert.ok(disabled, "start job button should be disabled");
    });

    it("clicking a job button selects it and enables start button", async () => {
        const btn = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        await btn.click();
        const hasSelected = await btn.evaluate(el => el.classList.contains("selected"));
        assert.ok(hasSelected, "clicked button should have selected class");
        const startText = await page.locator("#start-job-button").textContent();
        assert.strictEqual(startText, "Start Job: simple-job");
        const disabled = await page.locator("#start-job-button").isDisabled();
        assert.ok(!disabled, "start job button should be enabled");
    });

    it("textarea stays disabled for job without requireUserInput", async () => {
        // simple-job uses simple-task which has requireUserInput: false
        const disabled = await page.locator("#user-input-textarea").isDisabled();
        assert.ok(disabled, "textarea should remain disabled for jobs without user input");
    });

    it("clicking the same job button deselects it", async () => {
        const btn = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        await btn.click(); // deselect
        const hasSelected = await btn.evaluate(el => el.classList.contains("selected"));
        assert.ok(!hasSelected, "button should not have selected class");
        const startText = await page.locator("#start-job-button").textContent();
        assert.strictEqual(startText, "Job Not Selected");
        const disabled = await page.locator("#start-job-button").isDisabled();
        assert.ok(disabled, "start job button should be disabled");
        const textareaDisabled = await page.locator("#user-input-textarea").isDisabled();
        assert.ok(textareaDisabled, "textarea should be disabled when deselected");
    });

    it("clicking a different job button switches selection", async () => {
        const btn1 = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        const btn2 = page.locator('.matrix-job-btn[data-job-name="seq-job"]');
        await btn1.click(); // select simple-job
        await btn2.click(); // select seq-job (deselects simple-job)
        const has1 = await btn1.evaluate(el => el.classList.contains("selected"));
        const has2 = await btn2.evaluate(el => el.classList.contains("selected"));
        assert.ok(!has1, "first button should not be selected");
        assert.ok(has2, "second button should be selected");
        const startText = await page.locator("#start-job-button").textContent();
        assert.strictEqual(startText, "Start Job: seq-job");
    });

    it("textarea enables for job with requireUserInput", async () => {
        // Select input-job which uses input-task with requireUserInput: true
        const btn = page.locator('.matrix-job-btn[data-job-name="input-job"]');
        await btn.click();
        const textareaDisabled = await page.locator("#user-input-textarea").isDisabled();
        assert.ok(!textareaDisabled, "textarea should be enabled for jobs with requireUserInput");
        await btn.click(); // deselect
    });

    it("user input textarea is visible", async () => {
        const visible = await page.locator("#user-input-textarea").isVisible();
        assert.ok(visible, "user input textarea should be visible");
    });
});

describe("Web: jobTracking.html redirect", () => {
    let browser;
    let page;

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
    });

    after(async () => {
        await browser?.close();
    });

    it("redirects to index.html when no jobName or jobId parameter", async () => {
        await page.goto(`${BASE}/jobTracking.html`);
        await page.waitForTimeout(1000);
        const url = new URL(page.url());
        assert.strictEqual(url.pathname, "/index.html", "should redirect to index.html");
    });

    it("does not redirect when only jobName is present (preview mode)", async () => {
        await page.goto(`${BASE}/jobTracking.html?jobName=simple-job`);
        await page.waitForTimeout(2000);
        const url = new URL(page.url());
        assert.strictEqual(url.pathname, "/jobTracking.html", "should stay on jobTracking.html in preview mode");
    });

    it("redirects when only jobId is present", async () => {
        await page.goto(`${BASE}/jobTracking.html?jobId=fake-job-id`);
        await page.waitForTimeout(1000);
        const url = new URL(page.url());
        assert.strictEqual(url.pathname, "/index.html", "should redirect to index.html");
    });
});

// Helper: start a job and return its jobId for use in jobTracking tests
async function startJobForTest(jobName = "simple-job") {
    const data = await fetchJson(`/api/copilot/job/start/${jobName}`, {
        method: "POST",
        body: "C:\\Code\\VczhLibraries\\Tools\ntest",
    });
    return data.jobId;
}

describe("Web: jobTracking.html layout", () => {
    let browser;
    let page;
    let testJobId;

    before(async () => {
        await verifyEntryInstalled();
        testJobId = await startJobForTest();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobTracking.html?jobName=simple-job&jobId=${encodeURIComponent(testJobId)}`);
        await page.waitForTimeout(3000);
    });

    after(async () => {
        await browser?.close();
    });

    it("loads jobTracking.css stylesheet", async () => {
        const links = await page.evaluate(() =>
            Array.from(document.querySelectorAll('link[rel="stylesheet"]')).map((l) => l.getAttribute("href"))
        );
        assert.ok(links.includes("jobTracking.css"), "should include jobTracking.css");
    });

    it("has left-part and right-part", async () => {
        const leftVisible = await page.locator("#left-part").isVisible();
        const rightVisible = await page.locator("#right-part").isVisible();
        assert.ok(leftVisible, "left part should be visible");
        assert.ok(rightVisible, "right part should be visible");
    });

    it("has horizontal resize bar", async () => {
        const visible = await page.locator("#resize-bar").isVisible();
        assert.ok(visible, "resize bar should be visible");
    });

    it("job-part is visible", async () => {
        const visible = await page.locator("#job-part").isVisible();
        assert.ok(visible, "job part should be visible");
    });

    it("session-response-part shows chart JSON", async () => {
        const text = await page.locator("#session-response-part").textContent();
        const parsed = JSON.parse(text);
        assert.ok(parsed.job, "should have job definition");
        assert.ok(parsed.chart, "should have chart data");
        assert.ok(Array.isArray(parsed.chart.nodes), "chart nodes should be an array");
        assert.ok(parsed.chart.nodes.length > 0, "chart nodes should not be empty");
    });
});

describe("Web: jobs.html Start Job opens new window", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        const context = await browser.newContext();
        page = await context.newPage();
        await page.goto(`${BASE}/jobs.html?wb=C%3A%5CCode%5CVczhLibraries%5CTools`);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("Start Job button calls API and opens jobTracking.html in a new window/tab", async () => {
        // Select a job
        const btn = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        await btn.click();
        await page.waitForTimeout(500);

        // Listen for new page (popup/tab)
        const context = page.context();
        const [newPage] = await Promise.all([
            context.waitForEvent("page", { timeout: 30000 }),
            page.locator("#start-job-button").click()
        ]);
        await newPage.waitForLoadState("domcontentloaded");
        const url = new URL(newPage.url());
        assert.strictEqual(url.pathname, "/jobTracking.html", "should open jobTracking.html");
        assert.strictEqual(url.searchParams.get("jobName"), "simple-job", "should pass jobName");
        assert.ok(url.searchParams.get("jobId"), "should pass jobId from start API");
        await newPage.close();
    });
});

describe("Web: jobTracking.html Mermaid renderer", () => {
    let browser;
    let page;
    let testJobId;

    before(async () => {
        await verifyEntryInstalled();
        testJobId = await startJobForTest();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobTracking.html?jobName=simple-job&jobId=${encodeURIComponent(testJobId)}`);
        await page.waitForTimeout(3000);
    });

    after(async () => {
        await browser?.close();
    });

    it("renders SVG in job-part with Mermaid renderer", async () => {
        const svgCount = await page.locator("#job-part svg").count();
        assert.ok(svgCount > 0, "should render an SVG element in job-part");
    });

    it("session-response-part shows chart JSON with mermaid renderer", async () => {
        const text = await page.locator("#session-response-part").textContent();
        const parsed = JSON.parse(text);
        assert.ok(parsed.job, "should have job definition");
        assert.ok(parsed.chart, "should have chart data");
    });
});

describe("Web: jobTracking.html TaskNode click interaction (Mermaid)", () => {
    let browser;
    let page;
    let testJobId;

    before(async () => {
        await verifyEntryInstalled();
        testJobId = await startJobForTest();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobTracking.html?jobName=simple-job&jobId=${encodeURIComponent(testJobId)}`);
        await page.waitForTimeout(3000);
    });

    after(async () => {
        await browser?.close();
    });

    it("clicking a TaskNode selects it with thicker border in Mermaid", async () => {
        const nodeGroup = page.locator('#job-part svg g.node').first();
        await nodeGroup.click();
        await page.waitForTimeout(200);
        const strokeWidth = await nodeGroup.locator('rect, polygon, circle, ellipse, path').first().evaluate(el => el.style.strokeWidth);
        assert.strictEqual(strokeWidth, "5px", "should have thick stroke-width after click");
    });

    it("clicking the same TaskNode again unselects it in Mermaid", async () => {
        const nodeGroup = page.locator('#job-part svg g.node').first();
        await nodeGroup.click(); // unselect
        await page.waitForTimeout(200);
        const strokeWidth = await nodeGroup.locator('rect, polygon, circle, ellipse, path').first().evaluate(el => el.style.strokeWidth);
        assert.ok(strokeWidth !== "5px", "should not have thick stroke-width after second click");
    });

    it("clicking a TaskNode shows tab control in session response part", async () => {
        const nodeGroup = page.locator('#job-part svg g.node').first();
        await nodeGroup.click(); // select / inspect
        await page.waitForTimeout(500);
        const tabContainer = page.locator('.tab-container');
        const isVisible = await tabContainer.isVisible();
        assert.ok(isVisible, "tab container should be visible when inspecting a task");

        // Unselect to restore JSON view
        await nodeGroup.click();
        await page.waitForTimeout(500);
    });

    it("clicking a TaskNode again restores JSON view in session response part", async () => {
        // Make sure JSON is shown after unselect
        const text = await page.locator("#session-response-part").textContent();
        const parsed = JSON.parse(text);
        assert.ok(parsed.job, "should show job JSON when no task inspected");
    });
});

describe("Web: jobTracking.html job status bar", () => {
    let browser;
    let page;
    let testJobId;

    before(async () => {
        await verifyEntryInstalled();
        testJobId = await startJobForTest();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobTracking.html?jobName=simple-job&jobId=${encodeURIComponent(testJobId)}`);
        await page.waitForTimeout(3000);
    });

    after(async () => {
        await browser?.close();
    });

    it("has job status label", async () => {
        const label = page.locator("#job-status-label");
        const visible = await label.isVisible();
        assert.ok(visible, "job status label should be visible");
        const text = await label.textContent();
        assert.ok(text.startsWith("JOB:"), "label should start with JOB:");
    });

    it("has Stop Job button", async () => {
        const btn = page.locator("#stop-job-button");
        const visible = await btn.isVisible();
        assert.ok(visible, "Stop Job button should be visible");
        const text = await btn.textContent();
        assert.strictEqual(text, "Stop Job");
    });
});

describe("Web: jobs.html Preview button", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        const context = await browser.newContext();
        page = await context.newPage();
        await page.goto(`${BASE}/jobs.html?wb=C%3A%5CCode%5CVczhLibraries%5CTools`);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("Preview button is disabled by default", async () => {
        const disabled = await page.locator("#preview-button").isDisabled();
        assert.ok(disabled, "Preview button should be disabled by default");
    });

    it("Preview button enables when a job is selected", async () => {
        const btn = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        await btn.click();
        await page.waitForTimeout(200);
        const disabled = await page.locator("#preview-button").isDisabled();
        assert.ok(!disabled, "Preview button should be enabled after selecting a job");
    });

    it("Preview button disables when job is deselected", async () => {
        const btn = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        await btn.click(); // deselect
        await page.waitForTimeout(200);
        const disabled = await page.locator("#preview-button").isDisabled();
        assert.ok(disabled, "Preview button should be disabled after deselecting");
    });

    it("Preview button opens jobTracking.html without jobId", async () => {
        const btn = page.locator('.matrix-job-btn[data-job-name="simple-job"]');
        await btn.click();
        await page.waitForTimeout(200);

        const context = page.context();
        const [newPage] = await Promise.all([
            context.waitForEvent("page", { timeout: 10000 }),
            page.locator("#preview-button").click()
        ]);
        await newPage.waitForLoadState("domcontentloaded");
        const url = new URL(newPage.url());
        assert.strictEqual(url.pathname, "/jobTracking.html", "should open jobTracking.html");
        assert.strictEqual(url.searchParams.get("jobName"), "simple-job", "should pass jobName");
        assert.strictEqual(url.searchParams.get("jobId"), null, "should NOT pass jobId");
        await newPage.close();
    });
});

describe("Web: jobTracking.html preview mode", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobTracking.html?jobName=simple-job`);
        await page.waitForTimeout(3000);
    });

    after(async () => {
        await browser?.close();
    });

    it("shows JOB: PREVIEW status in preview mode", async () => {
        const label = page.locator("#job-status-label");
        const text = await label.textContent();
        assert.strictEqual(text, "JOB: PREVIEW", "should show PREVIEW status");
    });

    it("Stop Job button is not present in preview mode", async () => {
        const count = await page.locator("#stop-job-button").count();
        assert.strictEqual(count, 0, "Stop Job button should not exist in preview mode");
    });

    it("renders SVG flow chart in preview mode", async () => {
        const svgCount = await page.locator("#job-part svg").count();
        assert.ok(svgCount > 0, "should render an SVG element in job-part");
    });

    it("clicking ChartNode does nothing in preview mode", async () => {
        const nodeGroup = page.locator('#job-part svg g.node').first();
        const nodeCount = await nodeGroup.count();
        if (nodeCount > 0) {
            await nodeGroup.click();
            await page.waitForTimeout(500);
            // In preview mode, clicking should not show tab container
            const tabCount = await page.locator('.tab-container').count();
            assert.strictEqual(tabCount, 0, "tab container should not appear in preview mode");
        }
    });

    it("session-response-part shows chart JSON in preview mode", async () => {
        const text = await page.locator("#session-response-part").textContent();
        const parsed = JSON.parse(text);
        assert.ok(parsed.job, "should have job definition in preview mode");
        assert.ok(parsed.chart, "should have chart data in preview mode");
    });
});

describe("Web: jobs.html handles undefined grid cells", () => {
    let browser;
    let page;

    before(async () => {
        await verifyEntryInstalled();
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/jobs.html?wb=C%3A%5CCode%5CVczhLibraries%5CTools`);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("renders empty cells for undefined job entries without errors", async () => {
        // The page should load without JS errors
        const errors = [];
        page.on("pageerror", (err) => errors.push(err.message));
        await page.waitForTimeout(500);
        // Check that the matrix table exists and rendered
        const tableVisible = await page.locator("#matrix-table").isVisible();
        assert.ok(tableVisible, "matrix table should render successfully");
        assert.strictEqual(errors.length, 0, "should have no JS errors");
    });
});

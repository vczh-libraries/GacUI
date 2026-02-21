import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import { chromium } from "playwright";

const BASE = "http://localhost:8888";

describe("Web: index.html setup UI", () => {
    let browser;
    let page;

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(BASE);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("shows setup UI on load", async () => {
        const visible = await page.locator("#setup-ui").isVisible();
        assert.ok(visible, "setup UI should be visible");
    });

    it("session UI is hidden on load", async () => {
        const display = await page.evaluate(() => {
            return getComputedStyle(document.getElementById("session-ui")).display;
        });
        assert.strictEqual(display, "none");
    });

    it("loads all 3 stylesheets", async () => {
        const links = await page.evaluate(() =>
            Array.from(document.querySelectorAll('link[rel="stylesheet"]')).map((l) => l.getAttribute("href"))
        );
        assert.ok(links.includes("index.css"), "should include index.css");
        assert.ok(links.includes("messageBlock.css"), "should include messageBlock.css");
        assert.ok(links.includes("sessionResponse.css"), "should include sessionResponse.css");
    });

    it("populates model select with sorted models", async () => {
        const options = await page.locator("#model-select option").allTextContents();
        assert.ok(options.length > 0, "should have model options");
        // Verify sorted
        const sorted = [...options].sort((a, b) => a.localeCompare(b));
        assert.deepStrictEqual(options, sorted, "model options should be sorted");
    });

    it("defaults model to gpt-5.2", async () => {
        const selected = await page.locator("#model-select").inputValue();
        assert.strictEqual(selected, "gpt-5.2");
    });

    it("has working directory defaulting to REPO-ROOT", async () => {
        const value = await page.locator("#working-dir").inputValue();
        // Should be the repo root (non-empty, absolute path)
        assert.ok(value.length > 0, "working dir should not be empty");
        // Verify it matches the server's config
        const config = await (await fetch(`${BASE}/api/config`)).json();
        assert.strictEqual(value, config.repoRoot, "working dir should default to REPO-ROOT");
    });

    it("has New Job and Refresh buttons on the left, Start button on the right", async () => {
        const jobsButton = page.locator("#jobs-button");
        const refreshButton = page.locator("#refresh-button");
        const startButton = page.locator("#start-button");
        assert.ok(await jobsButton.isVisible(), "New Job button should be visible");
        assert.ok(await refreshButton.isVisible(), "Refresh button should be visible");
        assert.ok(await startButton.isVisible(), "Start button should be visible");
        assert.strictEqual(await jobsButton.textContent(), "New Job");
        assert.strictEqual(await refreshButton.textContent(), "Refresh");
        assert.strictEqual(await startButton.textContent(), "Start");

        // Verify grouping: New Job and Refresh in left div, Start on the right
        const leftDiv = page.locator("#setup-buttons-left");
        assert.ok(await leftDiv.count() > 0, "setup-buttons-left should exist");

        // Verify button positions: left buttons should be left of Start
        const jobsBox = await jobsButton.boundingBox();
        const startBox = await startButton.boundingBox();
        assert.ok(jobsBox.x < startBox.x, "New Job should be to the left of Start");
    });

    it("Start button is enabled after models load", async () => {
        const disabled = await page.locator("#start-button").isDisabled();
        assert.ok(!disabled, "Start button should be enabled after models are loaded");
    });

    it("Jobs button navigates to jobs.html with working directory", async () => {
        const wd = await page.locator("#working-dir").inputValue();
        // Intercept navigation by listening for the URL change
        const [response] = await Promise.all([
            page.waitForURL(/\/jobs\.html\?wb=/),
            page.locator("#jobs-button").click(),
        ]);
        const url = new URL(page.url());
        assert.strictEqual(url.pathname, "/jobs.html");
        assert.strictEqual(url.searchParams.get("wb"), wd);
    });
});

describe("Web: index.html running jobs list", () => {
    let browser;
    let page;

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(BASE);
        await page.waitForTimeout(2000);
    });

    after(async () => {
        await browser?.close();
    });

    it("running jobs list container exists", async () => {
        const exists = await page.locator("#running-jobs-list").count();
        assert.ok(exists > 0, "running jobs list should exist");
    });

    it("Refresh button populates running jobs list", async () => {
        await page.locator("#refresh-button").click();
        await page.waitForTimeout(1000);
        // Verify the list was populated (may be empty if no jobs running, but the fetch should work)
        const listEl = page.locator("#running-jobs-list");
        const isVisible = await listEl.isVisible();
        assert.ok(isVisible, "running jobs list should be visible after refresh");
    });
});

describe("Web: index.html project parameter", () => {
    let browser;
    let page;

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        await page.goto(`${BASE}/index.html?project=TestProject`);
        await page.waitForTimeout(1000);
    });

    after(async () => {
        await browser?.close();
    });

    it("sets working directory from project parameter", async () => {
        const value = await page.locator("#working-dir").inputValue();
        // Should be REPO-ROOT\..\TestProject
        const config = await (await fetch(`${BASE}/api/config`)).json();
        const repoRoot = config.repoRoot;
        const sep = repoRoot.includes("\\") ? "\\" : "/";
        const parentIdx = Math.max(repoRoot.lastIndexOf("/"), repoRoot.lastIndexOf("\\"));
        const parentDir = parentIdx > 0 ? repoRoot.substring(0, parentIdx) : repoRoot;
        const expected = parentDir + sep + "TestProject";
        assert.strictEqual(value, expected, `working dir should be ${expected}`);
    });
});

describe("Web: index.html session interaction", () => {
    let browser;
    let page;
    let sessionId;
    const consoleErrors = [];

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        page.on("console", (msg) => {
            if (msg.type() === "error") consoleErrors.push(msg.text());
        });
        await page.goto(BASE);
        await page.waitForTimeout(2000);

        // Find a free model
        const models = await (await fetch(`${BASE}/api/copilot/models`)).json();
        const freeModel = models.models.find((m) => m.multiplier === 0);
        assert.ok(freeModel, "need a free model for testing");

        // Start session and capture actual sessionId from network response
        await page.locator("#model-select").selectOption(freeModel.id);
        await page.locator("#working-dir").fill("C:\\Code\\VczhLibraries\\Tools");
        const responsePromise = page.waitForResponse(resp => resp.url().includes("/api/copilot/session/start/"));
        await page.locator("#start-button").click();
        const startResponse = await responsePromise;
        const startData = await startResponse.json();
        sessionId = startData.sessionId;
        await page.waitForTimeout(3000);
    });

    after(async () => {
        // Stop session if still open
        if (sessionId) {
            try {
                await fetch(`${BASE}/api/copilot/session/${sessionId}/stop`);
            } catch {
                // ignore
            }
        }
        await browser?.close();
    });

    it("hides setup UI and shows session UI after start", async () => {
        const setupVisible = await page.locator("#setup-ui").isVisible();
        assert.ok(!setupVisible, "setup UI should be hidden");
        const sessionVisible = await page.locator("#session-ui").isVisible();
        assert.ok(sessionVisible, "session UI should be visible");
    });

    it("session-part has session-response-container class from renderer", async () => {
        const hasClass = await page.evaluate(() =>
            document.getElementById("session-part").classList.contains("session-response-container")
        );
        assert.ok(hasClass, "session-part should have session-response-container class");
    });

    it("awaiting status element exists inside session-part", async () => {
        const exists = await page.evaluate(() => {
            const sp = document.getElementById("session-part");
            return sp.querySelector(".session-response-awaiting") !== null;
        });
        assert.ok(exists, "should have awaiting status element");
    });

    it("awaiting status is hidden when session is idle", async () => {
        const display = await page.evaluate(() => {
            const sp = document.getElementById("session-part");
            return sp.querySelector(".session-response-awaiting").style.display;
        });
        assert.strictEqual(display, "none", "awaiting should be hidden when idle");
    });

    it("send button is enabled initially", async () => {
        const disabled = await page.locator("#send-button").isDisabled();
        assert.ok(!disabled, "send button should be enabled");
    });

    it("task select combo box exists with (none) default", async () => {
        const taskSelect = page.locator("#task-select");
        const visible = await taskSelect.isVisible();
        assert.ok(visible, "task select should be visible");

        const options = await taskSelect.locator("option").allTextContents();
        assert.ok(options.length >= 1, "should have at least (none) option");
        assert.strictEqual(options[0], "(none)", "first option should be (none)");

        const selectedValue = await taskSelect.inputValue();
        assert.strictEqual(selectedValue, "", "(none) should be selected by default");
    });

    it("sends a request and creates User message block", async () => {
        await page.locator("#request-textarea").fill("What is 2+2? Just the number.");
        await page.locator("#send-button").click();

        // User block should appear immediately
        await page.waitForTimeout(500);
        const userBlocks = await page.locator('.message-block').count();
        assert.ok(userBlocks >= 1, "should have at least one message block (User)");

        // First block should contain our text
        const firstBlockText = await page.locator(".message-block .message-block-body").first().textContent();
        assert.ok(firstBlockText.includes("2+2"), "first block should contain user request");
    });

    it("send button disabled while waiting for response", async () => {
        // Should still be disabled from previous send
        const disabled = await page.locator("#send-button").isDisabled();
        assert.ok(disabled, "send button should be disabled during response");
    });

    it("awaiting status shown while waiting for response", async () => {
        const display = await page.evaluate(() => {
            const sp = document.getElementById("session-part");
            return sp.querySelector(".session-response-awaiting").style.display;
        });
        assert.strictEqual(display, "block", "awaiting should be shown during response");
    });

    it("response creates additional message blocks and re-enables send", async () => {
        // Wait for response to complete
        await page.waitForFunction(
            () => !document.getElementById("send-button").disabled,
            { timeout: 60000 }
        );

        const blockCount = await page.locator(".message-block").count();
        assert.ok(blockCount >= 2, `should have at least 2 message blocks (User + response), got ${blockCount}`);

        const sendDisabled = await page.locator("#send-button").isDisabled();
        assert.ok(!sendDisabled, "send button should be re-enabled");
    });

    it("awaiting status hidden after response completes", async () => {
        const display = await page.evaluate(() => {
            const sp = document.getElementById("session-part");
            return sp.querySelector(".session-response-awaiting").style.display;
        });
        assert.strictEqual(display, "none", "awaiting should be hidden after response");
    });

    it("completed message blocks have correct CSS classes", async () => {
        const blocks = await page.locator(".message-block.completed").count();
        assert.ok(blocks >= 2, "should have completed message blocks");
    });

    it("User block has completed class and is expanded", async () => {
        const userBlock = page.locator(".message-block").first();
        const hasCompleted = await userBlock.evaluate((el) => el.classList.contains("completed"));
        assert.ok(hasCompleted, "User block should be completed");
        const isCollapsed = await userBlock.evaluate((el) => el.classList.contains("collapsed"));
        assert.ok(!isCollapsed, "User block should be expanded");
    });

    it("resize bar exists and is visible", async () => {
        const visible = await page.locator("#resize-bar").isVisible();
        assert.ok(visible, "resize bar should be visible");
    });

    it("request textarea is visible and functional", async () => {
        const visible = await page.locator("#request-textarea").isVisible();
        assert.ok(visible, "textarea should be visible");
        await page.locator("#request-textarea").fill("test");
        const value = await page.locator("#request-textarea").inputValue();
        assert.strictEqual(value, "test");
        await page.locator("#request-textarea").fill("");
    });

    it("Stop Server and Close Session buttons are visible", async () => {
        const stopVisible = await page.locator("#stop-server-button").isVisible();
        assert.ok(stopVisible, "Stop Server button should be visible");
        const closeVisible = await page.locator("#close-session-button").isVisible();
        assert.ok(closeVisible, "Close Session button should be visible");
    });

    it("no console errors during session", () => {
        assert.strictEqual(consoleErrors.length, 0, `console errors: ${consoleErrors.join("; ")}`);
    });
});

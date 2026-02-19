import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import { chromium } from "playwright";

const BASE = "http://localhost:8888";

describe("Web: test.html", () => {
    let browser;
    let page;

    before(async () => {
        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
    });

    after(async () => {
        await browser?.close();
    });

    it("loads test.html and shows Hello, world!", async () => {
        await page.goto(`${BASE}/test.html`);
        await page.waitForTimeout(2000);
        const text = await page.locator("body").textContent();
        assert.strictEqual(text.trim(), "Hello, world!");
    });
});


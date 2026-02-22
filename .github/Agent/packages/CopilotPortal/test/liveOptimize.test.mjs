import { describe, it } from "node:test";
import assert from "node:assert/strict";
import { createLiveEntityState, pushLiveResponse, waitForLiveResponse, closeLiveEntity } from "../dist/sharedApi.js";

// Helper: create an entity and push a sequence of responses, return entity
function pushAll(entity, responses) {
    for (const r of responses) {
        pushLiveResponse(entity, r);
    }
    return entity;
}

// Helper: create a fresh entity with large countdown
function makeEntity() {
    return createLiveEntityState(60000);
}

// Helper: register a token by calling waitForLiveResponse (it will create the token)
// Then drain immediately available responses
async function registerAndDrainToken(entity, token) {
    const result = await waitForLiveResponse(entity, token, 1, "NotFound", "Closed");
    return result;
}

describe("Live optimization: onEndReasoning removes deltas", () => {
    it("removes all onReasoning deltas between onStartReasoning and onEndReasoning", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "c" });
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r1", completeContent: "abc" });

        // Should have: onStartReasoning, onEndReasoning (deltas removed)
        const callbacks = entity.responses.map(r => r.callback);
        assert.ok(!callbacks.includes("onReasoning"), "onReasoning deltas should be removed");
        assert.ok(callbacks.includes("onStartReasoning"), "onStartReasoning should remain");
        assert.ok(callbacks.includes("onEndReasoning"), "onEndReasoning should remain");
        assert.strictEqual(entity.responses.length, 2);
    });

    it("only removes deltas for the matching id", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "x" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r2", delta: "y" }); // different id
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r1", completeContent: "x" });

        // r2 delta should remain
        const r2Deltas = entity.responses.filter(r => r.callback === "onReasoning" && r.reasoningId === "r2");
        assert.strictEqual(r2Deltas.length, 1, "delta for r2 should remain untouched");
    });

    it("stops at onStartReasoning boundary (does not cross into previous block)", () => {
        const entity = makeEntity();
        // First reasoning block (already ended)
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r1", completeContent: "done" });
        // Stray delta from r2 before its start (edge case)
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r2" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r2", delta: "a" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r2", delta: "b" });
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r2", completeContent: "ab" });

        // Both blocks should have start + end, no deltas
        const r2Deltas = entity.responses.filter(r => r.callback === "onReasoning" && r.reasoningId === "r2");
        assert.strictEqual(r2Deltas.length, 0, "r2 deltas should be removed");
        assert.strictEqual(entity.responses.length, 4); // start1, end1, start2, end2
    });

    it("adjusts token positions when deltas are removed", async () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" });

        // Register a token and drain (reads start + delta = position 2)
        const result = await registerAndDrainToken(entity, "tok1");
        assert.strictEqual(result.responses.length, 2);
        assert.strictEqual(entity.tokens.get("tok1").position, 2);

        // Push another delta (index 2 — unread by tok1; "a" at index 1 is all-read but only 1, no merge)
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" });
        assert.strictEqual(entity.tokens.get("tok1").position, 2);

        // Now push end — removes "a" (index 1) and "b" (index 2), adjusts token position
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r1", completeContent: "ab" });

        // entity now has [onStartReasoning, onEndReasoning]
        assert.strictEqual(entity.responses.length, 2);
        assert.strictEqual(entity.responses[0].callback, "onStartReasoning");
        assert.strictEqual(entity.responses[1].callback, "onEndReasoning");
        // Token read 2 items originally. "a" removed at index 1 (position was > 1 → adjusted).
        // "b" removed at index 1 after previous removal. position 1 > 1? No. Position stays.
        const tokenState = entity.tokens.get("tok1");
        assert.strictEqual(tokenState.position, 1, "token position should be adjusted after delta removal");
    });
});

describe("Live optimization: onEndMessage removes deltas", () => {
    it("removes all onMessage deltas between onStartMessage and onEndMessage", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartMessage", messageId: "m1" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: "hello" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: " world" });
        pushLiveResponse(entity, { callback: "onEndMessage", messageId: "m1", completeContent: "hello world" });

        const callbacks = entity.responses.map(r => r.callback);
        assert.ok(!callbacks.includes("onMessage"), "onMessage deltas should be removed");
        assert.strictEqual(entity.responses.length, 2);
    });
});

describe("Live optimization: onReasoning merges deltas (no tokens)", () => {
    it("merges all deltas when no tokens are registered", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "c" });

        // Should merge the 3 deltas into 1
        const deltas = entity.responses.filter(r => r.callback === "onReasoning");
        assert.strictEqual(deltas.length, 1, "should merge all deltas into one");
        assert.strictEqual(deltas[0].delta, "abc", "merged delta should concatenate all");
    });

    it("does not merge across start boundary", () => {
        const entity = makeEntity();
        // Block 1 ended
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r1", completeContent: "x" });
        // Block 2 in progress
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r2" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r2", delta: "a" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r2", delta: "b" });

        const r2Deltas = entity.responses.filter(r => r.callback === "onReasoning" && r.reasoningId === "r2");
        assert.strictEqual(r2Deltas.length, 1, "r2 deltas should merge");
        assert.strictEqual(r2Deltas[0].delta, "ab");
    });
});

describe("Live optimization: onReasoning merges deltas (with tokens)", () => {
    it("merges unread deltas (>= largest token position)", async () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" });

        // Register token and drain (position = 2)
        await registerAndDrainToken(entity, "tok1");

        // Push more deltas — these are unread by tok1
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "c" });

        // The first delta at index 1 is at position < tok1.position (2), so it's "all-read"
        // The delta at index 2 ("b") and the newly pushed "c" at index 3 are >= tok1.position
        // "b" and "c" should merge together
        const unreadDeltas = entity.responses.filter((r, i) => r.callback === "onReasoning" && i >= 2);
        assert.strictEqual(unreadDeltas.length, 1, "unread deltas should merge");
        assert.strictEqual(unreadDeltas[0].delta, "bc", "merged unread delta");
    });

    it("merges all-read deltas (< smallest token position)", async () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });

        // Register token first so deltas aren't all merged immediately
        await registerAndDrainToken(entity, "tok1");
        assert.strictEqual(entity.tokens.get("tok1").position, 1);

        // Push delta "a" (index 1, unread, only 1 → no merge)
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" });
        // Drain tok1 to position 2
        const d1 = await waitForLiveResponse(entity, "tok1", 1, "NotFound", "Closed");
        assert.strictEqual(d1.responses[0].delta, "a");
        assert.strictEqual(entity.tokens.get("tok1").position, 2);

        // Push delta "b" (index 2, unread 1, all-read "a" at 1 is only 1 → no merge)
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" });
        // Drain tok1 to position 3
        const d2 = await waitForLiveResponse(entity, "tok1", 1, "NotFound", "Closed");
        assert.strictEqual(d2.responses[0].delta, "b");
        assert.strictEqual(entity.tokens.get("tok1").position, 3);

        // Push delta "c" → now "a"(idx 1) and "b"(idx 2) are both < position 3 → merge!
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "c" });

        const allDeltas = entity.responses.filter(r => r.callback === "onReasoning");
        assert.strictEqual(allDeltas.length, 2, "should have one merged read + one unread");
        assert.strictEqual(allDeltas[0].delta, "ab", "all-read deltas merged");
        assert.strictEqual(allDeltas[1].delta, "c", "unread delta remains");
        // Token position adjusted from 3 to 2 (one entry removed before position)
        assert.strictEqual(entity.tokens.get("tok1").position, 2);
    });

    it("does not merge deltas between smallest and largest positions", async () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" }); // index 1
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" }); // index 2
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "c" }); // index 3

        // tok1 drains all (position = 4)
        await registerAndDrainToken(entity, "tok1");
        // tok2 drains only first 2 (position = 2) — we need to create tok2 BEFORE adding more
        // Actually positions are set on drain. Let's do it differently.

        const entity2 = makeEntity();
        pushLiveResponse(entity2, { callback: "onStartReasoning", reasoningId: "r1" });

        // Register tok1 and tok2 by draining empty (will get HttpRequestTimeout or initial batch)
        const r1 = await waitForLiveResponse(entity2, "tok1", 1, "NotFound", "Closed");
        const r2 = await waitForLiveResponse(entity2, "tok2", 1, "NotFound", "Closed");
        // Both tokens at position 1 (read onStartReasoning)

        // Push delta "a" at index 1
        pushLiveResponse(entity2, { callback: "onReasoning", reasoningId: "r1", delta: "a" });
        // Drain tok1 to position 2
        const d1 = await waitForLiveResponse(entity2, "tok1", 1, "NotFound", "Closed");
        assert.strictEqual(d1.responses[0].delta, "a");

        // Now tok1 is at position 2, tok2 is at position 1
        // Push delta "b" — at index 2
        pushLiveResponse(entity2, { callback: "onReasoning", reasoningId: "r1", delta: "b" });

        // "a" at index 1: >= smallest(1)? yes. < largest(2)? yes. → in between, don't merge
        // "b" at index 2: >= largest(2)? yes. → unread group, but only 1 entry
        // Nothing should merge
        const deltas = entity2.responses.filter(r => r.callback === "onReasoning");
        assert.strictEqual(deltas.length, 2, "deltas between token positions should not merge");
    });

    it("adjusts positions correctly after merging all-read deltas", async () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });

        // Register token (position = 1 after reading start)
        await registerAndDrainToken(entity, "tok1");

        // Push and drain deltas one by one to avoid intermediate merging
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "a" });
        await waitForLiveResponse(entity, "tok1", 1, "NotFound", "Closed");
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "b" });
        await waitForLiveResponse(entity, "tok1", 1, "NotFound", "Closed");
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "c" });
        await waitForLiveResponse(entity, "tok1", 1, "NotFound", "Closed");

        // Token at position 4, entity: [start, "ab", "c"]
        // ("a" and "b" were merged when "c" was pushed, so position went from 4 to 3)
        const posBefore = entity.tokens.get("tok1").position;

        // Push new delta "d" — triggers merge of all-read entries before posBefore
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "d" });

        // Token position should have decreased if entries were merged
        const posAfter = entity.tokens.get("tok1").position;
        assert.ok(posAfter <= posBefore, "token position should be adjusted or unchanged after merge");

        // All entries still accessible
        const allDeltas = entity.responses.filter(r => r.callback === "onReasoning");
        const mergedContent = allDeltas.map(r => r.delta).join("");
        assert.strictEqual(mergedContent, "abcd", "all delta content preserved");
    });
});

describe("Live optimization: onMessage merges deltas", () => {
    it("merges message deltas identically to reasoning deltas", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartMessage", messageId: "m1" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: "hello" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: " " });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: "world" });

        const deltas = entity.responses.filter(r => r.callback === "onMessage");
        assert.strictEqual(deltas.length, 1, "should merge all message deltas");
        assert.strictEqual(deltas[0].delta, "hello world");
    });

    it("onEndMessage removes onMessage deltas scoped to block", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartMessage", messageId: "m1" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: "hi" });
        pushLiveResponse(entity, { callback: "onStartMessage", messageId: "m2" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m2", delta: "bye" });
        pushLiveResponse(entity, { callback: "onEndMessage", messageId: "m2", completeContent: "bye" });

        // m2 deltas removed, m1 delta remains
        const m1Deltas = entity.responses.filter(r => r.callback === "onMessage" && r.messageId === "m1");
        const m2Deltas = entity.responses.filter(r => r.callback === "onMessage" && r.messageId === "m2");
        assert.strictEqual(m1Deltas.length, 1, "m1 delta should remain");
        assert.strictEqual(m2Deltas.length, 0, "m2 deltas should be removed");
    });
});

describe("Live optimization: interleaved reasoning and message", () => {
    it("optimizations do not interfere across callback types", () => {
        const entity = makeEntity();
        pushLiveResponse(entity, { callback: "onStartReasoning", reasoningId: "r1" });
        pushLiveResponse(entity, { callback: "onReasoning", reasoningId: "r1", delta: "think" });
        pushLiveResponse(entity, { callback: "onStartMessage", messageId: "m1" });
        pushLiveResponse(entity, { callback: "onMessage", messageId: "m1", delta: "say" });
        pushLiveResponse(entity, { callback: "onEndReasoning", reasoningId: "r1", completeContent: "think" });
        pushLiveResponse(entity, { callback: "onEndMessage", messageId: "m1", completeContent: "say" });

        const callbacks = entity.responses.map(r => r.callback);
        assert.ok(!callbacks.includes("onReasoning"), "reasoning deltas removed");
        assert.ok(!callbacks.includes("onMessage"), "message deltas removed");
        assert.strictEqual(entity.responses.length, 4); // 2 starts + 2 ends
    });
});

import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";
import path from "node:path";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const serverScript = path.resolve(__dirname, "..", "dist", "index.js");
const windowsHidePatch = path.resolve(__dirname, "windowsHide.cjs");

// Spawn server as detached process in test mode so it runs independently of this script.
// On Windows, the Copilot SDK internally spawns node.exe to run its bundled CLI server.
// Without windowsHide, each spawn creates a visible console window that steals keyboard focus.
// We use --require to preload a CJS patch (windowsHide.cjs) that adds windowsHide: true
// to all child_process.spawn calls before any ESM modules (including the SDK) are loaded.
const child = spawn("node", ["--require", windowsHidePatch, serverScript, "--test"], {
    detached: true,
    stdio: "ignore",
    cwd: path.resolve(__dirname, ".."),
    windowsHide: true,
});
child.unref();

// Wait for server to be ready by polling api/test
const maxRetries = 30;
for (let i = 0; i < maxRetries; i++) {
    try {
        const res = await fetch("http://localhost:8888/api/test");
        if (res.ok) {
            console.log("Server is ready at http://localhost:8888");
            process.exit(0);
        }
    } catch {
        // Not ready yet
    }
    await new Promise((r) => setTimeout(r, 500));
}

console.error("Server failed to start within timeout");
process.exit(1);

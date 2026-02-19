import { SessionResponseRenderer } from "./sessionResponse.js";

// ---- State ----
let sessionId = null;
let livePollingActive = false;
let sendEnabled = false;

// ---- DOM references ----
const setupUi = document.getElementById("setup-ui");
const sessionUi = document.getElementById("session-ui");
const modelSelect = document.getElementById("model-select");
const workingDirInput = document.getElementById("working-dir");
const startButton = document.getElementById("start-button");
const jobsButton = document.getElementById("jobs-button");
const sessionPart = document.getElementById("session-part");
const requestTextarea = document.getElementById("request-textarea");
const sendButton = document.getElementById("send-button");
const stopServerButton = document.getElementById("stop-server-button");
const closeSessionButton = document.getElementById("close-session-button");
const resizeBar = document.getElementById("resize-bar");
const requestPart = document.getElementById("request-part");
const taskSelect = document.getElementById("task-select");

// ---- Session Response Renderer ----
const sessionRenderer = new SessionResponseRenderer(sessionPart);

// ---- Setup: Load models and defaults ----

async function loadModels() {
    try {
        const res = await fetch("/api/copilot/models");
        const data = await res.json();
        const models = data.models.sort((a, b) => a.name.localeCompare(b.name));
        modelSelect.innerHTML = "";
        for (const m of models) {
            const option = document.createElement("option");
            option.value = m.id;
            option.textContent = m.name;
            modelSelect.appendChild(option);
        }
        // Default to gpt-5.2
        const defaultOption = modelSelect.querySelector('option[value="gpt-5.2"]');
        if (defaultOption) {
            defaultOption.selected = true;
        }
        startButton.disabled = false;
    } catch (err) {
        console.error("Failed to load models:", err);
    }
}

async function initWorkingDir() {
    try {
        const res = await fetch("/api/config");
        const config = await res.json();
        const repoRoot = config.repoRoot;
        const params = new URLSearchParams(window.location.search);
        const project = params.get("project");
        if (project) {
            const sep = repoRoot.includes("\\") ? "\\" : "/";
            const parentIdx = Math.max(repoRoot.lastIndexOf("/"), repoRoot.lastIndexOf("\\"));
            const parentDir = parentIdx > 0 ? repoRoot.substring(0, parentIdx) : repoRoot;
            workingDirInput.value = parentDir + sep + project;
        } else {
            workingDirInput.value = repoRoot;
        }
    } catch (err) {
        console.error("Failed to load config:", err);
    }
}

// ---- Jobs Navigation ----

jobsButton.addEventListener("click", () => {
    const wd = workingDirInput.value;
    window.location.href = `/jobs.html?wb=${encodeURIComponent(wd)}`;
});

// ---- Start Session ----

startButton.addEventListener("click", async () => {
    const modelId = modelSelect.value;
    if (!modelId) return;

    startButton.disabled = true;
    try {
        const res = await fetch(`/api/copilot/session/start/${encodeURIComponent(modelId)}`, {
            method: "POST",
            body: workingDirInput.value,
        });
        const data = await res.json();
        if (data.error) {
            alert("Failed to start session: " + data.error);
            startButton.disabled = false;
            return;
        }
        sessionId = data.sessionId;
        setupUi.style.display = "none";
        sessionUi.style.display = "flex";
        sendEnabled = true;
        sendButton.disabled = false;
        startLivePolling();
    } catch (err) {
        alert("Failed to start session: " + err);
        startButton.disabled = false;
    }
});

// ---- Live Polling ----

function startLivePolling() {
    livePollingActive = true;
    pollLive();
}

async function pollLive() {
    while (livePollingActive) {
        try {
            const res = await fetch(`/api/copilot/session/${encodeURIComponent(sessionId)}/live`);
            if (!livePollingActive) break;
            const data = await res.json();
            if (!livePollingActive) break;

            if (data.error === "HttpRequestTimeout") {
                // Timeout, just resend
                continue;
            }
            if (data.error === "SessionNotFound") {
                livePollingActive = false;
                break;
            }
            if (data.sessionError) {
                console.error("Session error:", data.sessionError);
                continue;
            }
            if (data.callback) {
                processCallback(data);
            }
        } catch (err) {
            if (!livePollingActive) break;
            // Network error, retry after brief delay
            await new Promise(r => setTimeout(r, 500));
        }
    }
}

// ---- Process Callbacks ----

function processCallback(data) {
    const cb = data.callback;
    if (cb === "onGeneratedUserPrompt") {
        sessionRenderer.addUserMessage(data.prompt, "Task");
        return cb;
    }
    const result = sessionRenderer.processCallback(data);
    if (result === "onIdle") {
        setSendEnabled(true);
    }
    return result;
}

// ---- Send / Request ----

function setSendEnabled(enabled) {
    sendEnabled = enabled;
    sendButton.disabled = !enabled;
    sessionRenderer.setAwaiting(!enabled);
}

async function sendRequest() {
    if (!sendEnabled) return;
    const text = requestTextarea.value.trim();
    if (!text) return;

    setSendEnabled(false);
    requestTextarea.value = "";

    const selectedTask = taskSelect.value;

    if (selectedTask) {
        // Start a task with the selected task name
        try {
            await fetch(`/api/copilot/task/start/${encodeURIComponent(selectedTask)}/session/${encodeURIComponent(sessionId)}`, {
                method: "POST",
                body: text,
            });
        } catch (err) {
            console.error("Failed to start task:", err);
            setSendEnabled(true);
        }
    } else {
        // Talk to the session directly
        sessionRenderer.addUserMessage(text);
        try {
            await fetch(`/api/copilot/session/${encodeURIComponent(sessionId)}/query`, {
                method: "POST",
                body: text,
            });
        } catch (err) {
            console.error("Failed to send query:", err);
            setSendEnabled(true);
        }
    }
}

sendButton.addEventListener("click", sendRequest);

requestTextarea.addEventListener("keydown", (e) => {
    if (e.ctrlKey && e.key === "Enter") {
        e.preventDefault();
        sendRequest();
    }
});

// ---- Stop / Close ----

function closeWindow() {
    // Chrome blocks window.close() for tabs not opened by script.
    // Try to close, then fall back to a "safe to close" page.
    window.close();
    // If we're still here after a short delay, the browser blocked window.close().
    setTimeout(() => {
        document.title = "Session Ended";
        document.body.innerHTML = '<div style="display:flex;align-items:center;justify-content:center;height:100vh;font-family:sans-serif;color:#ccc;background:#1e1e1e;"><h1>Session ended — you may close this tab.</h1></div>';
    }, 200);
}

async function closeSession() {
    livePollingActive = false;
    try {
        await fetch(`/api/copilot/session/${encodeURIComponent(sessionId)}/stop`);
    } catch (err) {
        // Ignore errors during shutdown
    }
    closeWindow();
}

closeSessionButton.addEventListener("click", closeSession);

stopServerButton.addEventListener("click", async () => {
    livePollingActive = false;
    try {
        await fetch(`/api/copilot/session/${encodeURIComponent(sessionId)}/stop`);
        await fetch("/api/stop");
    } catch (err) {
        // Ignore errors during shutdown
    }
    closeWindow();
});

// ---- Resize Bar ----

let resizing = false;

resizeBar.addEventListener("mousedown", (e) => {
    e.preventDefault();
    resizing = true;
});

document.addEventListener("mousemove", (e) => {
    if (!resizing) return;
    const containerRect = sessionUi.getBoundingClientRect();
    const newRequestHeight = containerRect.bottom - e.clientY - resizeBar.offsetHeight / 2;
    const clamped = Math.max(100, Math.min(newRequestHeight, containerRect.height - 100));
    requestPart.style.height = clamped + "px";
});

document.addEventListener("mouseup", () => {
    resizing = false;
});

// ---- Init ----

async function loadTasks() {
    try {
        const res = await fetch("/api/copilot/task");
        const data = await res.json();
        // Keep the (none) option; add tasks
        for (const t of data.tasks) {
            const option = document.createElement("option");
            option.value = t.name;
            option.textContent = t.name;
            taskSelect.appendChild(option);
        }
    } catch (err) {
        console.error("Failed to load tasks:", err);
    }
}

loadModels();
initWorkingDir();
loadTasks();

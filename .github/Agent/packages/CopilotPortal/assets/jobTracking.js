import { SessionResponseRenderer } from "./sessionResponse.js";

// ---- Redirect if no jobName ----
const params = new URLSearchParams(window.location.search);
const jobName = params.get("jobName");
const jobId = params.get("jobId");
if (!jobName) {
    window.location.href = "/index.html";
}

const isPreviewMode = !jobId;

// ---- DOM references ----
const jobPart = document.getElementById("job-part");
const sessionResponsePart = document.getElementById("session-response-part");
const resizeBar = document.getElementById("resize-bar");
const rightPart = document.getElementById("right-part");

// ---- State ----
let chartController = null; // returned from renderFlowChartMermaid
let jobStatus = isPreviewMode ? "PREVIEW" : "RUNNING"; // PREVIEW | RUNNING | SUCCEEDED | FAILED | CANCELED
let jobStopped = false;

// Map: workId -> { taskId, sessions: Map<sessionId, { name, renderer, div, active }>, attemptCount }
const workIdData = {};

// Currently inspected workId (null = none)
let inspectedWorkId = null;

// ---- JSON display state ----
let jobToRender = null;
let chartToRender = null;
let jsonPre = null;

// ---- Session response tab control ----
let tabContainer = null;

// ---- Job status bar elements ----
let statusLabel = null;
let stopJobButton = null;

function createStatusBar() {
    const bar = document.createElement("div");
    bar.id = "job-status-bar";

    statusLabel = document.createElement("div");
    statusLabel.id = "job-status-label";
    statusLabel.textContent = `JOB: ${jobStatus}`;
    bar.appendChild(statusLabel);

    if (!isPreviewMode) {
        stopJobButton = document.createElement("button");
        stopJobButton.id = "stop-job-button";
        stopJobButton.textContent = "Stop Job";
        stopJobButton.addEventListener("click", async () => {
            if (jobStopped) return;
            try {
                await fetch(`/api/copilot/job/${encodeURIComponent(jobId)}/stop`, { method: "POST" });
                jobStopped = true;
                jobStatus = "CANCELED";
                updateStatusLabel();
                stopJobButton.disabled = true;
            } catch (err) {
                console.error("Failed to stop job:", err);
            }
        });
        bar.appendChild(stopJobButton);
    }

    return bar;
}

function updateStatusLabel() {
    if (statusLabel) {
        statusLabel.textContent = `JOB: ${jobStatus}`;
        statusLabel.className = "";
        statusLabel.classList.add(`job-status-${jobStatus.toLowerCase()}`);
    }
    if (stopJobButton) {
        stopJobButton.disabled = jobStatus !== "RUNNING";
    }
}

// ---- Session Response Part Management ----

function switchTabForWork(workId, sessionId) {
    const data = workIdData[workId];
    if (!data) return;
    if (data.activeTabSessionId === sessionId) return;
    data.activeTabSessionId = sessionId;

    if (inspectedWorkId !== workId || !tabContainer) return;

    const tabHeaders = tabContainer.querySelector(".tab-headers");
    if (!tabHeaders) return;

    for (const btn of tabHeaders.querySelectorAll(".tab-header-btn")) {
        btn.classList.toggle("active", btn.dataset.sessionId === sessionId);
    }
    for (const [sid, sInfo] of data.sessions) {
        sInfo.div.style.display = sid === sessionId ? "block" : "none";
    }
}

function showJsonView() {
    sessionResponsePart.innerHTML = "";
    if (tabContainer) {
        tabContainer = null;
    }
    jsonPre = document.createElement("pre");
    jsonPre.style.padding = "8px";
    jsonPre.textContent = JSON.stringify({ job: jobToRender, chart: chartToRender }, undefined, 4);
    sessionResponsePart.appendChild(jsonPre);
}

function showTaskSessionTabs(workId) {
    sessionResponsePart.innerHTML = "";
    const data = workIdData[workId];
    if (!data) {
        sessionResponsePart.textContent = "No session data for this task.";
        return;
    }

    // Reset active tab tracking so the first switchTabForWork always applies
    data.activeTabSessionId = null;

    tabContainer = document.createElement("div");
    tabContainer.className = "tab-container";

    const tabHeaders = document.createElement("div");
    tabHeaders.className = "tab-headers";
    tabContainer.appendChild(tabHeaders);

    const tabContent = document.createElement("div");
    tabContent.className = "tab-content";
    tabContainer.appendChild(tabContent);

    sessionResponsePart.appendChild(tabContainer);

    // Ensure "Driving" tab always appears first
    const sortedEntries = [...data.sessions.entries()].sort((a, b) => {
        if (a[1].name === "Driving") return -1;
        if (b[1].name === "Driving") return 1;
        return 0;
    });

    for (const [sessionId, sessionInfo] of sortedEntries) {
        const tabBtn = document.createElement("button");
        tabBtn.className = "tab-header-btn";
        tabBtn.textContent = sessionInfo.name;
        tabBtn.dataset.sessionId = sessionId;
        tabBtn.addEventListener("click", () => {
            switchTabForWork(workId, tabBtn.dataset.sessionId);
        });
        tabHeaders.appendChild(tabBtn);

        // Append the session's div to tab content (hidden by default)
        sessionInfo.div.style.display = "none";
        tabContent.appendChild(sessionInfo.div);
    }

    // Activate the first tab
    const firstEntry = sortedEntries[0];
    if (firstEntry) {
        switchTabForWork(workId, firstEntry[0]);
    }
}

function refreshSessionResponsePart() {
    if (inspectedWorkId !== null) {
        showTaskSessionTabs(inspectedWorkId);
    } else {
        showJsonView();
    }
}

function onInspect(workId) {
    inspectedWorkId = workId;
    refreshSessionResponsePart();
}

// ---- Live Polling Helpers ----

async function pollLive(url, handler, shouldStop) {
    const terminalPattern = /^(Session|Task|Jobs?)(Closed|NotFound)$/;

    // Acquire a token for this polling session
    let token;
    try {
        const tokenRes = await fetch("/api/token");
        const tokenData = await tokenRes.json();
        token = tokenData.token;
    } catch (err) {
        console.error("Failed to acquire token:", err);
        return;
    }

    while (true) {
        if (shouldStop()) break;
        try {
            const res = await fetch(`/api/${url}/${encodeURIComponent(token)}`);
            const data = await res.json();
            if (data.error === "HttpRequestTimeout") continue;
            if (data.error === "ParallelCallNotSupported") {
                await new Promise(r => setTimeout(r, 100));
                continue;
            }
            // Terminal: Closed or NotFound — drain complete
            if (data.error && terminalPattern.test(data.error)) break;
            // Batch response: process all responses in the batch
            if (data.responses) {
                for (const r of data.responses) {
                    handler(r);
                }
            }
        } catch (err) {
            console.error(`Poll error for ${url}:`, err);
            break;
        }
    }
}

// ---- Session Polling ----

function startSessionPolling(sessionId, workId) {
    const data = workIdData[workId];
    if (!data) return;

    const sessionInfo = data.sessions.get(sessionId);
    if (!sessionInfo) return;

    pollLive(
        `copilot/session/${encodeURIComponent(sessionId)}/live`,
        (response) => {
            if (response.sessionError) {
                console.error(`Session ${sessionId} error:`, response.sessionError);
                return;
            }
            if (response.callback) {
                sessionInfo.renderer.processCallback(response);
            }
        },
        () => false // Always drain to server terminal response
    );
}

// ---- Task Polling ----

function startTaskPolling(taskId, workId) {
    const data = workIdData[workId];
    if (!data) return;
    data.taskPollingActive = true;

    pollLive(
        `copilot/task/${encodeURIComponent(taskId)}/live`,
        (response) => {
            if (response.taskError) {
                console.error(`Task ${taskId} error:`, response.taskError);
                return;
            }
            const cb = response.callback;

            if (cb === "taskSessionStarted") {
                const sessionId = response.sessionId;
                const isDriving = response.isDriving;
                if (sessionId) {
                    if (isDriving) {
                        // Consolidate all driving sessions into one "Driving" tab
                        if (data.drivingSessionId) {
                            const oldInfo = data.sessions.get(data.drivingSessionId);
                            if (oldInfo) oldInfo.active = false;
                            data.sessions.delete(data.drivingSessionId);
                        }

                        let div, renderer;
                        if (data.drivingDiv) {
                            // Reuse existing driving renderer
                            div = data.drivingDiv;
                            renderer = data.drivingRenderer;
                        } else {
                            // First driving session
                            div = document.createElement("div");
                            div.className = "session-renderer-container";
                            renderer = new SessionResponseRenderer(div);
                            data.drivingDiv = div;
                            data.drivingRenderer = renderer;
                        }

                        data.drivingSessionId = sessionId;
                        data.sessions.set(sessionId, {
                            name: "Driving",
                            renderer,
                            div,
                            active: true,
                        });

                        // Update tab UI if inspecting
                        if (inspectedWorkId === workId && tabContainer) {
                            const tabHeaders = tabContainer.querySelector(".tab-headers");
                            const tabContent = tabContainer.querySelector(".tab-content");
                            if (tabHeaders && tabContent) {
                                const existingDrivingBtn = [...tabHeaders.querySelectorAll(".tab-header-btn")]
                                    .find(btn => btn.textContent === "Driving");
                                if (existingDrivingBtn) {
                                    // Update session ID reference on existing button
                                    existingDrivingBtn.dataset.sessionId = sessionId;
                                } else {
                                    // First driving session — insert tab at front
                                    const tabBtn = document.createElement("button");
                                    tabBtn.className = "tab-header-btn";
                                    tabBtn.textContent = "Driving";
                                    tabBtn.dataset.sessionId = sessionId;
                                    tabBtn.addEventListener("click", () => {
                                        switchTabForWork(workId, tabBtn.dataset.sessionId);
                                    });
                                    tabHeaders.insertBefore(tabBtn, tabHeaders.firstChild);
                                    div.style.display = "none";
                                    tabContent.insertBefore(div, tabContent.firstChild);
                                }
                            }
                        }

                        startSessionPolling(sessionId, workId);
                    } else {
                        // Task session — each gets its own tab
                        data.attemptCount = (data.attemptCount || 0) + 1;
                        const name = `Attempt #${data.attemptCount}`;

                        const div = document.createElement("div");
                        div.className = "session-renderer-container";
                        const renderer = new SessionResponseRenderer(div);

                        data.sessions.set(sessionId, {
                            name,
                            renderer,
                            div,
                            active: true,
                        });

                        // Add new tab header without switching to it
                        if (inspectedWorkId === workId && tabContainer) {
                            const tabHeaders = tabContainer.querySelector(".tab-headers");
                            const tabContent = tabContainer.querySelector(".tab-content");
                            if (tabHeaders && tabContent) {
                                const tabBtn = document.createElement("button");
                                tabBtn.className = "tab-header-btn";
                                tabBtn.textContent = name;
                                tabBtn.dataset.sessionId = sessionId;
                                tabBtn.addEventListener("click", () => {
                                    switchTabForWork(workId, sessionId);
                                });
                                tabHeaders.appendChild(tabBtn);

                                div.style.display = "none";
                                tabContent.appendChild(div);
                            }
                        }

                        startSessionPolling(sessionId, workId);
                    }
                }
            } else if (cb === "taskSessionStopped") {
                const sessionId = response.sessionId;
                if (sessionId) {
                    const sInfo = data.sessions.get(sessionId);
                    if (sInfo) {
                        sInfo.active = false;
                    }
                }
            } else if (cb === "taskSucceeded" || cb === "taskFailed") {
                // Mark all sessions inactive
                for (const [, sInfo] of data.sessions) {
                    sInfo.active = false;
                }
                data.taskPollingActive = false;
            } else if (cb === "taskDecision") {
                // Create a "User" message block in the driving session's renderer
                if (data.drivingRenderer) {
                    data.drivingRenderer.addUserMessage(response.reason, "TaskDecision");
                }
            }
        },
        () => false // Always drain to server terminal response
    );
}

// ---- Job Polling ----

function startJobPolling() {
    pollLive(
        `copilot/job/${encodeURIComponent(jobId)}/live`,
        (response) => {
            if (response.jobError) {
                jobStatus = "FAILED";
                updateStatusLabel();
                return;
            }
            const cb = response.callback;

            if (cb === "workStarted") {
                const workId = response.workId;
                const taskId = response.taskId;

                // Clear previous data if task runs again (loop scenario)
                if (workIdData[workId]) {
                    const oldData = workIdData[workId];
                    // Stop all old session polling
                    for (const [, sInfo] of oldData.sessions) {
                        sInfo.active = false;
                    }
                    oldData.taskPollingActive = false;
                }

                workIdData[workId] = {
                    taskId,
                    sessions: new Map(),
                    attemptCount: 0,
                    taskPollingActive: false,
                    activeTabSessionId: null,
                };

                // If the restarted work is currently inspected, clear tabs
                if (inspectedWorkId === workId) {
                    refreshSessionResponsePart();
                }

                // Update flow chart - running indicator
                if (chartController) {
                    chartController.setRunning(workId);
                }

                // Start task polling
                if (taskId) {
                    startTaskPolling(taskId, workId);
                }
            } else if (cb === "workStopped") {
                const workId = response.workId;
                const succeeded = response.succeeded;

                if (chartController) {
                    if (succeeded) {
                        chartController.setCompleted(workId);
                    } else {
                        chartController.setFailed(workId);
                    }
                }
            } else if (cb === "jobSucceeded") {
                jobStatus = "SUCCEEDED";
                updateStatusLabel();
            } else if (cb === "jobFailed") {
                jobStatus = "FAILED";
                updateStatusLabel();
            } else if (cb === "jobCanceled") {
                jobStatus = "CANCELED";
                updateStatusLabel();
            }
        },
        () => false // Always drain to server terminal response
    );
}

// ---- Load initial job status ----
async function loadInitialJobStatus() {
    if (isPreviewMode || !jobId) return;
    try {
        const res = await fetch(`/api/copilot/job/${encodeURIComponent(jobId)}/status`);
        const data = await res.json();
        if (data.error) return;

        // Update job status
        if (data.status === "Succeeded") {
            jobStatus = "SUCCEEDED";
        } else if (data.status === "Failed") {
            jobStatus = "FAILED";
        } else if (data.status === "Canceled") {
            jobStatus = "CANCELED";
            jobStopped = true;
        } else {
            jobStatus = "RUNNING";
        }
        updateStatusLabel();

        // Update task statuses on the chart
        if (data.tasks && chartController) {
            for (const task of data.tasks) {
                if (task.status === "Running") {
                    chartController.setRunning(task.workIdInJob);
                } else if (task.status === "Succeeded") {
                    chartController.setCompleted(task.workIdInJob);
                } else if (task.status === "Failed") {
                    chartController.setFailed(task.workIdInJob);
                }
            }
        }
    } catch (err) {
        console.error("Failed to load initial job status:", err);
    }
}

// ---- Load job data and render chart ----
async function loadJobData() {
    try {
        const res = await fetch("/api/copilot/job");
        const data = await res.json();
        const jobDefinition = data.jobs[jobName];
        if (!jobDefinition) {
            jobPart.textContent = `Job "${jobName}" not found.`;
            return;
        }

        const chart = data.chart && data.chart[jobName];
        if (!chart || !chart.nodes || chart.nodes.length === 0) {
            jobPart.textContent = "No chart data available for this job.";
            return;
        }

        jobToRender = jobDefinition;
        chartToRender = chart;

        // Show JSON initially
        showJsonView();

        // Create status bar
        const statusBar = createStatusBar();

        // Build job part layout
        jobPart.innerHTML = "";
        jobPart.appendChild(statusBar);

        const chartContainer = document.createElement("div");
        chartContainer.id = "chart-container";
        jobPart.appendChild(chartContainer);

        // Render with Mermaid
        chartController = await renderFlowChartMermaid(chart, chartContainer, isPreviewMode ? () => {} : onInspect);

        // Load initial job status before starting live polling
        await loadInitialJobStatus();

        // Start job live polling only when not in preview mode
        if (!isPreviewMode) {
            startJobPolling();
        }
    } catch (err) {
        console.error("Failed to load job data:", err);
        jobPart.textContent = "Failed to load job data.";
    }
}

// ---- Resize bar (horizontal) ----
let resizing = false;

resizeBar.addEventListener("mousedown", (e) => {
    resizing = true;
    e.preventDefault();
});

document.addEventListener("mousemove", (e) => {
    if (!resizing) return;
    const totalWidth = document.body.clientWidth;
    const barWidth = resizeBar.offsetWidth;
    let rightWidth = totalWidth - e.clientX - barWidth / 2;
    if (rightWidth < 200) rightWidth = 200;
    if (rightWidth > totalWidth - 200) rightWidth = totalWidth - 200;
    rightPart.style.width = rightWidth + "px";
});

document.addEventListener("mouseup", () => {
    resizing = false;
});

// ---- Init ----
loadJobData();

// ---- Redirect if no working directory ----
const params = new URLSearchParams(window.location.search);
const workingDir = params.get("wb");
if (!workingDir) {
    window.location.href = "/index.html";
}

// ---- State ----
let selectedJobName = null;
let jobsData = null;

// ---- DOM references ----
const matrixPart = document.getElementById("matrix-part");
const userInputTextarea = document.getElementById("user-input-textarea");
const startJobButton = document.getElementById("start-job-button");
const resizeBar = document.getElementById("resize-bar");
const leftPart = document.getElementById("left-part");
const rightPart = document.getElementById("right-part");

// ---- Load jobs data ----
async function loadJobs() {
    try {
        const res = await fetch("/api/copilot/job");
        jobsData = await res.json();
        renderMatrix();
    } catch (err) {
        console.error("Failed to load jobs:", err);
    }
}

// ---- Matrix Rendering ----
function renderMatrix() {
    matrixPart.innerHTML = "";
    const grid = jobsData.grid;

    if (grid.length === 0) {
        matrixPart.textContent = "No jobs available.";
        return;
    }

    const table = document.createElement("table");
    table.id = "matrix-table";

    // Title row
    const titleRow = document.createElement("tr");
    // Count max columns: keyword + max jobs
    const maxJobCols = Math.max(...grid.map(row => row.jobs.length));
    const totalCols = 1 + maxJobCols;
    const titleCell = document.createElement("th");
    titleCell.colSpan = totalCols - 1;
    titleCell.textContent = "Available Jobs";
    titleCell.className = "matrix-title";
    titleRow.appendChild(titleCell);
    const stopCell = document.createElement("th");
    stopCell.className = "matrix-stop-cell";
    const stopBtn = document.createElement("button");
    stopBtn.id = "stop-server-button";
    stopBtn.textContent = "Stop Server";
    stopBtn.addEventListener("click", async () => {
        try {
            await fetch("/api/stop");
        } catch {
            // ignore
        }
        window.close();
        setTimeout(() => {
            document.title = "Server Stopped";
            document.body.style.cssText = "display:flex;align-items:center;justify-content:center;height:100vh;width:100vw;font-family:sans-serif;color:#ccc;background:#1e1e1e;margin:0;padding:0;";
            document.body.innerHTML = '<h1>Server stopped — you may close this tab.</h1>';
        }, 200);
    });
    stopCell.appendChild(stopBtn);
    titleRow.appendChild(stopCell);
    table.appendChild(titleRow);

    // Data rows
    for (const row of grid) {
        const tr = document.createElement("tr");

        // Keyword column
        const kwCell = document.createElement("td");
        kwCell.textContent = row.keyword;
        kwCell.className = "matrix-keyword";
        tr.appendChild(kwCell);

        // Job columns
        for (let i = 0; i < maxJobCols; i++) {
            const jobCell = document.createElement("td");
            if (i < row.jobs.length) {
                const col = row.jobs[i];
                if (col !== undefined && col !== null) {
                    const btn = document.createElement("button");
                    btn.textContent = col.name;
                    btn.className = "matrix-job-btn";
                    btn.dataset.jobName = col.jobName;
                    btn.addEventListener("click", () => onJobButtonClick(btn, col.jobName));
                    jobCell.appendChild(btn);
                }
                // undefined renders an empty cell
            }
            tr.appendChild(jobCell);
        }

        table.appendChild(tr);
    }

    matrixPart.appendChild(table);
}

// ---- Job Selection ----
function onJobButtonClick(btn, jobName) {
    if (selectedJobName === jobName) {
        // Deselect
        btn.classList.remove("selected");
        selectedJobName = null;
        startJobButton.disabled = true;
        startJobButton.textContent = "Job Not Selected";
        previewButton.disabled = true;
        userInputTextarea.disabled = true;
    } else {
        // Deselect previous
        const prev = matrixPart.querySelector(".matrix-job-btn.selected");
        if (prev) prev.classList.remove("selected");
        // Select new
        btn.classList.add("selected");
        selectedJobName = jobName;
        startJobButton.disabled = false;
        startJobButton.textContent = `Start Job: ${jobName}`;
        previewButton.disabled = false;
        // Enable text box only when requireUserInput is true
        const job = jobsData.jobs[jobName];
        userInputTextarea.disabled = !(job && job.requireUserInput);
    }
}

// ---- Start Job ----
startJobButton.addEventListener("click", async () => {
    if (!selectedJobName) return;
    startJobButton.disabled = true;
    try {
        const userInput = userInputTextarea.value || "";
        const body = workingDir + "\n" + userInput;
        const res = await fetch(`/api/copilot/job/start/${encodeURIComponent(selectedJobName)}`, {
            method: "POST",
            body: body,
        });
        const data = await res.json();
        if (data.error) {
            alert("Failed to start job: " + data.error);
            startJobButton.disabled = false;
            return;
        }
        if (data.jobId) {
            const url = `/jobTracking.html?jobName=${encodeURIComponent(selectedJobName)}&jobId=${encodeURIComponent(data.jobId)}`;
            window.open(url, "_blank");
        } else {
            alert("Failed to start job: unexpected response");
        }
    } catch (err) {
        alert("Failed to start job: " + err.message);
    }
    startJobButton.disabled = false;
});

// ---- Preview Button ----
const previewButton = document.getElementById("preview-button");
previewButton.addEventListener("click", () => {
    if (!selectedJobName) return;
    const url = `/jobTracking.html?jobName=${encodeURIComponent(selectedJobName)}`;
    window.open(url, "_blank");
});

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
loadJobs();

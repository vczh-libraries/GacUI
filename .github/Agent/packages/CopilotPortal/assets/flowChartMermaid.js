// ---- Mermaid Flow Chart Renderer ----
// Renders a ChartGraph using Mermaid.js to generate a flowchart diagram.

function mermaidGetHintKey(hint) {
    return Array.isArray(hint) ? hint[0] : hint;
}

function mermaidBuildDefinition(chart) {
    const lines = ["graph TD"];

    // Define nodes
    for (const node of chart.nodes) {
        const hintKey = mermaidGetHintKey(node.hint);
        const id = `N${node.id}`;
        const label = node.label || "";

        switch (hintKey) {
            case "TaskNode":
                lines.push(`    ${id}["${label}"]`);
                break;
            case "CondNode":
                lines.push(`    ${id}{{"${label}"}}`);
                break;
            case "ParBegin":
            case "ParEnd":
                lines.push(`    ${id}[" "]`);
                break;
            case "AltEnd":
                lines.push(`    ${id}[" "]`);
                break;
            case "CondBegin":
                lines.push(`    ${id}[" "]`);
                break;
            case "CondEnd":
                lines.push(`    ${id}{" "}`);
                break;
            case "LoopEnd":
                lines.push(`    ${id}((" "))`);
                break;
            default:
                lines.push(`    ${id}["${label || hintKey}"]`);
                break;
        }
    }

    // Define edges
    for (const node of chart.nodes) {
        if (node.arrows) {
            for (const arrow of node.arrows) {
                const from = `N${node.id}`;
                const to = `N${arrow.to}`;
                if (arrow.label) {
                    lines.push(`    ${from} -->|"${arrow.label}"| ${to}`);
                } else {
                    lines.push(`    ${from} --> ${to}`);
                }
            }
        }
    }

    // Apply styles
    for (const node of chart.nodes) {
        const hintKey = mermaidGetHintKey(node.hint);
        const id = `N${node.id}`;

        switch (hintKey) {
            case "TaskNode":
                lines.push(`    style ${id} fill:#dbeafe,stroke:#3b82f6,stroke-width:2px,color:#1e3a5f`);
                break;
            case "CondNode":
                lines.push(`    style ${id} fill:#fef9c3,stroke:#eab308,stroke-width:2px,color:#92400e`);
                break;
            case "ParBegin":
            case "ParEnd":
                lines.push(`    style ${id} fill:#222,stroke:#222,stroke-width:2px,color:#222,font-size:0px,min-width:40px,min-height:6px,padding:0px`);
                break;
            case "AltEnd":
                lines.push(`    style ${id} fill:#fce7f3,stroke:#db2777,stroke-width:2px,color:#fce7f3,font-size:0px,min-width:40px,min-height:6px,padding:0px`);
                break;
            case "CondBegin":
                lines.push(`    style ${id} fill:#fef9c3,stroke:#eab308,stroke-width:2px,color:#fef9c3,font-size:0px,min-width:40px,min-height:6px,padding:0px`);
                break;
            case "CondEnd":
                lines.push(`    style ${id} fill:#fef9c3,stroke:#eab308,stroke-width:2px,color:#fef9c3,font-size:0px,padding:0px`);
                break;
            case "LoopEnd":
                lines.push(`    style ${id} fill:#f3f4f6,stroke:#9ca3af,stroke-width:2px,color:#f3f4f6,font-size:0px,padding:0px`);
                break;
        }
    }

    return lines.join("\n");
}

async function renderFlowChartMermaid(chart, container, onInspect) {
    const definition = mermaidBuildDefinition(chart);

    // Build maps for TaskNode/CondNode click handling and workId lookup
    const taskNodeIds = [];
    const nodeIdToWorkId = {};
    for (const node of chart.nodes) {
        const hintKey = mermaidGetHintKey(node.hint);
        if (hintKey === "TaskNode" || hintKey === "CondNode") {
            const nid = `N${node.id}`;
            taskNodeIds.push(nid);
            // hint is [hintKey, workIdInJob]
            if (Array.isArray(node.hint) && node.hint.length >= 2) {
                nodeIdToWorkId[nid] = node.hint[1];
            }
        }
    }

    // Render with Mermaid
    const { svg } = await mermaid.render("mermaid-chart", definition);
    container.innerHTML = "";
    container.innerHTML = svg;

    // Fix SVG viewBox to ensure nothing is clipped (especially emoji indicators on leftmost nodes)
    const svgEl = container.querySelector("svg");
    if (svgEl) {
        // Get the bounding box of the entire SVG content
        const bbox = svgEl.getBBox();
        const padding = 24;
        const vbX = bbox.x - padding;
        const vbY = bbox.y - padding;
        const vbW = bbox.width + padding * 2;
        const vbH = bbox.height + padding * 2;
        svgEl.setAttribute("viewBox", `${vbX} ${vbY} ${vbW} ${vbH}`);
        // Set explicit dimensions so the SVG doesn't collapse or scale unexpectedly
        svgEl.style.width = `${vbW}px`;
        svgEl.style.height = `${vbH}px`;
        svgEl.style.minWidth = `${vbW}px`;
        svgEl.style.minHeight = `${vbH}px`;
    }

    // Ctrl+Scroll zoom on the chart container
    let zoomLevel = 1;
    const zoomMin = 0.2;
    const zoomMax = 3;
    const zoomStep = 0.1;
    container.addEventListener("wheel", (e) => {
        if (!e.ctrlKey) return;
        e.preventDefault();
        const delta = e.deltaY > 0 ? -zoomStep : zoomStep;
        zoomLevel = Math.min(zoomMax, Math.max(zoomMin, zoomLevel + delta));
        const svgInner = container.querySelector("svg");
        if (svgInner) {
            svgInner.style.transformOrigin = "top left";
            svgInner.style.transform = `scale(${zoomLevel})`;
        }
    }, { passive: false });

    // Track currently selected TaskNode/CondNode
    let currentSelectedGroup = null;
    let currentSelectedOriginalStrokeWidth = null;
    let currentSelectedWorkId = null;

    // Map workId -> DOM group for status updates
    const workIdToGroup = {};
    const workIdToTextEl = {};

    // Add click handlers for TaskNode/CondNode elements
    for (const nodeId of taskNodeIds) {
        const nodeEl = container.querySelector(`[id^="flowchart-${nodeId}-"]`);
        if (!nodeEl) continue;
        const group = nodeEl.closest("g.node") || nodeEl;
        group.style.cursor = "pointer";

        const workId = nodeIdToWorkId[nodeId];
        if (workId !== undefined) {
            workIdToGroup[workId] = group;
            const textEl = group.querySelector(".nodeLabel") || group.querySelector("text");
            workIdToTextEl[workId] = textEl;
        }

        group.addEventListener("click", () => {
            const wid = nodeIdToWorkId[nodeId];
            const shapeEl = group.querySelector("rect, polygon, circle, ellipse, path");

            if (currentSelectedGroup === group) {
                // Unselect
                if (shapeEl && currentSelectedOriginalStrokeWidth !== null) {
                    shapeEl.style.strokeWidth = currentSelectedOriginalStrokeWidth;
                }
                currentSelectedGroup = null;
                currentSelectedOriginalStrokeWidth = null;
                currentSelectedWorkId = null;
                if (onInspect) onInspect(null);
            } else {
                // Unselect previous
                if (currentSelectedGroup) {
                    const prevShape = currentSelectedGroup.querySelector("rect, polygon, circle, ellipse, path");
                    if (prevShape && currentSelectedOriginalStrokeWidth !== null) {
                        prevShape.style.strokeWidth = currentSelectedOriginalStrokeWidth;
                    }
                }
                // Select new
                currentSelectedOriginalStrokeWidth = shapeEl ? (shapeEl.style.strokeWidth || shapeEl.getAttribute("style")?.match(/stroke-width:\s*([^;]+)/)?.[1] || getComputedStyle(shapeEl).strokeWidth) : null;
                if (shapeEl) {
                    shapeEl.style.strokeWidth = "5px";
                }
                currentSelectedGroup = group;
                currentSelectedWorkId = wid;
                if (onInspect) onInspect(wid);
            }
        });
    }

    // Return controller for status updates
    return {
        // Helper: position an indicator at the center of the left border of the node
        _positionIndicator(indicator, group) {
            // Try to find the shape element (rect, polygon, etc.) of the node
            const shape = group.querySelector("rect, polygon, circle, ellipse, path");
            if (shape) {
                const shapeBBox = shape.getBBox ? shape.getBBox() : null;
                if (shapeBBox) {
                    // Center of the left border, but outside
                    indicator.setAttribute("x", String(shapeBBox.x - 20));
                    indicator.setAttribute("y", String(shapeBBox.y + shapeBBox.height / 2 + 5));
                    return;
                }
            }
            // Fallback: use text position
            const textEl = workIdToTextEl[parseInt(indicator.getAttribute("data-work-id"))];
            if (textEl) {
                const textBBox = textEl.getBBox ? textEl.getBBox() : null;
                if (textBBox) {
                    indicator.setAttribute("x", String(textBBox.x - 20));
                    indicator.setAttribute("y", String(textBBox.y + textBBox.height * 0.8));
                }
            }
        },
        // Set a node to running state (green triangle emoji)
        setRunning(workId) {
            const group = workIdToGroup[workId];
            if (!group) return;
            this._clearIndicator(workId);
            const indicator = document.createElementNS("http://www.w3.org/2000/svg", "text");
            indicator.textContent = "\u25B6\uFE0F"; // ▶️ green triangle emoji
            indicator.setAttribute("fill", "#22c55e");
            indicator.setAttribute("font-size", "14");
            indicator.setAttribute("class", "task-status-indicator");
            indicator.setAttribute("data-work-id", String(workId));
            const parent = group;
            parent.insertBefore(indicator, parent.firstChild);
            this._positionIndicator(indicator, group);
        },
        // Set a node to completed state (green tick emoji)
        setCompleted(workId) {
            const group = workIdToGroup[workId];
            if (!group) return;
            this._clearIndicator(workId);
            const indicator = document.createElementNS("http://www.w3.org/2000/svg", "text");
            indicator.textContent = "\u2705"; // ✅ green tick emoji
            indicator.setAttribute("font-size", "14");
            indicator.setAttribute("class", "task-status-indicator");
            indicator.setAttribute("data-work-id", String(workId));
            const parent = group;
            parent.insertBefore(indicator, parent.firstChild);
            this._positionIndicator(indicator, group);
        },
        // Set a node to failed state (red cross emoji)
        setFailed(workId) {
            const group = workIdToGroup[workId];
            if (!group) return;
            this._clearIndicator(workId);
            const indicator = document.createElementNS("http://www.w3.org/2000/svg", "text");
            indicator.textContent = "\u274C"; // ❌ red cross emoji
            indicator.setAttribute("font-size", "14");
            indicator.setAttribute("class", "task-status-indicator");
            indicator.setAttribute("data-work-id", String(workId));
            const parent = group;
            parent.insertBefore(indicator, parent.firstChild);
            this._positionIndicator(indicator, group);
        },
        _clearIndicator(workId) {
            const svgEl = container.querySelector("svg");
            if (svgEl) {
                const existing = svgEl.querySelectorAll(`.task-status-indicator[data-work-id="${workId}"]`);
                existing.forEach(el => el.remove());
            }
        },
        get inspectedWorkId() {
            return currentSelectedWorkId;
        },
    };
}

// Export as global
window.renderFlowChartMermaid = renderFlowChartMermaid;

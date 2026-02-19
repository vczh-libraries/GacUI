// CJS preload: patch child_process.spawn on Windows to add windowsHide: true,
// preventing spawned console windows from stealing keyboard focus during tests.
"use strict";

if (process.platform === "win32") {
    const cp = require("child_process");
    const originalSpawn = cp.spawn;
    cp.spawn = function patchedSpawn(command, args, options) {
        if (Array.isArray(args)) {
            options = Object.assign({}, options, { windowsHide: true });
        } else if (args && typeof args === "object") {
            args = Object.assign({}, args, { windowsHide: true });
        }
        return originalSpawn.call(this, command, args, options);
    };
}

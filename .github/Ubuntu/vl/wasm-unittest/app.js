const { createServer } = require("node:http");
const { glob, readFile, realpath, stat } = require("node:fs/promises");
const { dirname, isAbsolute, relative, resolve, sep } = require("node:path");

function opfsPath(value) {
    if (typeof value !== "string" || !value || value.includes("\\") || value.includes("\0") ||
        value.split("/").some(part => !part || part === "." || part === "..")) {
        throw new Error(`Invalid OPFS path: ${value}`);
    }
    return value;
}

async function containedFile(root, name) {
    const file = await realpath(resolve(root, name));
    const path = relative(root, file);
    if (isAbsolute(path) || path === ".." || path.startsWith(`..${sep}`)) {
        throw new Error(`Fixture escapes rootFolder: ${name}`);
    }
    return file;
}

async function main() {
    const [, , configArgument, portArgument] = process.argv;
    if (!configArgument || !/^\d+$/.test(portArgument) || Number(portArgument) < 1 || Number(portArgument) > 65535) {
        throw new Error("Usage: node app.js vbuild port");
    }
    const configPath = resolve(configArgument);
    const config = JSON.parse(await readFile(configPath, "utf8"))["WASM=YES"];
    if (!config || typeof config !== "object" || Array.isArray(config)) {
        throw new Error('vbuild must contain a "WASM=YES" configuration object.');
    }

    const fixtures = new Set();
    const folders = new Set();
    let root;
    if (config.rootFolder !== undefined) {
        if (typeof config.rootFolder !== "string" || isAbsolute(config.rootFolder)) {
            throw new Error("rootFolder must be relative to the vbuild file.");
        }
        root = await realpath(resolve(dirname(configPath), config.rootFolder));
        if (!(await stat(root)).isDirectory()) throw new Error("rootFolder must be a directory.");
        for (const key of ["folders", "includes", "excludes"]) {
            if (config[key] !== undefined && (!Array.isArray(config[key]) || config[key].some(value => typeof value !== "string"))) {
                throw new Error(`${key} must be an array of strings.`);
            }
        }
        for (const folder of config.folders ?? []) folders.add(opfsPath(folder));
        const includes = config.includes ?? [];
        const excludes = config.excludes ?? [];
        for (const pattern of [...includes, ...excludes]) opfsPath(pattern);
        if (includes.length) {
            for await (const match of glob(includes, { cwd: root, exclude: excludes })) {
                const file = await containedFile(root, match);
                if ((await stat(file)).isFile()) fixtures.add(opfsPath(match.split(sep).join("/")));
            }
        }
    }

    // Parents are inferred by the browser; report only explicit empty leaves.
    const entries = [...fixtures.keys(), ...folders];
    for (const entry of entries) {
        const parts = entry.split("/");
        while (parts.length > 1) {
            parts.pop();
            const parent = parts.join("/");
            if (fixtures.has(parent)) throw new Error(`File is also a parent directory: ${parent}`);
            folders.delete(parent);
        }
    }
    for (const folder of folders) {
        if (fixtures.has(folder)) throw new Error(`File is also an explicit directory: ${folder}`);
    }
    const manifest = JSON.stringify({ files: [...fixtures.keys()].sort(), folders: [...folders].sort() });
    const files = new Map([
        ["/", ["app.html", "text/html; charset=utf-8"]],
        ["/index.html", ["app.html", "text/html; charset=utf-8"]],
        ["/app.html", ["app.html", "text/html; charset=utf-8"]],
        ["/app.mjs", ["app.mjs", "text/javascript"]],
        ["/app.worker.js", ["app.worker.js", "text/javascript"]],
        ["/app.wasm", ["app.wasm", "application/wasm"]],
        ["/app.debug.wasm", ["app.debug.wasm", "application/wasm"]],
    ]);

    createServer(async (request, response) => {
        response.setHeader("Cross-Origin-Opener-Policy", "same-origin");
        response.setHeader("Cross-Origin-Embedder-Policy", "require-corp");
        response.setHeader("Cache-Control", "no-store");
        if (request.method !== "GET") {
            response.writeHead(405, { Allow: "GET" });
            response.end("Method not allowed");
            return;
        }
        let path;
        try { path = decodeURIComponent(new URL(request.url, "http://127.0.0.1").pathname); }
        catch {
            response.writeHead(400);
            response.end("Invalid URL");
            return;
        }
        if (path === "/OPFS") {
            response.setHeader("Content-Type", "application/json");
            response.end(manifest);
            return;
        }
        const fixture = path.startsWith("/OPFS/") ? path.slice(6) : undefined;
        const file = files.get(path);
        if (file || fixtures.has(fixture)) {
            try {
                const content = file
                    ? await readFile(resolve(__dirname, file[0]))
                    : await readFile(await containedFile(root, fixture));
                response.setHeader("Content-Type", file ? file[1] : "application/octet-stream");
                response.end(content);
                return;
            } catch (error) {
                if (error.code !== "ENOENT") throw error;
            }
        }
        response.writeHead(404);
        response.end("Not found");
    }).listen(Number(portArgument), "127.0.0.1", () => {
        console.log(`WebAssembly unit tests: http://127.0.0.1:${portArgument}/`);
        console.log(`OPFS fixtures: ${fixtures.size} files, ${folders.size} empty directories.`);
    });
}

main().catch(error => {
    console.error(error);
    process.exitCode = 1;
});

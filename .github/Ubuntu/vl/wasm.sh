#!/bin/bash
set -e

if [ $# -ne 1 ]; then
    echo "Usage: wasm.sh target" >&2
    exit 1
fi

TARGET_DIR="$(dirname -- "$1")"
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p -- "${TARGET_DIR}"
if [ -d "${TARGET_DIR}/app.html" ] || [ -d "${TARGET_DIR}/app.sh" ] || [ -d "${TARGET_DIR}/app.js" ] || [ -d "${TARGET_DIR}/index.html" ] || [ -d "$1" ] || [ -d "$1.tmp" ]; then
    echo "Wasm package outputs must be files." >&2
    exit 1
fi
cp -- "${SCRIPT_DIR}/wasm-unittest/"* "${TARGET_DIR}/"
if [ -f ./vbuild ]; then
    node --input-type=module - "${TARGET_DIR}" <<'JS'
import { readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';

const config = JSON.parse(readFileSync('vbuild', 'utf8'));
if (config['WASM=YES']?.embedPthreadWorker === true) {
    const folder = process.argv[2];
    const modulePath = join(folder, 'app.mjs');
    const source = readFileSync(modulePath, 'utf8');
    const worker = readFileSync(join(folder, 'app.worker.js'), 'utf8');
    const exportPattern = /export default Module;\s*$/;
    if (!exportPattern.test(source)) {
        throw new Error('Unsupported Emscripten module export for embedded pthread packaging.');
    }
    writeFileSync(modulePath, source.replace(exportPattern, '') + `
// Keep the pthread bootstrap in this module so deployment needs only app.mjs/app.wasm.
export default function(options = {}) {
    if (options.ENVIRONMENT_IS_PTHREAD === true) return Module(options);
    const workerUrl = URL.createObjectURL(new Blob([${JSON.stringify(worker)}], { type: 'text/javascript' }));
    const locateFile = options.locateFile;
    return Module({
        mainScriptUrlOrBlob: import.meta.url,
        ...options,
        locateFile(file, prefix) {
            if (file === 'app.worker.js') return workerUrl;
            return locateFile === undefined ? prefix + file : locateFile(file, prefix);
        },
    });
}
`);
}
JS
fi
chmod +x -- "${TARGET_DIR}/app.sh"
ln -sf -- app.html "${TARGET_DIR}/index.html"
# Publish the make target only after all package preparation succeeds.
cp -- "${TARGET_DIR}/app.wasm" "$1.tmp"
mv -f -- "$1.tmp" "$1"

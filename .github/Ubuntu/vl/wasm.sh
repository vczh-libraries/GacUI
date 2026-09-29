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
chmod +x -- "${TARGET_DIR}/app.sh"
ln -sf -- app.html "${TARGET_DIR}/index.html"
# Publish the make target only after all package preparation succeeds.
cp -- "${TARGET_DIR}/app.wasm" "$1.tmp"
mv -f -- "$1.tmp" "$1"

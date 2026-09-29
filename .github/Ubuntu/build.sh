#!/bin/bash
set -e

function Usage {
    echo "Usage: build.sh [-b|--build|-f|--full-build|--build-gcc|--full-build-gcc]"
    echo "                [-bw|--build-wasm|-fbw|--full-build-wasm]"
    echo "                [-c|--build-coverage|-fc|--full-build-coverage]"
}

if [ $# -gt 1 ]; then
    Usage
    exit 1
fi

case "${1:--b}" in
    -b|--build|-f|--full-build|--build-gcc|--full-build-gcc|-bw|--build-wasm|-fbw|--full-build-wasm|-c|--build-coverage|-fc|--full-build-coverage) ;;
    --help) Usage; exit 0 ;;
    *) Usage; exit 1 ;;
esac

export VCPROOT="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
export PATH="${VCPROOT}/vl/cmd:${PATH}"
export SHELL=/bin/bash

vmake --make

vbuild "${1:--b}"

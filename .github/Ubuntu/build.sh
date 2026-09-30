#!/bin/bash
set -e

function Usage {
    echo "Usage: build.sh [-b|--build|-f|-fb|--full-build|--build-gcc|--full-build-gcc] [-o|--optimize]"
    echo "                [-bw|--build-wasm|-fbw|--full-build-wasm]"
    echo "                [-c|--build-coverage|-fc|--full-build-coverage]"
    echo "-o/--optimize selects -O2 for native builds only; default is -O0."
}

if [ $# -gt 2 ]; then
    Usage
    exit 1
fi

for BUILD_ARG in "$@"; do
case "${BUILD_ARG}" in
    -b|--build|-f|-fb|--full-build|--build-gcc|--full-build-gcc|-bw|--build-wasm|-fbw|--full-build-wasm|-c|--build-coverage|-fc|--full-build-coverage|-o|--optimize) ;;
    --help) Usage; exit 0 ;;
    *) Usage; exit 1 ;;
esac
done

export VCPROOT="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
export PATH="${VCPROOT}/vl/cmd:${PATH}"
export SHELL=/bin/bash

vmake --make

vbuild "$@"

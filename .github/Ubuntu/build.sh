#!/bin/bash
set -e

function Usage {
    echo "Usage: build.sh [-f]"
}

if [ $# -gt 1 ]; then
    Usage
    exit 1
fi

if [ $# -eq 1 ] && [ "$1" != "-f" ]; then
    Usage
    exit 1
fi

export VCPROOT="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
export PATH="${VCPROOT}/vl/cmd:${PATH}"
export SHELL=/bin/bash

vmake --make

if [ "$1" == "-f" ]; then
    vbuild -f
else
    vbuild -b
fi

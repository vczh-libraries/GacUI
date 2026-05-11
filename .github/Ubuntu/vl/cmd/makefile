#!/bin/bash

function Help {
    echo "Usage:"
    echo "--make"
    echo "    Update ./makefile and ./vmake.txt from ./vmake."
}

function Make {
    if [ -a vmake ]; then
        vt4 --preprocess ./vmake | vt4 --evaluate | ${SHELL} > makefile
    else
        echo "Cannot find ./vmake ."
        exit 1
    fi
}

case $1 in
    --help)
    Help
    ;;

    --make)
    Make
    ;;

    *)
    echo "Use --help for more information."
    ;;
esac

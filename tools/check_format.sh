#!/bin/bash
# $1 - path to look for source files

if [ -z ${1} ]; then
    echo "No source path specified. Aborting."
    exit 1
fi

find ${1} -iname '*.h' -o -iname '*.cpp' -o -iname '*.c' | xargs clang-format -style=file -fallback-style=none -i

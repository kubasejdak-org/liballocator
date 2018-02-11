#!/bin/bash
# $1 - path to look for source files

if [ -z ${1} ]; then
    echo "No source path specified. Aborting."
    exit 1
fi

tools/code/check-format.sh ${1}
if [ ${?} -ne 0 ]; then
    echo "Failed to run check_format.sh script."
    exit 2
fi

git diff-index --quiet HEAD
if [ ${?} -ne 0 ]; then
    echo "Bad source code format detected for the following files:"
    git diff-index --name-status HEAD
    exit 3
fi

echo "Format check OK."

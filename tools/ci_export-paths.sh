#!/bin/bash

set -ev

EXPORTS_FILE="~/path_exports"

ls ~
if [ ! -f "~/path_exports" ]; then
    echo "No ~/path_exports file found. Skipping."
    exit 0
fi

EXPORTED_PATHS=""
while IFS='' read -r LINE || [[ -n "$LINE" ]]; do
    echo "Read line ${LINE}"
    EXPORTED_PATHS=${LINE}:${EXPORTED_PATHS}
done < "~/path_exports"

echo "EXPORTED_PATHS = ${EXPORTED_PATHS}"
echo "export PATH=${EXPORTED_PATHS}:${PATH}" >> ~/.bash_profile
cat ~/.bash_profile

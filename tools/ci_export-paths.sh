#!/bin/bash

set -ev

EXPORTS_FILE="~/path_exports"

if [ ! -f ${EXPORTS_FILE} ]; then
    echo "No ${EXPORTS_FILE} file found. Skipping."
    exit 0
fi

EXPORTED_PATHS=""
while IFS='' read -r LINE || [[ -n "$LINE" ]]; do
    EXPORTED_PATHS=${LINE}:${EXPORTED_PATHS}
done < ${EXPORTS_FILE}

echo "export PATH=${EXPORTED_PATHS}:${PATH}" >> ~/.bash_profile

#!/bin/bash

set -ev

EXPORTS_FILE="${HOME}/path_exports"

if [ ! -f ${EXPORTS_FILE} ]; then
    echo "No ${EXPORTS_FILE} file found. Skipping."
    exit 0
fi

EXPORTED_PATHS=""
while IFS='' read -r LINE || [[ -n "$LINE" ]]; do
    if [ "${EXPORTED_PATHS}" == "" ]; then
        EXPORTED_PATHS=${LINE}
    else
        EXPORTED_PATHS=${LINE}:${EXPORTED_PATHS}
    fi
done < ${EXPORTS_FILE}

echo "export PATH=${EXPORTED_PATHS}:${PATH}" >> ~/.bash_profile

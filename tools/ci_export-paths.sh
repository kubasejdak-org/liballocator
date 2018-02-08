#!/bin/bash

set -ev

EXPORTS_FILE="${HOME}/path_exports"

ls ~
if [ ! -f ${EXPORTS_FILE} ]; then
    echo "No ${EXPORTS_FILE} file found. Skipping."
    exit 0
fi

EXPORTED_PATHS=""
while IFS='' read -r LINE || [[ -n "$LINE" ]]; do
    echo "Read line ${LINE}"
    EXPORTED_PATHS=${LINE}:${EXPORTED_PATHS}
done < ${EXPORTS_FILE}

echo "EXPORTED_PATHS = ${EXPORTED_PATHS}"
echo "export PATH=${EXPORTED_PATHS}:${PATH}" >> ~/.bash_profile
cat ~/.bash_profile

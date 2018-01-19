#!/bin/bash
# $1 - arm-none-eabi-gcc version to be installed (e.g. 3.8.2)
# $2 - Host OS (Linux or macOS)

VERSION=${1}
if [ "${2}" == "Linux" ]; then
    OS="linux"
elif [ "${2}" == "macOS" ]; then
    OS="mac"
fi

if [ -z ${VERSION} ]; then
    echo "No arm-none-eabi-gcc version specified. Aborting."
    exit 1
fi

if [ -z ${OS} ]; then
    echo "No host OS specified. Aborting."
    exit 2
fi

echo "Installing arm-none-eabi-gcc v${VERSION}"

case "${VERSION}" in
    "7.2.0")
        PACKAGE_NAME="gcc-arm-none-eabi-7-2017-q4-major"
        PACKAGE_BIN_NAME="${PACKAGE_NAME}-${OS}.tar.bz2"
        if [ "${OS}" == "linux" ]; then
            PACKAGE_URL="http://bit.ly/2FNac93"
        else
            PACKAGE_URL="http://bit.ly/2FPJ5dM"
        fi
        ;;
    *)
        echo "Unsupported arm-none-eabi-gcc version."
        exit 3

esac

wget --no-check-certificate ${PACKAGE_URL} -O ${PACKAGE_BIN_NAME}
tar -xjf ${PACKAGE_BIN_NAME}

export PATH=${PWD}/${PACKAGE_NAME}/bin:${PATH}

echo "Installing arm-none-eabi-gcc v${VERSION} OK."

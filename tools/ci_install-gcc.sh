#!/bin/bash
# $1 - gcc major version to be installed (e.g. 7)
# $2 - Host OS (Linux or macOS)
# $3 - Export CC flag (true/false)

set -ev

VERSION=${1}
if [ "${2}" == "linux" ]; then
    OS="linux"
elif [ "${2}" == "osx" ]; then
    OS="mac"
fi
EXPORT=${3}

if [ -z ${VERSION} ]; then
    echo "No gcc version specified. Aborting."
    exit 1
fi

if [ -z ${OS} ]; then
    echo "No host OS specified. Aborting."
    exit 2
fi

if [ -z ${EXPORT} ]; then
    echo "No export CC flag specified. Using 'true' by default."
    EXPORT=true
fi

echo "Installing gcc v${VERSION}"

if [ "${OS}" == "linux" ]; then
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo apt-get update -qq
    sudo apt-get install gcc-${VERSION} g++-${VERSION} -y
else
    # Workaround for Travis macOS image problem.
    brew cask uninstall --force oclint
    brew install gcc@${VERSION}
fi

if [ "${EXPORT}" == "true" ]; then
    echo "export CC=gcc-${VERSION}" >> ~/.bash_profile
    echo "export CXX=g++-${VERSION}" >> ~/.bash_profile
fi

echo "Installing gcc v${VERSION} OK."

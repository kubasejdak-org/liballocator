#!/bin/bash
# $1 - clang version to be installed (e.g. 3.8.2)
# $2 - Host OS (Linux or macOS)
# $3 - Export CC flag (true/false)

set -ev

VERSION=${1}
OS=${2}
EXPORT=${3}

if [ -z ${VERSION} ]; then
    echo "No clang version specified. Aborting."
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

echo "Installing clang v${VERSION}"

if [ "${OS}" == "linux" ]; then
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

    case "${VERSION}" in
    "3.9" | "4.0" | "5.0" | "6.0")
        sudo apt-add-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-${VERSION} main" -y
        sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
        ;;
    *)
        echo "Unsupported clang version."
        exit 3
        ;;
    esac

    sudo apt-get update -qq
    sudo apt-get install clang-${VERSION} libc++-dev -y
else
    MAJOR_VERSION=`echo ${VERSION} | cut -d . -f 1`
    # Workaround for Travis macOS image problem.
    brew cask uninstall --force oclint
    brew install llvm@${MAJOR_VERSION} gcc@7

    echo "/usr/local/opt/llvm/bin" >> ~/path_exports
fi

if [ "${EXPORT}" == "true" ]; then
    echo "export CC=clang-${VERSION}" >> ~/.bash_profile
    echo "export CXX=clang-${VERSION}" >> ~/.bash_profile
fi

echo "Installing clang v${VERSION} OK."

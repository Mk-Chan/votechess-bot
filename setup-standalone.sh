#!/bin/bash

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

bash "${DIR}/setup.sh"

echo "Setting up local binutils, gcc, cmake..."

mkdir -p "${DIR}/tools/binutils"
mkdir -p "${DIR}/tools/gcc"
mkdir -p "${DIR}/tools/cmake"

BINUTILS_VERSION=2.32
BINUTILS_VERSION_FULL=binutils-${BINUTILS_VERSION}

if [ ! -f "${DIR}/tools/binutils/bin/ld" ]; then
    echo "Fetching binutils..."
    if [ ! -f "${DIR}/tools/binutils/${BINUTILS_VERSION_FULL}.tar.gz" ]; then
        wget -O "${DIR}/tools/binutils/${BINUTILS_VERSION_FULL}.tar.gz" "https://ftp.gnu.org/gnu/binutils/${BINUTILS_VERSION_FULL}.tar.gz"
    fi
    echo "Unzipping binutils..."
    tar -x -C "${DIR}/tools/binutils/" -f "${DIR}/tools/binutils/${BINUTILS_VERSION_FULL}.tar.gz"
    cd "${DIR}/tools/binutils/${BINUTILS_VERSION_FULL}/"
    echo "Building binutils..."
    ./configure \
        --prefix="${DIR}/tools/binutils/" \
        --enable-ld=default \
        --enable-plugins \
        --enable-shared \
        --enable-64-bit-bfd \
        --with-system-zlib
    make tooldir="${DIR}/tools/binutils/" -j8
    make tooldir="${DIR}/tools/binutils/" -k check
    make tooldir="${DIR}/tools/binutils/" install
    cd "${DIR}"
    rm -rf "${DIR}/tools/binutils/${BINUTILS_VERSION_FULL}/"
    echo "Built binutils!"
else
    echo "Found binutils!"
fi

export PATH="${DIR}/tools/binutils/bin/:${PATH}"
export LD="${DIR}/tools/binutils/bin/ld"

GCC_VERSION=9.2.0
GCC_VERSION_FULL=gcc-${GCC_VERSION}

if [ ! -f "${DIR}/tools/gcc/bin/g++" ]; then
    echo "Fetching gcc..."
    if [ ! -f "${DIR}/tools/gcc/${GCC_VERSION_FULL}.tar.gz" ]; then
        wget -O "${DIR}/tools/gcc/${GCC_VERSION_FULL}.tar.gz" "https://ftp.gnu.org/gnu/gcc/${GCC_VERSION_FULL}/${GCC_VERSION_FULL}.tar.gz"
    fi
    echo "Unzipping gcc..."
    tar -x -C "${DIR}/tools/gcc/" -f "${DIR}/tools/gcc/${GCC_VERSION_FULL}.tar.gz"
    cd "${DIR}/tools/gcc/${GCC_VERSION_FULL}/"
    "${DIR}/tools/gcc/${GCC_VERSION_FULL}/contrib/download_prerequisites"
    mkdir -p "${DIR}/tools/gcc/${GCC_VERSION_FULL}/build/"
    cd "${DIR}/tools/gcc/${GCC_VERSION_FULL}/build/"
    echo "Building gcc..."
    ../configure \
        --prefix="${DIR}/tools/gcc/" \
        --enable-threads=posix \
        --host=x86_64-linux-gnu \
        --target=x86_64-linux-gnu \
        --disable-multilib \
        --with-system-zlib \
        --with-arch=x86-64 \
        --with-tune=generic \
        --enable-checking=release \
        --enable-languages=c,c++
    make -j8
    make install
    cd "${DIR}"
    rm -rf "${DIR}/tools/gcc/${GCC_VERSION_FULL}/"
    echo "Built gcc!"
else
    echo "Found gcc!"
fi

export PATH="${DIR}/tools/gcc/bin/:${PATH}"
export CC="${DIR}/tools/gcc/bin/gcc"
export CXX="${DIR}/tools/gcc/bin/g++"

CMAKE_VERSION=3.15.3
CMAKE_VERSION_FULL=cmake-${CMAKE_VERSION}-Linux-x86_64

if [ ! -f "${DIR}/tools/cmake/bin/cmake" ]; then
    echo "Fetching cmake..."
    if [ ! -f "${DIR}/tools/cmake/${CMAKE_VERSION_FULL}.tar.gz" ]; then
        wget -O "${DIR}/tools/cmake/${CMAKE_VERSION_FULL}.tar.gz" "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${CMAKE_VERSION_FULL}.tar.gz"
    fi
    echo "Unzipping cmake..."
    tar -x -C "${DIR}/tools/cmake/" -f "${DIR}/tools/cmake/${CMAKE_VERSION_FULL}.tar.gz"
    mv "${DIR}/tools/cmake/${CMAKE_VERSION_FULL}"/* "${DIR}/tools/cmake/"
    rm -rf "${DIR}/tools/cmake/${CMAKE_VERSION_FULL}/"
    echo "Built cmake!"
else
    echo "Found cmake!"
fi

export PATH="${DIR}/tools/cmake/bin/:${PATH}"

echo "Done!"

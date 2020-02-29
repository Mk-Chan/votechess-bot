#!/bin/bash

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

echo "Installing library dependencies..."
packages="$(tr '\n' ' ' <<< "$(cat "${DIR}/vcpkg-packages.txt")")"
if [ ! -z "${packages}" -a "${packages}" != " " ]; then
    "${DIR}/tools/vcpkg/vcpkg" install ${packages}
fi

TARGET=${1:-all}
BUILD_TYPE=${2:-Debug}
BUILD_TYPE_LOWERCASE="$(tr '[:upper:]' '[:lower:]' <<< "${BUILD_TYPE}")"
CMAKE_BUILD_TYPE="$(tr '[:lower:]' '[:upper:]' <<< "${BUILD_TYPE_LOWERCASE:0:1}")${BUILD_TYPE_LOWERCASE:1}"

BUILD_THREADS=${3:-1}

CMAKE_BUILD_DIR=build/${BUILD_TYPE_LOWERCASE}
mkdir -p "${CMAKE_BUILD_DIR}"
cd "${CMAKE_BUILD_DIR}"

if [ -d "${DIR}/tools/cmake" ] && [ -f "${DIR}/tools/cmake/bin/cmake" ]; then
    export PATH="${DIR}/tools/cmake/bin/:${PATH}"
    echo "Using tools/cmake/bin/cmake..."
fi
if [ -d "${DIR}/tools/binutils" ] && [ -f "${DIR}/tools/binutils/bin/ld" ]; then
    export LD="${DIR}/tools/binutils/bin/ld"
    export PATH="${DIR}/tools/binutils/bin/:${PATH}"
    echo "Using tools/binutils/bin/ld..."
fi
if [ -d "${DIR}/tools/gcc" ] && [ -f "${DIR}/tools/gcc/bin/gcc" ]; then
    export CC="${DIR}/tools/gcc/bin/gcc"
    echo "Using tools/gcc/bin/gcc..."
    export CXX="${DIR}/tools/gcc/bin/g++"
    echo "Using tools/gcc/bin/g++..."
    export PATH="${DIR}/tools/gcc/bin/:${PATH}"
fi

echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DCMAKE_TOOLCHAIN_FILE="${DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    ../..

echo "Building..."
cmake --build . --target ${TARGET} -- -j ${BUILD_THREADS}

echo "Done!"

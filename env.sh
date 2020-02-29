#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
PATH="${DIR}/tools/gcc/bin/:${DIR}/tools/cmake/bin/:${DIR}/tools/binutils/bin/:${PATH}"

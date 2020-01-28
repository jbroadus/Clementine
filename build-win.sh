#!/bin/bash

TYPE="Debug"
IMG="eu.gcr.io/clementine-data/mingw-w64"
CMD="cmake .. -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32.cmake -DCMAKE_BUILD_TYPE=$TYPE && make -j8"
VOL="-v $(pwd):/build -w /build/mingw32"
ENV="-e PKG_CONFIG_PATH=/target/lib/pkgconfig"
mkdir -p mingw32
podman run $VOL $ENV $IMG /usr/bin/bash -c "$CMD"


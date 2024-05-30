#!/bin/bash

daisysp_dir=externals/DaisySP

pushd $daisysp_dir

cmake -Bbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
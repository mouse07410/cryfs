#!/bin/bash -ex
#
# This file has two purposes:
#  - simplify/automate configuration and build of CryFS for specific
#    environment (e.g., all the dependent libraries are in /opt/local);
#  - serve as a run-script for "git bisect" to help finding the
#    offending commit.
#

rm -rf build
mkdir -p build
cd build

# Normal build on MacOS
CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=off"
# Self-explanatory - build on MacOS without OpenMP (useful when neither Macports nor Brew is there)
#CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=off -DDISABLE_OPENMP=ON"
# Build on MacOS, enforcing Macports-installed Clang (currently not necessary, kept for historic reasons)
#CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=off -DCMAKE_C_COMPILER=clang-mp-6.0 -DCMAKE_CXX_COMPILER=clang++-mp-6.0"

# Installation prefix compatible with where Macports installs stuff on MacOS
CMAKEFLAGS="${CMAKEFLAGS} -DCMAKE_INSTALL_PREFIX=/opt/local"

# Ensure cryfs does not try to reach over Internet for updates
CMAKEFLAGS="${CMAKEFLAGS} -DBoost_USE_STATIC_LIBS=off -DCRYFS_UPDATE_CHECKS=off"

# Provide verbose output - useful to debug build process
#CMAKEFLAGS="${CMAKEFLAGS} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"

cmake .. ${CMAKEFLAGS} -DCMAKE_C_FLAGS="-I/opt/local/include"
make -j 4 V=1
#make check
cd ..
exit 0


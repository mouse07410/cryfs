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

# CMake flags needed for a normal build on MacOS
CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=off "

############################################################
# The following section deals with various ways to configure
# OpenMP on MacOS using Clang from Xcode or installed by
# Macports. You need to uncomment only one of those options,
# or none if the above default works (and finds libomp).
#
# If instead of Macports you use Brew, change the absolute paths
# for libomp to what they should be for Brew.
#
# Self-explanatory - build on MacOS without OpenMP (useful when neither
# Macports nor Brew is there, so no libomp is available)
#CMAKEFLAGS="${CMAKEFLAGS} -DDISABLE_OPENMP=ON"
#
# Build on MacOS, enforcing Macports-installed Clang
#CMAKEFLAGS="${CMAKEFLAGS} -DCMAKE_C_COMPILER=clang-mp-6.0 -DCMAKE_CXX_COMPILER=clang++-mp-6.0"
#
# Using AppleClang from Xcode with Macports-installed libomp (currently does not work)
OMP_CMAKE_C_FLAGS="-DOpenMP_C_FLAGS=-I/opt/local/include/libomp"
OMP_CMAKE_CXX_FLAGS="-DOpenMP_CXX_FLAGS=-I/opt/local/include/libomp"
OMP_CMAKE_OMP="-Xclang -fopenmp"
OMP_CMAKE_C_LIBNAMES="-DOpenMP_C_LIB_NAMES=omp"
OMP_CMAKE_CXX_LIBNAMES="-DOpenMP_CXX_LIB_NAMES=omp"
CMAKEFLAGS="${CMAKEFLAGS} ${OMP_CMAKE_C_FLAGS} ${OMP_CMAKE_CXX_FLAGS} ${OMP_CMAKE_C_LIBNAMES} ${OMP_CMAKE_CXX_LIBNAMES} -DOpenMP_omp_LIBRARY=/opt/local/lib/libomp/libomp.dylib"
############################################################


# Installation prefix compatible with where Macports installs stuff on MacOS
CMAKEFLAGS="${CMAKEFLAGS} -DCMAKE_INSTALL_PREFIX=/opt/local"

# Ensure cryfs does not try to reach over Internet for updates
CMAKEFLAGS="${CMAKEFLAGS} -DBoost_USE_STATIC_LIBS=off -DCRYFS_UPDATE_CHECKS=off"

# Provide verbose output - useful to debug build process
CMAKEFLAGS="${CMAKEFLAGS} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"

cmake .. ${CMAKEFLAGS} -DCMAKE_C_FLAGS="-I/opt/local/include" -DCMAKE_CXX_FLAGS="${CXXFLAGS} ${OMP_CMAKE_OMP} ${OMP_CMAKE_CXX_FLAGS}"
time make -j 4 V=1
#make check
cd ..
exit 0


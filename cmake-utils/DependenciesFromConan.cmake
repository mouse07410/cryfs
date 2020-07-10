include(cmake-utils/conan.cmake)

conan_cmake_run(
    CONANFILE conanfile.py
    BUILD missing)

conan_basic_setup(TARGETS SKIP_STD NO_OUTPUT_DIRS)

add_library(CryfsDependencies_range-v3 INTERFACE)
target_link_libraries(CryfsDependencies_range-v3 INTERFACE CONAN_PKG::range-v3)

add_library(CryfsDependencies_spdlog INTERFACE)
target_link_libraries(CryfsDependencies_spdlog INTERFACE CONAN_PKG::spdlog)

add_library(CryfsDependencies_boost INTERFACE)
target_link_libraries(CryfsDependencies_boost INTERFACE CONAN_PKG::boost)

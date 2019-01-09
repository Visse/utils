cmake_minimum_required(VERSION 2.6)


add_library(VisseUtils INTERFACE)
add_library(Visse::Utils ALIAS VisseUtils)

target_include_directories(VisseUtils INTERFACE include/)


option(VISSE_UTILS_BUILD_TESTS "Build tests for Visse::Utils" OFF)

if (VISSE_UTILS_BUILD_TESTS)
    add_subdirectory (tests/)
endif (VISSE_UTILS_BUILD_TESTS)


cmake_minimum_required(VERSION 2.8)

project(CXX11-test CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -pthread -Wl,--no-as-needed")
set(CMAKE_BUILD_TYPE "debug")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
find_package(WEOS REQUIRED)

include_directories(
    ${WEOS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../common
    ../3rdparty
    ../3rdparty/gtest-full
)

add_definitions("-DBOOST_DISABLE_ASSERTS")

set(COMMON_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/testutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gtest-full/gtest/gtest-all.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gtest-full/gtest/gtest_main.cc
)

# Add the sources for the wrapper to COMMON_SOURCES.
weos_use_wrapper(CXX11 SOURCE_LIST COMMON_SOURCES)

function(add_test_executable name sources)
    add_executable(${name} ${sources})
endfunction()

macro(add_test_directory _dir)
    add_subdirectory(../${_dir} ${_dir})
endmacro()

# Recurse into the "subdirectories" which contain the actual tests.
#add_test_directory(callback)
#add_test_directory(memorypool)
add_test_directory(mutex)
#add_test_directory(semaphore)
add_test_directory(thread)

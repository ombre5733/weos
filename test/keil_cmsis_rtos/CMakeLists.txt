#*******************************************************************************
# WEOS - Wrapper for embedded operating systems
#
# Copyright (c) 2013-2015, Manuel Freiberger
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#*******************************************************************************

cmake_minimum_required(VERSION 2.8)

set(TOOLCHAIN "" CACHE STRING "The toolchain for building (GCC-ARM, ARMCC).")

if(TOOLCHAIN STREQUAL "GCC-ARM")
    message(STATUS "Using the GCC-ARM toolchain")
    set(CMAKE_TOOLCHAIN_FILE "../common/toolchain-gcc-arm.cmake")
    set(CMAKE_USER_MAKE_RULES_OVERRIDE "../common/toolchain-gcc-arm-override.cmake")
elseif(TOOLCHAIN STREQUAL "ARMCC")
    message(STATUS "Using the ARMCC toolchain")
    set(CMAKE_TOOLCHAIN_FILE "../common/toolchain-armcc.cmake")
else()
    message(FATAL_ERROR "Unknown toolchain '${TOOLCHAIN}'")
endif()

project(Keil-CMSIS-RTOS-test C CXX ASM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
find_package(WEOS REQUIRED)

include_directories(
    ${WEOS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../common
    ../3rdparty
    ../3rdparty/cmsis
    ../3rdparty/gtest-stripped
    ../3rdparty/keil_cmsis_rtos/INC
)

add_definitions("-DBOOST_DISABLE_ASSERTS")

set(COMMON_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/RTX_Conf_CM.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/system_stm32f4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/testutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gtest-stripped/gtest/gtest-all.cc
)

# Add the sources for the wrapper to COMMON_SOURCES.
weos_use_wrapper(Keil-CMSIS-RTOS SOURCE_LIST COMMON_SOURCES)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "ARM")
    add_definitions("-DBOOST_COMPILER_CONFIG=\"boost/config/compiler/common_edg.hpp\"")

    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --cpu=Cortex-M4 --apcs=interwork")

    set(COMMON_FLAGS "--cpu=Cortex-M4 -O2 --apcs=interwork --exceptions")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} --gnu")

    # Link against Keil's CMSIS-RTOS library.
    set(CMSIS_RTX_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/keil_cmsis_rtos/LIB/ARM/RTX_CM4.lib")

    # Add the necessary startup files.
    list(APPEND COMMON_SOURCES
             ${CMAKE_CURRENT_SOURCE_DIR}/../common/armcc/startup_stm32f4xx.s
             ${CMAKE_CURRENT_SOURCE_DIR}/../common/armcc/retarget.cpp
    )

    # Add the scatter file.
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} --scatter ${CMAKE_CURRENT_SOURCE_DIR}/../common/armcc/linker.sct")

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(COMMON_FLAGS "-Wall -g -O2 -ffunction-sections -fdata-sections -fno-builtin")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=gnu++11")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMMON_FLAGS}")

    # Link against Keil's CMSIS-RTOS library.
    set(CMSIS_RTX_LIB "RTX_CM4")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/keil_cmsis_rtos/LIB/GCC")

    # Add the necessary startup files.
    list(APPEND COMMON_SOURCES
             ${CMAKE_CURRENT_SOURCE_DIR}/../common/gcc/startup_stm32f4xx.S
             ${CMAKE_CURRENT_SOURCE_DIR}/../common/gcc/newlib_retarget.c
    )

    # Add the linker script.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Tstm32f4xx.ld")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../common/gcc")
endif()

# This function adds a target to create an executable and a binary image file.
# The target of the binary image file is added to the dependency "bin", i.e.
# when the top-level bin-target is built, all other binaries shall be built,
# too.
function(add_test_executable name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} ${CMSIS_RTX_LIB})
    add_binary_image(${name}.bin ${name})
    add_custom_target(${name}_bin ALL DEPENDS ${name}.bin)
endfunction()

macro(add_test_directory _dir)
    add_subdirectory(../${_dir} ${_dir})
endmacro()

# Recurse into the "subdirectories" which contain the actual tests.
add_test_directory(atomic)
add_test_directory(functional)
add_test_directory(memorypool)
add_test_directory(messagequeue)
add_test_directory(mutex)
add_test_directory(semaphore)
add_test_directory(thread)
add_test_directory(type_traits)

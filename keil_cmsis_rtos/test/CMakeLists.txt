#*******************************************************************************
# WEOS - Wrapper for embedded operating systems
#
# Copyright (c) 2013, Manuel Freiberger
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

set(TOOLCHAIN "" CACHE STRING "The toolchain for building (GCC, ARMCC).")

if(TOOLCHAIN STREQUAL "GCC")
    include("toolchain-gcc-arm.cmake")
else()
    message(FATAL_ERROR "Unknown toolchain '${TOOLCHAIN}'")
endif()

project(test C CXX ASM)

#set(BOOST_INCLUDEDIR "/usr/include/boost")
#find_package(Boost REQUIRED)

if(TOOLCHAIN STREQUAL "GCC")
    set(COMMON_FLAGS "-Wall -g -O2")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=gnu++11")
endif()

include_directories(
    .
    ../../3rdparty
    ../../3rdparty/keil_cmsis_rtos/INC
#    ${Boost_INCLUDE_DIRS}
)

# Specify the location of the user_config.hpp file.
add_definitions("-DWEOS_USER_CONFIG=\"common/user_config.hpp\"")

set(COMMON_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/common/RTX_Conf_CM.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common/system_stm32f4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gtest-all.cc
)

if(TOOLCHAIN STREQUAL "GCC")
    list(APPEND COMMON_SOURCES
             ${CMAKE_CURRENT_SOURCE_DIR}/common/gcc/startup_stm32f4xx.S
             ${CMAKE_CURRENT_SOURCE_DIR}/common/gcc/newlib_retarget.c
    )

    # Add the linker script.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Tstm32f4xx.ld")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/common/gcc")

    # Link against the CMSIS RTX.
    set(CMSIS_RTX_LIB "RTX_CM4")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/keil_cmsis_rtos/LIB/GCC")
endif()

# A target to build all test binaries. CMake-files in the subdirectories can
# add their targets as dependencies to this target. Thus, invoking this target
# builds all binaries at once.
add_custom_target(bin)

# This function adds a target to create an executable and a binary file.
# The target of the binary file is added to the dependency "bin", i.e. when
# the top-level bin-target is built, all other binaries shall be built, too.
function(add_executable_and_bin name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} ${CMSIS_RTX_LIB})
    add_binary_image(${name}.bin ${name})
    add_custom_target(${name}_bin DEPENDS ${name}.bin)
    add_dependencies(bin ${name}_bin)
endfunction()

file(COPY runAllTests.py DESTINATION .)

# Walk into the subdirectories which contain the actual tests.
#add_subdirectory(chrono)
add_subdirectory(mutex)
add_subdirectory(semaphore)

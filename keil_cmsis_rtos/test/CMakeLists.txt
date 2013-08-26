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

include("toolchain.cmake")

cmake_minimum_required(VERSION 2.8)

project(test C CXX ASM)

#set(BOOST_INCLUDEDIR "/usr/include/boost")
#find_package(Boost REQUIRED)

set(NANO_FLAGS "--specs=nano.specs")

# Flags for semihosting.
set(SEMIHOST_FLAGS "--specs=rdimon.specs -lc -lc -lrdimon")
# Flags is no hosting is requested.
set(NOHOST_FLAGS "-lc -lc -lnosys")
# Flags for map file creation.
set(MAP_FLAGS "-Wl,-Map=${PROJECT_NAME}.map")

set(COMMON_FLAGS "-g -O0 -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS   ${COMMON_FLAGS})
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++11")
set(CMAKE_EXE_LINKER_FLAGS "${NANO_FLAGS} ${NOHOST_FLAGS} -Wl,--gc-sections -Tstm32f4xx.ld ${MAP_FLAGS}")

include_directories(
    .
    ../../3rdparty
#    ${Boost_INCLUDE_DIRS}
)

# Add the directory containing the linker script.
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

set(COMMON_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/common/startup_stm32f4xx.S
        ${CMAKE_CURRENT_SOURCE_DIR}/common/newlib_retarget.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common/system_stm32f4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common/test_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gtest/gtest-all.cc
)

# A target to build all test binaries. CMake-files in the subdirectories can
# add their targets as dependencies to this target. Thus, invoking this target
# builds all binaries at once.
add_custom_target(bin)

# This function adds a target to create an executable and a binary file.
# The target of the binary file is added to the dependency "bin", i.e. when
# the top-level bin-target is built, all other binaries shall be built, too.
function(add_executable_and_bin name sources)
    add_executable(${name} ${sources})
    add_custom_command(
        OUTPUT ${name}.bin
        COMMAND ${OBJCOPY} -O binary ${name} ${name}.bin
        DEPENDS ${name}
    )
    add_custom_target(${name}_bin DEPENDS ${name}.bin)
    add_dependencies(bin ${name}_bin)
endfunction()

file(COPY runAllTests.py DESTINATION .)

# Walk into the subdirectories which implement the actual tests.
add_subdirectory(chrono)

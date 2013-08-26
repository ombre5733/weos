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

include_directories(.)
#include_directories(${Boost_INCLUDE_DIRS})

# Add the directory containing the linker script.
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

set(COMMON_SOURCES
        common/startup_stm32f4xx.S
        common/newlib_retarget.c
        common/system_stm32f4xx.c
        common/test_main.cpp
        gtest/gtest-all.cc
)
add_executable(${PROJECT_NAME} ${COMMON_SOURCES})

add_custom_command(
    OUTPUT ${PROJECT_NAME}.bin
    COMMAND /opt/gcc-arm-none-eabi-4_7-2013q2/bin/arm-none-eabi-objcopy -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}
)
add_custom_target(bin DEPENDS ${PROJECT_NAME}.bin)

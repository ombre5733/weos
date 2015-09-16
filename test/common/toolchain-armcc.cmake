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

set(CMAKE_SYSTEM_NAME Generic)

include(CMakeForceCompiler)

find_program(_armcc_executable "armcc")
if (NOT _armcc_executable)
    message(FATAL_ERROR "Unable to locate 'armcc'.")
endif()

get_filename_component(_armcc_path ${_armcc_executable} PATH)

cmake_force_c_compiler("${_armcc_path}/armcc" ARM)
cmake_force_cxx_compiler("${_armcc_path}/armcc" ARM)

set(CMAKE_ASM_COMPILER "${_armcc_path}/armasm")
set(CMAKE_ASM_COMPILER_ID_RUN TRUE)
set(CMAKE_ASM_COMPILER_ID ARM)
set(CMAKE_ASM_COMPILER_WORKS TRUE)
set(CMAKE_ASM_COMPILER_FORCED TRUE)

set(CMAKE_AR "${_armcc_path}/armar")
set(CMAKE_LINKER "${_armcc_path}/armlink")

set(FROMELF "${_armcc_path}/fromelf")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_C_COMPILE_OBJECT   "<CMAKE_C_COMPILER>   <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

set(CMAKE_C_LINK_EXECUTABLE   "${CMAKE_LINKER} ${ARMLINK_SPECIAL_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET> <OBJECTS>")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} ${ARMLINK_SPECIAL_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET> <OBJECTS>")

set(CMAKE_C_CREATE_STATIC_LIBRARY "${CMAKE_AR} --create <TARGET> <OBJECTS>")



# A convenience function to create a binary (downloadable) image.
function(add_binary_image outputFile inputFile)
    add_custom_command(
        OUTPUT ${outputFile}
        COMMAND ${FROMELF} --bin --output ${outputFile} ${inputFile}
        DEPENDS ${inputFile}
    )
endfunction()

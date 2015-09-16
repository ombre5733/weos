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

find_program(_gcc_executable "arm-none-eabi-gcc")
if (_gcc_executable)
    get_filename_component(_gcc_path ${_gcc_executable} PATH)
    get_filename_component(_gcc_ext ${_gcc_executable} EXT)
    cmake_force_c_compiler("${_gcc_path}/arm-none-eabi-gcc${_gcc_ext}" GNU)
    cmake_force_cxx_compiler("${_gcc_path}/arm-none-eabi-g++${_gcc_ext}" GNU)
    set(OBJCOPY "${_gcc_path}/arm-none-eabi-objcopy${_gcc_ext}")
else()
    message(FATAL_ERROR "Unable to locate 'arm-none-eabi-gcc'.")
endif()


# A convenience function to create a binary (downloadable) image.
function(add_binary_image outputFile inputFile)
    add_custom_command(
        OUTPUT ${outputFile}
        COMMAND ${OBJCOPY} -O binary ${inputFile} ${outputFile}
        DEPENDS ${inputFile}
    )
endfunction()

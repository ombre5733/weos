#*******************************************************************************
# WEOS - Wrapper for embedded operating systems
#
# Copyright (c) 2013-2014, Manuel Freiberger
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

include(CMakeParseArguments)

cmake_parse_arguments(_args "" "TARGET;SOURCE_LIST" "" ${SUBMODULE_OPTIONS})

if (NOT "${_args_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(FATAL_ERROR "WEOS CXX11: Unknown arguments '${_args_UNPARSED_ARGUMENTS}'.")
endif()

# The source files which are necessary for this wrapper.
set(_sources
        "${WEOS_ROOT_DIR}/cxx11/thread.cpp")

# If a TARGET is specified, we create a static library from the wrapper's
# sources and link with it.
if(_args_TARGET)
    set(_library_name "weos-Keil-CMSIS-${_args_TARGET}")
    add_library(${_library_name} STATIC ${_sources})
    target_link_libraries(${_args_TARGET} ${_library_name})
endif()

# If a SOURCE_LIST is specified, the wrapper's sources are added to it.
if(_args_SOURCE_LIST)
    set(_new_list ${${_args_SOURCE_LIST}})
    list(APPEND _new_list ${_sources})
    set(${_args_SOURCE_LIST} ${_new_list} PARENT_SCOPE)
endif()

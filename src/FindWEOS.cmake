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

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/config.hpp")
    file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/config.hpp" _weos_string
                 REGEX ".*#ifndef.*WEOS_CONFIG_HPP.*")
    if(NOT "${_weos_string}" STREQUAL "")
        set(WEOS_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
        set(WEOS_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WEOS DEFAULT_MSG WEOS_ROOT_DIR)


function(weos_use_wrapper _wrapper)
    # Generate a list of all possible WEOS wrappers.
    file(GLOB cmake_files
              RELATIVE "${WEOS_INCLUDE_DIRS}"
              "${WEOS_INCLUDE_DIRS}/weos-*.cmake")
    foreach(_iter ${cmake_files})
        STRING(REPLACE "weos-" "" _temp ${_iter})
        STRING(REPLACE ".cmake" "" _temp ${_temp})
        list(APPEND _all_wrappers ${_temp})
    endforeach()

    # Now check if the requested wrapper is available.
    list(FIND _all_wrappers ${_wrapper} _index)
    if(_index EQUAL -1)
        set(_wrapper_hint "")
        foreach(_iter ${_all_wrappers})
            if(NOT "${_wrapper_hint}" STREQUAL "")
                set(_wrapper_hint "${_wrapper_hint}, ")
            endif()
            set(_wrapper_hint "${_wrapper_hint}'${_iter}'")
        endforeach()
        message(FATAL_ERROR "There is no WEOS wrapper '${_wrapper}'. Possible values are: ${_wrapper_hint}.")
    endif()

    # Finally load the wrapper.
    set(SUBMODULE_OPTIONS ${ARGN})
    include("${WEOS_INCLUDE_DIRS}/weos-${_wrapper}.cmake")
endfunction()

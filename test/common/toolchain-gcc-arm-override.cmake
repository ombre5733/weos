#*******************************************************************************
# WEOS - Wrapper for embedded operating systems
#
# Copyright (c) 2013-2016, Manuel Freiberger
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

# Flags for newlib.
set(_newlib_nano_flags "--specs=nano.specs")
# Flags for semihosting.
set(_semihost_flags "--specs=rdimon.specs -lc -lc -lrdimon")
# Flags if no hosting is requested.
set(_nohost_flags "-lc -lc -lnosys")
# Flags for map file creation.
set(_map_flags "-Wl,-Map=${PROJECT_NAME}.map")

# Set the initial value of the ASM, C, C++ and linker flags.
# Note: The ASM flags have to be specified directly and not via an *_INIT
# variable.
set(_shared_flags "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_ASM_FLAGS ${_shared_flags})
set(CMAKE_C_FLAGS_INIT   ${_shared_flags})
set(CMAKE_CXX_FLAGS_INIT ${_shared_flags})
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_shared_flags} ${_newlib_nano_flags} ${_nohost_flags} -Wl,--gc-sections ${_map_flags}")

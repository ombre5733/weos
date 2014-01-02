# Flags for newlib.
set(_newlib_nano_flags "--specs=nano.specs")
# Flags for semihosting.
set(_semihost_flags "--specs=rdimon.specs -lc -lc -lrdimon")
# Flags if no hosting is requested.
set(_nohost_flags "-lc -lc -lnosys")
# Flags for map file creation.
set(_map_flags "-Wl,-Map=${PROJECT_NAME}.map")


set(_shared_flags "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_ASM_FLAGS_INIT ${_shared_flags})
set(CMAKE_C_FLAGS_INIT   ${_shared_flags})
set(CMAKE_CXX_FLAGS_INIT ${_shared_flags})
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_shared_flags} ${_newlib_nano_flags} ${_nohost_flags} -Wl,--gc-sections ${_map_flags}")

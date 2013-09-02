set(CMAKE_SYSTEM_NAME Generic)

include(CMakeForceCompiler)

CMAKE_FORCE_C_COMPILER("/opt/gcc-arm-none-eabi-4_7-2013q2/bin//arm-none-eabi-gcc" GNU)
CMAKE_FORCE_CXX_COMPILER("/opt/gcc-arm-none-eabi-4_7-2013q2/bin//arm-none-eabi-g++" GNU)

set(OBJCOPY "/opt/gcc-arm-none-eabi-4_7-2013q2/bin/arm-none-eabi-objcopy")

# Flags for newlib.
set(NANO_FLAGS "--specs=nano.specs")
# Flags for semihosting.
set(SEMIHOST_FLAGS "--specs=rdimon.specs -lc -lc -lrdimon")
# Flags is no hosting is requested.
set(NOHOST_FLAGS "-lc -lc -lnosys")
# Flags for map file creation.
set(MAP_FLAGS "-Wl,-Map=${PROJECT_NAME}.map")

set(COMMON_FLAGS "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_ASM_FLAGS ${COMMON_FLAGS})
set(CMAKE_C_FLAGS   ${COMMON_FLAGS})
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++11")
set(CMAKE_EXE_LINKER_FLAGS "${NANO_FLAGS} ${NOHOST_FLAGS} -Wl,--gc-sections ${MAP_FLAGS}")

function(add_binary_image outputFile inputFile)
    add_custom_command(
        OUTPUT ${outputFile}
        COMMAND ${OBJCOPY} -O binary ${inputFile} ${outputFile}
        DEPENDS ${inputFile}
    )
endfunction()

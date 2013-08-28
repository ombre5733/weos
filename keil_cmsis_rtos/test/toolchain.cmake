set(CMAKE_SYSTEM_NAME Generic)

include(CMakeForceCompiler)

CMAKE_FORCE_C_COMPILER("/opt/gcc-arm-none-eabi-4_7-2013q2/bin//arm-none-eabi-gcc" GNU)
CMAKE_FORCE_CXX_COMPILER("/opt/gcc-arm-none-eabi-4_7-2013q2/bin//arm-none-eabi-g++" GNU)

set(OBJCOPY "/opt/gcc-arm-none-eabi-4_7-2013q2/bin/arm-none-eabi-objcopy")

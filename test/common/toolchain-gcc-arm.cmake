set(CMAKE_SYSTEM_NAME Generic)

include(CMakeForceCompiler)

find_program(_gcc_executable "arm-none-eabi-gcc")
if (_gcc_executable)
    get_filename_component(_gcc_path ${_gcc_executable} PATH)
    cmake_force_c_compiler("${_gcc_path}/arm-none-eabi-gcc" GNU)
    cmake_force_cxx_compiler("${_gcc_path}/arm-none-eabi-g++" GNU)
    set(OBJCOPY "${_gcc_path}/arm-none-eabi-objcopy")
else()
    message(FATAL_ERROR "Unable to locate 'arm-none-eabi-gcc'.")
endif()

function(add_binary_image outputFile inputFile)
    add_custom_command(
        OUTPUT ${outputFile}
        COMMAND ${OBJCOPY} -O binary ${inputFile} ${outputFile}
        DEPENDS ${inputFile}
    )
endfunction()

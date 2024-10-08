set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(WCH_SDK_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/riscv-gcc)
    set(TOOLPATH ${CMAKE_CURRENT_LIST_DIR}/riscv-gcc/xpack-riscv-none-elf-gcc/bin/)
endif()

# toolchains setting
if(NOT TOOLPATH)
    message(FATAL_ERROR "please use install_toolchains.sh download toolchains or set your TOOLPATH for wch gcc.")
else() 
    set(CMAKE_C_COMPILER ${TOOLPATH}/riscv-none-elf-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLPATH}/riscv-none-elf-g++)
    set(CMAKE_ASM_COMPILER ${TOOLPATH}/riscv-none-elf-gcc)
    set(CMAKE_AR ${TOOLPATH}/riscv-none-elf-ar)
    set(CMAKE_OBJCOPY ${TOOLPATH}/riscv-none-elf-objcopy)
    set(CMAKE_OBJDUMP ${TOOLPATH}/riscv-none-elf-objdump)
    set(SIZE ${TOOLPATH}/riscv-none-elf-size)
endif()

include(${WCH_SDK_PATH}/cmake/config.cmake)
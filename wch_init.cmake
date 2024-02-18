set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# toolchains setting
if(NOT TOOLPATH)
    message(FATAL_ERROR "please set TOOLPATH for wch gcc")
else(<condition>)
    set(CMAKE_C_COMPILER ${TOOLPATH}/riscv-none-elf-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLPATH}/riscv-none-elf-g++)
    set(CMAKE_ASM_COMPILER ${TOOLPATH}/riscv-none-elf-gcc)
    set(CMAKE_AR ${TOOLPATH}/riscv-none-elf-ar)
    set(CMAKE_OBJCOPY ${TOOLPATH}/riscv-none-elf-objcopy)
    set(CMAKE_OBJDUMP ${TOOLPATH}/riscv-none-elf-objdump)
    set(SIZE ${TOOLPATH}/riscv-none-elf-size)
endif()
set(TOOLPATH  /home/sullivan/bin/compiler/MRS_Toolchain_Linux_x64_V1.90/RISC-V_Embedded_GCC12/bin)

if(NOT CHIP_NAME)
    message(FATAL_ERROR "please set CHIP_NAME for ch32")
elseif(CHIP_NAME STREQUAL "ch58x")
    add_compile_options(-march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore)
    add_compile_options(-fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common)
    
    include_directories(${WCH_SDK_PATH}/hal/CH58X/include)
    aux_source_directory(${WCH_SDK_PATH}/hal/CH58X/src FILE_SRC)

    set(LINKER_SCRIPT  ${WCH_SDK_PATH}/hal/CH58X/Link.ld)
    add_link_options(
                -nostartfiles 
                -Xlinker --gc-sections  
                -Wl,--print-memory-usage
                # -Wl,-Map,${PROJECT_NAME}.map 
                --specs=nano.specs 
                --specs=nosys.specs)
    add_link_options(-T ${LINKER_SCRIPT})

    function(config_app param)
        # current app source file
        add_executable(${param})
        aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src APP_SRC)
        target_sources(${param} PRIVATE 
            ${FILE_SRC}
            ${WCH_SDK_PATH}/hal/CH58X/startup_CH583.S
            ${APP_SRC}
            ${WCH_SDK_PATH}/hal/CH58X/Link.ld
        )
        target_include_directories(${param} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_options(${param} PRIVATE 
            -Wl,-Map,${param}.map 
        )
        target_link_libraries(${param} 
            ${WCH_SDK_PATH}/hal/CH58X/lib/LIBCH58xBLE.a
            ${WCH_SDK_PATH}/hal/CH58X/lib/libISP583.a
            ${WCH_SDK_PATH}/hal/CH58X/lib/libRV3UFI.a
            ${WCH_SDK_PATH}/hal/CH58X/lib/LIBWCHLWNS.a
        )

        # set(HEX_FILE ${PROJECT_BINARY_DIR}/app.hex)
        # set(BIN_FILE ${PROJECT_BINARY_DIR}/app.bin)
        # add_custom_command(TARGET ${param} POST_BUILD
        #     COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${param}> ${HEX_FILE}
        #     COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${param}> ${BIN_FILE}
        # )
        # add_custom_command(
        #         TARGET ${param} POST_BUILD
        #         COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${param}> ${HEX_FILE}
        #         COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${param}> ${BIN_FILE}
        # )
        set(HEX_FILE ${PROJECT_BINARY_DIR}/app.hex)
        add_custom_command(
                TARGET ${param} POST_BUILD
                COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${param}> ${HEX_FILE}
                #COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${param}> ${BIN_FILE}
        )
    endfunction()
endif()


add_compile_options(-O3)
add_compile_options(-Wall)
#add_definitions(-DDEBUG=1)



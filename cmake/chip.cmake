function(enable_cpp app_name chip_name)
    target_compile_options(${app_name} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:
        ${common_flags} 
        #-fno-rtti 
        -fno-exceptions
        -fno-threadsafe-statics
        -Werror 
        -Wno-return-type 
        -Wno-unused-function
        -Wno-volatile  
        -Wno-deprecated-declarations 
        -Wno-unused-variable
        -Wno-strict-aliasing
        -fpermissive> # Handling obsolete code without warning 
    )

    if(${chip_name} STREQUAL "ch32v307")
        add_custom_command(
            TARGET ${app_name} PRE_BUILD
            COMMAND sed -i '368c \ \ \  la a0,__libc_fini_array' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i '369c \ \ \  call atexit' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i '370c \ \ \  call __libc_init_array' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i '371c \ \ \  jal  SystemInit' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i '372c \ \ \  la t0, main' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i '373c \ \ \  csrw mepc, t0' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i '374c \ \ \  mret' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
       )  
    
    elseif(${chip_name} STREQUAL "ch32v003")
        add_custom_command(
            TARGET ${app_name} PRE_BUILD
            COMMAND sed -i '161c \ \ \  la a0,__libc_fini_array' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
            COMMAND sed -i '162c \ \ \  call atexit' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
            COMMAND sed -i '163c \ \ \  call __libc_init_array' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
            COMMAND sed -i '164c \ \ \  jal  SystemInit' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
            COMMAND sed -i '165c \ \ \  la t0, main' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
            COMMAND sed -i '166c \ \ \  csrw mepc, t0' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
            COMMAND sed -i '167c \ \ \  mret' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v00x.S
        )  
    elseif(${chip_name} STREQUAL "ch592")
        add_custom_command(
            TARGET ${app_name} PRE_BUILD
            COMMAND sed -i '178c \ \ \  ori t0, t0, 3' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '179c \ \ \  csrw mtvec, t0' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '180c \ \ \  la a0,__libc_fini_array' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '181c \ \ \  call atexit' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '182c \ \ \  call __libc_init_array' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '183c \ \ \  la t0, main' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '184c \ \ \  csrw mepc, t0' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            COMMAND sed -i '185c \ \ \  mret' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_CH592.S
            )
    endif()
    target_compile_definitions(${app_name} PUBLIC WCH_USE_CPP)
endfunction()

function(enable_printf app_name chip_name printf_target)
    if(${chip_name} STREQUAL "ch32v307")
        if(${printf_target} STREQUAL "sdi")
            add_custom_command(
                TARGET ${app_name} PRE_BUILD 
                COMMAND sed -i 's/define SDI_PRINT \ \ SDI_PR_CLOSE/define SDI_PRINT SDI_PR_OPEN/g' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/debug.h
            ) 
            target_compile_definitions(${app_name} PUBLIC WCH_USE_SDI_PRINTF)
        elseif(${printf_target} STREQUAL "uart1")
        elseif(${printf_target} STREQUAL "uart2")
        else()
        endif()
    elseif(${chip_name} STREQUAL "ch32v003")
    elseif(${chip_name} STREQUAL "ch592")
        if(${printf_target} STREQUAL "uart1")
            target_compile_definitions(${app_name} PUBLIC DEBUG=Debug_UART1)
        elseif(${printf_target} STREQUAL "uart2")
            target_compile_definitions(${app_name} PUBLIC DEBUG=Debug_UART2)
        elseif(${printf_target} STREQUAL "usb")
            target_compile_definitions(${app_name} PUBLIC Debug_USB)
        endif()
    endif()  
endfunction()

function(set_ram_and_flash app_name chip_name ram flash)
    if(${chip_name} STREQUAL "ch32v307")
        add_custom_command(
            TARGET ${app_name} PRE_BUILD
            COMMAND sed -i '26c 	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = ${flash}K' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
            COMMAND sed -i '27c 	RAM (xrw) : ORIGIN = 0x20000000, LENGTH = ${ram}K' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
        )
    elseif(${chip_name} STREQUAL "ch592")
    # TODO
    else()
        message(FATAL_ERROR "${chip_name} is not support set ramor flash!")
    endif()
endfunction()

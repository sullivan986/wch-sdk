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
        -fpermissive #处理过时的代码不发出警告
        >
    )
    if(${chip_name} STREQUAL "ch32v307")
        add_custom_command(
            TARGET ${app_name} PRE_BUILD
            COMMAND sed -i '368c \ \ \  la a0,__libc_fini_array' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '369c \ \ \  call atexit' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '370c \ \ \  call __libc_init_array' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '371c \ \ \  jal  SystemInit' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '372c \ \ \  la t0, main' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '373c \ \ \  csrw mepc, t0' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '374c \ \ \  mret' ${CMAKE_BINARY_DIR}/tmp_file/startup_ch32v30x_D8C.S
            COMMAND sed -i '252c void _fini () {}' ${CMAKE_BINARY_DIR}/tmp_file/debug.c
            COMMAND sed -i '253c void _init () {}' ${CMAKE_BINARY_DIR}/tmp_file/debug.c
        )   
    elseif(${chip_name} STREQUAL "ch59x")
    # TODO
    endif()
endfunction()

function(enable_printf app_name chip_name printf_target)
    if(${chip_name} STREQUAL "ch32v307")
        if(printf_target STREQUAL "sdi")
        add_custom_command(
            TARGET ${app_name} PRE_BUILD 
            COMMAND sed -i 's/define SDI_PRINT \ \ SDI_PR_CLOSE/define SDI_PRINT SDI_PR_OPEN/g' ${CMAKE_BINARY_DIR}/tmp_file/debug.h
        ) 
        elseif(${chip_name} STREQUAL "uart1")
            #todo
        endif()
    elseif(${chip_name} STREQUAL "ch59x")
    # TODO
    endif()
endfunction()

function(set_ram_and_flash app_name chip_name ram flash)
    if(${chip_name} STREQUAL "ch32v307")
        add_custom_command(
            TARGET ${CMAKE_CURRENT_PROJECT_PARAM} PRE_BUILD
            COMMAND sed -i '26c 	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = ${flash}K' ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND sed -i '27c 	RAM (xrw) : ORIGIN = 0x20000000, LENGTH = ${ram}K' ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
        )
    elseif(${chip_name} STREQUAL "ch59x")
    # TODO
    endif()
endfunction()

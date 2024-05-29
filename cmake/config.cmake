include(${WCH_SDK_PATH}/cmake/chip.cmake)
include(${WCH_SDK_PATH}/cmake/use_lib.cmake)



function(config_common_mcu app_name chip_name)
    if(chip_name STREQUAL "ch32v307")
        set(APP_HAL_PATH ${WCH_SDK_PATH}/libs/ch32v307/EVT/EXAM)
        set(APP_SERIES_NAME ch32v30x)
        set(APP_STARTUP_FILE startup_${APP_SERIES_NAME}_D8C.S)

        set(CHIP_COMPILE_OPS_ARCH rv32imafcxw)
        set(CHIP_COMPILE_OPS_ABI ilp32f)
        set(CHIP_COMPILE_OPS_SDL 8)
        set(CHIP_COMPILE_OPS_SR -msave-restore)
    elseif(chip_name STREQUAL "ch32v003")
        set(APP_HAL_PATH ${WCH_SDK_PATH}/libs/ch32v003/EVT/EXAM)
        set(APP_SERIES_NAME ch32v00x)
        set(APP_STARTUP_FILE startup_${APP_SERIES_NAME}.S)

        set(CHIP_COMPILE_OPS_ARCH rv32ecxw)
        set(CHIP_COMPILE_OPS_ABI ilp32e)
        set(CHIP_COMPILE_OPS_SDL 0)
        set(CHIP_COMPILE_OPS_SR -msave-restore)
    endif()

    # compile options
    aux_source_directory(${APP_HAL_PATH}/SRC/Peripheral/src APP_ALL_SRC_2)
    target_include_directories(${app_name} PRIVATE
        ${APP_HAL_PATH}/SRC/Peripheral/inc
        ${APP_HAL_PATH}/SRC/Core
    )

    target_compile_options(${app_name} PRIVATE
        -march=${CHIP_COMPILE_OPS_ARCH}
        -mabi=${CHIP_COMPILE_OPS_ABI}
        -msmall-data-limit=${CHIP_COMPILE_OPS_SDL}
        ${CHIP_COMPILE_OPS_SR}
        -fmessage-length=0 # automatic line feeds for compilation information.
        -fsigned-char
        -ffunction-sections
        -fdata-sections
        -fno-common # Disable implicit sharing of global variables.
        # -fsingle-precision-constant 
        -Wunused
        -Wuninitialized
        -lm
        -DUTENSIL_SET_CHIP_${chip_name}
        ${EXTRA_CO}
    )

    target_link_options(${app_name} PRIVATE
        -nostartfiles 
        -Xlinker --gc-sections  
        -Wl,--print-memory-usage
        # -Wl,-Map,${PROJECT_NAME}.map 
        --specs=nano.specs 
        --specs=nosys.specs
        #-static-libstdc++
        -march=${CHIP_COMPILE_OPS_ARCH}
        -mabi=${CHIP_COMPILE_OPS_ABI}
        -flto
        -T ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
        -Wl,-Map,${app_name}.map
    )

    target_sources(${app_name} PRIVATE
        ${APP_ALL_SRC_2}
        ${APP_HAL_PATH}/SRC/Core/core_riscv.c
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_STARTUP_FILE}
    )
    
    target_link_libraries(${app_name} PUBLIC
        utensil
        printfloat
    )

    configure_file(
        ${WCH_SDK_PATH}/configure/${chip_name}/Link.ld
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
        COPYONLY
    )

    configure_file(
        ${APP_HAL_PATH}/SRC/Startup/${APP_STARTUP_FILE}
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_STARTUP_FILE}
        COPYONLY
    )

    configure_file(
        ${APP_HAL_PATH}/SRC/Debug/debug.h
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/debug.h
        COPYONLY
    )

    configure_file(
        ${APP_HAL_PATH}/SRC/Debug/debug.c
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/debug.c
        COPYONLY
    )

    configure_file(
        ${WCH_SDK_PATH}/configure/${chip_name}/${APP_SERIES_NAME}_conf.h
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_SERIES_NAME}_conf.h
        COPYONLY
    )

    configure_file(
        ${WCH_SDK_PATH}/configure/${chip_name}/${APP_SERIES_NAME}_it.h
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_SERIES_NAME}_it.h
        COPYONLY
    )

    configure_file(
        ${WCH_SDK_PATH}/configure/${chip_name}/${APP_SERIES_NAME}_it.c
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_SERIES_NAME}_it.c
        COPYONLY
    )

    configure_file(
        ${WCH_SDK_PATH}/configure/${chip_name}/system_${APP_SERIES_NAME}.h
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/system_${APP_SERIES_NAME}.h
        COPYONLY
    )

    configure_file(
        ${WCH_SDK_PATH}/configure/${chip_name}/system_${APP_SERIES_NAME}.c
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/system_${APP_SERIES_NAME}.c
        COPYONLY
    )       
endfunction()


function(config_ble_mcu app_name chip_name)
    if(chip_name STREQUAL "ch592")
        set(APP_HAL_PATH ${WCH_SDK_PATH}/libs/ch592/EVT/EXAM)
        set(APP_SERIES_NAME CH592)
        set(APP_STARTUP_FILE startup_${APP_SERIES_NAME}.S)
    endif()

    # compile options
    aux_source_directory(${APP_HAL_PATH}/SRC/StdPeriphDriver APP_ALL_SRC_2)
    aux_source_directory(${APP_HAL_PATH}/BLE/HAL/ APP_ALL_SRC_3)
    target_include_directories(${app_name} PRIVATE
        ${APP_HAL_PATH}/SRC/StdPeriphDriver/inc
        ${APP_HAL_PATH}/SRC/RVMSIS
        ${APP_HAL_PATH}/BLE/HAL/include
        ${APP_HAL_PATH}/BLE/LIB
    )

    target_compile_options(${app_name} PRIVATE
        -march=rv32imac
        -mabi=ilp32
        -mcmodel=medany
        -msmall-data-limit=8
        -mno-save-restore
        -fmessage-length=0 # automatic line feeds for compilation information.
        -fsigned-char
        -ffunction-sections
        -fdata-sections
        -fno-common # Disable implicit sharing of global variables.
        -DUTENSIL_SET_CHIP_${chip_name}
    )

    target_link_options(${app_name} PRIVATE
        -nostartfiles 
        -Xlinker --gc-sections  
        -Wl,--print-memory-usage
        --specs=nano.specs 
        --specs=nosys.specs
        -T ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
        -Wl,-Map,${app_name}.map
    )

    target_sources(${app_name} PRIVATE
        ${APP_ALL_SRC_2}
        ${APP_ALL_SRC_3}
        ${APP_HAL_PATH}/SRC/RVMSIS/core_riscv.c
        ${APP_HAL_PATH}/BLE/LIB/ble_task_scheduler.S
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_STARTUP_FILE}
    )

    target_link_libraries(${app_name}
        ${APP_HAL_PATH}/BLE/LIB/LIBCH59xBLE.a
        ${APP_HAL_PATH}/SRC/StdPeriphDriver/libISP592.a
    )


    configure_file(
        ${APP_HAL_PATH}/SRC/Ld/Link.ld
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
        COPYONLY
    )

    configure_file(
        ${APP_HAL_PATH}/SRC/Startup/${APP_STARTUP_FILE}
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/${APP_STARTUP_FILE}
        COPYONLY
    )
endfunction()


function(config_app app_name chip_name)
    set(CMAKE_CURRENT_PROJECT_PARAM "${app_name}" CACHE STRING INTERNAL FORCE)

    string(FIND "${ARGN}" "enable_rtos" APP_ENABLE_RTOS)
    string(FIND "${ARGN}" "enable_cpp" APP_ENABLE_CPP)
    string(FIND "${ARGN}" "enable_printf_sdi" APP_ENABLE_PRINT_SDI)
    string(FIND "${ARGN}" "enable_printf_uart1" APP_ENABLE_PRINT_UART1)
    string(FIND "${ARGN}" "enable_printf_uart2" APP_ENABLE_PRINT_UART2)
    string(FIND "${ARGN}" "enable_printf_uart3" APP_ENABLE_PRINT_UART3)
    string(FIND "${ARGN}" "enable_tflite" APP_ENABLE_TFLITE)

    string(FIND "${ARGN}" "r32k_f288k" APP_RAM32K_FLASH288K)
    string(FIND "${ARGN}" "r64k_f256k" APP_RAM64K_FLASH256K)
    string(FIND "${ARGN}" "r96k_f224k" APP_RAM96K_FLASH224K)
    string(FIND "${ARGN}" "r128k_f192k" APP_RAM128K_FLASH192K)
    string(FIND "${ARGN}" "r128k_f480k" APP_RAM128K_FLASH480K)
    
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tmp_file/${chip_name})
    
    add_executable(${app_name})
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src APP_ALL_SRC_0)
    aux_source_directory(${CMAKE_BINARY_DIR}/tmp_file/${chip_name} APP_ALL_SRC_1)

    target_include_directories(${app_name} PRIVATE
        ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_sources(${app_name} PRIVATE
        ${APP_ALL_SRC_0}
        ${APP_ALL_SRC_1}
    )

    
    set(common_mcu_list "ch32v307" "ch32v003")
    set(ble_mcu_list "ch592")

    list(FIND common_mcu_list ${chip_name} index_common)
    list(FIND ble_mcu_list ${chip_name} index_ble)
    
    if(index_common GREATER_EQUAL 0)
        config_common_mcu(${app_name} ${chip_name})
    elseif(index_ble GREATER_EQUAL 0)
        config_ble_mcu(${app_name} ${chip_name})
    else()
        message(FATAL_ERROR "${chip_name} is not support")
    endif()
      

    set(HEX_FILE ${PROJECT_BINARY_DIR}/${app_name}.hex)
    set(BIN_FILE ${PROJECT_BINARY_DIR}/${app_name}.bin)
    add_custom_command(
        TARGET ${app_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${app_name}> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${app_name}> ${BIN_FILE}
    )


    if(NOT ${APP_ENABLE_RTOS} EQUAL -1) #enable_rtos must come before enable_cpp because enable_rtos affects startup_ch32v30x_D8C.S.
        enable_rtos(${app_name} ${chip_name})
    endif()

    if(NOT ${APP_ENABLE_CPP} EQUAL -1)
        enable_cpp(${app_name} ${chip_name})
    endif()

    if(NOT ${APP_ENABLE_PRINT_SDI} EQUAL -1)
        enable_printf(${app_name} ${chip_name} sdi)
    endif()

    if(NOT ${APP_ENABLE_TFLITE} EQUAL -1)
        enable_tflite(${app_name})
    endif()

    # cionfig ram and flash size
    if(NOT ${APP_RAM32K_FLASH288K} EQUAL -1)
        set_ram_and_flash(${app_name} ${chip_name} 32 288)
    endif()

    if(NOT ${APP_RAM64K_FLASH256K} EQUAL -1)
        set_ram_and_flash(${app_name} ${chip_name} 64 256)
    endif()

    if(NOT ${APP_RAM96K_FLASH224K} EQUAL -1)
        set_ram_and_flash(${app_name} ${chip_name} 96 224)
    endif()

    if(NOT ${APP_RAM128K_FLASH192K} EQUAL -1)
        set_ram_and_flash(${app_name} ${chip_name} 128 192)
    endif()

    if(NOT ${APP_RAM128K_FLASH480K} EQUAL -1)
        set_ram_and_flash(${app_name} ${chip_name} 128 480)
    endif()
endfunction()



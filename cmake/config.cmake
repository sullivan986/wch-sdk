include(${WCH_SDK_PATH}/cmake/chip.cmake)
include(${WCH_SDK_PATH}/cmake/use_lib.cmake)

if(NOT CHIP_NAME)
    message(FATAL_ERROR "please set CHIP_NAME for ch32")
elseif(CHIP_NAME STREQUAL "ch32v307")
    aux_source_directory(${WCH_SDK_PATH}/hal/CH32V307/src FILE_SRC)
    function(config_app param)
        set(CMAKE_CURRENT_PROJECT_PARAM "${param}" CACHE STRING INTERNAL FORCE)
        # current app source file
        add_executable(${param})
        aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src APP_SRC)
        target_sources(${param} PRIVATE
            ${FILE_SRC}
            ${WCH_SDK_PATH}/hal/CH32V307/startup_ch32v30x_D8C.S
            ${APP_SRC}
        )
        target_include_directories(${param} PRIVATE
            ${WCH_SDK_PATH}/hal/CH32V307/include
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_compile_options(${param}  PRIVATE 
            -march=rv32imafcxw
            -mabi=ilp32f
            -msmall-data-limit=8
            -mno-save-restore
            -fmessage-length=0
            -fsigned-char
            -ffunction-sections
            -fdata-sections
            -fsingle-precision-constant
            -Wunused
            -Wuninitialized
            -lm
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
                -Wno-strict-aliasing>
                #-fpermissive
        )
        target_link_options(${param} PRIVATE
            -nostartfiles 
            -Xlinker --gc-sections  
            -Wl,--print-memory-usage
            # -Wl,-Map,${PROJECT_NAME}.map 
            --specs=nano.specs 
            --specs=nosys.specs
            #-static-libstdc++
            -march=rv32imafcxw 
            -mabi=ilp32f 
            -flto
            -T ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            -Wl,-Map,${param}.map 
        )

        set(HEX_FILE ${PROJECT_BINARY_DIR}/app.hex)
        set(BIN_FILE ${PROJECT_BINARY_DIR}/app.bin)
        add_custom_command(
            TARGET ${param} PRE_BUILD
            COMMAND rm -r -f ${CMAKE_BINARY_DIR}/tmp_file
            COMMAND mkdir ${CMAKE_BINARY_DIR}/tmp_file
            COMMAND cp ${WCH_SDK_PATH}/hal/CH32V307/Link.ld ${CMAKE_BINARY_DIR}/tmp_file
        )
        add_custom_command(
            TARGET ${param} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${param}> ${HEX_FILE}
            COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${param}> ${BIN_FILE}
        )
        target_link_libraries(${param} PUBLIC
            utensil
        )
    endfunction()
    
elseif(CHIP_NAME STREQUAL "ch58x")
    include_directories(${WCH_SDK_PATH}/hal/CH58X/include)
    aux_source_directory(${WCH_SDK_PATH}/hal/CH58X/src FILE_SRC)
    function(config_app param)
        # current app source file
        add_executable(${param})
        aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src APP_SRC)
        target_sources(${param} PRIVATE 
            ${FILE_SRC}
            ${WCH_SDK_PATH}/hal/CH58X/startup_CH583.S
            ${APP_SRC}
        )
        target_include_directories(${param} PRIVATE
            ${WCH_SDK_PATH}/hal/CH58X/include
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_compile_options(${param}  PRIVATE
            -march=rv32imac
            -mabi=ilp32
            -mcmodel=medany
            -msmall-data-limit=8
            -mno-save-restore
            -fmessage-length=0
            -fsigned-char 
            -ffunction-sections 
            -fdata-sections 
            -fno-common
        )
        target_link_options(${param} PRIVATE
            -nostartfiles 
            -Xlinker --gc-sections  
            -Wl,--print-memory-usage
            # -Wl,-Map,${PROJECT_NAME}.map 
            --specs=nano.specs 
            --specs=nosys.specs
            -T ${WCH_SDK_PATH}/hal/CH58X/Link.ld
            -Wl,-Map,app.map 
        )
        set(HEX_FILE ${PROJECT_BINARY_DIR}/app.hex)
        set(BIN_FILE ${PROJECT_BINARY_DIR}/app.bin)
        add_custom_command(
                TARGET ${param} POST_BUILD
                COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${param}> ${HEX_FILE}
                COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${param}> ${BIN_FILE}
        )        
        target_link_libraries(${param} 
            ${WCH_SDK_PATH}/hal/CH58X/lib/LIBCH58xBLE.a
            ${WCH_SDK_PATH}/hal/CH58X/lib/libISP583.a
            ${WCH_SDK_PATH}/hal/CH58X/lib/libRV3UFI.a
            ${WCH_SDK_PATH}/hal/CH58X/lib/LIBWCHLWNS.a
        )
    endfunction()
endif()
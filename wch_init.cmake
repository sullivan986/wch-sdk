set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
 
# toolchains setting
if(NOT TOOLPATH)
    message(FATAL_ERROR "please set TOOLPATH for wch gcc")
else() 
    set(CMAKE_C_COMPILER ${TOOLPATH}/riscv-none-elf-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLPATH}/riscv-none-elf-g++)
    set(CMAKE_ASM_COMPILER ${TOOLPATH}/riscv-none-elf-gcc)
    set(CMAKE_AR ${TOOLPATH}/riscv-none-elf-ar)
    set(CMAKE_OBJCOPY ${TOOLPATH}/riscv-none-elf-objcopy)
    set(CMAKE_OBJDUMP ${TOOLPATH}/riscv-none-elf-objdump)
    set(SIZE ${TOOLPATH}/riscv-none-elf-size)
endif()

# rtos
add_library(freertos INTERFACE)
# utensil
add_library(utensil INTERFACE)
# cherry USB
add_library(Cherry_USB INTERFACE)
# wasm
add_library(wasm_runtime INTERFACE)
# tflite
add_library(tflite INTERFACE)

# freertos for ch32v307
aux_source_directory(${WCH_SDK_PATH}/libs/FreeRTOS freertos_source)
target_include_directories(freertos INTERFACE
    ${WCH_SDK_PATH}/libs/FreeRTOS/include
    ${WCH_SDK_PATH}/libs/FreeRTOS/portable/GCC/RISC-V
    ${WCH_SDK_PATH}/libs/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions
)
target_sources(freertos PUBLIC
    ${freertos_source}
    #${WCH_SDK_PATH}/libs/FreeRTOS/portable/Common/mpu_wrappers.c
    ${WCH_SDK_PATH}/libs/FreeRTOS/portable/MemMang/heap_4.c 
    ${WCH_SDK_PATH}/libs/FreeRTOS/portable/GCC/RISC-V/port.c
    ${WCH_SDK_PATH}/libs/FreeRTOS/portable/GCC/RISC-V/portASM.S
)

# utensil tools
target_include_directories(utensil INTERFACE
    ${WCH_SDK_PATH}/libs/utensil
)
target_link_libraries(utensil INTERFACE
    Cherry_USB
)

# wasm runtime
target_include_directories(wasm_runtime INTERFACE
)
#aux_source_directory()
target_sources(wasm_runtime INTERFACE
    ${WCH_SDK_PATH}/libs/wasm-micro-runtime/core/iwasm/common/arch/invokeNative_riscv.S
)

# tflite
target_include_directories(tflite INTERFACE
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/gemmlowp
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/flatbuffers/include
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/ruy
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/kissfft
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/signal/micro/kernels
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/signal/src
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/signal/src/kiss_fft_wrappers
)
file(GLOB_RECURSE tflite_sources 
"${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/*.c" 
"${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/*.cc"
"${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/*.cpp") 
# set(tflite_dir "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow/lite")
# set(signal_dir "${CMAKE_CURRENT_SOURCE_DIR}/signal")
# set(tfmicro_dir "${tflite_dir}/micro")
# set(tfmicro_frontend_dir "${tflite_dir}/experimental/microfrontend/lib")
# set(tfmicro_kernels_dir "${tfmicro_dir}/kernels")
# file(GLOB srcs_micro
#           "${tfmicro_dir}/*.cc"
#           "${tfmicro_dir}/*.c")
# file(GLOB src_micro_frontend
#           "${tfmicro_frontend_dir}/*.c"
#           "${tfmicro_frontend_dir}/*.cc")
# file(GLOB srcs_tflite_bridge
#           "${tfmicro_dir}/tflite_bridge/*.c"
#           "${tfmicro_dir}/tflite_bridge/*.cc")
# file(GLOB srcs_kernels
#           "${tfmicro_kernels_dir}/*.c"
#           "${tfmicro_kernels_dir}/*.cc")
# file(GLOB signal_micro_kernels
#           "${signal_dir}/micro/kernels/*.c"
#           "${signal_dir}/micro/kernels/*.cc")
# file(GLOB signal_src
#           "${signal_dir}/src/*.c"
#           "${signal_dir}/src/*.cc")
# set(signal_srcs
#           "${signal_micro_kernels}"
#           "${signal_src}"
#           "${signal_dir}/src/kiss_fft_wrappers/kiss_fft_float.cc"
#           "${signal_dir}/src/kiss_fft_wrappers/kiss_fft_int16.cc"
#           "${signal_dir}/src/kiss_fft_wrappers/kiss_fft_int32.cc")
list(REMOVE_ITEM tflite_sources
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/add.cc"
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/conv.cc"
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/depthwise_conv.cc"
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/fully_connected.cc"
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/mul.cc"
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/pooling.cc"
          "${WCH_SDK_PATH}/libs/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn/softmax.cc")
target_sources(tflite INTERFACE
    ${tflite_sources}
)

 
# chip 
if(NOT CHIP_NAME)
    message(FATAL_ERROR "please set CHIP_NAME for ch32")
elseif(CHIP_NAME STREQUAL "ch32v307")
    aux_source_directory(${WCH_SDK_PATH}/hal/CH32V307/src FILE_SRC)
    function(config_app param)
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
                -fno-rtti 
                -fno-exceptions
                -fno-threadsafe-statics
                -Werror 
                -Wno-return-type 
                -Wno-unused-function
                -Wno-volatile  
                -Wno-deprecated-declarations 
                -Wno-unused-variable
                -Wno-strict-aliasing>
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
            -T ${WCH_SDK_PATH}/hal/CH32V307/Link.ld
            -Wl,-Map,${param}.map 
        )

        set(HEX_FILE ${PROJECT_BINARY_DIR}/app.hex)
        set(BIN_FILE ${PROJECT_BINARY_DIR}/app.bin)
        add_custom_command(
                TARGET ${param} POST_BUILD
                COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${param}> ${HEX_FILE}
                COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${param}> ${BIN_FILE}
        )
        target_link_libraries(${param} PUBLIC
            freertos
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
#add_definitions(-DDEBUG=1)



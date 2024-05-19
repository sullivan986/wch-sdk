# freertos for ch32v307
add_library(freertos INTERFACE)
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



# cherry USB
add_library(Cherry_USB INTERFACE)


# wasm
add_library(wasm_runtime INTERFACE)
target_include_directories(wasm_runtime INTERFACE
)
target_sources(wasm_runtime INTERFACE
    ${WCH_SDK_PATH}/libs/wasm-micro-runtime/core/iwasm/common/arch/invokeNative_riscv.S
)


# tflite
add_library(tflite INTERFACE)
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

# utensil
add_library(utensil INTERFACE)
target_include_directories(utensil INTERFACE
    ${WCH_SDK_PATH}/libs/utensil
)
target_link_libraries(utensil INTERFACE
    Cherry_USB
    tflite
)


function(enable_rtos)
    if(CHIP_NAME STREQUAL "ch32v307")      
        add_custom_command(
            TARGET ${CMAKE_CURRENT_PROJECT_PARAM} PRE_BUILD
            COMMAND rm ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND cp ${WCH_SDK_PATH}/hal/CH32V307/LinkRtos.ld ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
        )
    elseif(CHIP_NAME STREQUAL "ch58x")
        # TODO
    endif()
    target_link_libraries(${CMAKE_CURRENT_PROJECT_PARAM} PUBLIC freertos)
endfunction()

function(enable_tflite)
    target_compile_options(${CMAKE_CURRENT_PROJECT_PARAM} PRIVATE
        -DTF_LITE_STATIC_MEMORY
        -DTF_LITE_DISABLE_X86_NEON
        -DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS
        -DTF_LITE_USE_GLOBAL_MAX
        -DTF_LITE_USE_GLOBAL_MIN
        -Wno-error=double-promotion
        -lprintfloat
    )
    target_link_libraries(${CMAKE_CURRENT_PROJECT_PARAM} PUBLIC tflite printfloat)
    #target_link_libraries(${CMAKE_CURRENT_PROJECT_PARAM} PRIVATE -lm)
    get_target_property(MyLib_COMPILE_OPTIONS ${CMAKE_CURRENT_PROJECT_PARAM} COMPILE_OPTIONS)
    if(MyLib_COMPILE_OPTIONS)
        string(REPLACE "-fsingle-precision-constant" "" MyLib_COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
        set_target_properties(${CMAKE_CURRENT_PROJECT_PARAM} PROPERTIES COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
    endif()
endfunction()
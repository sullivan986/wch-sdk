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
    #${WCH_SDK_PATH}/libs/esp-tflite-micro/
    ${WCH_SDK_PATH}/libs/tflite-micro/
    ${WCH_SDK_PATH}/libs/tflite-micro/signal/micro/kernels
    ${WCH_SDK_PATH}/libs/tflite-micro/signal/src
    ${WCH_SDK_PATH}/libs/tflite-micro/signal/src/kiss_fft_wrappers
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/gemmlowp
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/flatbuffers/include
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/ruy
    ${WCH_SDK_PATH}/libs/esp-tflite-micro/third_party/kissfft
    ${WCH_SDK_PATH}/libs/utensil/tflite
)
set(tflite_dir "${WCH_SDK_PATH}/libs/tflite-micro/tensorflow/lite")
set(signal_dir "${WCH_SDK_PATH}/libs/tflite-micro/signal")
file(GLOB tflite_all_srcs
    "${tflite_dir}/core/c/common.cc"
    "${tflite_dir}/core/api/*.cc"
    "${tflite_dir}/kernels/*.cc"
    "${tflite_dir}/kernels/*/*.cc"
    "${tflite_dir}/kernels/*/*/*.cc"
    "${tflite_dir}/micro/*.c"
    "${tflite_dir}/micro/*.cc"
    "${tflite_dir}/micro/tflite_bridge/*.c"
    "${tflite_dir}/micro/tflite_bridge/*.cc"
    "${tflite_dir}/micro/kernels/*.c"
    "${tflite_dir}/micro/kernels/*.cc"
    "${tflite_dir}/micro/memory_planner/*.cc"
    "${tflite_dir}/micro/arena_allocator/*.cc"
    "${tflite_dir}/schema/*.cc"
    
    "${signal_dir}/micro/kernels/*.c"
    "${signal_dir}/micro/kernels/*.cc"
    "${signal_dir}/src/*.c"
    "${signal_dir}/src/*.cc"
    "${signal_dir}/src/kiss_fft_wrappers/*.cc"
    "${signal_dir}/src/tensorflow_core/kernels/*.cc"
    "${signal_dir}/src/tensorflow_core/ops/*.cc"
)
file(GLOB will_remove_src 
    "${tflite_dir}/micro/*_test.cc"
    "${tflite_dir}/micro/kernels/*_test.cc"
    "${tflite_dir}/micro/memory_planner/*_test.cc"
    "${tflite_dir}/micro/arena_allocator/*_test.cc"
    "${signal_dir}/micro/kernels/*_test.cc"
)
list(REMOVE_ITEM tflite_all_srcs
            ${will_remove_src}
          )
target_sources(tflite INTERFACE
    ${tflite_all_srcs}
)

# utensil
add_library(utensil INTERFACE)
target_include_directories(utensil INTERFACE
    ${WCH_SDK_PATH}/libs/utensil
)
target_link_libraries(utensil INTERFACE
    Cherry_USB
)


function(enable_rtos STACK_size)
    if(CHIP_NAME STREQUAL "ch32v307")
        configure_file(
            ${WCH_SDK_PATH}/hal/CH32V307/FreeRTOSConfig.h.in
            ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h 
        )
        add_custom_command(
            TARGET ${CMAKE_CURRENT_PROJECT_PARAM} PRE_BUILD
            COMMAND rm ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND cp ${WCH_SDK_PATH}/hal/CH32V307/LinkRtos.ld ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND cp ${WCH_SDK_PATH}/hal/CH32V307/FreeRTOSConfig.h.in ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h
            COMMAND sed -i "s/@STACK_size@/${STACK_size}/g" ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h
        )
    elseif(CHIP_NAME STREQUAL "ch58x")
        # TODO
    endif()
    target_link_libraries(${CMAKE_CURRENT_PROJECT_PARAM} PUBLIC freertos)
endfunction()

function(enable_tflite)
    target_compile_options(${CMAKE_CURRENT_PROJECT_PARAM} PRIVATE
        #-DTF_LITE_STATIC_MEMORY
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
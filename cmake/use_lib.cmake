# freertos for ch32v307
add_library(freertos INTERFACE)
set(LIB_FREERTOS_PATH ${WCH_SDK_PATH}/libs/ch32v307/EVT/EXAM/FreeRTOS/FreeRTOS_Core/FreeRTOS)
aux_source_directory(${LIB_FREERTOS_PATH} lib_freertos_source)
target_include_directories(freertos INTERFACE
    ${LIB_FREERTOS_PATH}/include
    ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V
    #${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions
)
configure_file(
    ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions/freertos_risc_v_chip_specific_extensions.h
    ${CMAKE_BINARY_DIR}/tmp_file/freertos_risc_v_chip_specific_extensions.h
)
target_sources(freertos PUBLIC
    ${lib_freertos_source}
    #${WCH_SDK_PATH}/libs/FreeRTOS/portable/Common/mpu_wrappers.c
    ${LIB_FREERTOS_PATH}/portable/MemMang/heap_4.c 
    ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/port.c
    ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/portASM.S
)

# cherry USB
add_library(Cherry_USB INTERFACE)

# wasm has three rt: jit aot interpreter
add_library(wasm_runtime INTERFACE)
set(wamr_core_dir "${WCH_SDK_PATH}/libs/wasm-micro-runtime/core")
target_include_directories(wasm_runtime INTERFACE
    ${wamr_core_dir}
    ${wamr_core_dir}/iwasm/common
    ${wamr_core_dir}/iwasm/include
    ${wamr_core_dir}/iwasm/interpreter
    ${wamr_core_dir}/shared/mem-alloc
    ${wamr_core_dir}/shared/mem-alloc/ems
    ${wamr_core_dir}/shared/platform/include
    ${wamr_core_dir}/shared/utils
    ${WCH_SDK_PATH}/libs/utensil/wasm
)
file(GLOB wamr_all_srcs
    "${wamr_core_dir}/iwasm/common/*.c"
    "${wamr_core_dir}/iwasm/interpreter/*.c"
    "${wamr_core_dir}/iwasm/libraries/libc-builtin/*.c"
    "${wamr_core_dir}/shared/mem-alloc/*.c"
    "${wamr_core_dir}/shared/mem-alloc/ems/*.c"
    "${wamr_core_dir}/shared/utils/*.c"
    "${wamr_core_dir}/shared/platform/common/freertos/*.c"
    "${wamr_core_dir}/shared/platform/common/math/*.c"
    "${wamr_core_dir}/shared/platform/common/math/*.c"
    "${WCH_SDK_PATH}/libs/utensil/wasm/*.c"
)
target_sources(wasm_runtime INTERFACE
    ${wamr_all_srcs}
    ${wamr_core_dir}/iwasm/common/arch/invokeNative_riscv.S
    # ${wamr_core_dir}/iwasm/aot/arch/aot_reloc_riscv.c # disable aot, If we want to use aot on an embedded system, why don't we just upgrade the program?
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
set(tflite_signal_dir "${WCH_SDK_PATH}/libs/tflite-micro/signal")
 
file(GLOB will_remove_src 
    "${tflite_dir}/micro/*_test.cc"
    "${tflite_dir}/micro/kernels/*_test.cc"
    "${tflite_dir}/micro/memory_planner/*_test.cc"
    "${tflite_dir}/micro/arena_allocator/*_test.cc"
    "${tflite_signal_dir}/micro/kernels/*_test.cc"
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
        # configure_file(
        #     ${WCH_SDK_PATH}/hal/CH32V307/config/FreeRTOSConfig.h.in
        #     ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h 
        # )
        configure_file(
            ${LIB_FREERTOS_PATH}/../User/FreeRTOSConfig.h
            ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h
            COPYONLY
        )
        add_custom_command(
            TARGET ${CMAKE_CURRENT_PROJECT_PARAM} PRE_BUILD
            COMMAND rm ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND cp ${WCH_SDK_PATH}/hal/CH32V307/config/LinkRtos.ld ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND sed -i 's/define ARCH_FPU 0/define ARCH_FPU 1/g' ${CMAKE_BINARY_DIR}/tmp_file/freertos_risc_v_chip_specific_extensions.h
            # COMMAND cp ${WCH_SDK_PATH}/hal/CH32V307/config/FreeRTOSConfig.h.in ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h
           # COMMAND sed -i "s/@STACK_size@/${STACK_size}/g" ${CMAKE_BINARY_DIR}/tmp_file/FreeRTOSConfig.h
        )
    elseif(CHIP_NAME STREQUAL "ch59x")
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

# function(enable_wasm)
#     target_compile_options(${CMAKE_CURRENT_PROJECT_PARAM} PRIVATE
#         -DBH_MALLOC=wasm_runtime_malloc
#         -DBH_FREE=wasm_runtime_free
#         -DBUILD_TARGET_RISCV32_ILP32
#         -DWASM_ENABLE_GLOBAL_HEAP_POOL=1
#         -DWASM_GLOBAL_HEAP_SIZE=65536 # runtime stack size 64kb
#         -DWASM_ENABLE_INTERP=1
#         -DWASM_ENABLE_LIBC_BUILTIN=1
#         -DWAMR_BUILD_PLATFORM=WCH
#     )
#     # set (WAMR_BUILD_PLATFORM "wch")
#     # set (WAMR_BUILD_TARGET "RISCV32_ILP32")
#     # set (WAMR_BUILD_INTERP 1)
#     # set (WAMR_BUILD_LIBC_BUILTIN 1)
#     # set (WAMR_BUILD_LIBC_WASI 1)
#     # set (WAMR_BUILD_FAST_INTERP 0)
#     # set (WAMR_BUILD_GLOBAL_HEAP_POOL 1)
#     # set (WAMR_BUILD_GLOBAL_HEAP_SIZE 65536) # 64 KB
#     # set (WAMR_BUILD_AOT 0)
#     # set (WAMR_ROOT_DIR ${WCH_SDK_PATH}/libs/wasm-micro-runtime)
#     #set (SHARED_PLATFORM_CONFIG ${WCH_SDK_PATH}/libs/utensil/wasm/shared_platform.cmake)
#     #include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
#     #target_sources(${CMAKE_CURRENT_PROJECT_PARAM} PRIVATE ${WAMR_RUNTIME_LIB_SOURCE})
#     target_link_libraries(${CMAKE_CURRENT_PROJECT_PARAM} PUBLIC wasm_runtime)
# endfunction(enable_wasm)

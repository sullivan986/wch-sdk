# freertos for ch32v307
add_library(freertos INTERFACE)
set(LIB_FREERTOS_PATH ${WCH_SDK_PATH}/libs/ch32v307/EVT/EXAM/FreeRTOS/FreeRTOS_Core/FreeRTOS)
aux_source_directory(${LIB_FREERTOS_PATH} LIB_FREERTOS_SRC)
target_include_directories(freertos INTERFACE
    ${LIB_FREERTOS_PATH}/include
    ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V
    #${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions
)

aux_source_directory(${LIB_FREERTOS_POSIX_PATH}/FreeRTOS-Plus-POSIX/source LIB_FREERTOS_POSIX_SRC)
target_sources(freertos INTERFACE
    ${LIB_FREERTOS_SRC}
    ${LIB_FREERTOS_PATH}/portable/MemMang/heap_4.c 
    #${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/port.c
    ${WCH_SDK_PATH}/configure/ch32v307/port.c
    ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/portASM.S
)

# cherry USB
add_library(Cherry_USB INTERFACE)

# wasm has three rt: jit aot interpreter, interpreter has the smallest volume, so use it.
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
    #"${wamr_core_dir}/iwasm/interpreter/*.c"
    "${wamr_core_dir}/iwasm/interpreter/wasm_interp_classic.c"
    "${wamr_core_dir}/iwasm/interpreter/wasm_loader.c"
    "${wamr_core_dir}/iwasm/interpreter/wasm_runtime.c"
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
)
set(tflite_dir "${WCH_SDK_PATH}/libs/tflite-micro/tensorflow/lite")
set(tflite_signal_dir "${WCH_SDK_PATH}/libs/tflite-micro/signal")
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
 
"${tflite_signal_dir}/micro/kernels/*.c"
"${tflite_signal_dir}/micro/kernels/*.cc"
"${tflite_signal_dir}/src/*.c"
"${tflite_signal_dir}/src/*.cc"
"${tflite_signal_dir}/src/kiss_fft_wrappers/*.cc"
"${tflite_signal_dir}/src/tensorflow_core/kernels/*.cc"
"${tflite_signal_dir}/src/tensorflow_core/ops/*.cc"
)
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

#lvgl8
add_library(lvgl8 INTERFACE)
set(lvgl8_dir "${WCH_SDK_PATH}/libs/lvgl8")
file(GLOB lvgl8_all_src 
    "${lvgl8_dir}/demos/*.c"
    "${lvgl8_dir}/demos/*/*.c"
    "${lvgl8_dir}/demos/*/*/*.c"
    "${lvgl8_dir}/demos/*/*/*/*.c"
    "${lvgl8_dir}/src/*.c"
    "${lvgl8_dir}/src/*/*.c"
    "${lvgl8_dir}/src/*/*/*.c"
    "${lvgl8_dir}/src/*/*/*/*.c"
)
target_include_directories(lvgl8 INTERFACE
    ${lvgl8_dir}
    ${lvgl8_dir}/demos
)
target_sources(lvgl8 INTERFACE
    ${lvgl8_all_src}
)

#lvgl9
add_library(lvgl9 INTERFACE)
set(lvgl9_dir "${WCH_SDK_PATH}/libs/lvgl9")
file(GLOB lvgl9_all_src 
    "${lvgl9_dir}/demos/*.c"
    "${lvgl9_dir}/demos/*/*.c"
    "${lvgl9_dir}/demos/*/*/*.c"
    "${lvgl9_dir}/src/*.c"
    "${lvgl9_dir}/src/*/*.c"
    "${lvgl9_dir}/src/*/*/*.c"
    "${lvgl9_dir}/src/*/*/*/*.c"
)
target_include_directories(lvgl9 INTERFACE
    ${lvgl9_dir}
    ${lvgl9_dir}/demos
)
target_sources(lvgl9 INTERFACE
    ${lvgl9_all_src}
)


# utensil
add_library(utensil INTERFACE)
target_include_directories(utensil INTERFACE
    ${WCH_SDK_PATH}/libs/utensil
)
target_link_libraries(utensil INTERFACE
    Cherry_USB
)


function(enable_rtos app_name chip_name)
    if(${chip_name} STREQUAL "ch32v307")
        configure_file(
            ${LIB_FREERTOS_PATH}/../User/FreeRTOSConfig.h
            ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/FreeRTOSConfig.h
            COPYONLY
        )
        configure_file(
            ${LIB_FREERTOS_PATH}/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions/freertos_risc_v_chip_specific_extensions.h
            ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/freertos_risc_v_chip_specific_extensions.h
            COPYONLY
        )
        add_custom_command(
            TARGET ${app_name} PRE_BUILD
            COMMAND rm -f ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
            COMMAND rm -f ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND cp ${WCH_SDK_PATH}/configure/ch32v307/LinkRtos.ld ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/Link.ld
            COMMAND cp ${LIB_FREERTOS_PATH}/../Startup/startup_ch32v30x_D8C.S ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/startup_ch32v30x_D8C.S
            COMMAND sed -i 's/define ARCH_FPU 0/define ARCH_FPU 1/g' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/freertos_risc_v_chip_specific_extensions.h
            # COMMAND sed -i 's/12/20/g' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/FreeRTOSConfig.h
            # COMMAND sed -i '110c \ \ \  la t0, main' ${CMAKE_BINARY_DIR}/tmp_file/${chip_name}/FreeRTOSConfig.h
        ) 
    elseif(${chip_name} STREQUAL "ch59x")
        # TODO

    else()
        message(FATAL_ERROR "${chip_name} is not support freertos!")
    endif()
    target_link_libraries(${app_name} PRIVATE freertos)
    target_compile_definitions(${app_name} PRIVATE
        WCH_USE_LIB_FREERTOS
    )
endfunction()


function(enable_tflite app_name)
    target_compile_definitions(${app_name} PRIVATE
        TF_LITE_DISABLE_X86_NEON
        TF_LITE_USE_GLOBAL_CMATH_FUNCTIONS
        TF_LITE_USE_GLOBAL_MAX
        TF_LITE_USE_GLOBAL_MIN
    )
    target_compile_options(${app_name} PRIVATE
        -Wno-error=double-promotion
    )
    get_target_property(MyLib_COMPILE_OPTIONS ${app_name} COMPILE_OPTIONS)
    if(MyLib_COMPILE_OPTIONS)
        string(REPLACE "-fsingle-precision-constant" "" MyLib_COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
        set_target_properties(${app_name} PROPERTIES COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
    endif()
    target_link_libraries(${app_name} PRIVATE tflite printfloat)
    target_compile_definitions(${app_name} PRIVATE
        WCH_USE_LIB_TFLITE
    )
endfunction()

function(enable_wasm app_name)
    target_compile_definitions(${app_name} PRIVATE
        BH_MALLOC=wasm_runtime_malloc
        BH_FREE=wasm_runtime_free
        BUILD_TARGET_RISCV32_ILP32
        WASM_ENABLE_GLOBAL_HEAP_POOL=1
        WASM_GLOBAL_HEAP_SIZE=65536 # runtime stack size 64kb
        WASM_ENABLE_INTERP=1
        WASM_ENABLE_LIBC_BUILTIN=1
        WAMR_BUILD_PLATFORM=WCH
    )
    # set (WAMR_BUILD_PLATFORM "wch")
    # set (WAMR_BUILD_TARGET "RISCV32_ILP32")
    # set (WAMR_BUILD_INTERP 1)
    # set (WAMR_BUILD_LIBC_BUILTIN 1)
    # set (WAMR_BUILD_LIBC_WASI 1)
    # set (WAMR_BUILD_FAST_INTERP 0)
    # set (WAMR_BUILD_GLOBAL_HEAP_POOL 1)
    # set (WAMR_BUILD_GLOBAL_HEAP_SIZE 65536) # 64 KB
    # set (WAMR_BUILD_AOT 0)
    # set (WAMR_ROOT_DIR ${WCH_SDK_PATH}/libs/wasm-micro-runtime)
    # set (SHARED_PLATFORM_CONFIG ${WCH_SDK_PATH}/libs/utensil/wasm/shared_platform.cmake)
    # include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
    # target_sources(${app_name} PRIVATE ${WAMR_RUNTIME_LIB_SOURCE})
    target_link_libraries(${app_name} PRIVATE wasm_runtime)
    target_compile_definitions(${app_name} PRIVATE
        WCH_USE_LIB_WASM
    )
endfunction(enable_wasm)


function(enable_lvgl8 app_name)
    target_compile_definitions(${app_name} PRIVATE
        LV_TICK_PERIOD_MS=1
        LV_LVGL_H_INCLUDE_SIMPLE
    )
    target_link_libraries(${app_name} PRIVATE lvgl8)
    target_compile_definitions(${app_name} PRIVATE
        WCH_USE_LIB_LVGL8
    )
endfunction(enable_lvgl8)

function(enable_lvgl9 app_name)
    target_compile_definitions(${app_name} PRIVATE
        LV_TICK_PERIOD_MS=1
        LV_LVGL_H_INCLUDE_SIMPLE
    )
    target_link_libraries(${app_name} PRIVATE lvgl9)
    target_compile_definitions(${app_name} PRIVATE
        WCH_USE_LIB_LVGL9
    )
endfunction(enable_lvgl9)

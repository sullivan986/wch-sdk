cmake_minimum_required(VERSION 3.20)

# 工具链设置
set(TOOLPATH  /PATH/TO/GCC/BIN)
set(WCH_SDK_PATH ../../../..)
set(CHIP_NAME "ch32v307")

# 项目设置
project(ch32v307 C CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

include(${WCH_SDK_PATH}/wch_init.cmake)

config_app(ch32v307_tflite_hello_world)

target_compile_options(ch32v307_tflite_hello_world PRIVATE
-DTF_LITE_STATIC_MEMORY
-DTF_LITE_DISABLE_X86_NEON
-DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS
-DTF_LITE_USE_GLOBAL_MAX
-DTF_LITE_USE_GLOBAL_MIN
-Wno-error=double-promotion
-lprintfloat
)


target_link_libraries(ch32v307_tflite_hello_world PUBLIC
    tflite
    printfloat
)

target_link_libraries(ch32v307_tflite_hello_world PRIVATE -lm)

get_target_property(MyLib_COMPILE_OPTIONS ch32v307_tflite_hello_world COMPILE_OPTIONS)
if(MyLib_COMPILE_OPTIONS)
  string(REPLACE "-fsingle-precision-constant" "" MyLib_COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
  #string(REPLACE "-mtune=native" "" MyLib_COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
  set_target_properties(ch32v307_tflite_hello_world PROPERTIES COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
endif()
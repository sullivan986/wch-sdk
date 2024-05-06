cmake_minimum_required(VERSION 3.20)

# 工具链设置
set(TOOLPATH  /PATH/TO/GCC/BIN)
set(WCH_SDK_PATH ../..)
set(CHIP_NAME "ch32v307")

# 项目设置
project(ch32v307 C CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

include(${WCH_SDK_PATH}/wch_init.cmake)

config_app(ch32v307_tflite)

target_compile_options(ch32v307_tflite PRIVATE
-DTF_LITE_STATIC_MEMORY
-DTF_LITE_DISABLE_X86_NEON
-DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS
-DTF_LITE_USE_GLOBAL_MAX
-DTF_LITE_USE_GLOBAL_MIN

# -Wno-error=attributes
# -Wno-error=shadow
# -Wno-maybe-uninitialized
# #-Wno-missing-field-initializers
# -Wno-error=sign-compare
-Wno-error=double-promotion
# -Wno-type-limits
# -Wno-error=stringop-overread

# -O3
# -Wstrict-aliasing -Wno-unused-parameter -Wall -Wextra -Wvla
# -Wsign-compare -Wdouble-promotion -Wswitch -Wunused-function
# -Wshadow -Wunused-variable -fno-unwind-tables

# -fmessage-length=0 -Wno-nonnull

)

target_compile_options(test2 PRIVATE $<$<COMPILE_LANGUAGE:CXX>:
                                      ${common_flags} -fno-rtti -fno-exceptions
                                      -fno-threadsafe-statics -Werror -Wno-return-type 
                                      -Wno-volatile  -Wno-deprecated-declarations
                                      -Wno-strict-aliasing>)

target_link_libraries(ch32v307_tflite PUBLIC
    tflite
    printfloat
)

target_link_libraries(ch32v307_tflite PRIVATE -lm)

get_target_property(MyLib_COMPILE_OPTIONS ch32v307_tflite COMPILE_OPTIONS)
if(MyLib_COMPILE_OPTIONS)
  string(REPLACE "-fsingle-precision-constant" "" MyLib_COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
  #string(REPLACE "-mtune=native" "" MyLib_COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
  set_target_properties(ch32v307_tflite PROPERTIES COMPILE_OPTIONS "${MyLib_COMPILE_OPTIONS}")
endif()
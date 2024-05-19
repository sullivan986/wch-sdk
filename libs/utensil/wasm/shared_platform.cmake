set (PLATFORM_SHARED_DIR ${CMAKE_CURRENT_LIST_DIR})

add_definitions(-DBH_PLATFORM_WCH)

include_directories(${PLATFORM_SHARED_DIR})
include_directories(${wamr_core_dir}/shared/platform/include)

if(${CONFIG_MINIMAL_LIBC})
    include (${wamr_core_dir}/shared/platform/common/freertos/platform_api_freertos.cmake)
endif()

file (GLOB_RECURSE source_all ${PLATFORM_SHARED_DIR}/*.c)

set (PLATFORM_SHARED_SOURCE ${source_all} ${PLATFORM_COMMON_FREERTOS_SOURCE})


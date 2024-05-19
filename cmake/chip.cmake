if(CHIP_NAME STREQUAL "ch32v307")
    
elseif(CHIP_NAME STREQUAL "ch58x")
elseif(CHIP_NAME STREQUAL "ch59x")
endif()


function(set_ram_and_flash ram flash)
    if(CHIP_NAME STREQUAL "ch32v307")
        add_custom_command(
            TARGET ${CMAKE_CURRENT_PROJECT_PARAM} PRE_BUILD
            COMMAND sed -i '26c 	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = ${flash}K' ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
            COMMAND sed -i '27c 	RAM (xrw) : ORIGIN = 0x20000000, LENGTH = ${ram}K' ${CMAKE_BINARY_DIR}/tmp_file/Link.ld
        )
    elseif(CHIP_NAME STREQUAL "ch58x")
    # TODO
    endif()
endfunction()
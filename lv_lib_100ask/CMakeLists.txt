if(ESP_PLATFORM)

    file(GLOB_RECURSE SOURCES src/*.c)

    idf_build_get_property(LV_MICROPYTHON LV_MICROPYTHON)

    if (LV_MICROPYTHON)
        idf_component_register(SRCS ${SOURCES}
                               INCLUDE_DIRS . src
                               REQUIRES lv_lib_100ask)
    else()
        idf_component_register(SRCS ${SOURCES}
                               INCLUDE_DIRS . src
                               REQUIRES lv_lib_100ask)
    endif()

    target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LIB_100ASK_ESP_PLATFORM")

else()

    file(GLOB_RECURSE SOURCES src/*.c)
    add_library(lv_lib_100ask STATIC ${SOURCES})
    target_link_libraries(lv_lib_100ask PUBLIC lvgl)

endif()
if(ESP_PLATFORM)

    file(GLOB_RECURSE SOURCES src/*.c test/*.c)

    idf_build_get_property(LV_MICROPYTHON LV_MICROPYTHON)

    if (LV_MICROPYTHON)
        idf_component_register(SRCS ${SOURCES}
                               INCLUDE_DIRS . src test ../
                               REQUIRES lv_lib_100ask)
    else()
        idf_component_register(SRCS ${SOURCES}
                               INCLUDE_DIRS . src test
                               REQUIRES lv_lib_100ask)
    endif()

    target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LIB_100ASK_ESP_PLATFORM")

else()

    file(GLOB_RECURSE SOURCES src/*.c test/*.c)
    add_library(lv_lib_100ask STATIC ${SOURCES})

    # Add LVGL include directory
    target_include_directories(lv_lib_100ask PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

    # Use simple include for lvgl.h
    target_compile_definitions(lv_lib_100ask PRIVATE LV_LVGL_H_INCLUDE_SIMPLE=1)

    # Enable 2048 game
    target_compile_definitions(lv_lib_100ask PRIVATE LV_USE_100ASK_2048=1)

endif()


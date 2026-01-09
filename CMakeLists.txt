cmake_minimum_required(VERSION 3.10)

# Project configuration
project(v833_lv9_demos C CXX)
set(PROJECT_VERSION "1.0.0")

# CMake policies
cmake_policy(SET CMP0079 NEW)

# Build configuration
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

# Cross-compilation toolchain paths (for ARM musl toolchain)
set(CROSS_COMPILE_INCLUDE_DIRS
    /srv/alsa/include
    /srv/ffmpeg/include
    /srv/zlib-armv7/include
)

set(CROSS_COMPILE_LIBRARY_DIRS
    /srv/evdev/lib
    /srv/openssl/lib
    /srv/zlib/lib
    /srv/ffmpeg/lib
    /srv/alsa/lib
)

# Add cross-compile include directories
foreach(DIR ${CROSS_COMPILE_INCLUDE_DIRS})
    include_directories(${DIR})
endforeach()

# LVGL configuration
set(LV_BUILD_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "" FORCE)
set(LV_BUILD_SET_CONFIG_OPTS ON CACHE BOOL
    "create CMAKE variables from lv_conf_internal.h" FORCE)

# Add LVGL subdirectory
add_subdirectory(lvgl)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(EVDEV REQUIRED libevdev)

# Project include directories
set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/virsual_novel
)

# Project source files
set(PROJECT_SOURCES
    # Main application
    src/main.c

    # Core library modules
    src/lib/audio.c
    src/lib/button.c
    src/lib/container.c
    src/lib/events.c
    src/lib/file_manager.c
    src/lib/player.c
    src/lib/settings.c

    # Visual Novel Engine
    src/lib/virsual_novel/cJSON.c
    src/lib/virsual_novel/data_parser.c
    src/lib/virsual_novel/resource_manager.c
    src/lib/virsual_novel/simple_json.c
    src/lib/virsual_novel/visual_novel_engine.c
)

# Project header files (for installation)
set(PROJECT_HEADERS
    src/main.h
    src/lib/audio.h
    src/lib/button.h
    src/lib/container.h
    src/lib/events.h
    src/lib/file_manager.h
    src/lib/player.h
    src/lib/settings.h
    src/lib/virsual_novel/cJSON.h
    src/lib/virsual_novel/data_parser.h
    src/lib/virsual_novel/resource_manager.h
    src/lib/virsual_novel/simple_json.h
    src/lib/virsual_novel/visual_novel_engine.h
)

# External libraries
set(EXTERNAL_LIBRARIES
    ssl
    crypto
    avcodec
    avutil
    avformat
    swscale
    swresample
    avdevice
    m
    z
    pthread
    asound
    evdev
)

# Create lvgl_linux static library
add_library(lvgl_linux STATIC ${PROJECT_SOURCES})

# Set library properties
set_target_properties(lvgl_linux PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${PROJECT_HEADERS}"
)

# Add include directories for lvgl_linux
target_include_directories(lvgl_linux PUBLIC
    ${PROJECT_INCLUDE_DIRS}
    ${EVDEV_INCLUDE_DIRS}
)

# Set compile definitions
set_target_properties(lvgl_linux PROPERTIES
    COMPILE_DEFINITIONS "${LVGL_COMPILER_DEFINITIONS}"
)

# Add library directories for linking
target_link_directories(lvgl_linux PUBLIC
    ${CROSS_COMPILE_LIBRARY_DIRS}
)

# Create main executable
add_executable(lvglsim src/main.c)

# Set executable properties
set_target_properties(lvgl_linux PROPERTIES
    LINKER_LANGUAGE C
)

# Add include directories for executable
target_include_directories(lvglsim PRIVATE
    ${PROJECT_INCLUDE_DIRS}
    ${CROSS_COMPILE_INCLUDE_DIRS}
)

# Add library directories for linking
target_link_directories(lvglsim PUBLIC
    ${CROSS_COMPILE_LIBRARY_DIRS}
)

# Link libraries
target_link_libraries(lvglsim
    lvgl_linux
    lvgl
    lvgl_linux
    ${EXTERNAL_LIBRARIES}
)

# Linker flags for executable
target_link_options(lvglsim PRIVATE
    -static-libgcc
    -static-libstdc++
)

# Installation rules
install(TARGETS lvgl_linux lvglsim
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/v833_lv9_demos
)

# Custom targets
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lvglsim
    DEPENDS lvglsim
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running lvglsim"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMENT "Cleaning all build artifacts"
)

# Print configuration summary
message(STATUS "")
message(STATUS "==========================================")
message(STATUS "  V833 LV9 Demos Configuration Summary")
message(STATUS "==========================================")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Standard:     ${CMAKE_C_STANDARD}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:       ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "==========================================")
message(STATUS "")
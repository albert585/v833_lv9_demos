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

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Force -O2 optimization for all build types
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -g -O2" CACHE STRING "C compiler flags" FORCE)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -O2" CACHE STRING "C++ compiler flags" FORCE)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Linker flags (LDFLAGS)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Cross-compilation toolchain paths (for ARM musl toolchain)
set(CROSS_COMPILE_INCLUDE_DIRS /srv/alsa/include /srv/ffmpeg/include /srv/zlib-armv7/include)

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

# Add lv_lib_100ask subdirectory
add_subdirectory(src/lib/lv_lib_100ask)

# Project include directories
set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/virsual_novel
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/lv_lib_100ask/

)

# Project source files (auto-discover all .c files in src/ except main.c)
file(GLOB PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/virsual_novel/*.c    )

# Project header files (auto-discover all .h files in src/)
file(GLOB PROJECT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/virsual_novel/*.h     )

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
)

# Set compile definitions
set_target_properties(lvgl_linux PROPERTIES
    COMPILE_DEFINITIONS "${LVGL_COMPILER_DEFINITIONS}"
)

# Create main executable
add_executable(lvglsim src/main.c)

# Strip debug symbols from lvglsim after building
add_custom_command(TARGET lvglsim POST_BUILD
    COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:lvglsim>
    COMMENT "Stripping debug symbols from lvglsim"
)

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
# Link libraries using traditional linker flags
target_link_libraries(lvglsim lvgl_linux lvgl lv_lib_100ask -L/srv/evdev/lib -L/srv/openssl/lib -L/srv/zlib/lib -L/srv/ffmpeg/lib -L/srv/alsa/lib -Wl,-Bstatic -levdev -Wl,-Bdynamic -lssl -lcrypto -lavcodec -lavutil -lavformat -lswscale -lswresample -lavdevice -lasound -lz -lm -lpthread -ldl)

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

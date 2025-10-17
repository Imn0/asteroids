cmake_minimum_required(VERSION 3.10)
project(asteroids C)

set(CMAKE_C_STANDARD 11)

if(NOT MSVC)
    add_compile_options(-O2 -Wall -Wextra -Wpedantic -g -Wno-strict-prototypes -Wno-gnu-statement-expression)
endif()

include_directories(src/headers src/common)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL")
set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/SDL_image")
set(SDL2_MIXER_DIR "${CMAKE_SOURCE_DIR}/SDL_mixer")
set(SDL2_TTF_DIR "${CMAKE_SOURCE_DIR}/SDL_ttf")

# Set include directories for bundled SDL
set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/include")
set(SDL2_IMAGE_INCLUDE_DIRS "${SDL2_IMAGE_DIR}/include")
set(SDL2_MIXER_INCLUDE_DIRS "${SDL2_MIXER_DIR}/include")
set(SDL2_TTF_INCLUDE_DIRS "${SDL2_TTF_DIR}/include")

# Build bundled SDL libraries statically
add_subdirectory(${SDL2_DIR} EXCLUDE_FROM_ALL)
add_subdirectory(${SDL2_IMAGE_DIR} EXCLUDE_FROM_ALL)
add_subdirectory(${SDL2_MIXER_DIR} EXCLUDE_FROM_ALL)
add_subdirectory(${SDL2_TTF_DIR} EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SRC
    "src/*.c"
    "src/**/*.c"
    "src/**/**/*.c"
    "src/**/**/**/*.c"
)
add_executable(asteroids ${SRC})

target_include_directories(asteroids PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

if(MSVC)
    set(CMAKE_C_FLAGS "/O2 /experimental:c11atomics /Tc" CACHE INTERNAL "")
    target_link_libraries(asteroids PRIVATE ws2_32)
    # Link against static runtime on Windows
    set_property(TARGET asteroids PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(asteroids PRIVATE
    SDL2::SDL2-static
    SDL2::SDL2main
    SDL2_image::SDL2_image-static
    SDL2_mixer::SDL2_mixer-static
    SDL2_ttf::SDL2_ttf-static
)

if(UNIX AND NOT APPLE)
    target_link_libraries(asteroids PRIVATE m)
endif()

set_target_properties(asteroids PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(
    TARGET asteroids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
    COMMENT "Creating symbolic link to assets directory"
    # Only execute if the symlink does not already exist
    BYPRODUCTS ${CMAKE_BINARY_DIR}/assets
)

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/asteroids
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    DEPENDS asteroids
)
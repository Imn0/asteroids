cmake_minimum_required(VERSION 3.10)
project(asteroids C)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 11)

add_compile_options(-O2 -Wall -Wextra -Wpedantic -g)

include_directories(src/headers src/common)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Source files
file(GLOB_RECURSE SRC
    "src/*.c"
    "src/**/*.c"
    "src/**/**/*.c"
    "src/**/**/**/*.c"
)
add_executable(asteroids ${SRC})


target_include_directories(asteroids
    PRIVATE ${SDL2_INCLUDE_DIRS}
    PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS}
    PRIVATE ${SDL2_TTF_INCLUDE_DIRS}
    PRIVATE ${SDL2_MIXER_INCLUDE_DIRS}
)

if(MSVC)
    target_link_libraries(asteroids PRIVATE m SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
elseif(WIN32)
    target_link_libraries(asteroids PRIVATE m ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer mingw32)
else()
    target_link_libraries(asteroids PRIVATE m ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
endif()

if(WIN32)
    add_custom_command(TARGET asteroids POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE:SDL2_image::SDL2_image>
        $<TARGET_FILE:SDL2_ttf::SDL2_ttf>
        $<TARGET_FILE:SDL2_mixer::SDL2_mixer>
        $<TARGET_FILE_DIR:base>
    )
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
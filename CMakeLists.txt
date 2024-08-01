cmake_minimum_required(VERSION 3.10)
project(asteroids C)

set(CMAKE_C_STANDARD 11)

if(NOT MSVC)
    add_compile_options(-O2 -Wall -Wextra -Wpedantic -g -Wno-strict-prototypes)
endif()

include_directories(src/headers src/common)

if(NOT MSVC)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
endif()

file(GLOB_RECURSE SRC
    "src/*.c"
    "src/**/*.c"
    "src/**/**/*.c"
    "src/**/**/**/*.c"
)
add_executable(asteroids ${SRC})

if(MSVC)
set(CMAKE_C_FLAGS "/O2 /experimental:c11atomics /Tc" CACHE INTERNAL "")

    set(SDL2_INCLUDE_DIRS "C:/VC_lib/SDL2-2.30.5/include" CACHE PATH "Path to SDL2 include directory")
    set(SDL2_LIBRARIES "C:/VC_lib/SDL2-2.30.5/lib/x64" CACHE PATH "Path to SDL2 library directory")
    set(SDL2_IMAGE_INCLUDE_DIRS "C:/VC_lib/SDL2_image-2.8.2/include" CACHE PATH "Path to SDL2_image include directory")
    set(SDL2_IMAGE_LIBRARIES "C:/VC_lib/SDL2_image-2.8.2/lib/x64" CACHE PATH "Path to SDL2_image library directory")
    set(SDL2_TTF_INCLUDE_DIRS "C:/VC_lib/SDL2_ttf-2.22.0/include" CACHE PATH "Path to SDL2_ttf include directory")
    set(SDL2_TTF_LIBRARIES "C:/VC_lib/SDL2_ttf-2.22.0/lib/x64" CACHE PATH "Path to SDL2_ttf library directory")
    set(SDL2_MIXER_INCLUDE_DIRS "C:/VC_lib/SDL2_mixer-2.8.0/include" CACHE PATH "Path to SDL2_mixer include directory")
    set(SDL2_MIXER_LIBRARIES "C:/VC_lib/SDL2_mixer-2.8.0/lib/x64" CACHE PATH "Path to SDL2_mixer library directory")

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
    include_directories(${SDL2_TTF_INCLUDE_DIRS})
    include_directories(${SDL2_MIXER_INCLUDE_DIRS})

    target_link_libraries(asteroids PRIVATE 
        ws2_32
        ${SDL2_LIBRARIES}/SDL2.lib
        ${SDL2_LIBRARIES}/SDL2main.lib
        ${SDL2_IMAGE_LIBRARIES}/SDL2_image.lib
        ${SDL2_TTF_LIBRARIES}/SDL2_ttf.lib
        ${SDL2_MIXER_LIBRARIES}/SDL2_mixer.lib
    )
else()
    target_include_directories(asteroids
        PRIVATE ${SDL2_INCLUDE_DIRS}
        PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS}
        PRIVATE ${SDL2_TTF_INCLUDE_DIRS}
        PRIVATE ${SDL2_MIXER_INCLUDE_DIRS}
    )

    target_link_libraries(asteroids PRIVATE m ${SDL2_LIBRARIES} SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
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

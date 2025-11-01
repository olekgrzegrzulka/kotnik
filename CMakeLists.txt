cmake_minimum_required(VERSION 3.5...3.31)

project(kotnik)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror=shadow -Wuninitialized -Werror=switch -Werror=return-type ${MY_FLAGS} -O3")
SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OUTPUT_NAME "${PROJECT_NAME}")
set(BUILD_SHARED_LIBS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(lib/freetype)
add_subdirectory(lib/glfw)

set(SOURCES
    ./src/main.cpp
    ./src/world.cpp
    ./src/chunk.cpp
    ./src/glad.c
    ./src/input.cpp
    ./src/chunk_mesh.cpp
    ./src/chunk_worker.cpp
    ./src/world_gen.cpp
    ./src/biome.cpp
    ./src/cubes.cpp
    ./src/biome_map.cpp
    ./src/world_renderer.cpp
    ./src/skybox.cpp
    ./src/clouds.cpp
    ./src/world_lighter.cpp
    ./src/texture.cpp
    ./src/shader.cpp
    ./src/water_overlay.cpp
    ./src/framebuffer.cpp

    ./src/ui/button.cpp
    ./src/ui/font_face.cpp
    ./src/ui/label.cpp
    ./src/ui/slider.cpp
    ./src/ui/sprite.cpp
    ./src/ui/text_input.cpp
    ./src/ui/texture_atlas.cpp
    ./src/ui/ui.cpp
    ./src/ui/widget.cpp
    ./src/ui/pause_menu.cpp
    ./src/ui/settings_menu.cpp
)

add_executable(${OUTPUT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

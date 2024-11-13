cmake_minimum_required(VERSION 3.5)

project(kotnik)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror=shadow -Werror=switch -Werror=return-type ${MY_FLAGS} -O3")
set(CMAKE_EXE_LINKER_FLAGS "-lglfw")
SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OUTPUT_NAME "${PROJECT_NAME}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

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
)

add_executable(${OUTPUT_NAME} ${SOURCES})

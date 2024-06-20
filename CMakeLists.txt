cmake_minimum_required(VERSION 3.5)

project (cube_game)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror=shadow -Werror=switch ${MY_FLAGS} -Werror=return-type -O3 -o cube_game.out" )
set(CMAKE_EXE_LINKER_FLAGS "-lglfw")
SET (CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OUTPUT_NAME "${PROJECT_NAME}.out")


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(SOURCES
    ./src/main.cpp
    ./src/world.cpp
    ./src/chunk.cpp
    ./src/glad.c
    ./src/input.cpp
    ./src/chunk_renderer.cpp
    ./src/chunk_worker.cpp
    ./src/biome_map.cpp
    ./src/world_gen.cpp
    ./src/biome.cpp
)

add_executable(${OUTPUT_NAME} ${SOURCES})

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror=shadow ${MY_FLAGS} -Werror=return-type -O3" )
set(CMAKE_EXE_LINKER_FLAGS "-lglfw")
SET (CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (cube_game)

# Needed so that cmake uses our find modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# include_directories(src)


set(SOURCES
    ./src/main.cpp
    ./src/world.cpp
    ./src/chunk.cpp
    ./src/glad.c
    ./src/input.cpp
    ./src/chunk_renderer2.cpp
    ./src/chunk_worker.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
# target_link_libraries(-lglfw)

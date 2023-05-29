cmake_minimum_required(VERSION 3.1)
project(Todalu)

file(GLOB SRC_FILES src/*.cpp)
file(GLOB HEADER_FILES src/include/*.h)
set(LIB_FILES lib/linenoise/linenoise.c)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Clang format
add_custom_target(format COMMAND clang-format 
  -style=Google -i ${SRC_FILES} ${HEADER_FILES})

include_directories(src/include lib/linenoise)

add_executable(todalu ${SRC_FILES} ${LIB_FILES})

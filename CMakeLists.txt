cmake_minimum_required(VERSION 3.1)
project(Todalu)

include_directories(/usr/include/readline src/include)
file(GLOB SRC_FILES src/*.cpp)
file(GLOB HEADER_FILES src/include/*.h)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Clang format
add_custom_target(format COMMAND clang-format 
  -style=Google -i ${SRC_FILES} ${HEADER_FILES})

add_executable(todalu ${SRC_FILES})
target_link_libraries(todalu readline)

cmake_minimum_required(VERSION 3.1)
project(Todalu)

file(GLOB SRC_FILES src/*.cpp)
file(GLOB HEADER_FILES src/include/*.h)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
# LLVM flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/lib/llvm-14/include -std=c++14 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")

# Clang format
add_custom_target(format COMMAND clang-format 
-style=Google -i ${SRC_FILES} ${HEADER_FILES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/granthalaya.tdl ${CMAKE_BINARY_DIR}/granthalaya.tdl COPYONLY)
add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/granthalaya.h
COMMAND xxd ARGS -i granthalaya.tdl granthalaya.h
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/granthalaya.tdl
VERBATIM
)

set_property(SOURCE src/repl.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/granthalaya.h)
include_directories(/usr/include/readline src/include ${CMAKE_CURRENT_BINARY_DIR})

add_executable(todalu ${SRC_FILES})
target_link_libraries(todalu readline LLVM-14)

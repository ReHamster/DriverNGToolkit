cmake_minimum_required(VERSION 3.1.6)

project(cmdlib)

# cmd lib
add_library(
    cmdlib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cmdlib.cpp
)

target_include_directories(cmdlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
cmake_minimum_required(VERSION 3.16)
project(cereal CXX)
set(CMAKE_CXX_STANDARD 20)

# Root Project
add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

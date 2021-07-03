cmake_minimum_required(VERSION 3.1.6)

project(luafilesystem )

# luafilesystem lib
add_library(
    luafilesystem STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lfs.c
)

target_include_directories(luafilesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lua)
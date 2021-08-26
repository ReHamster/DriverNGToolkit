cmake_minimum_required(VERSION 3.16)
project(DriverNG)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE HITMAN_BM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.asm)

add_library(DriverNG STATIC ${HITMAN_BM_SOURCES})

target_include_directories(DriverNG PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(DriverNG HF::Core DriverNGHook::IClient ImGui::Core ImGui::DX9 cereal cmdlib lua sol2::sol2_single luafilesystem dinput8 dxguid)
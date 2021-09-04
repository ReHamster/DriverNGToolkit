cmake_minimum_required(VERSION 3.16)
project(DriverNGHook)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_CRT_SECURE_NO_WARNINGS=1) #MSVS Stuff

add_subdirectory(Modules/cmdlib)
add_subdirectory(Modules/ImGui)
add_subdirectory(Modules/HF)
add_subdirectory(Modules/gtest)
add_subdirectory(Modules/lua)
add_subdirectory(Modules/luafilesystem)
add_subdirectory(Modules/sol2)

add_subdirectory(DriverNG)
add_subdirectory(Client)

file(GLOB_RECURSE REHITMAN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DLL/source/*.cpp)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(DriverNGHook SHARED ${REHITMAN_SOURCES})
target_include_directories(DriverNGHook PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/DLL/include)

target_link_libraries(DriverNGHook PUBLIC DriverNGHook::IClient cmdlib)
target_link_libraries(DriverNGHook PRIVATE DriverNG ImGui::Core ImGui::DX9)

target_compile_definitions(DriverNGHook PRIVATE)
set_target_properties(DriverNGHook PROPERTIES SUFFIX ".asi")
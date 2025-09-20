cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(nexus_keyboard_overlay)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif()

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(nexus_keyboard_overlay SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/utils.cpp include/UiKey.hpp)
target_link_libraries(nexus_keyboard_overlay PRIVATE imgui ntdll comdlg32)
set_property(TARGET nexus_keyboard_overlay PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

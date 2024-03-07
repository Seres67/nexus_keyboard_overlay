cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# cross compilers to use for C, C++ and Fortran
set(CMAKE_C_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_Fortran_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-gfortran)
set(CMAKE_RC_COMPILER /usr/bin/${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")

project(nexus_keyboard_overlay)

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(nexus_keyboard_overlay SHARED src/Settings.cpp src/Shared.cpp src/entry.cpp src/Key.cpp src/utils.cpp)
target_link_libraries(nexus_keyboard_overlay imgui)

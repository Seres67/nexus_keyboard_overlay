cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)

if (LINUX)
    message(STATUS "we're on linux ...")
    set(CMAKE_SYSTEM_NAME Windows)
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

    # cross compilers to use for C, C++ and Fortran
    set(CMAKE_C_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_Fortran_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-gfortran)
    set(CMAKE_RC_COMPILER /usr/lib64/ccache/${TOOLCHAIN_PREFIX}-windres)

    # target environment on the build host system
    set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

    # modify default behavior of FIND_XXX() commands
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

if (WIN32)
    message(STATUS "oh no, not you MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /02")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug")
endif()

if (LINUX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif()

project(nexus_keyboard_overlay)

include_directories(src)

# load imgui as a cmake project
add_subdirectory(src/imgui)

# load mumble as a cmake project
add_subdirectory(src/mumble)

# load nexus as a cmake project
add_subdirectory(src/nexus)

# load nlohmann/json as a cmake project
add_subdirectory(src/nlohmann)

add_library(nexus_keyboard_overlay SHARED src/Settings.cpp src/Shared.cpp src/entry.cpp src/Settings.h src/Shared.h)
target_link_libraries(nexus_keyboard_overlay imgui json nexus mumble)

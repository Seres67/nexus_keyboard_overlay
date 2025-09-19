cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(nexus_keyboard_overlay)

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(nexus_keyboard_overlay SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/utils.cpp include/UiKey.hpp)
target_link_libraries(nexus_keyboard_overlay PRIVATE imgui)
set_property(TARGET nexus_keyboard_overlay PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# copy dll to game directory
add_custom_command(TARGET nexus_keyboard_overlay POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:nexus_keyboard_overlay>
        "C:\\Games\\Guild Wars 2\\addons\\nexus_keyboard_overlay.dll.update"
        COMMENT "Copying nexus_keyboard_overlay.dll.update to game directory"
)

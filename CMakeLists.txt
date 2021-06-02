cmake_minimum_required(VERSION 3.10.0)
project(dmsc-visualizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Examples
OPTION ( BUILD_SAMPLES
  "If the examples are built as well." ON )
if(BUILD_SAMPLES)
    add_subdirectory(./examples)
endif()

# GLM library
find_package(glm REQUIRED)

# GLFW library
# TODO not found -> local path
find_package(glfw3 REQUIRED)

add_library(dmsc STATIC)

set(imgui_files
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_sources(dmsc
    PRIVATE
        src/visuals.cpp
        src/instance.cpp
        src/opengl_widgets.cpp
        src/opengl_primitives.cpp
        src/glad/glad.c
        ${imgui_files}
)

target_include_directories(dmsc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        libs/imgui
        libs/imgui/backends    
        libs/stb    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dmsc glm::glm glfw)

if(UNIX)
    set_target_properties(dmsc PROPERTIES COMPILE_FLAGS -pthread LINK_FLAGS -pthread)
    target_link_libraries(dmsc ${CMAKE_DL_LIBS})
endif()

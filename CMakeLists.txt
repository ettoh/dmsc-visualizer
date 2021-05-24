cmake_minimum_required(VERSION 3.10.0)
project(dmsc-visualizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# QT library
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Widgets Gui REQUIRED)

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
add_subdirectory(./src/solver)
add_subdirectory(./src/gui)

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
        src/gui.cpp
        src/instance.cpp
        src/timetable.cpp
        src/glad/glad.c
        ${imgui_files}
)

target_include_directories(dmsc
    PRIVATE
        libs/imgui
        libs/imgui/backends
    PUBLIC # todo for now ok ... but has to be private
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dmsc Qt5::Widgets glm::glm glfw solver gui)

if(UNIX)
    target_link_libraries(dmsc ${CMAKE_DL_LIBS})
endif()

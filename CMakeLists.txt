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

add_library(vdmsc STATIC)
add_subdirectory(./src/instance)
add_subdirectory(./src/solver)
add_subdirectory(./src/gui)

target_include_directories(vdmsc
    PUBLIC # todo for now ok ... but has to be private
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vdmsc Qt5::Widgets instance solver gui)

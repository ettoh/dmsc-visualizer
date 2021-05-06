cmake_minimum_required(VERSION 3.10.0)
project(dmsc_solver VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# Compiler
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# QT library
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Widgets Gui REQUIRED)

# GLM
find_package(glm REQUIRED)

# Executable
add_executable(dmsc_solver main.cpp)
add_subdirectory(./instance)
add_subdirectory(./solver)
add_subdirectory(./gui)
target_link_libraries(dmsc_solver Qt5::Widgets instance solver gui)

# Copy shader and binary files
# If only the shader files changed - we need them to be copied even if gui hasn't changed
add_custom_target(always ALL)
add_custom_command(
    TARGET always PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:dmsc_solver>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/gui/shaders $<TARGET_FILE_DIR:dmsc_solver>/shaders
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:dmsc_solver>/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/gui/bin $<TARGET_FILE_DIR:dmsc_solver>/bin
    VERBATIM)

# Windows QT dll's
if (WIN32)
    if(NOT DEFINED ENV{QTDIR})
        MESSAGE(WARNING "QTDIR environment variable not set. Be sure to set the QTDIR cmake cache variable to the Qt root install location!")
    else()
        find_program(exec_windep windeployqt PATHS $ENV{QTDIR}/bin/)
        if(NOT exec_windep)
            message(WARNING "Unable to find windeployqt!")
        endif()

        add_custom_command(TARGET dmsc_solver POST_BUILD 
            COMMAND ${exec_windep} $<TARGET_FILE:dmsc_solver>)
    endif()
endif (WIN32)

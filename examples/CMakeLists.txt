# Executables
add_executable(sample_imgui imgui.cpp)
add_executable(sample_simulation simulation.cpp)
target_link_libraries(sample_imgui dmsc)
target_link_libraries(sample_simulation dmsc)

# Copy binary files
# TODO put these into a dedicated cmake-function to access it from "outside"
# TODO put resources into dmsc library OR more flexible path to resources ...
# If only the shader files changed - we need them to be copied even if gui hasn't changed
add_custom_target(always ALL)
add_custom_command(
    TARGET always POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_simulation>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/bin/shader $<TARGET_FILE_DIR:sample_simulation>/shaders
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_simulation>/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/bin/textures $<TARGET_FILE_DIR:sample_simulation>/bin
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

        add_custom_command(TARGET sample_simulation POST_BUILD 
            COMMAND ${exec_windep} $<TARGET_FILE:sample_simulation>)
    endif()
endif (WIN32)

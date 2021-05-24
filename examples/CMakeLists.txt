# Executables
add_executable(sample_main_window main_window.cpp)
add_executable(sample_imgui imgui.cpp)
target_link_libraries(sample_main_window dmsc)
target_link_libraries(sample_imgui dmsc)

# TODO put resources into dmsc library OR more flexible path to resources ...
#Copy shader and binary files
#If only the shader files changed - we need them to be copied even if gui hasn't changed
add_custom_target(always ALL)
add_custom_command(
    TARGET always POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_main_window>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/src/gui/shaders $<TARGET_FILE_DIR:sample_main_window>/shaders
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_main_window>/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/src/gui/bin $<TARGET_FILE_DIR:sample_main_window>/bin
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

        add_custom_command(TARGET sample_main_window POST_BUILD 
            COMMAND ${exec_windep} $<TARGET_FILE:sample_main_window>)
    endif()
endif (WIN32)

# Executable
add_executable(sample_all main.cpp)
target_link_libraries(sample_all vdmsc)

# TODO put resources into vdmsc library OR more flexible path to resources ...
#Copy shader and binary files
#If only the shader files changed - we need them to be copied even if gui hasn't changed
add_custom_target(always ALL)
add_custom_command(
    TARGET always POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_all>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/src/gui/shaders $<TARGET_FILE_DIR:sample_all>/shaders
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_all>/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/src/gui/bin $<TARGET_FILE_DIR:sample_all>/bin
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

        add_custom_command(TARGET sample_all POST_BUILD 
            COMMAND ${exec_windep} $<TARGET_FILE:sample_all>)
    endif()
endif (WIN32)

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
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_simulation>/shader
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/resources/shader $<TARGET_FILE_DIR:sample_simulation>/shader
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:sample_simulation>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_SOURCE_DIR}/resources/textures $<TARGET_FILE_DIR:sample_simulation>/textures
    VERBATIM)

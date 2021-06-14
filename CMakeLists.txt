cmake_minimum_required(VERSION 3.13.0)
project(dmsc-visualizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLM library
find_package(glm REQUIRED)

# GLFW library
find_package(glfw3 REQUIRED)

add_library(dmsc STATIC)
set_target_properties(dmsc PROPERTIES PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

# Debug flags
if(MSVC)
    target_compile_options(dmsc PRIVATE /W4
        /wd4458 # declaration of 'identifier' hides class member
        /wd4100 # 'identifier' : unreferenced formal parameter
    )
    target_link_options(dmsc PUBLIC /NODEFAULTLIB:MSVCRT)
else()
  target_compile_options(dmsc PRIVATE -Wall -Wextra -pedantic)
endif()

# Macro for copying resource files
macro(copy_resources target)
    get_target_property(source_dir dmsc PROJECT_SOURCE_DIR) # to be independent of the calling target/project
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${source_dir}/resources/shader $<TARGET_FILE_DIR:${target}>/shader
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${source_dir}/resources/textures $<TARGET_FILE_DIR:${target}>/textures
        VERBATIM)
endmacro()

# Examples
OPTION(BUILD_SAMPLES
    "If the examples are built as well." ON)
if(BUILD_SAMPLES)
    add_subdirectory(./examples)
endif()

# external source files
set(external_files
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/glad/src/glad.c
)

# solver source files
set(solver_files
    src/solver.cpp
    src/solver/greedy_next.cpp
    src/solver/greedy_next_khop.cpp
)

# source files
target_sources(dmsc
    PRIVATE
        src/visuals.cpp
        src/instance.cpp
        src/opengl_widgets.cpp
        src/opengl_primitives.cpp        
        ${solver_files}
        ${external_files}
)

target_include_directories(dmsc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        external/imgui
        external/imgui/backends
        external/stb
        external/glad/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dmsc glm::glm glfw)

if(UNIX)
    set_target_properties(dmsc PROPERTIES COMPILE_FLAGS -pthread LINK_FLAGS -pthread)
    target_link_libraries(dmsc ${CMAKE_DL_LIBS})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(gui
    main_window.h main_window.ui main_window.cpp 
    opengl_widget.h opengl_widget.cpp
    opengl_toolkit.h opengl_toolkit.cpp
    dialog_instance.h dialog_instance.ui dialog_instance.cpp
    dialog_add_edges.h dialog_add_edges.ui dialog_add_edges.cpp
)

target_link_libraries(gui Qt5::Widgets glm::glm solver dmsc) # TODO temporary

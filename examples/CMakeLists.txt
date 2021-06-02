# Executables
add_executable(sample_imgui imgui.cpp)
add_executable(sample_simulation simulation.cpp)
target_link_libraries(sample_imgui dmsc)
target_link_libraries(sample_simulation dmsc)

copy_resources(sample_simulation)

add_executable(sample_instances instances.cpp)
add_executable(sample_solver solver.cpp)
target_link_libraries(sample_instances dmsc)
target_link_libraries(sample_solver dmsc)

# copy texture and shader files into the executable folder
copy_resources(sample_instances)

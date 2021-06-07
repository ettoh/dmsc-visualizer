add_executable(sample_instances instances.cpp)
target_link_libraries(sample_instances dmsc)

# copy texture and shader files into the executable folder
copy_resources(sample_instances)

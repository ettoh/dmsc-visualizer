add_executable(sample_instances instances.cpp)
add_executable(sample_solver solver.cpp)
add_executable(sample_custom custom_animation.cpp)
add_executable(sample_freezeTag freeze_tag.cpp)
target_link_libraries(sample_instances dmsc)
target_link_libraries(sample_solver dmsc)
target_link_libraries(sample_custom dmsc)
target_link_libraries(sample_freezeTag dmsc)

# copy texture and shader files into the executable folder
# here all examples are in the same folder ... so have to call it just once
copy_resources(sample_instances)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(instance 
    orbit.h edge.h 
    problem_instance.h problem_instance.cpp
    vector3d.h vector3d.cpp 
    timetable.h)
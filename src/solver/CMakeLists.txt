include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(solver 
    greedy_next.h greedy_next.cpp
    scan_cover.h
    solver.h solver.cpp 
    iterated_local_search.h iterated_local_search.cpp 
    simulated_annealing.h simulated_annealing.cpp)
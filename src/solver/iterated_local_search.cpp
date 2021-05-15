#include "iterated_local_search.h"
#include "greedy_next.h"
#include "vdmsc/edge.h"
#include <ctime>
#include <random>

ScanCover ILS::solve() {
    // start time for computation time
    auto t_start = std::chrono::system_clock::now();
    satellite_orientation.clear();
    int max_pertubation = 10;

    // initial solution
    GreedyNext greedy = GreedyNext(instance);
    ScanCover best_solution = greedy.solve();
    EdgeOrder best_order = toEdgeOrder(best_solution);
    EdgeOrder inital_solutution = best_order;
    float best = best_solution.getScanTime();
    size_t n = inital_solutution.size();

    for (int it = 0; it < max_pertubation; it++) {
        // allowed to continue?
        if (solver_abort == true) {
            return ScanCover();
        }

        EdgeOrder local_minimum = localSearch(inital_solutution);
        float local_minimum_time = evaluateEdgeOrder(local_minimum).getScanTime();

        if (local_minimum_time < best) {
            best = local_minimum_time;
            best_solution = evaluateEdgeOrder(local_minimum);
            best_order = local_minimum;
        }

        // permutataion of initial candidate => perform edge swaps
        inital_solutution = best_order;
        std::mt19937 generator(static_cast<long int>(std::time(nullptr)));
        std::uniform_int_distribution<> distr(0, (int)inital_solutution.size() - 1);
        for (int swap = 0; swap < 5; swap++) {
            // choose edges to swap
            int edge_1 = distr(generator);
            int edge_2 = distr(generator);

            // perform swap
            int tmp = inital_solutution[edge_1];
            inital_solutution[edge_1] = inital_solutution[edge_2];
            inital_solutution[edge_2] = tmp;
        }

        if (callback != nullptr)
            callback((float)it / max_pertubation);
    }

    // end time for computation time
    auto t_end = std::chrono::system_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;
    best_solution.setComputationTime(diff.count());

    return best_solution;
}

EdgeOrder ILS::localSearch(EdgeOrder init_order) {
    EdgeOrder ls_best_order = init_order;

    while (true) {
        EdgeOrder ls_best_neighbour = ls_best_order;
        float ls_best_neighbour_time = evaluateEdgeOrder(ls_best_neighbour).getScanTime();
        for (int i = 0; i < (int)ls_best_order.size() - 1; i++) {
            for (int j = i + 1; j < ls_best_order.size(); j++) {
                EdgeOrder neighbour = ls_best_order;
                int tmp = neighbour[i];
                neighbour[i] = neighbour.at(j);
                neighbour[j] = tmp;
                ScanCover s = evaluateEdgeOrder(neighbour);
                if (s.getScanTime() < ls_best_neighbour_time) {
                    ls_best_neighbour = neighbour;
                    ls_best_neighbour_time = evaluateEdgeOrder(ls_best_neighbour).getScanTime();
                }
            }

            if (ls_best_neighbour_time < evaluateEdgeOrder(ls_best_order).getScanTime()) {
                ls_best_order = ls_best_neighbour;
            } else {
                break;
            }
        }

        return ls_best_order;
    }
}

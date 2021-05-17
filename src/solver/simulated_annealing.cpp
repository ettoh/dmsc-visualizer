#include "simulated_annealing.h"
#include "greedy_next.h"
#include <ctime>
#include <random>

namespace dmsc {

ScanCover SimulatedAnnealing::solve() {
    auto t_start = std::chrono::system_clock::now();
    satellite_orientation.clear();

    LineGraph line_graph = instance.lineGraph();

    // init random generator
    std::mt19937 generator(static_cast<long int>(std::time(nullptr)));
    std::uniform_int_distribution<> distr(0, (int)instance.edges.size() - 1);
    std::uniform_real_distribution<float> prob_distr(0.0f, 1.0f);

    // random initial solution
    EdgeOrder random_order;
    for (int i = 0; i < instance.edges.size(); i++) {
        random_order.push_back(i);
    }
    for (int i = 0; i < 1000; i++) {
        // choose random swap
        int a = distr(generator);
        int b;
        do {
            b = distr(generator);
        } while (a == b);
        int temp = random_order[a];
        random_order[a] = random_order[b];
        random_order[b] = temp;
    }
    ScanCover random_solution = evaluateEdgeOrder(random_order);

    // good initial solution
    GreedyNext greedy = GreedyNext(instance);
    ScanCover greedy_solution = greedy.solve();
    EdgeOrder greedy_order = toEdgeOrder(greedy_solution);

    // set initial solution
    ScanCover current_solution = greedy_solution;
    EdgeOrder current_order = greedy_order;
    ScanCover best_solution = current_solution;
    EdgeOrder initial_order = toEdgeOrder(current_solution);
    float best_scan_time = best_solution.getScanTime();

    // initial parameters
    float temperature_0 = 2700.0f;
    int k_max = 2000;
    int restarts = 1;
    int progress = 0;

    if (initial_order.size() <= 1) {
        return best_solution;
    }

    for (int k = 0; k < k_max; k) {
        if (k >= k_max && restarts-- >= 0)
            k = 1;

        float temperature = temperature_0 * powf(0.96f, static_cast<float>(++k));

        // allowed to continue?
        if (solver_abort == true) {
            return ScanCover();
        }

        // choose random neighbour
        EdgeOrder neighbour = current_order;
        int pos_to_swap = distr(generator);                                     // pick an edge
        AdjacentList adj_list = line_graph.edges[neighbour[pos_to_swap]]; // which edges are adjacent to the picked one?
        if (adj_list.size() == 0)
            continue;

        std::uniform_int_distribution<> distr2(0, (int)adj_list.size() - 1);
        int edge_to_swap = adj_list.at(distr2(generator));

        // find index where b is stored in current order
        auto edge_position = std::find(neighbour.begin(), neighbour.end(), edge_to_swap);
        if (edge_position != neighbour.end()) {
            *edge_position = neighbour.at(pos_to_swap);
            neighbour[pos_to_swap] = edge_to_swap;
        } else {
            continue;
        }

        // choose neighbour or not?
        ScanCover new_solution = evaluateEdgeOrder(neighbour);
        if (new_solution.getScanTime() <= current_solution.getScanTime()) {
            current_solution = new_solution;
            current_order = neighbour;
        } else {
            float prob = std::exp(-(new_solution.getScanTime() - current_solution.getScanTime()) / temperature);
            if (prob_distr(generator) <= prob) {
                current_solution = new_solution;
                current_order = neighbour;
            }
        }

        // track global minimum
        if (current_solution.getScanTime() < best_scan_time) {
            best_solution = current_solution;
            best_scan_time = best_solution.getScanTime();
        }

        if (callback != nullptr)
            callback((float)progress++ / (restarts * k_max + k_max));
    }

    // end time for computation time
    auto t_end = std::chrono::system_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;
    best_solution.setComputationTime(diff.count());
    return best_solution;
}

} // namespace dmsc
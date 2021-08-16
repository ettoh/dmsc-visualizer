#include "dmsc/solver/greedy_next_khop.hpp"
#include <deque>
#include <set>

namespace dmsc {
namespace solver {

/**
 * @brief Bunch together all information how a scheduled communication between two satellites can be performed (and
 * tracks the current progress).
 */
struct GreedyNextKHop::Communication {
    ScheduledCommunication scheduled_communication = {~0u, ~0u};
    AdjacencyList possible_paths = AdjacencyList(0, AdjacencyList::Item(~0u, ~0u));
    uint32_t forward_idx = 0u; // index of current vertex for the forward direction (sat1 -> sat2)
};

// ------------------------------------------------------------------------------------------------

Solution GreedyNextKHop::solve() {
    // start time for computation time
    auto t_start = std::chrono::system_clock::now();

    // init variables
    ScanCover scan_cover;
    float curr_time = 0.0;
    satellite_orientation.clear();

    // select edges for computation
    std::vector<Communication> remaining_communications;
    for (const ScheduledCommunication& c : instance.scheduled_communications) {
        Communication communication;
        auto paths = findPaths(c.first, c.second);
        if (paths.first) { // there is at least one path from a to b
            communication.scheduled_communication = c;
            communication.possible_paths = paths.second;
            communication.forward_idx = c.first;
            remaining_communications.push_back(communication);
        }
    }

    // choose the best edge in each iteration
    while (remaining_communications.size() > 0) {
        uint32_t chosen_communication = ~0u;
        uint32_t chosen_neighbour = ~0u;
        float t_next = INFINITY; // absolute time

        // find the best edge depending on the time passed
        for (uint32_t i = 0; i < remaining_communications.size(); i++) {
            const Communication& com = remaining_communications[i];

            std::map<uint32_t, AdjacencyList::Item> possible_neighbours = com.possible_paths[com.forward_idx];
            // iterate over all possibilities to continue the currently chosen path
            bool path_possible = false; // is there at least one edge we can use? (will be visible in the future)
            for (const auto& neighbour : possible_neighbours) {
                const InterSatelliteLink& link = instance.getISLs()[neighbour.second.isl_idx];
                float next_communication = nextCommunication(link, curr_time);

                // will the edge become visible on the future?
                if (next_communication < INFINITY) {
                    path_possible = true;
                }

                // edge is avaible earlier
                if (next_communication < t_next) {
                    t_next = next_communication;
                    chosen_neighbour = neighbour.first;
                    chosen_communication = i;
                }

                // it's not getting better
                if (t_next - curr_time == 0.f)
                    break;
            }

            // we cant continue the current path
            if (!path_possible) {
                remaining_communications.erase(remaining_communications.begin() + i);
                i--; // the size of the remaining communications just reduced by 1
            }
        }

        // no "next" edge was found
        if (chosen_communication == ~0u || chosen_neighbour == ~0u || t_next == INFINITY) {
            break;
        }

        // update communication
        Communication& com = remaining_communications[chosen_communication];
        uint32_t isl_idx = com.possible_paths[com.forward_idx][chosen_neighbour].isl_idx;
        com.forward_idx = chosen_neighbour;

        // add edge to solution
        const InterSatelliteLink* isl = &instance.getISLs()[isl_idx];
        scan_cover.insert({isl_idx, t_next});

        // if chosen communication is done now, remove it
        if (com.forward_idx == com.scheduled_communication.second) {
            remaining_communications.erase(remaining_communications.begin() + chosen_communication);
        }

        // update satellite orientations
        glm::vec3 new_orientations = isl->getOrientation(t_next);
        satellite_orientation[&isl->getV1()] = TimelineEvent<glm::vec3>(t_next, t_next, new_orientations);
        satellite_orientation[&isl->getV2()] = TimelineEvent<glm::vec3>(t_next, t_next, -new_orientations);

        // update time
        curr_time = t_next;
    }

    // end time for computation time
    auto t_end = std::chrono::system_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;

    Solution solution;
    solution.computation_time = diff.count();
    solution.scan_cover = scan_cover;
    return solution;
}

// ------------------------------------------------------------------------------------------------

std::pair<bool, AdjacencyList> GreedyNextKHop::findPaths(const uint32_t origin_idx,
                                                         const uint32_t destination_idx) const {
    // store path in correct order and sorted (to be able to find already visited vertices)
    using Subpath = std::pair<std::vector<uint32_t>, std::set<uint32_t>>;

    AdjacencyList result(instance.satelliteCount(), AdjacencyList::Item(0u, ~0u));
    const AdjacencyList& global_adj = instance.getAdjacencyMatrix();
    std::deque<Subpath> subpaths = {{{origin_idx}, {origin_idx}}};
    bool path_found = false; // at least one path found?

    while (subpaths.size() > 0) {
        Subpath subpath = subpaths.front();
        subpaths.pop_front();

        // iterate over all neighbour vertices of the last vertex in current subpath
        for (const auto& neighbour : global_adj[subpath.first.back()]) {
            // vertex visited before?
            if (subpath.second.find(neighbour.first) != subpath.second.end()) {
                continue;
            }

            // path complete?
            if (neighbour.first == destination_idx) {
                // add path to the final adjacency matrix
                path_found = true;
                for (uint32_t i = 0; (i + 1) < subpath.first.size(); i++) {
                    uint32_t from_idx = subpath.first[i];
                    uint32_t to_idx = subpath.first[i + 1];
                    auto edge_used = global_adj[from_idx].find(to_idx);

                    if (edge_used != global_adj[from_idx].end()) {
                        result.matrix[from_idx][to_idx] = edge_used->second;
                    } else {
                        printf("The adjacency list does not include an entry (%u, %u).\n", from_idx, to_idx);
                        assert(false);
                        exit(EXIT_FAILURE);
                    }
                }
                // add last edge (which was not part of the subpath before) to the final adjacency matrix;
                result.matrix[subpath.first.back()][destination_idx] = neighbour.second;
                continue;
            }

            // can we go deeper?
            // k is number of "extra" satellites - e.g. k=1 allows 3 edges (origin, hop, target)
            if (!(subpath.first.size() > k + 2)) {
                Subpath new_subpath = subpath;
                new_subpath.first.push_back(neighbour.first); // add vertex to the path
                new_subpath.second.insert(neighbour.first);   // mark vertex as visited
                subpaths.push_back(new_subpath);
            }
        }
    }

    return {path_found, result};
}

} // namespace solver
} // namespace dmsc

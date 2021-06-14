#include "dmsc/solver/greedy_next_khop.hpp"
#include <deque>
#include <set>

namespace dmsc {
namespace solver {

/**
 * @brief Bunch together all information how a scheduled communication between two satellites can be performed (and
 * tracks the current progress).
 * A scheduled intersatellite communication has to be probed from both sides. With
 * intermediate satellites (multihop) allowed, for each direction a different route can be chosen. We have to keep track
 * both paths.
 */
struct GreedyNextKHop::Communication {
    const InterSatelliteLink* scheduled_link = nullptr;
    AdjacencyMatrix possible_paths = AdjacencyMatrix(0);
    uint32_t forward_idx = 0u;  // index of current vertex for the forward direction (sat1 -> sat2)
    uint32_t backward_idx = 0u; // index of current vertex for the backward direction (sat2 -> sat1)
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
    for (const InterSatelliteLink& e : instance.getEdges()) {
        float t_communication = nextCommunication(e, 0.0f);
        if (t_communication < INFINITY && !e.isOptional()) {
            Communication communication;
            auto res = findPaths(e.getV1Idx(), e.getV2Idx());
            if (res.first) { // there is at least one path from a to b
                communication.possible_paths = res.second;
                communication.forward_idx = e.getV1Idx();
                communication.backward_idx = e.getV2Idx();
                communication.scheduled_link = &e;
                remaining_communications.push_back(communication);
            }
        }
    }

    // choose best edge in each iteration
    while (remaining_communications.size() > 0) {
        uint32_t chosen_communication = 0u;
        uint32_t chosen_neighbour = 0u;
        bool forward_chosen = true;
        float t_next = INFINITY; // absolute time

        // find best edge depending on the time passed
        for (int i = 0; i < remaining_communications.size(); i++) {
            const Communication& com = remaining_communications[i];

            // forward direction not done
            if (com.forward_idx != com.scheduled_link->getV2Idx()) {
                std::vector<AdjacencyMatrix::Item> possible_neighbours = com.possible_paths.matrix[com.forward_idx];
                // iterate over all possibilities to continue the currently chosen path
                for (uint32_t neighbour = 0u; neighbour < possible_neighbours.size(); neighbour++) {
                    if (possible_neighbours[neighbour].weight == 1u) { // this vertex is part of a possible path
                        const InterSatelliteLink& link = instance.getEdges()[possible_neighbours[neighbour].edge_idx];
                        float next_communication = nextCommunication(link, curr_time);

                        // edge is avaible earlier
                        if (next_communication < t_next) {
                            t_next = next_communication;
                            chosen_neighbour = neighbour;
                            chosen_communication = i;
                            forward_chosen = true;
                        }

                        // it's not getting better
                        if (t_next - curr_time == 0.f)
                            break;
                    }
                }
            }

            // backward direction not done
            if (com.backward_idx != com.scheduled_link->getV1Idx()) {
                std::vector<AdjacencyMatrix::Item> possible_neighbours = com.possible_paths.column(com.backward_idx);
                // iterate over all possibilities to continue the currently chosen path
                for (uint32_t neighbour = 0u; neighbour < possible_neighbours.size(); neighbour++) {
                    if (possible_neighbours[neighbour].weight == 1u) { // this vertex is part of a possible path
                        const InterSatelliteLink& link = instance.getEdges()[possible_neighbours[neighbour].edge_idx];
                        float next_communication = nextCommunication(link, curr_time);

                        // edge is avaible earlier
                        if (next_communication < t_next) {
                            t_next = next_communication;
                            chosen_neighbour = neighbour;
                            chosen_communication = i;
                            forward_chosen = false;
                        }

                        // it's not getting better
                        if (t_next - curr_time == 0.f)
                            break;
                    }
                }
            }
        }

        // refresh orientation of chosen satellites
        // todo only recalculate needed satellites ...

        // update communication
        Communication& com = remaining_communications[chosen_communication];
        uint32_t edge_idx = ~0u;
        if (forward_chosen) {
            edge_idx = com.possible_paths.matrix[com.forward_idx][chosen_neighbour].edge_idx;
            com.forward_idx = chosen_neighbour;
        } else {
            edge_idx = com.possible_paths.column(com.backward_idx)[chosen_neighbour].edge_idx;
            com.backward_idx = chosen_neighbour;
        }

        // add edge to solution
        const InterSatelliteLink* isl = &instance.getEdges()[edge_idx];
        scan_cover.insert({edge_idx, t_next});

        // if chosen communication is done now, remove it
        if (com.forward_idx == com.scheduled_link->getV2Idx() && com.backward_idx == com.scheduled_link->getV1Idx()) {
            remaining_communications.erase(remaining_communications.begin() + chosen_communication);
        }

        // update satellite orientations
        EdgeOrientation new_orientations = isl->getOrientation(t_next);
        satellite_orientation[&isl->getV1()] = TimelineEvent<glm::vec3>(
            new_orientations.sat1.start, new_orientations.sat1.start, new_orientations.sat1.direction);
        satellite_orientation[&isl->getV2()] = TimelineEvent<glm::vec3>(
            new_orientations.sat2.start, new_orientations.sat2.start, new_orientations.sat2.direction);

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

// return a directed graph (origin_idx -> destination_idx)
std::pair<bool, AdjacencyMatrix> GreedyNextKHop::findPaths(const uint32_t origin_idx,
                                                           const uint32_t destination_idx) const {
    // store path in correct order and sorted (to be able to find already visited vertices)
    using Subpath = std::pair<std::vector<uint32_t>, std::set<uint32_t>>;

    AdjacencyMatrix result(instance.edgeSize(), AdjacencyMatrix::Item(0u, ~0u));
    const AdjacencyMatrix& global_adj = instance.getAdjacencyMatrix();
    std::deque<Subpath> subpaths = {{{origin_idx}, {origin_idx}}};
    bool path_found = false; // at least one path found?

    while (subpaths.size() > 0) {
        Subpath subpath = subpaths.front();
        subpaths.pop_front();

        // iterate over all neighbour vertices of the last vertex in current subpath
        for (uint32_t neighbour_vertex = 0; neighbour_vertex < global_adj.matrix[subpath.first.back()].size();
             neighbour_vertex++) {
            // is vertex neighbour?
            if (global_adj.matrix[subpath.first.back()][neighbour_vertex].weight == 0u) {
                continue;
            }

            // vertex visited before?
            if (subpath.second.find(neighbour_vertex) != subpath.second.end()) {
                continue;
            }

            // path complete?
            if (neighbour_vertex == destination_idx) {
                // add path to the final adjacency matrix
                path_found = true;
                for (uint32_t i = 0; (i + 1) < subpath.first.size(); i++) {
                    uint32_t from_idx = subpath.first[i];
                    uint32_t to_idx = subpath.first[i + 1];
                    AdjacencyMatrix::Item edge_used = global_adj.matrix[from_idx][to_idx];
                    result.matrix[from_idx][to_idx] = edge_used;
                }
                // add last edge (which was not part of the subpath before) to the final adjacency matrix
                AdjacencyMatrix::Item edge_used = global_adj.matrix[subpath.first.back()][destination_idx];
                result.matrix[subpath.first.back()][destination_idx] = edge_used;
                continue;
            }

            // can we go deeper?
            // k is number of "extra" satellites - e.g. k=1 allows 3 edges (origin, hop, target)
            if (!(subpath.first.size() > k + 2)) {
                Subpath new_subpath = subpath;
                new_subpath.first.push_back(neighbour_vertex); // add vertex to the path
                new_subpath.second.insert(neighbour_vertex);   // mark vertex as visited
                subpaths.push_back(new_subpath);
            }
        }
    }

    return {path_found, result};
}

} // namespace solver
} // namespace dmsc

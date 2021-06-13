#include "dmsc/solver/greedy_next.hpp"

namespace dmsc {
namespace solver {

Solution GreedyNext::solve() {
    // start time for computation time
    auto t_start = std::chrono::system_clock::now();

    // init variables
    ScanCover scan_cover;
    float curr_time = 0.0;
    satellite_orientation.clear();

    // select edges for computation
    std::vector<const InterSatelliteLink*> remaining_edges;
    for (const InterSatelliteLink& e : instance.getEdges()) {
        float t_communication = nextCommunication(e, 0.0f);
        if (t_communication < INFINITY && !e.isOptional()) {
            remaining_edges.push_back(&e);
        }
    }

    // choose best edge in each iteration.
    while (remaining_edges.size() > 0) {
        int best_edge_pos = 0;   // position in remaining edges
        float t_next = INFINITY; // absolute time

        // find best edge depending on the time passed
        for (int i = 0; i < remaining_edges.size(); i++) {
            const InterSatelliteLink& e = *remaining_edges.at(i);
            float next_communication = nextCommunication(e, curr_time);

            // edge is avaible earlier
            if (next_communication < t_next) {
                t_next = next_communication;
                best_edge_pos = i;
            }

            // it's not getting better
            if (t_next == 0)
                break;
        }

        // refresh orientation of chosen satellites
        // todo only recalculate needed satellites ...
        const InterSatelliteLink* e = remaining_edges.at(best_edge_pos);
        EdgeOrientation new_orientations = e->getOrientation(t_next);
        satellite_orientation[&e->getV1()] = TimelineEvent<glm::vec3>(
            new_orientations.sat1.start, new_orientations.sat1.start, new_orientations.sat1.direction);
        satellite_orientation[&e->getV2()] = TimelineEvent<glm::vec3>(
            new_orientations.sat2.start, new_orientations.sat2.start, new_orientations.sat2.direction);

        // map position in remaining edges to position in all edges
        std::ptrdiff_t edge_index = remaining_edges[best_edge_pos] - &instance.getEdges()[0];
        // add edge
        scan_cover.insert({static_cast<uint32_t>(edge_index), t_next});
        remaining_edges.erase(remaining_edges.begin() + best_edge_pos);
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

} // namespace solver
} // namespace dmsc
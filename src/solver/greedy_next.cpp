#include "dmsc/solver/greedy_next.hpp"

namespace dmsc {
namespace solver {

// TODO are the instance and solver compatible?
// greedy next ignores the schedules communications - it scans all isl
Solution GreedyNext::solve() {
    // start time for computation time
    auto t_start = std::chrono::system_clock::now();

    // init variables
    ScanCover scan_cover;
    float curr_time = 0.0;
    satellite_orientation.clear();

    // select edges for computation
    std::vector<const InterSatelliteLink*> remaining_edges;
    for (const InterSatelliteLink& e : instance.getISLs()) {
        float t_communication = nextCommunication(e, 0.0f);
        if (t_communication < INFINITY) {
            remaining_edges.push_back(&e);
        }
    }

    // choose the best edge in each iteration.
    while (remaining_edges.size() > 0) {
        int best_edge_pos = 0;   // position in remaining edges
        float t_next = INFINITY; // absolute time

        // find the best edge depending on the time passed
        for (size_t i = 0; i < remaining_edges.size(); i++) {
            const InterSatelliteLink& e = *remaining_edges.at(i);
            float next_communication = nextCommunication(e, curr_time);

            // edge is avaible earlier
            if (next_communication < t_next) {
                t_next = next_communication;
                best_edge_pos = i;
            }

            // it's not getting better
            if (t_next - curr_time == 0.f)
                break;
        }

        // refresh orientation of chosen satellites.
        const InterSatelliteLink* e = remaining_edges.at(best_edge_pos);
        EdgeOrientation new_orientations = e->getOrientation(t_next);
        satellite_orientation[&e->getV1()] = TimelineEvent<glm::vec3>(t_next, t_next, new_orientations.first);
        satellite_orientation[&e->getV2()] = TimelineEvent<glm::vec3>(t_next, t_next, new_orientations.second);

        // map position in remaining edges to position in all edges
        std::ptrdiff_t edge_index = remaining_edges[best_edge_pos] - &instance.getISLs()[0];
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

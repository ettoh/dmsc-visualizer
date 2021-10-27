#ifndef DMSC_SOLUTION_TYPES_H
#define DMSC_SOLUTION_TYPES_H

#include <map>
#include <vector>

namespace dmsc {

/// index of an edge and the time when this edge is scanned - time in [sec]
using ScanCover = std::multimap<uint32_t, float>;

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 * 
 */
struct DmscSolution {
    float computation_time = 0.f; // [sec]
    float scan_time = 0.f;        // [sec]
    ScanCover scan_cover;

    /**
     * @brief The edge with the given index will be scanned at the given time.
     *
     * @param edge_idx Index of edge in the underlying physical instance.
     * @param time [sec]
     */
    void scheduleEdge(const uint32_t edge_idx, const float time) { scan_cover.insert({edge_idx, time}); }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 * 
 */
struct FreezeTagSolution {
    float computation_time = 0.f; // [sec]
    float scan_time = 0.f;        // [sec]
    ScanCover scan_cover;
    std::vector<size_t> satellites_with_message; // initial satellites

    /**
     * @brief The edge with the given index will be scanned at the given time.
     *
     * @param edge_idx Index of edge in the underlying physical instance.
     * @param time [sec]
     */
    void scheduleEdge(const uint32_t edge_idx, const float time) { scan_cover.insert({edge_idx, time}); }
};

} // namespace dmsc

#endif

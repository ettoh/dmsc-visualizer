#ifndef SOLVER_H
#define SOLVER_H

#include "instance.hpp"
#include "satellite.hpp"
#include "timeline.hpp"
#include <chrono>
#include <map>

namespace dmsc {

using ScanCover = std::map<uint32_t, float>; // when an edge is scanned - time in [sec]

struct Solution {
    float computation_time = 0.f; // [sec]
    float scan_time = 0.f;        // [sec]
    ScanCover scan_cover;
};

// ------------------------------------------------------------------------------------------------

using EdgeOrder = std::vector<int>;

class Solver {
  public:
    Solver(const PhysicalInstance& instance)
        : instance(PhysicalInstance(instance)) {
        createCache();
    };

    virtual Solution solve() = 0;
    float lowerBound();

  protected:
    /**
     * @brief Calculate the time (beginning at time x) when an egde can be scanned the next time.
     * The central mass and turn costs are considered.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for next communication possible.
     */
    float nextCommunication(const InterSatelliteLink& edge, const float time_0);

    /** Return the time when the edge is visible for next time beginning at time t0.
     * If the corresponding time slot was evalutated before - use the cached version to reduce computation
     * time.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for next visibility. INFINITY if the edge will never be visible.
     */
    float nextVisibility(const InterSatelliteLink& edge, const float t0);

    /**
     * @brief
     * @param scan_cover A sorted scan cover.
     * @return
     */
    EdgeOrder toEdgeOrder(const ScanCover& scan_cover);

    ScanCover evaluateEdgeOrder(const EdgeOrder& edge_order);

    const PhysicalInstance instance;
    const float step_size = 1.0f; // [sec]
    std::map<const Satellite*, TimelineEvent<glm::vec3>>
        satellite_orientation; // Last known orientation for each satellite
                               // and the time when it changed.

  private:
    /** Calculate the time (beginning at time t0) when an edge is no longer interrupted by the central mass.
     * This is done by iterating over t and check each time step if the edge is visible.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for next visibility. INFINITY if the edge will never be visible.
     */
    float findNextVisiblity(const InterSatelliteLink& edge, const float t0) const;

    /** Calculate the time (beginning at time t0) when an edge is no longer visible.
     * This is done by iterating over t and check each time step if the edge is visible.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for end of visibility. INFINITY if the edge will never disappear.
     */
    float findLastVisible(const InterSatelliteLink& edge, const float t0) const;

    void createCache();

    std::map<const InterSatelliteLink*, Timeline<>> edge_time_slots;
    // Timetable<InterSatelliteLink, TimeSlot> edge_time_slots = Timetable<InterSatelliteLink, TimeSlot>();
    std::map<const InterSatelliteLink*, float>
        edge_cache_progress; // max. time for which cache (for visibility) is avaiable

    [[deprecated]] bool sphereIntersection(const InterSatelliteLink& edge, const float time);
};

} // namespace dmsc

#endif
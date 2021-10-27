#ifndef DMSC_SOLVER_H
#define DMSC_SOLVER_H

#include "instance.hpp"
#include "satellite.hpp"
#include "timeline.hpp"
#include <map>

namespace dmsc {

class Solver {
  public:
    Solver(const PhysicalInstance& instance)
        : instance(instance) {
        createCache();
    };

  protected:
    /**
     * @brief Calculates the time (beginning at time x) when an egde can be scanned the next time.
     * The central mass and turn costs are considered.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for next communication possible.
     */
    float nextCommunication(const InterSatelliteLink& edge, const float time_0);

    /** Returns the time when the edge is visible for next time beginning at time t0.
     * If the corresponding time slot was evalutated before - use the cached version to reduce computation
     * time.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for next visibility. INFINITY if the edge will never be visible.
     */
    float nextVisibility(const InterSatelliteLink& edge, const float t0);

    const PhysicalInstance instance;
    const float step_size = 1.0f; // [sec]
    std::map<const Satellite*, TimelineEvent<glm::vec3>>
        satellite_orientation; // Last known orientation for each satellite and the time when it changed.

  private:
    /** Calculates the time (beginning at time t0) when an edge is no longer interrupted by the central mass.
     * This is done by iterating over t and check each time step if the edge is visible.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for next visibility. INFINITY if the edge will never be visible.
     */
    float findNextVisiblity(const InterSatelliteLink& edge, const float t0) const;

    /** Calculates the time (beginning at time t0) when an edge is no longer visible.
     * This is done by iterating over t and check each time step if the edge is visible.
     * @param time_0 [sec] start time
     * @return Absolute time in [sec] for end of visibility. INFINITY if the edge will never disappear.
     */
    float findLastVisible(const InterSatelliteLink& edge, const float t0) const;

    void createCache();

    std::map<const InterSatelliteLink*, Timeline<>> edge_time_slots;
    std::map<const InterSatelliteLink*, float>
        edge_cache_progress; // max. time for which cache (for visibility) is avaiable
};

} // namespace dmsc

#endif

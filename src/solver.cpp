#include "dmsc/solver.hpp"
#include "dmsc/glm_include.hpp"
#include <cmath>
#include <ctime>
#include <fstream>
#include <random>

namespace dmsc {

/* float Solver::lowerBound() {
    float lower_bound = 0;
    for (const InterSatelliteLink& e : instance.getEdges()) {
        float t = nextVisibility(e, 0.0);
        if (t > lower_bound && t < INFINITY)
            lower_bound = t;
    }
    return lower_bound;
} */

// ------------------------------------------------------------------------------------------------

float Solver::nextCommunication(const InterSatelliteLink& edge, const float time_0) {
    // edge is never visible?
    float t_visible = nextVisibility(edge, time_0);
    if (t_visible >= INFINITY) {
        return INFINITY;
    }

    // get current orientation of both satellites
    TimelineEvent<glm::vec3> sat1 = satellite_orientation[&edge.getV1()];
    TimelineEvent<glm::vec3> sat2 = satellite_orientation[&edge.getV2()];

    // edge can be scanned directly?
    if (edge.canAlign(sat1, sat2, t_visible)) {
        return t_visible;
    }

    // satellites can't align => search for a time where they can
    // max time to align ==> time for a 180 deg turn
    float t_max = std::max(static_cast<float>(M_PI) / edge.getV1().getRotationSpeed(),
                           static_cast<float>(M_PI) / edge.getV2().getRotationSpeed());
    t_max += edge.getPeriod();

    for (float t = t_visible; t <= time_0 + t_max; t += step_size) {
        if (edge.isBlocked(t)) { // skip time where edge is blocked
            // find next slot where edge is visible
            float t_relative = fmodf(t, edge.getPeriod());
            auto search = edge_time_slots.find(&edge);
            float t_next = 0.0f;

            if (search != edge_time_slots.end()) {
                t_next = search->second.nextTimeWithEvent(t_relative, true);
            }

            if (t_next < t_relative) { // loop applied
                t += t_next + edge.getPeriod() - t_relative;
            } else { // loop not applied
                t += t_next - t_relative;
            }
        }

        if (edge.canAlign(sat1, sat2, t)) { // edge can be scanned
            if (!edge.isBlocked(t)) {
                return t;
            }
        }
    }

    // communication is never possible
    return INFINITY;
}

// ------------------------------------------------------------------------------------------------

void Solver::createCache() {
    for (const auto& edge : instance.getISL()) {
        for (float t = 0.0f; t < edge.getPeriod(); t += step_size) {
            float t_next = findNextVisiblity(edge, t);
            if (t_next == INFINITY || t_next >= edge.getPeriod()) {
                break;
            }

            float t_end = findLastVisible(edge, t_next);
            if (t_end == INFINITY || t_end >= edge.getPeriod()) {
                t_end = edge.getPeriod();
            }

            TimelineEvent<> slot(t_next, t_end);
            edge_time_slots[&edge].insert(slot);
            t = slot.t_end;
        }
    }
}

// ------------------------------------------------------------------------------------------------

float Solver::nextVisibility(const InterSatelliteLink& edge, const float t0) {
    if (edge_time_slots[&edge].size() == 0) {
        return INFINITY;
    }

    float t = std::fmod(t0, edge.getPeriod());
    float n_periods = edge.getPeriod() * (int)(t0 / edge.getPeriod());

    float t_next = edge_time_slots[&edge].nextTimeWithEvent(t, true);
    if (t_next < t) { // loop applied
        n_periods += edge.getPeriod();
    }

    return t_next + n_periods;
}

// ------------------------------------------------------------------------------------------------

float Solver::findNextVisiblity(const InterSatelliteLink& edge, const float t0) const {
    for (float t = t0; t <= t0 + edge.getPeriod(); t += step_size) {
        if (!edge.isBlocked(t)) {
            return t;
        }
    }
    // edge is never visible
    return INFINITY;
}

// ------------------------------------------------------------------------------------------------

float Solver::findLastVisible(const InterSatelliteLink& edge, const float t0) const {
    for (float t = t0; t <= t0 + edge.getPeriod(); t += step_size) {
        if (edge.isBlocked(t)) {
            return t - step_size;
        }
    }
    // edge is never visible
    return INFINITY;
}

} // namespace dmsc

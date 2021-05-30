#include "solver.h"
#include <cmath>

#include "dmsc/glm_include.hpp"
#include <ctime>
#include <fstream>
#include <random>

namespace dmsc {

std::atomic<bool> Solver::solver_abort = false;

float Solver::lowerBound() {
    float lower_bound = 0;
    for (const InterSatelliteLink& e : instance.edges) {
        float t = nextVisibility(e, 0.0);
        if (t > lower_bound && t < INFINITY)
            lower_bound = t;
    }
    return lower_bound;
}

float Solver::nextCommunication(const InterSatelliteLink& edge, const float time_0) {
    // edge is never visible?
    float t_visible = nextVisibility(edge, time_0);
    if (t_visible >= INFINITY) {
        return INFINITY;
    }

    // get current orientation of both satellites
    // TODO improve? simplify? -> use invalid events (see canAlign function)
    TimelineEvent<glm::vec3> sat1;
    auto search = satellite_orientation.find(&edge.getV1());
    if (search != satellite_orientation.end()) {
        sat1 = satellite_orientation[&edge.getV1()];
    } else {
        sat1 = TimelineEvent<glm::vec3>(0.f, 0.f, glm::vec3(0.f));
    }

    TimelineEvent<glm::vec3> sat2;
    search = satellite_orientation.find(&edge.getV2());
    if (search != satellite_orientation.end()) {
        sat2 = satellite_orientation[&edge.getV2()];
    } else {
        sat2 = TimelineEvent<glm::vec3>(0.f, 0.f, glm::vec3(0.f));
    }

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

void Solver::createCache() {
    for (const auto& edge : instance.edges) {
        for (float t = 0.0f; t < edge.getPeriod(); t += step_size) {
            // allowed to continue?
            if (solver_abort == true)
                throw std::runtime_error("Cache generation aborted");

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

float Solver::nextVisibility(const InterSatelliteLink& edge, const float t0) {
    if (edge_time_slots[&edge].size() == 0) {
        return INFINITY;
    }

    const InterSatelliteLink* p = &edge;
    float t = std::fmod(t0, edge.getPeriod());
    float n_periods = edge.getPeriod() * (int)(t0 / edge.getPeriod());

    float t_next = edge_time_slots[&edge].nextTimeWithEvent(t, true);
    if (t_next < t) { // loop applied
        n_periods += edge.getPeriod();
    }

    return t_next + n_periods;
}

float Solver::findNextVisiblity(const InterSatelliteLink& edge, const float t0) const {
    for (float t = t0; t <= t0 + edge.getPeriod(); t += step_size) {
        if (!edge.isBlocked(t)) {
            return t;
        }
    }
    // edge is never visible
    return INFINITY;
}

float Solver::findLastVisible(const InterSatelliteLink& edge, const float t0) const {
    for (float t = t0; t <= t0 + edge.getPeriod(); t += step_size) {
        if (edge.isBlocked(t)) {
            return t - step_size;
        }
    }
    // edge is never visible
    return INFINITY;
}

bool Solver::sphereIntersection(const InterSatelliteLink& edge, const float time) {
    // represent edge as a unit vector with origin at one of the satellites
    glm::vec3 sat1 = edge.getV1().cartesian_coordinates(time);
    glm::vec3 sat2 = edge.getV2().cartesian_coordinates(time);
    glm::vec3 direction = glm::normalize(sat2 - sat1);

    // Parameter of sphere (Earth)
    glm::vec3 sphere_center = glm::vec3();
    float radius_earth = 6378; // todo store in instance

    // Check for intersection with a sphere (earth)
    glm::vec3 distance_to_center = sat1 - sphere_center;

    float a = glm::dot(direction, distance_to_center);
    float l = glm::length(distance_to_center);
    float discr = a * a - (glm::dot(distance_to_center, distance_to_center) - (radius_earth * radius_earth));

    // no intersection at all
    if (discr <= 0.0)
        return false;

    // intersection in opposite direction? => d1 and d2 negative
    float d1 = -a + sqrt(discr);
    float d2 = -a - sqrt(discr);
    if (d1 < 0.0 && d2 < 0.0)
        return false;

    // intersection after the ray hit the second satellite
    float dist_between_sat = glm::length(sat1 - sat2);
    if (d1 >= dist_between_sat && d2 >= dist_between_sat)
        return false;

    return true;
}

ScanCover Solver::evaluateEdgeOrder(const EdgeOrder& edge_order) {
    satellite_orientation.clear();
    float t = 0.0f;
    ScanCover result;

    for (int i : edge_order) {
        const InterSatelliteLink& e = instance.edges.at(i);
        float last_change_sat1 = 0.0f;
        float last_change_sat2 = 0.0f;

        // When was the last time the satellites turned?
        auto search = satellite_orientation.find(&e.getV1());
        if (search != satellite_orientation.end()) {
            last_change_sat1 = search->second.t_begin;
        }

        search = satellite_orientation.find(&e.getV2());
        if (search != satellite_orientation.end()) {
            last_change_sat2 = search->second.t_begin;
        }

        // Calculate time when the edge is scanned.
        float t_min = std::max(last_change_sat1, last_change_sat2);
        float t_next = nextCommunication(e, t_min);
        if (t_next >= INFINITY) {
            continue;
        }
        EdgeOrientation orientation = e.getOrientation(t_next);
        result.addEdgeDialog(i, t_next, orientation);

        // refresh satellite orientations
        satellite_orientation[&e.getV1()] =
            TimelineEvent<glm::vec3>(orientation.sat1.start, orientation.sat1.start, orientation.sat1.direction);
        satellite_orientation[&e.getV2()] =
            TimelineEvent<glm::vec3>(orientation.sat2.start, orientation.sat2.start, orientation.sat2.direction);
    }
    result.sort();
    return result;
}

EdgeOrder Solver::toEdgeOrder(const ScanCover& scan_cover) {
    EdgeOrder edge_order;
    edge_order.reserve(scan_cover.size());

    for (const auto& e : scan_cover) {
        edge_order.emplace_back(e.edge_index);
    }
    return edge_order;
}

} // namespace dmsc
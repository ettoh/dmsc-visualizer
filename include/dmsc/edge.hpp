#ifndef DMSC_EDGE_H
#define DMSC_EDGE_H

#include "dmsc/glm_include.hpp"
#include "satellite.hpp"
#include "timeline.hpp"
#include <vector>

namespace dmsc {

// TODO delete
struct EdgeOrientation {
    Orientation sat1 = Orientation();
    Orientation sat2 = Orientation();
};

class InterSatelliteLink {
  public:
    // only defined for circular Orbits!
    InterSatelliteLink(const uint32_t& v1_idx, const uint32_t& v2_idx, const std::vector<Satellite>& satellites,
                       const CentralMass cm)
        : v1_idx(v1_idx)
        , v2_idx(v2_idx)
        , cm(cm) {

        if (v1_idx >= satellites.size() || v2_idx >= satellites.size()) {
            printf("There is no such satellite in given vector!\n");
            assert(false);
            exit(EXIT_FAILURE);
        }

        v1 = &satellites[v1_idx];
        v2 = &satellites[v2_idx];
        period = v1->getPeriod();
        if (v1->getSemiMajorAxis() != v2->getSemiMajorAxis()) {
            period = v1->getPeriod() * v2->getPeriod(); // [sec]
        }

        max_angle = std::acos(cm.radius_central_mass / v1->getSemiMajorAxis()) +
                    std::acos(cm.radius_central_mass / v2->getSemiMajorAxis()); // [rad]
    };

    /**
     * @brief !ONLY DEFINED FOR CIRCULAR ORBTIS!
     * @return True, if edge is blocked.
     */
    bool isBlocked(const float time) const {
        glm::vec3 sat1 = v1->cartesian_coordinates(time);
        glm::vec3 sat2 = v2->cartesian_coordinates(time);

        if (v1->getEccentricity() == 0.0f &&
            v2->getEccentricity() == 0.0f) { // both satellites are circular => easier to compute
            double angle_sats =
                std::acos(glm::dot(sat1, sat2) / (v1->getSemiMajorAxis() * v2->getSemiMajorAxis())); // [rad]

            glm::vec3 tmp = sat1 + sat2;
            if (std::abs(tmp.x) < 0.01f && std::abs(tmp.y) < 0.01f && std::abs(tmp.z) < 0.01f) {
                angle_sats = M_PI;
            }
            return angle_sats > max_angle;
        } else { // elliptical Orbits => radius length not contant
            return true;
        }
    }

    /**
     * @brief Check if there is enough time for both satellites to face each other for a scan at time t.
     * @param origin Start alignment of both satellites and the time they changed for the last time.
     * @return True, if alignment can be performed.
     */
    // TODO rework behavior if TimelineEvent is "invalid" - what if satellite was not part of a communication before ...
    bool canAlign(const TimelineEvent<glm::vec3>& sat1, const TimelineEvent<glm::vec3>& sat2, const float t) const {
        // TODO rework
        EdgeOrientation target = getOrientation(t);
        float angle_sat1 = .0f;
        float angle_sat2 = .0f;
        float time_sat1 = 0.f;
        float time_sat2 = 0.f;

        // calc angle between orientations; direction vectors must be length 1
        if (sat1.isValid()) {
            angle_sat1 = std::acos(glm::dot(sat1.data, target.sat1.direction)); // [rad]
            time_sat1 = sat1.t_begin;
        } else {
            time_sat1 = 0.f; // event is invalid, so assume that sat1 was not part of a communication yet
        }

        if (sat2.isValid()) {
            angle_sat2 = std::acos(glm::dot(sat2.data, target.sat2.direction)); // [rad]
            time_sat2 = sat2.t_begin;
        } else {
            time_sat2 = 0.f; // event is invalid, so assume that sat1 was not part of a communication yet
        }

        // time needed for alignment
        float turn_time_s1 = angle_sat1 / v1->getRotationSpeed(); // [sec]
        float turn_time_s2 = angle_sat2 / v2->getRotationSpeed(); // [sec]

        // enough time?
        if (turn_time_s1 > t - time_sat1 || turn_time_s2 > t - time_sat2) {
            return false;
        }

        return true;
    }

    /**
     * @brief Calculate the directions for both satellites to face each other.
     * @param t [sec] time
     * @return Two direction vectors and the time when they face each other.
     */
    EdgeOrientation getOrientation(const float time) const {
        EdgeOrientation result;
        glm::vec3 sat1 = v1->cartesian_coordinates(time);
        glm::vec3 sat2 = v2->cartesian_coordinates(time);
        result.sat1.direction = glm::normalize(sat2 - sat1);
        result.sat2.direction = glm::normalize(sat1 - sat2);
        result.sat1.start = time;
        result.sat2.start = time;
        return result;
    }

    // GETTER
    float getPeriod() const { return period; }
    float getMaxAngle() const { return max_angle; }
    const Satellite& getV1() const { return *v1; }
    const Satellite& getV2() const { return *v2; }
    const uint32_t getV1Idx() const { return v1_idx; }
    const uint32_t getV2Idx() const { return v2_idx; }
    float getRadiusCentralMass() const { return cm.radius_central_mass; }

  private:
    const Satellite* v1;
    const Satellite* v2;
    uint32_t v1_idx;
    uint32_t v2_idx;
    float period;    // [sec] time until satellite constellations repeat
    float max_angle; // [rad] max angle for satellites to see each other
    CentralMass cm;
};

} // namespace dmsc

#endif

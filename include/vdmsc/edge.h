#ifndef EDGE_H
#define EDGE_H

#include "Orbit.h"
#include "vdmsc/glm_include.h"

namespace dmsc {

struct EdgeOrientation {
    Orientation sat1 = Orientation();
    Orientation sat2 = Orientation();
};

struct Edge {
  private:
    const dmsc::Orbit* v1;
    const dmsc::Orbit* v2;
    float period;              // [sec] time until satellite constellations repeat
    float max_angle;           // [rad] max angle for satellites to see each other
    float radius_central_mass; // [km]

  public:
    // only defined for circular Orbits!
    Edge(const dmsc::Orbit* v1, const dmsc::Orbit* v2, const float radius_central_mass)
        : v1{v1}, v2{v2}, radius_central_mass(radius_central_mass) {

        period = v1->getPeriod();
        if (v1->getSemiMajorAxis() != v2->getSemiMajorAxis()) {
            period = v1->getPeriod() * v2->getPeriod(); // [sec]
        }

        max_angle = std::acos(radius_central_mass / v1->getSemiMajorAxis()) +
                    std::acos(radius_central_mass / v2->getSemiMajorAxis()); // [rad]
    };

    /**
     * @brief !ONLY DEFINED FOR CIRCULAR ORBTIS!
     * @return True, if edge is blocked.
     */
    bool isBlocked(const float time) const {
        glm::vec3 sat1 = v1->cartesian_coordinates(time);
        glm::vec3 sat2 = v2->cartesian_coordinates(time);

        if (v1->getEccentricity() == 0.0f && v2->getEccentricity() == 0.0f) { // both satellites are circular => easier to compute
            double angle_sats = std::acos(glm::dot(sat1, sat2) / (v1->getSemiMajorAxis() * v2->getSemiMajorAxis())); // [rad]

            // glm::vec3
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
    bool canAlign(const Orientation& sat1, const Orientation& sat2, const float t) const {
        EdgeOrientation target = getOrientation(t);
        float angle_sat1 = .0f;
        float angle_sat2 = .0f;

        // calc angle between orientations; direction vectors must be length 1
        if (sat1.direction != glm::vec3(0.0f)) {
            angle_sat1 = std::acos(glm::dot(sat1.direction, target.sat1.direction)); // [rad]
        }

        if (sat2.direction != glm::vec3(0.0f)) {
            angle_sat2 = std::acos(glm::dot(sat2.direction, target.sat2.direction)); // [rad]
        }

        // time needed for alignment
        float turn_time_s1 = angle_sat1 / v1->getMeanRotationSpeed(); // [sec]
        float turn_time_s2 = angle_sat2 / v2->getMeanRotationSpeed(); // [sec]

        // enough time?
        if (turn_time_s1 > t - sat1.start || turn_time_s2 > t - sat2.start) {
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

    float getPeriod() const { return period; }
    float getMaxAngle() const { return max_angle; }
    const dmsc::Orbit& getV1() const { return *v1; }
    const dmsc::Orbit& getV2() const { return *v2; }
    float getRadiusCentralMass() const { return radius_central_mass; }
};

} // namespace dmsc

#endif
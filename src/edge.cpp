#include "dmsc/edge.hpp"

namespace dmsc {

InterSatelliteLink::InterSatelliteLink(const uint32_t& v1_idx, const uint32_t& v2_idx,
                                       const std::vector<Satellite>& satellites, const CentralMass cm)
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
};

// ------------------------------------------------------------------------------------------------

bool InterSatelliteLink::isBlocked(const float time) const {
    // represent edge as a unit vector with origin at one of the satellites
    glm::vec3 sat1 = v1->cartesian_coordinates(time);
    glm::vec3 sat2 = v2->cartesian_coordinates(time);
    glm::vec3 direction = glm::normalize(sat2 - sat1);

    // check for intersection with a sphere (earth)
    glm::vec3 sphere_center = glm::vec3(0.f);
    glm::vec3 origin = sat1 - sphere_center;

    float a = glm::dot(direction, direction);
    float b = 2.f * glm::dot(origin, direction);
    float c = glm::dot(origin, origin) - cm.radius_central_mass * cm.radius_central_mass;
    float discr = b * b - 4 * a * c;

    if (discr < .0f) { // no intersection at all
        return false;
    } else { // intersection possible (in front of or behind the ray-origin)
        float t = -b - sqrt(discr);
        if (t > .0f)
            return true;

        t = -b + sqrt(discr);
        return t > .0f;
    }
}

// ------------------------------------------------------------------------------------------------

bool InterSatelliteLink::canAlign(const TimelineEvent<glm::vec3>& sat1, const TimelineEvent<glm::vec3>& sat2,
                                  const float t) const {
    glm::vec3 target = getOrientation(t);
    float angle_sat1 = .0f;
    float angle_sat2 = .0f;
    float time_sat1 = .0f;
    float time_sat2 = .0f;

    // calc angle between orientations; direction vectors must be length 1
    if (sat1.isValid()) {
        angle_sat1 = std::acos(glm::dot(sat1.data, target)); // [rad]
        time_sat1 = sat1.t_begin;
    } else {
        time_sat1 = 0.f; // event is invalid, so assume that sat1 was not part of a communication yet
    }

    if (sat2.isValid()) {
        angle_sat2 = std::acos(glm::dot(sat2.data, -target)); // [rad]
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

// ------------------------------------------------------------------------------------------------

glm::vec3 InterSatelliteLink::getOrientation(const float time) const {
    glm::vec3 sat1 = v1->cartesian_coordinates(time);
    glm::vec3 sat2 = v2->cartesian_coordinates(time);
    return glm::normalize(sat2 - sat1);
}

} // namespace dmsc

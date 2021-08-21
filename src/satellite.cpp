#include "dmsc/satellite.hpp"

namespace dmsc {

bool StateVector::isSameOrbit(const StateVector& e) const {
    return height_perigee == e.height_perigee && eccentricity == e.eccentricity && inclination == e.inclination &&
           argument_periapsis == e.argument_periapsis && raan == e.raan;
}

// ------------------------------------------------------------------------------------------------

Satellite::Satellite(const StateVector sv, const CentralMass& cm)
    : sv(sv)
    , cm(cm) {

    semi_major_axis = (sv.height_perigee + cm.radius_central_mass) / (1 - sv.eccentricity);
    period = 2.0f * static_cast<float>(M_PI) * sqrtf(powf(semi_major_axis, 3.0f) / cm.gravitational_parameter); // [sec]
    mean_angular_speed = (2.0f * static_cast<float>(M_PI)) / period; // [rad/sec]
}

// ------------------------------------------------------------------------------------------------

glm::vec3 Satellite::cartesian_coordinates(const float time) const {
    float current_true_anomaly = 0.0f;

    // find true anomaly
    if (sv.eccentricity == 0.0) { // circular orbit - easier to calculate
        current_true_anomaly = sv.initial_true_anomaly + mean_angular_speed * time;
    } else if (sv.eccentricity < 1.0f && sv.eccentricity > 0.0f) { // ellipse - numerical iteration needed
        float mean_anomaly =
            fmodf((2.0f * static_cast<float>(M_PI) / period) * time, 2.0f * static_cast<float>(M_PI)); // [rad]
        float x = mean_anomaly;
        float x_next;

        for (int i = 0; i < 30; i++) {
            x_next = x - ((x - sv.eccentricity * sinf(x) - mean_anomaly) / (1 - sv.eccentricity * cosf(x)));
            if (fabsf(x_next - x) <= 0.00001f) {
                break;
            }
            x = x_next;
        }
        current_true_anomaly =
            2 * atanf(std::sqrt((1 + sv.eccentricity) / (1 - sv.eccentricity)) * tanf(x_next / 2.0f));
    }

    return cartesian_coordinates_angle(current_true_anomaly);
}

// ------------------------------------------------------------------------------------------------

glm::vec3 Satellite::cartesian_coordinates_angle(const float true_anomaly) const {
    glm::vec3 coords = glm::vec3(0.f);
    float radius = 0.f;

    // catch hyperbola, parabola and invalid orbits
    if (sv.eccentricity < 0.f || sv.eccentricity >= 1.f) {
        printf("Orbits with an eccentricity of %f can not be displayed. Eccentricity has to be in range [0,1).\n",
               sv.eccentricity);
        assert(false);
        exit(EXIT_FAILURE);
    }

    // potentially improves performance by avoiding the more complex equation (output would be the same)
    if (sv.eccentricity == 0.f) {
        radius = semi_major_axis;
    } else {
        radius = (semi_major_axis - semi_major_axis * sv.eccentricity * sv.eccentricity) /
                 (1 + sv.eccentricity * cosf(true_anomaly));
    }

    // Equation 2.16 (MIS)
    float rotation_angle = sv.argument_periapsis + true_anomaly;
    coords.x = radius * (cos(rotation_angle) * sin(sv.raan) + sin(rotation_angle) * cos(sv.inclination) * cos(sv.raan));
    coords.y = radius * (sin(rotation_angle) * sin(sv.inclination));
    coords.z = radius * (cos(rotation_angle) * cos(sv.raan) - sin(rotation_angle) * cos(sv.inclination) * sin(sv.raan));
    return coords;
}

} // namespace dmsc

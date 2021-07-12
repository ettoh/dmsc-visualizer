#ifndef DMSC_ORBIT_H
#define DMSC_ORBIT_H

#define _USE_MATH_DEFINES
#include "dmsc/glm_include.hpp"
#include <exception>
#include <iostream>
#include <math.h>

namespace dmsc {

/**
 * @brief TODO
 */
struct CentralMass {
    float gravitational_parameter = 398599; // [km^3 / s^2] (default: earth)
    float radius_central_mass = 6378;       // [km] (default: earth)
};

/**
 * @brief Contains all parameters that are needed to describe a Keplerian orbit.
 * For more information on the parameters, search for "Orbital elements for Kepler orbits" - you can do it!
 *
 * Note that these values only describe the shape and position of an keplerian orbit around an >undefined< central mass.
 * In order to compute such things like position or period you need information about the central mass (i.e. radius and
 * mass).
 */
struct StateVector {
    float height_perigee = 200.0f;     // [km] height of the perigee above the central mass
    float eccentricity = 0.0f;         // [rad]
    float inclination = 0.0f;          // [rad]
    float argument_periapsis = 0.0f;   // [rad]
    float raan = 0.0f;                 // [rad] right ascension of the ascending node
    float rotation_speed = .005f;      // [rad/sec] speed of rotation (for the satellite to orientate)
    float initial_true_anomaly = 0.0f; // [rad]

    /**
     * @brief Return true if the shape and position of two keplarian orbits are equal. The initial true anomaly is not
     * taken into account!
     */
    bool operator==(const StateVector& e) {
        return height_perigee == e.height_perigee && eccentricity == e.eccentricity && inclination == e.inclination &&
               argument_periapsis == e.argument_periapsis && raan == e.raan && rotation_speed == e.rotation_speed;
    } // TODO test

    bool operator!=(const StateVector& e) { return !(*this == e); } // TODO test
};

// ------------------------------------------------------------------------------------------------

class Satellite {
  public:
    /**
     * @brief Construct a new Orbit object from a given StateVector and initial true anomaly.
     *
     * @param initial_true_anomaly [rad]
     * @param gravitational_parameter [km^3 / s^2]
     */
    Satellite(const StateVector sv, const CentralMass& cm)
        : sv(sv)
        , cm(cm) {

        semi_major_axis = (sv.height_perigee + cm.radius_central_mass) / (1 - sv.eccentricity);
        period =
            2.0f * static_cast<float>(M_PI) * sqrtf(powf(semi_major_axis, 3.0f) / cm.gravitational_parameter); // [sec]
        mean_angular_speed = (2.0f * static_cast<float>(M_PI)) / period; // [rad/sec]
    }

    /**
     * @brief Transform a satellite position into 3D cartesian coordinates.
     * z-axis: vernal point; y-axis: up-direction; x-axis: normal
     * @param true_anomaly [rad] Determines satellite position in orbit.
     * @return (x, y, z) coordinates
     */
    glm::vec3 cartesian_coordinates_angle(const float true_anomaly) const {
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
        coords.x =
            radius * (cos(rotation_angle) * sin(sv.raan) + sin(rotation_angle) * cos(sv.inclination) * cos(sv.raan));
        coords.y = radius * (sin(rotation_angle) * sin(sv.inclination));
        coords.z =
            radius * (cos(rotation_angle) * cos(sv.raan) - sin(rotation_angle) * cos(sv.inclination) * sin(sv.raan));
        return coords;
    }

    /**
     * @brief Transform a satellite position into 3D cartesian coordinates.
     * z-axis: vernal point; y-axis: up-direction; x-axis: normal
     * @param time [sec] Determines satellite position in orbit.
     * @return (x, y, z) coordinates
     */
    glm::vec3 cartesian_coordinates(const float time) const {
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

    // GETTER
    float getPeriod() const { return period; }
    float getSemiMajorAxis() const { return semi_major_axis; }
    float getEccentricity() const { return sv.eccentricity; }
    float getRotationSpeed() const { return sv.rotation_speed; }
    float getTrueAnomaly() const { return sv.initial_true_anomaly; }
    float getRaan() const { return sv.raan; }
    float getArgumentPeriapsis() const { return sv.argument_periapsis; }
    float getInclination() const { return sv.inclination; }
    float getHeightPerigee() const { return sv.height_perigee; }

    // SETTER
    void setRotationSpeed(float speed) { sv.rotation_speed = speed; };

  private:
    StateVector sv;
    CentralMass cm;
    float period;             // [sec]
    float mean_angular_speed; // [rad / sec]
    float semi_major_axis;    // [km]
};

// ------------------------------------------------------------------------------------------------

// TODO delete
struct Orientation {
    glm::vec3 direction = glm::vec3(0.0f);
    float start = .0f;
    // float end = -1.0f;
    friend bool operator<(const Orientation& e, const Orientation& f) { return e.start < f.start; }
    Orientation() = default;
    Orientation(const float start) { this->start = start; }
};

} // namespace dmsc

#endif
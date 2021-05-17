#ifndef ORBIT_H
#define ORBIT_H

#define _USE_MATH_DEFINES
#include "vdmsc/glm_include.h"
#include <exception>
#include <iostream>
#include <math.h>

namespace dmsc {

/**
 * @brief Contains all parameters that are needed to describe a Keplerian orbit.
 * For more information on the parameters, search for "Orbital elements for Kepler orbits" - you can do it!
 *
 * Note that these values only describe the shape and position of an keplerian orbit around an >undefined< central mass. In order
 * to compute such things like position or period you need information about the central mass (i.e. radius and mass).
 */
struct StateVector {
    float height_perigee = 200.0f;   // [km] height of the perigee above the central mass
    float eccentricity = 0.0f;       // [rad]
    float inclination = 0.0f;        // [rad]
    float argument_periapsis = 0.0f; // [rad]
    float raan = 0.0f;               // [rad] right ascension of the ascending node
    float rotation_speed = .005f;    // [rad/sec] speed of rotation (for the satellite to orientate)
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
    Satellite(const StateVector sv, const float initial_true_anomaly, const float gravitational_parameter,
              const float radius_central_mass)
        : sv(sv)
        , initial_true_anomaly(initial_true_anomaly)
        , gravitational_parameter(gravitational_parameter)
        , radius_central_mass(radius_central_mass) {

        semi_major_axis = (sv.height_perigee + radius_central_mass) / (1 - sv.eccentricity);
        period = 2.0f * static_cast<float>(M_PI) * sqrtf(powf(semi_major_axis, 3.0f) / gravitational_parameter); // [sec]
        mean_angular_speed = (2.0f * static_cast<float>(M_PI)) / period;                                         // [rad/sec]
    }

    /**
     * @brief Transform a satellite position into 3D cartesian coordinates.
     * z-axis: vernal point; y-axis: up-direction; x-axis: normal
     * @param time [sec] Determines satellite position in orbit.
     * @return (x, y, z) coordinates
     */
    glm::vec3 cartesian_coordinates(const float time) const {
        glm::vec3 coords;
        float current_true_anomaly = 0.0f;
        float radius = 0.0f;

        // find true anomaly
        if (sv.eccentricity == 0.0) { // circular orbit - easier to calculate
            current_true_anomaly = initial_true_anomaly + mean_angular_speed * time;
            radius = semi_major_axis;
        } else if (sv.eccentricity < 1.0f && sv.eccentricity > 0.0f) { // ellipse - numerical iteration needed
            float mean_anomaly =
                fmodf((2.0f * static_cast<float>(M_PI) / period) * time, 2.0f * static_cast<float>(M_PI)); // [rad]
            float x = mean_anomaly;                                                                        // todo
            float x_next;

            for (int i = 0; i < 30; i++) {
                x_next = x - ((x - sv.eccentricity * sinf(x) - mean_anomaly) / (1 - sv.eccentricity * cosf(x)));
                if (fabsf(x_next - x) <= 0.00001f) {
                    break;
                }
                x = x_next;
            }
            current_true_anomaly = 2 * atanf(std::sqrt((1 + sv.eccentricity) / (1 - sv.eccentricity)) * tanf(x_next / 2.0f));
            radius = (semi_major_axis - semi_major_axis * sv.eccentricity * sv.eccentricity) /
                     (1 + sv.eccentricity * cosf(current_true_anomaly));
            // todo startwert falls e gro�
            // todo fehler?
        } else { // hyperbola, parabola, invalid orbits
            throw std::logic_error("Function not defined for this type of orbits!");
        }

        // Equation 2.16 (MIS) - ONLY Circular Orbits
        float rotation_angle = sv.argument_periapsis + current_true_anomaly;
        coords.x = radius * (cos(rotation_angle) * sin(sv.raan) + sin(rotation_angle) * cos(sv.inclination) * cos(sv.raan));
        coords.y = radius * (sin(rotation_angle) * sin(sv.inclination));
        coords.z = radius * (cos(rotation_angle) * cos(sv.raan) - sin(rotation_angle) * cos(sv.inclination) * sin(sv.raan));
        return coords;
    }

    // GETTER
    float getPeriod() const { return period; }
    float getSemiMajorAxis() const { return semi_major_axis; }
    float getEccentricity() const { return sv.eccentricity; }
    float getRotationSpeed() const { return sv.rotation_speed; }
    float getTrueAnomaly() const { return initial_true_anomaly; }
    float getRaan() const { return sv.raan; }
    float getArgumentPeriapsis() const { return sv.argument_periapsis; }
    float getInclination() const { return sv.inclination; }
    float getHeightPerigee() const { return sv.height_perigee; }

    // SETTER
    void setRotationSpeed(float speed) { sv.rotation_speed = speed; };

  private:
    StateVector sv;
    float gravitational_parameter; // [km^3 / s^2]
    float radius_central_mass;     // [km]
    float period;                  // [sec]
    float mean_angular_speed;      // [rad / sec]
    float initial_true_anomaly;    // [rad]
    float semi_major_axis;         // [km]
};

// ------------------------------------------------------------------------------------------------

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
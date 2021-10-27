#ifndef DMSC_ORBIT_H
#define DMSC_ORBIT_H

#define _USE_MATH_DEFINES
#include "dmsc/glm_include.hpp"
#include <exception>
#include <iostream>
#include <math.h>

namespace dmsc {

/**
 * @brief Necessary parameters to describe the central mass.
 * If the central mass is the earth, you can use the default values.
 */
struct CentralMass {
    float gravitational_parameter = 398599; // [km^3 / s^2] (default: earth)
    float radius_central_mass = 6378;       // [km] (default: earth)
};

/**
 * @brief Contains all parameters that are needed to describe an Keplerian orbit.
 * For more information on the parameters, search for "Orbital elements for Kepler orbits" - you can do it!
 *
 * Note that these values only describe the shape and position of an keplerian orbit around an >undefined< central mass.
 * In order to compute such things like position or period, you need information about the central mass (i.e. radius and
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
    float cone_angle = 0.f;            // [rad]

    bool operator==(const StateVector& e) {
        return height_perigee == e.height_perigee && eccentricity == e.eccentricity && inclination == e.inclination &&
               argument_periapsis == e.argument_periapsis && raan == e.raan && rotation_speed == e.rotation_speed &&
               initial_true_anomaly == e.initial_true_anomaly && cone_angle == e.cone_angle;
    }

    bool operator!=(const StateVector& e) { return !(*this == e); }

    /**
     * @brief Returns true, if the two state vectors describe the same orbit (i.e. same shape and same location -
     * rotation speed and initial position may be different).
     */
    bool isSameOrbit(const StateVector& e) const;
};

// ------------------------------------------------------------------------------------------------

class Satellite {
  private:
    StateVector sv;           // parameters that describe the satellite position and its orbit around the central mass
    CentralMass cm;           // parameters of the central mass
    float period;             // [sec] time required for one revolution around the central mass
    float mean_angular_speed; // [rad / sec]
    float semi_major_axis;    // [km] semi-major axis of the ellipse that describes the orbit of the satellite

  public:
    /**
     * @brief Constructs a new satellite from a given StateVector.
     *
     * @param initial_true_anomaly [rad]
     * @param gravitational_parameter [km^3 / s^2]
     */
    Satellite(const StateVector sv, const CentralMass& cm);

    /**
     * @brief Transforms a satellite position into 3D cartesian coordinates.
     * z-axis: vernal point; y-axis: up-direction; x-axis: normal
     * @param true_anomaly [rad] Determines satellite position in orbit.
     * @return (x, y, z) coordinates
     */
    glm::vec3 cartesian_coordinates_angle(const float true_anomaly) const;

    /**
     * @brief Transforms a satellite position into 3D cartesian coordinates.
     * z-axis: vernal point; y-axis: up-direction; x-axis: normal
     * @param time [sec] Determines satellite position in orbit.
     * @return (x, y, z) coordinates
     */
    glm::vec3 cartesian_coordinates(const float time) const;

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
    float getConeAngle() const { return sv.cone_angle; }

    // SETTER
    void setRotationSpeed(float speed) { sv.rotation_speed = speed; };
};

} // namespace dmsc

#endif

#ifndef ORBIT_H
#define ORBIT_H

#define _USE_MATH_DEFINES
#include "vector3d.h" // todo get rid?
#include <cmath>
#include <exception>
#include <iostream>
#include <math.h>

struct Orbit {
  private:
    float gravitational_parameter;   // [km^3 / s^2]
    float semi_major_axis;           // [km]
    float period;                    // [sec]
    float mean_angular_speed;        // [rad / sec]
    float true_anomaly = 0.0f;       // [rad]
    float eccentricity = 0.0f;       // [rad]
    float inclination = 0.0f;        // [rad]
    float argument_periapsis = 0.0f; // [rad]
    float raan = 0.0f;               // [rad] right ascension of the ascending node
    float rotation_speed = .005f;    // [rad/sec] speed of rotation for the satellite to orientate

  public:
    // Circular Orbit in 2D
    Orbit(const float gravitational_parameter, const float semi_major_axis)
        : gravitational_parameter(gravitational_parameter), semi_major_axis(semi_major_axis) {

        period = 2.0f * static_cast<float>(M_PI) * sqrtf(powf(semi_major_axis, 3.0f) / gravitational_parameter); // [sec]
        mean_angular_speed = (2.0f * static_cast<float>(M_PI)) / period; // [rad/sec]
    }

    // Circular orbit in 2D with initial true anomaly
    Orbit(const bool in_rad, const float gravitational_parameter, const float semi_major_axis,
          float true_anomaly)
        : Orbit(gravitational_parameter, semi_major_axis) {

        if (!in_rad)
            true_anomaly *= static_cast<float>(M_PI) / 180.0f;

        this->true_anomaly = true_anomaly;
    };

    // Orbit in 3D
    Orbit(const bool in_rad, const float gravitational_parameter, const float semi_major_axis,
          const float true_anomaly, float inclination, float raan, float perigee, const float rotation_speed,
          const float eccentricity)
        : Orbit(in_rad, gravitational_parameter, semi_major_axis, true_anomaly) {

        if (!in_rad) {
            inclination *= static_cast<float>(M_PI) / 180.0f;
            raan *= static_cast<float>(M_PI) / 180.0f;
            perigee *= static_cast<float>(M_PI) / 180.0f;
        }

        this->inclination = inclination;
        this->raan = raan;
        this->argument_periapsis = perigee;
        this->rotation_speed = rotation_speed;
        this->eccentricity = eccentricity;
    };

    float getPeriod() const { return period; }
    float getSemiMajorAxis() const { return semi_major_axis; }
    float getEccentricity() const { return eccentricity; }
    float getMeanRotationSpeed() const { return rotation_speed; }
    float getTrueAnomaly() const { return true_anomaly; }
    float getRaan() const { return raan; }
    float getArgumentPeriapsis() const { return argument_periapsis; }
    float getInclination() const { return inclination; }

    void setRotationSpeed(float speed) { rotation_speed = speed; };

    /**
     * @brief Transform a satellite position into 3D cartesian coordinates.
     * z-axis: vernal point; y-axis: up-direction; x-axis: normal
     * @param time [sec] Determines satellite position in orbit.
     * @return (x, y, z) coordinates
     */
    Vector3D cartesian_coordinates(const float time) const {
        Vector3D cartesian_coordinates;
        float current_true_anomaly = 0.0f;
        float radius = 0.0f;

        // find true anomaly
        if (eccentricity == 0.0) { // circular orbit - easier to calculate
            current_true_anomaly = true_anomaly + mean_angular_speed * time;
            radius = semi_major_axis;
        } else if (eccentricity < 1.0f && eccentricity > 0.0f) { // ellipse - numerical iteration needed
            float mean_anomaly = fmodf((2.0f * static_cast<float>(M_PI) / period) * time, 2.0f * static_cast<float>(M_PI)); // [rad]
            float x = mean_anomaly; // todo
            float x_next;

            for (int i = 0; i < 30; i++) {
                x_next = x - ((x - eccentricity * sinf(x) - mean_anomaly) /
                              (1 - eccentricity * cosf(x)));
                if (fabsf(x_next - x) <= 0.00001f) {
                    break;
                }
                x = x_next;
            }
            current_true_anomaly =
                2 * atanf(std::sqrt((1 + eccentricity) / (1 - eccentricity)) * tanf(x_next / 2.0f));
            radius = (semi_major_axis - semi_major_axis * eccentricity * eccentricity) /
                     (1 + eccentricity * cosf(current_true_anomaly));
            // todo startwert falls e gro�
            // todo fehler?
        } else { // hyperbola, parabola, invalid orbits
            throw std::logic_error("Function not defined for this type of orbits!");
        }

        // Equation 2.16 (MIS) - ONLY Circular Orbits
        float rotation_angle = argument_periapsis + current_true_anomaly;
        cartesian_coordinates.x =
            radius * (cos(rotation_angle) * sin(raan) + sin(rotation_angle) * cos(inclination) * cos(raan));
        cartesian_coordinates.y = radius * (sin(rotation_angle) * sin(inclination));
        cartesian_coordinates.z =
            radius * (cos(rotation_angle) * cos(raan) - sin(rotation_angle) * cos(inclination) * sin(raan));
        return cartesian_coordinates;
    }
};

struct Orientation {
    Vector3D direction = Vector3D(0.0f);
    float start = .0f;
    //float end = -1.0f;
    friend bool operator<(const Orientation& e, const Orientation& f) { return e.start < f.start; }
    Orientation() = default;
    Orientation(const float start) { this->start = start; }
};

#endif
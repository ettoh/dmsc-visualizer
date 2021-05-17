#ifndef INSTANCE_H
#define INSTANCE_H

#include "edge.h"
#include "satellite.h"
#include <map>
#include <string>
#include <vector>

namespace dmsc {

using AdjacentList = std::vector<int>;

// ------------------------------------------------------------------------------------------------

struct LineGraph {
    std::vector<AdjacentList> edges;
};

// ------------------------------------------------------------------------------------------------

struct Instance {
  private:
    float gravitational_parameter = 398599;                    // earth [km^3 / s^2]
    float radius_central_mass = 6378;                          // equatorial radius (earth) [km]
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format

  public:
    std::vector<Satellite> orbits;
    std::vector<Edge> edges;

    Instance() = default;
    /**
     * @param gravitational_parameter [km^3 / s^2]
     * @param radius_central_mass [km]
     */
    Instance(const float gravitational_parameter, const float radius_central_mass)
        : gravitational_parameter(gravitational_parameter)
        , radius_central_mass(radius_central_mass) {}
    Instance(const Instance& source);
    Instance(const std::string& file);
    Instance& operator=(const Instance& source);

    void addOrbit(const Satellite& orbit) { orbits.push_back(Satellite(orbit)); }
    float getRadiusCentralMass() const { return radius_central_mass; }
    void removeInvalidEdges();
    LineGraph lineGraph() const;
    bool save(const std::string& file) const;
};

// ------------------------------------------------------------------------------------------------

float rad(const float deg); // convert degrees to radians
float deg(const float rad); // convert radians to degrees

} // namespace dmsc

#endif
#ifndef INSTANCE_H
#define INSTANCE_H

#include "edge.h"
#include "orbit.h"
#include <map>
#include <string>
#include <vector>

using AdjacentList = std::vector<int>;

struct LineGraph {
    std::vector<AdjacentList> edges;
};

struct ProblemInstance {
  private:
    float gravitational_parameter = 398599;                    // earth [km^3 / s^2]
    float radius_central_mass = 6378;                          // equatorial radius (earth) [km]
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format

  public:
    std::vector<Orbit> orbits;
    std::vector<Edge> edges;

    ProblemInstance() = default;
    /**
     * @param gravitational_parameter [km^3 / s^2]
     * @param radius_central_mass [km]
     */
    ProblemInstance(const float gravitational_parameter, const float radius_central_mass)
        : gravitational_parameter(gravitational_parameter), radius_central_mass(radius_central_mass) {}
    ProblemInstance(const ProblemInstance& source);
    ProblemInstance(const std::string& file);
    ProblemInstance& operator=(const ProblemInstance& source);

    void addOrbit(const Orbit& orbit) { orbits.push_back(Orbit(orbit)); }
    float getRadiusCentralMass() const { return radius_central_mass; }
    void removeInvalidEdges();
    LineGraph lineGraph() const;
    bool save(const std::string& file) const;
};

#endif
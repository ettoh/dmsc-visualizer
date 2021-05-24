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

/**
 * @brief Edge between two vertices.
 */
struct Edge {
    uint32_t from_idx; // Index of vertices. If Edge is not bidirectional, this is the origin.
    uint32_t to_idx;   // Index of vertices. If Edge is not bidirectional, this is the target.
    bool optional;     // If true, no communication is scheduled for this edge.

    Edge(const uint32_t from_idx, const uint32_t to_idx, const bool optional = false)
        : from_idx(from_idx)
        , to_idx(to_idx)
        , optional(optional) {}
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 *
 */
struct Instance {
    CentralMass cm;
    std::vector<StateVector> satellites;
    std::vector<Edge> edges;

    Instance() = default;

    /**
     * @brief Construct a new Instance object from a given file.
     *
     * @param file Path to file where the instance is stored.
     */
    Instance(const std::string& file);

    /**
     * @brief Save the instance to the given file.
     */
    void save(const std::string& file) const;

  private:
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format
};

// ------------------------------------------------------------------------------------------------

struct LineGraph {
    std::vector<AdjacentList> edges;
};

// ------------------------------------------------------------------------------------------------

class PhysicalInstance {
  private:
    CentralMass cm;
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format

  public:
    std::vector<Satellite> satellites;
    std::vector<InterSatelliteLink> edges;

    PhysicalInstance() = default;
    /**
     * @param gravitational_parameter [km^3 / s^2]
     * @param radius_central_mass [km]
     */
    PhysicalInstance(const CentralMass& cm)
        : cm(cm) {}
    PhysicalInstance(const Instance& raw_instance);
    PhysicalInstance(const PhysicalInstance& source); // Copy
    PhysicalInstance(const std::string& file);
    PhysicalInstance& operator=(const PhysicalInstance& source);

    float getRadiusCentralMass() const { return cm.radius_central_mass; }
    void removeInvalidEdges();
    LineGraph lineGraph() const;
    bool save(const std::string& file) const;
};

// ------------------------------------------------------------------------------------------------

float rad(const float deg); // convert degrees to radians
float deg(const float rad); // convert radians to degrees

} // namespace dmsc

#endif